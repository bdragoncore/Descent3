/**
 * @file multi_ui_real_tests.cpp
 * @brief Unit tests for Descent3/multi_ui.cpp: auto-connect arg parsing, mouselook.
 *
 * @details
 * gating, multiplayer config save/load dispatch, and the small menu helpers.
 * UI classes are stubbed (vtables emitted) and game subsystems recorded.
 *
 * This harness validates the behavior of `Descent3/multi_ui.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/multi_ui.cpp`
 * @par Harness
 * `multi_ui_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/multi_ui.cpp
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "vecmat_external.h"
#include "multi_ui.h"
#include "multi_dll_mgr.h"
#include "multi_external.h"
#include "application.h"
#include "appdatabase.h"
#include "descent.h"
#include "config.h"
#include "game.h"
#include "Mission.h"
#include "pilot_class.h"
#include "ship.h"
#include "objinfo.h"
#include "player.h"
#include "hlsoundlib.h"
#include "ddio.h"
#include "cfile.h"
#include "bitmap.h"
#include "renderer.h"
#include "ui.h"
#include "uisys.h"
#include "SmallViews.h"
#include "newui.h"
#include "newui_core.h"
#include "uires.h"
#include "mem.h"
#include "menu.h"
#include "stringtable.h"
#include "args.h"
#include "grtext.h"
#include "multi.h"

// ==== recorder helpers ====
static std::vector<std::string> g_rec;
static void REC(const std::string &s) { g_rec.push_back(s); }
static size_t Pos(const std::string &s) {
  for (size_t i = 0; i < g_rec.size(); i++)
    if (g_rec[i] == s)
      return i;
  return g_rec.size();
}

static std::vector<std::string> g_dll_calls;
static std::vector<std::string> g_settings_paths;
static std::vector<std::string> g_msgboxes;
static int s_warp_result = 1;
static int s_msgbox_ret = 1;
static bool s_pathdlg_ret = true;

// ==== GameArgs plumbing (real layout from args.h) ====
char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];
void ResetArgs() {
  memset(GameArgs, 0, sizeof(GameArgs));
  strcpy(GameArgs[0], "d3test");
}
void PushArg(const char *a) {
  for (int i = 1; i < MAX_ARGS; i++) // args are 1-indexed
    if (GameArgs[i][0] == 0) {
      strncpy(GameArgs[i], a, MAX_CHARS_PER_ARG - 1);
      return;
    }
}
int FindArg(const char *which, int start) {
  for (int i = start; i < MAX_ARGS; i++)
    if (!strcasecmp(which, GameArgs[i]))
      return i;
  return 0;
}
const char *GetArg(int index) { return GameArgs[index][0] ? GameArgs[index] : nullptr; }

// ==== oeApplication ====
class TestApp : public oeApplication {
public:
  void init() override {}
  void get_info(void *) override {}
  int flags(void) const override { return 0; }
  unsigned defer() override { return 0; }
  void delay(float) override {}
  void set_defer_handler(void (*f)(bool)) override {}
};
oeApplication *Descent = nullptr;

// ==== Database ====
class TestDatabase : public oeAppDatabase {
public:
  std::map<std::string, std::vector<uint8_t>> store;
  bool create_record(const char *) override { return true; }
  bool lookup_record(const char *) override { return true; }
  bool read(const char *label, char *entry, int *entrylen) override {
    auto it = store.find(label);
    if (it == store.end())
      return false;
    int n = std::min<int>(*entrylen, (int)it->second.size());
    memcpy(entry, it->second.data(), n);
    *entrylen = n;
    return true;
  }
  bool read(const char *label, void *entry, int wordsize) override {
    auto it = store.find(label);
    if (it == store.end())
      return false;
    int n = std::min<int>(wordsize, (int)it->second.size());
    memcpy(entry, it->second.data(), n);
    return true;
  }
  bool read(const char *label, bool *entry) override {
    auto it = store.find(label);
    if (it == store.end())
      return false;
    *entry = it->second.empty() ? false : it->second[0] != 0;
    return true;
  }
  bool write(const char *label, const char *entry, int entrylen) override {
    store[label] = std::vector<uint8_t>((const uint8_t *)entry, (const uint8_t *)entry + entrylen);
    return true;
  }
  bool write(const char *label, int entry) override {
    std::vector<uint8_t> v(4);
    memcpy(v.data(), &entry, 4);
    store[label] = v;
    return true;
  }
  void get_user_name(char *buffer, size_t *size) override {
    strncpy(buffer, "tester", *size);
    *size = strlen("tester");
  }
};
static TestDatabase s_db;
oeAppDatabase *Database = &s_db;

// ==== game globals ====
extern int MultiDLLGameStarting;
char Auto_login_addr[MAX_AUTO_LOGIN_STUFF_LEN];
char Auto_login_port[MAX_AUTO_LOGIN_STUFF_LEN];
tMission Current_mission{};
pilot Current_pilot;
ship Ships[MAX_SHIPS];
object_info Object_info[MAX_OBJECT_IDS];
netgame_info Netgame{};
bool Multi_got_player_list = false;
char *Multi_recieved_player_list = nullptr;
int Game_fonts[8] = {7};
bool Mem_low_memory_mode = false;
bool Mem_superlow_memory_mode = false;
void (*DebugBreak_callback_stop)() = nullptr;
void (*DebugBreak_callback_resume)() = nullptr;

// ==== memory CFILE (real FILE* backing) ====
CFILE *cfopen(const std::filesystem::path &path, const char *mode) {
  FILE *f = fopen(path.string().c_str(), mode);
  if (!f)
    return nullptr;
  CFILE *c = new CFILE{};
  c->name = strdup(path.string().c_str());
  c->file = f;
  c->lib_handle = -1;
  return c;
}
void cfclose(CFILE *c) {
  if (c) {
    if (c->file)
      fclose(c->file);
    free(c->name);
  }
  delete c;
}
int32_t cf_ReadInt(CFILE *c, bool little_endian) {
  uint8_t b[4];
  if (fread(b, 1, 4, c->file) != 4)
    return 0;
  if (little_endian)
    return b[0] | (b[1] << 8) | (b[2] << 16) | ((uint32_t)b[3] << 24);
  return ((uint32_t)b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}
void cf_WriteInt(CFILE *c, int32_t i) {
  uint8_t b[4] = {(uint8_t)i, (uint8_t)(i >> 8), (uint8_t)(i >> 16), (uint8_t)(i >> 24)};
  fwrite(b, 1, 4, c->file);
}
std::filesystem::path cf_GetWritableBaseDirectory() { return "/tmp/opencode/d3writable"; }
std::vector<std::filesystem::path> cf_LocateMultiplePaths(const std::filesystem::path &) { return {}; }

// ==== ddio ====
bool ddio_Init(ddio_init_info *) { return true; }
void ddio_Close() {}
void ddio_KeyFlush() {}
void ddio_MouseMode(int) {}
void ddio_SetKeyboardLanguage(int) {}
bool ddio_SetWorkingDir(const char *) { return true; }
bool ddio_CreateLockFile(const std::filesystem::path &) { return true; }
void ddio_DoForeachFile(const std::filesystem::path &, const std::regex &,
                        const std::function<void(std::filesystem::path)> &) {}
std::filesystem::path ddio_GetBasePath() { return "/tmp/opencode"; }
std::filesystem::path ddio_GetPrefPath(const char *, const char *) { return "/tmp/opencode/d3pref"; }
std::filesystem::path ddio_GetTmpFileName(const std::filesystem::path &b, const char *p) {
  return b / (std::string(p) + "t");
}

// ==== bitmap / renderer / grtext ====
void bm_InitBitmaps() {}
void bm_FreeBitmap(int) {}
int bm_AllocLoadFileBitmap(const char *, int, int) { return 33; }
bool bm_CreateChunkedBitmap(int, chunked_bitmap *) { return true; }
void bm_DestroyChunkedBitmap(chunked_bitmap *) {}
void rend_ClearScreen(ddgr_color) {}
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, uint8_t) {}
void rend_DrawPolygon2D(int, g3Point **, int) {}
bool rend_InitWindowMode() { return true; }
void rend_SetAlphaType(int8_t) {}
void rend_SetAlphaValue(uint8_t) {}
void rend_SetFlatColor(ddgr_color) {}
void rend_SetLighting(light_state) {}
void rend_SetTextureType(texture_type) {}
void rend_SetZBufferState(int8_t) {}
void rend_Flip() {}
void rend_GetRenderState(rendering_state *) {}
void grtext_Init() {}
int grfont_GetHeight(int) { return 16; }
int grtext_GetTextLineWidth(const char *) { return 10; }
void grtext_SetFont(int) {}
void grtext_SetColor(ddgr_color) {}
void grtext_SetAlpha(uint8_t) {}
void grtext_Puts(int, int, const char *) {}
void grtext_Flush() {}

// ==== sound system ====
static float s_master_volume = 1.0f;
static int s_ll_quantity = 0;
int hlsSystem::InitSoundLib(oeApplication *, char, char, bool) { return 1; }
void hlsSystem::SetLLSoundQuantity(int n) { s_ll_quantity = n; }
int hlsSystem::GetLLSoundQuantity() { return s_ll_quantity; }
void hlsSystem::PauseSounds(bool) {}
void hlsSystem::ResumeSounds() {}
void hlsSystem::SetMasterVolume(float v) { s_master_volume = v; }
float hlsSystem::GetMasterVolume() { return s_master_volume; }
hlsSystem::hlsSystem() {}
void hlsSystem::KillSoundLib(bool) {}
hlsSystem Sound_system;

// ==== misc game stubs ====
void mem_Init() {}
void *mem_malloc_sub(int size, const char *, int) { return malloc(size); }
void mem_free_sub(void *p) { free(p); }
bool error_Init(bool, const char *) { return true; }
void DebugGraph_Initialize() {}
void rtp_Init() {}
void PrintDedicatedMessage(const char *, ...) {}
void SetScreenMode(int sm, bool) { REC(std::string("screenmode:") + std::to_string(sm)); }
int GetScreenMode() { return 0; }
void StartFrame(bool) {}
void EndFrame() {}
void PauseGame() {}
void ResumeGame() {}
void D3DeferHandler(bool) {}
void CreateSplashScreen(const char *, int) { REC("splashcreate"); }
void CloseSplashScreen() { REC("splashclose"); }
int DoUI() { return 0; }
int PollUI(void) { return 0; }
int FindObjectIDName(const char *) { return 0; }
void MultiFlushAllIncomingBuffers() { REC("flushbuf"); }
void MultiProcessIncoming() { REC("processincoming"); }
void RequestPlayerList(network_address *) { REC("reqplayerlist"); }
bool PlayerSetShipPermission(int, char *, bool) { REC("setshipperm"); return true; }
bool PlayerIsShipAllowed(int, int) { REC("shipallowed"); return true; }
void MultiSaveSettings(const std::filesystem::path &p) {
  g_settings_paths.push_back(p.string());
  REC("savemset");
}
void MultiLoadSettings(const std::filesystem::path &p) {
  g_settings_paths.push_back(p.string());
  REC("loadmset");
}
int LoadMultiDLL(const char *name) {
  g_dll_calls.push_back(std::string("load:") + name);
  return 1; // success
}
void CallMultiDLL(int eventnum) { g_dll_calls.push_back(std::string("call:") + std::to_string(eventnum)); }
void FreeMultiDLL() { g_dll_calls.push_back("free"); }
int DisplayLevelWarpDlg(int) { REC("warpdlg"); return s_warp_result; }
int DoMessageBox(const char *title, const char *msg, int type, ddgr_color, ddgr_color) {
  g_msgboxes.push_back(std::string(title) + "|" + msg + "|" + std::to_string(type));
  return s_msgbox_ret;
}
bool DoPathFileDialog(bool save, std::filesystem::path &, const char *,
                      const std::vector<std::string> &, int) {
  REC(std::string("pathdlg:") + (save ? "save" : "load"));
  return s_pathdlg_ret;
}
const char *GetStringFromTable(int id) {
  switch (id) {
  case 4:
    return "Cancel";
  case 142:
    return "Done";
  case 372:
    return "Save Multiplayer Settings";
  case 373:
    return "Load Multiplayer Settings";
  case 376:
    return "Allowed Ships";
  case 377:
    return "Disallowed Ships";
  case 378:
    return "Allow";
  case 379:
    return "Disallow";
  case 374:
    return "Allowed items";
  case 375:
    return "Disallowed items";
  case 795:
    return "nomouselook";
  case 796:
    return "joinanyway";
  default:
    return "str";
  }
}

namespace D3 {
float ChronoTimer::GetTime() { return 0.0f; }
} // namespace D3

// ==== UI class stubs ====
// (UIGadget's other virtuals are inline in ui.h; only these are declared-only)
void UIGadget::OnFormat() {}
void UIGadget::OnSelect() {}
void UIGadget::Destroy() {}
UIWindow::UIWindow() {}
UIWindow::~UIWindow() {}
void UIWindow::Open() {}
void UIWindow::Close() {}
void UIWindow::Destroy() {}
int UIWindow::Process() { return 0; }
void UIWindow::OnDraw() {}
void UIWindow::OnUserProcess() {}
void UIWindow::OnKeyDown(int) {}
void UIWindow::OnKeyUp(int) {}
void UIButton::OnLostFocus() {}
void UIButton::OnGainFocus() {}
void UIButton::OnKeyDown(int) {}
void UIButton::OnKeyUp(int) {}
void UIButton::OnMouseBtnDown(int) {}
void UIButton::OnMouseBtnUp(int) {}
void UIButton::OnDraw() {}
void UIButton::OnFormat() {}
void UIButton::OnDestroy() {}
void UIStatic::OnDraw() {}
void UIStatic::OnFormat() {}
void UIStatic::OnDestroy() {}
bool UIBitmapItem::draw(int, int, tUIDrawClass) { return false; }
int UIBitmapItem::width() { return 0; }
int UIBitmapItem::height() { return 0; }
UIItem *UIBitmapItem::CopyUIItem() { return nullptr; }
bool UITextItem::draw(int, int, tUIDrawClass) { return false; }
int UITextItem::width() { return 0; }
int UITextItem::height() { return 0; }
UIItem *UITextItem::CopyUIItem() { return nullptr; }
int UITextItem::m_DefaultFont = 0;
int UITextItem::m_Sat = 0;
char UITextItem::dummy_str[4];
UITextItem::UITextItem(const char *, unsigned int, unsigned char) {}
UITextItem::UITextItem(int, const char *, unsigned int, unsigned char) {}
UITextItem::~UITextItem() {}
const UITextItem &UITextItem::operator=(const UITextItem &) { return *this; }
void UIText::Create(UIWindow *, UITextItem *, int, int, int) {}
void UIText::OnFormat() {}
newuiSheet::newuiSheet() {}
void newuiSheet::AddButton(const char *, int16_t, int16_t) {}
newuiListBox *newuiSheet::AddListBox(int16_t, int16_t, int16_t, uint16_t) { return nullptr; }
void newuiSheet::AddLongButton(const char *, int16_t, int16_t) {}
void newuiSheet::AddText(const char *, ...) {}
void newuiSheet::NewGroup(const char *, int16_t, int16_t, tAlignment, int16_t) {}
newuiListBox::newuiListBox() {}
void newuiListBox::AddItem(const char *) {}
void newuiListBox::Create(UIWindow *, int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t) {}
void newuiListBox::RemoveItem(const char *) {}
void newuiListBox::RemoveAll() {}
int newuiListBox::GetCurrentIndex() { return 0; }
void newuiListBox::SetCurrentIndex(int) {}
bool newuiListBox::GetCurrentItem(char *, int) { return false; }
bool newuiListBox::GetItem(int, char *, int) { return false; }
bool newuiListBox::SetCurrentItem(const char *) { return false; }
void newuiListBox::SetSelectChangeCallback(void (*)(int)) {}
void newuiListBox::SetSelectChangeCallback(void (*)(int, void *), void *) {}
void newuiListBox::OnLostFocus() {}
void newuiListBox::OnGainFocus() {}
void newuiListBox::OnDraw() {}
void newuiListBox::OnKeyDown(int) {}
void newuiListBox::OnKeyUp(int) {}
void newuiListBox::OnMouseBtnDown(int) {}
void newuiListBox::OnMouseBtnUp(int) {}
void newuiListBox::OnNotifySelect(UIGadget *) {}
void newuiListBox::OnSelect() {}
void newuiListBox::OnDestroy() {}
void newuiListBox::OnUserProcess() {}
void newuiListBox::OnAttachToWindow() {}
void newuiListBox::OnDetachFromWindow() {}
newuiTiledWindow::newuiTiledWindow() {}
void newuiTiledWindow::OnDraw() {}
void newuiTiledWindow::OnDestroy() {}
void newuiTiledWindow::Create(const char *, int16_t, int16_t, int16_t, int16_t, int) {}
int newuiTiledWindow::DoUI() { return 0; }
newuiSheet *newuiTiledWindow::GetSheet() { return nullptr; }
NewUIWindow::NewUIWindow() {}
void NewUIWindow::Create(int, int, int, int, int) {}
void NewUIWindow::LoadBackgroundImage(const char *) {}
void NewUIWindow::OnDraw() {}
void NewUIWindow::OnDestroy() {}
void NewUIWindowLoadBackgroundImage(NewUIWindow *, const char *) {}
void NewUIEdit::Create(UIWindow *, int, int, int, int, int, int) {}
void NewUIEdit::OnDraw() {}
void NewUIListBox::Create(UIWindow *, int, int, int, int, int, int) {}
void NewUIListBox::OnDraw() {}
void NewUIButton::Create(UIWindow *, int, UIItem *, int, int, int, int, int) {}
void NewUIButton::OnDraw() {}
void NewUIButton::OnFormat() {}
NewUIGameWindow::NewUIGameWindow() {}
void NewUIGameWindow::Create(int, int, int, int, int) {}
newuiButton::newuiButton() {}
void newuiButton::Create(UIWindow *, int16_t, const char *, int16_t, int16_t, int16_t) {}
void newuiButton::InitStates(const char *, bool, int16_t) {}
void newuiButton::OnDraw() {}
void newuiButton::OnDestroy() {}
void newuiButton::OnFormat() {}
void newuiButton::OnLostFocus() {}
void newuiButton::OnGainFocus() {}
void newuiArrowButton::Create(UIWindow *, int16_t, int16_t, const char *, int16_t, int16_t) {}
void newuiArrowButton::Show(bool) {}
void newuiArrowButton::OnMouseBtnDown(int) {}
void newuiArrowButton::OnMouseBtnUp(int) {}
void newuiArrowButton::OnDraw() {}
void UIHotspot::Create(UIWindow *, int, int, UIItem *, UIItem *, int, int, int, int, int) {}
void UIHotspot::OnKeyDown(int) {}
void UIHotspot::OnMouseBtnDown(int) {}
void UIHotspot::OnMouseBtnUp(int) {}
void UIHotspot::OnFormat() {}
void UIHotspot::OnDraw() {}
void UIHotspot::OnLostFocus() {}
void UIHotspot::OnGainFocus() {}
void UIHotspot::OnDestroy() {}
void UIListBox::AddItem(const UIItem *) {}
UIItem *UIListBox::GetItem(int) const { return nullptr; }
int UIListBox::GetSelectedIndex() const { return 0; }
void UIListBox::RemoveItem(const UIItem *) {}
void UIListBox::SelectItem(const UIItem *) {}
void UIListBox::SetHiliteColor(unsigned int) {}
void UIListBox::SetSelectedColor(unsigned int) {}
void UIListBox::OnLostFocus() {}
void UIListBox::OnGainFocus() {}
void UIListBox::OnDraw() {}
void UIListBox::OnKeyDown(int) {}
void UIListBox::OnKeyUp(int) {}
void UIListBox::OnMouseBtnDown(int) {}
void UIListBox::OnMouseBtnUp(int) {}
void UIListBox::OnSelect() {}
void UIListBox::OnDestroy() {}
void UIListBox::OnUserProcess() {}
void UIEdit::GetText(char *, int) {}
void UIEdit::SetText(const char *) {}
void UIEdit::OnLostFocus() {}
void UIEdit::OnGainFocus() {}
void UIEdit::OnDraw() {}
void UIEdit::OnKeyDown(int) {}
void UIEdit::OnKeyUp(int) {}
void UIEdit::OnMouseBtnDown(int) {}
void UIEdit::OnMouseBtnUp(int) {}
void UIEdit::OnSelect() {}
void UIEdit::OnDestroy() {}
pilot::pilot() {}
pilot::~pilot() {}

// ==== TESTS ====

/**
 * @brief GTest fixture for MultiUITest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class MultiUITest : public ::testing::Test {
protected:
  void SetUp() override {
    g_rec.clear();
    g_dll_calls.clear();
    g_settings_paths.clear();
    g_msgboxes.clear();
    ResetArgs();
    Descent = new TestApp();
    s_db.store.clear();
    MultiDLLGameStarting = 0;
    memset(Auto_login_addr, 0, sizeof(Auto_login_addr));
    memset(Auto_login_port, 0, sizeof(Auto_login_port));
    Current_pilot.mouselook_control = false;
    Current_mission.num_levels = 1;
    Current_mission.cur_level = 1;
    s_warp_result = 1;
    s_msgbox_ret = 1;
    s_pathdlg_ret = true;
  }
  void TearDown() override { delete Descent; }
};

// mouselook gating: off-pilot always joins; on-pilot depends on the game flag
/**
 * @test MultiUITest.MouselookCheckGatesOnPilotPreference
 * @brief Verifies mouselook Check Gates On Pilot Preference.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, MouselookCheckGatesOnPilotPreference) {
  // pilot doesn't use mouselook -> always allowed
  Current_pilot.mouselook_control = false;
  EXPECT_TRUE(DoPlayerMouselookCheck(NF_ALLOW_MLOOK));
  EXPECT_TRUE(DoPlayerMouselookCheck(0));

  // pilot uses mouselook and the game allows it
  Current_pilot.mouselook_control = true;
  EXPECT_TRUE(DoPlayerMouselookCheck(NF_ALLOW_MLOOK));

  // pilot uses mouselook but the game forbids it -> asks, answer yes
  s_msgbox_ret = 1;
  EXPECT_TRUE(DoPlayerMouselookCheck(0));
  ASSERT_EQ(g_msgboxes.size(), 1u);
  EXPECT_NE(g_msgboxes[0].find("joinanyway"), std::string::npos);

  // answer no
  g_msgboxes.clear();
  s_msgbox_ret = 0;
  EXPECT_FALSE(DoPlayerMouselookCheck(0));
}

// -pxo host:port parses and triggers the PXO dll auto-login
/**
 * @test MultiUITest.AutoConnectPXOParsesHostAndPort
 * @brief Verifies auto Connect PXOParses Host And Port.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, AutoConnectPXOParsesHostAndPort) {
  PushArg("-pxo");
  PushArg("pxo.parallax.com:3456");
  int ret = AutoConnectPXO();
  EXPECT_EQ(std::string(Auto_login_addr), "pxo.parallax.com");
  EXPECT_EQ(std::string(Auto_login_port), "3456");
  EXPECT_EQ(ret, MultiDLLGameStarting);
  bool saw_load = false, saw_call = false;
  for (auto &s : g_dll_calls) {
    if (s == "load:parallax online")
      saw_load = true;
    if (s == "call:6") // MT_AUTO_LOGIN
      saw_call = true;
  }
  EXPECT_TRUE(saw_load);
  EXPECT_TRUE(saw_call);
}

// -pxo with no port falls back to the "port" arg (no dash)
/**
 * @test MultiUITest.AutoConnectPXOUsesFallbackPort
 * @brief Verifies auto Connect PXOUses Fallback Port.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, AutoConnectPXOUsesFallbackPort) {
  PushArg("-pxo");
  PushArg("pxo.parallax.com");
  PushArg("port");
  PushArg("9999");
  AutoConnectPXO();
  EXPECT_EQ(std::string(Auto_login_addr), "pxo.parallax.com");
  EXPECT_EQ(std::string(Auto_login_port), "9999");
}

// missing -pxo means no auto-connect at all
/**
 * @test MultiUITest.AutoConnectPXORequiresArg
 * @brief Verifies auto Connect PXORequires Arg.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, AutoConnectPXORequiresArg) {
  int ret = AutoConnectPXO();
  EXPECT_EQ(ret, 0);
  EXPECT_TRUE(g_dll_calls.empty());
}

// -directip host:port splits in place; -ip copies the whole token (no split)
/**
 * @test MultiUITest.AutoConnectLANIPParsesBothForms
 * @brief Verifies auto Connect LANIPParses Both Forms.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, AutoConnectLANIPParsesBothForms) {
  PushArg("-directip");
  PushArg("192.168.1.5:13900");
  AutoConnectLANIP();
  EXPECT_EQ(std::string(Auto_login_addr), "192.168.1.5");
  EXPECT_EQ(std::string(Auto_login_port), "13900");
  bool saw = false;
  for (auto &s : g_dll_calls)
    if (s == "load:Direct TCP~IP")
      saw = true;
  EXPECT_TRUE(saw);

  g_dll_calls.clear();
  ResetArgs();
  PushArg("ip");
  PushArg("10.0.0.2:13900");
  AutoConnectLANIP();
  // the "ip" branch does not split on ':' -- it stores the full token
  EXPECT_EQ(std::string(Auto_login_addr), "10.0.0.2:13900");
  EXPECT_EQ(std::string(Auto_login_port), "");
}

// -heat reaches the HEAT.NET dll
/**
 * @test MultiUITest.AutoConnectHeatParsesHost
 * @brief Verifies auto Connect Heat Parses Host.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, AutoConnectHeatParsesHost) {
  PushArg("-heat");
  PushArg("heat.net:1234");
  AutoConnectHeat();
  EXPECT_EQ(std::string(Auto_login_addr), "heat.net");
  bool saw = false;
  for (auto &s : g_dll_calls)
    if (s == "load:HEAT.NET")
      saw = true;
  EXPECT_TRUE(saw);
}

// config save routes through the path dialog and appends .mps
/**
 * @test MultiUITest.ConfigSaveUsesDialogAndMpsExtension
 * @brief Verifies config Save Uses Dialog And Mps Extension.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, ConfigSaveUsesDialogAndMpsExtension) {
  MultiDoConfigSave();
  ASSERT_EQ(g_settings_paths.size(), 1u);
  std::filesystem::path p = g_settings_paths[0];
  EXPECT_EQ(p.extension(), ".mps");
  EXPECT_NE(p.string().find("custom/settings"), std::string::npos);
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "savemset"), g_rec.end());
}

// config load dispatches to MultiLoadSettings with the dialog path
/**
 * @test MultiUITest.ConfigLoadDispatchesToLoader
 * @brief Verifies config Load Dispatches To Loader.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, ConfigLoadDispatchesToLoader) {
  MultiDoConfigLoad();
  ASSERT_EQ(g_settings_paths.size(), 1u);
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "loadmset"), g_rec.end());
}

// returning to the game list flushes and calls the dll return event
/**
 * @test MultiUITest.ReturnToGameListFlushesAndCallsDLL
 * @brief Verifies return To Game List Flushes And Calls DLL.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, ReturnToGameListFlushesAndCallsDLL) {
  int ret = ReturnMultiplayerGameMenu();
  EXPECT_EQ(ret, MultiDLLGameStarting);
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "flushbuf"), g_rec.end());
  bool saw = false;
  for (auto &s : g_dll_calls)
    if (s == "call:8") // MT_RETURN_TO_GAME_LIST
      saw = true;
  EXPECT_TRUE(saw);
}

// single-level missions skip the warp dialog; multi-level uses its result
/**
 * @test MultiUITest.LevelSelectionSkipsWarpForSingleLevel
 * @brief Verifies level Selection Skips Warp For Single Level.
 *
 * @details
 * Exercises the MultiUITest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_ui.cpp
 * @ingroup descent3_tests
 */
TEST_F(MultiUITest, LevelSelectionSkipsWarpForSingleLevel) {
  Current_mission.num_levels = 1;
  int r = MultiLevelSelection();
  EXPECT_EQ(r, 1);
  EXPECT_TRUE(g_rec.empty() || std::find(g_rec.begin(), g_rec.end(), "warpdlg") == g_rec.end());

  Current_mission.num_levels = 5;
  s_warp_result = 3;
  r = MultiLevelSelection();
  EXPECT_EQ(r, 3);
  EXPECT_EQ(Current_mission.cur_level, 3);

  // a non-positive warp result clamps to level 1 and signals cancel
  s_warp_result = 0;
  r = MultiLevelSelection();
  EXPECT_EQ(r, -1);
  EXPECT_EQ(Current_mission.cur_level, 1);
}
