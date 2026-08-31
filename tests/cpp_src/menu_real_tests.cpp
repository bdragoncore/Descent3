/**
 * @file menu_real_tests.cpp
 * @brief Tests for Descent3/menu.cpp — main menu, command-line auto connect,.
 *
 * @details
 * new-game mission dialog and level-warp dialog.
 *
 * menu.cpp is linked for real together with newui_core.cpp and the stock
 * ui library (same cascade trick as newuicore/config targets). Everything
 * else (game hooks, mission database, multiplayer glue) is stubbed here.
 * mmInterface/mmItem live in mmItem.cpp/menuman.cpp which are not part of
 * any test build, so their out-of-class members are defined below.
 *
 * NOTE: engine quirk pinned by NewGameOkWithoutSelectionShowsError —
 * pressing OK in the mission dialog without a listbox click yields
 * GetCurrentIndex() == -1 and the load-error message box instead of
 * starting the game.
 *
 * This harness validates the behavior of `Descent3/menu.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/menu.cpp`
 * @par Harness
 * `menu_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/menu.cpp
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <deque>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "args.h"
#include "cfile.h"
#include "ddio.h"
#include "gamesave.h"
#include "gamesequence.h"
#include "game.h"
#include "LoadLevel.h"
#include "mem.h"
#include "menu.h"
#include "Mission.h"
#include "mmItem.h"
#include "multi_dll_mgr.h"
#include "multi_ui.h"
#include "newui_core.h"
#include "pilot.h"
#include "pilot_class.h"
#include "ship.h"
#include "stringtable.h"

// ==== recorder ====

// Menu-item ids are #defines local to menu.cpp; mirror the RELEASE set.
#define IDV_QUIT 0xff
#define IDV_NEWGAME 10
#define IDV_MULTIPLAYER 11
#define IDV_OPTIONS 12
#define IDV_PILOT 13
#define IDV_LOADGAME 14
#define IDV_PLAYDEMO 15
#define IDV_CREDITS 16

// Globals defined by menu.cpp itself.
extern bool Auto_connected;
extern bool Directplay_lobby_launched_game;
extern bool IsRestoredGame;
extern bool FirstGame;

// Functions defined by menu.cpp (menu.h only declares MainMenu).
bool ProcessCommandLine();
bool MenuNewGame();
int DisplayLevelWarpDlg(int max_level);

// Globals normally defined by args.cpp / manager.cpp.
char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];
int GameArgc = 1;
ship Ships[MAX_SHIPS];
player Players[MAX_PLAYERS];

static const char *SkipArgPrefix(const char *s) {
  while (*s == '-' || *s == '+')
    s++;
  return s;
}
int FindArg(const char *which, int start) {
  for (int i = start; i < GameArgc; i++)
    if (!strcasecmp(SkipArgPrefix(GameArgs[i]), SkipArgPrefix(which)))
      return i;
  return 0;
}
const char *GetArg(int index) {
  return (index >= 0 && index < GameArgc) ? GameArgs[index] : nullptr;
}

struct Rec {
  std::vector<std::string> strings;
  template <typename... A> void operator()(A... a) {
    std::ostringstream os;
    (os << ... << a);
    strings.push_back(os.str());
  }
  size_t size() const { return strings.size(); }
  void clear() { strings.clear(); }
};
#define REC(var, ...) (var)(__VA_ARGS__)

static Rec g_screenmode, g_gamemode, g_funcmode, g_flip, g_loadmission,
    g_updatemsns, g_pilotselect, g_pltread, g_msgbox, g_waitmsg, g_menuscene,
    g_calldll, g_loadmultidll, g_openlib, g_closelib, g_loadlevelinfo,
    g_setshipperm, g_resetperm;
static std::deque<int> s_doui_seq;
static std::vector<std::tuple<int, int, std::string, int>> s_additems;
static std::vector<int> s_music_regions;
static bool s_loadmultidll_result = false;
static bool s_autopxo_result = false, s_lanip_result = false,
            s_heat_result = false;
static bool s_mainmp_result = false, s_returnmp_result = false;
static bool s_loadgame_result = false, s_loaddemo_result = false;
static bool s_loadmission_ok = true;
static int s_msgbox_answer = 0;
static int s_highest_level = 0;
static int s_ship_permission = 0;
static int s_find_mission_data = 0;
static const int kOpenLibHandle = 777;
static std::string s_pilot_filename;
static std::string s_last_loaded_mission;
static std::map<std::string, bool> s_msn_single;       // filename -> single?
static std::map<std::string, std::string> s_msn_names; // filename -> name
static std::map<std::string, std::string> s_level_names;
static int s_getmsn_calls = 0;
static std::map<std::string, std::vector<std::filesystem::path>> g_dir_files;

// globals referenced by menu.cpp but normally defined elsewhere
bool Mem_quick_exit = false;
bool IsCheater = false;
bool Demo_looping = false;
bool Demo_restart = false;
int MultiDLLGameStarting = 0;
char Auto_login_addr[MAX_AUTO_LOGIN_STUFF_LEN];
char Auto_login_port[MAX_AUTO_LOGIN_STUFF_LEN];
tMission Current_mission;

#include <SDL3/SDL.h>
#include "bitmap.h"
#include "d3music.h"
#include "hlsoundlib.h"
#include "Inventory.h"

// ==== shared stub tail (mirrors config/newuicore test layers) ====
int Max_window_w = 640;
int Max_window_h = 480;
bool Multi_bail_ui_menu = false;
int Game_fonts[NUM_FONTS] = {0};
BOOL TCP_active = 0;
bms_bitmap GameBitmaps[MAX_BITMAPS];
bool Game_fullscreen = false;
SDL_Window *GSDLWindow = nullptr;

static function_mode s_function_mode = MENU_MODE;
function_mode GetFunctionMode() { return s_function_mode; }
void SetFunctionMode(function_mode mode) {
  s_function_mode = mode;
  REC(g_funcmode, mode);
}

// Escape hatch for real newuiTiledWindow::DoUI loops: when armed, every
// deferred frame forces UI_frame_result, so the dialog returns promptly.
// (DoUI resets UI_frame_result to -1 on entry, hence the latch.)
extern int UI_frame_result;
static bool s_defer_force_exit = false;
static int s_defer_escape = UID_CANCEL;

struct FakeApp : oeApplication {
  void init() override {}
  void get_info(void *) override {}
  int flags() const override { return 0; }
  unsigned defer() override {
    REC(g_waitmsg, "defer");
    if (s_defer_force_exit)
      UI_frame_result = s_defer_escape;
    return 0;
  }
  void delay(float) override {}
  void set_defer_handler(void (*)(bool)) override {}
};
static FakeApp g_fake_app;
oeApplication *Descent = &g_fake_app;

namespace D3 {
float ChronoTimer::GetTime() { return 0.0f; }
int64_t ChronoTimer::GetTimeMS() { return 0; }
void ChronoTimer::SleepMS(int) {}
void ChronoTimer::Initialize() {}
} // namespace D3

void (*DebugBreak_callback_stop)() = nullptr;
void (*DebugBreak_callback_resume)() = nullptr;
void Error(const char *, ...) {}

// bitmap stubs
static uint16_t s_gamma_bitmap_data[128 * 128];
int bm_AllocLoadFileBitmap(const char *, int, int) { return 900; }
int bm_AllocBitmap(int w, int h, int) { return (w == 128 && h == 128) ? 901 : -1; }
uint16_t *bm_data(int handle, int) { return (handle == 901) ? s_gamma_bitmap_data : nullptr; }
int bm_w(int, int) { return 64; }
int bm_h(int, int) { return 64; }
bool bm_CreateChunkedBitmap(int, chunked_bitmap *) { return true; }
void bm_DestroyChunkedBitmap(chunked_bitmap *) {}
void bm_FreeBitmap(int) {}

void *mem_malloc_sub(int size, const char *, int) { return malloc(size); }
void mem_free_sub(void *p) { free(p); }
char *mem_strdup_sub(const char *s, const char *, int) { return strdup(s); }

// input stubs: dead keyboard/mouse
bool ddio_GetAdjKeyState(int) { return false; }
void ddio_KeyFlush() {}
int ddio_KeyInKey() { return 0; }
bool ddio_MouseGetEvent(int *btn, bool *state) {
  if (btn)
    *btn = 0;
  if (state)
    *state = false;
  return false;
}
int ddio_MouseGetState(int *x, int *y, int *dx, int *dy, int *z, int *dz) {
  if (x)
    *x = 0;
  if (y)
    *y = 0;
  if (dx)
    *dx = 0;
  if (dy)
    *dy = 0;
  if (z)
    *z = 0;
  if (dz)
    *dz = 0;
  return 0;
}
void ddio_MouseReset() {}
void ddio_MouseSetVCoords(int, int) {}
void ddio_ff_GetInfo(bool *found, bool *) {
  if (found)
    *found = false;
}

// renderer / frame stubs (rend_Flip lives with the other recorders above)
void rend_ClearScreen(ddgr_color) {}
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, uint8_t) {}
void rend_DrawLine(int, int, int, int) {}
void rend_DrawPolygon2D(int, g3Point **, int) {}
void rend_DrawScaledBitmap(int, int, int, int, int, float, float, float, float,
                           int, const float *) {}
void rend_DrawFontCharacter(int, int, int, int, int, float, float, float, float) {}
void rend_EndFrame() {}
void rend_StartFrame(int, int, int, int, int) {}
void rend_SetAlphaType(int8_t) {}
void rend_SetAlphaValue(uint8_t) {}
void rend_SetColorModel(color_model) {}
void rend_SetFlatColor(ddgr_color) {}
void rend_SetLighting(light_state) {}
void rend_SetOverlayType(uint8_t) {}
void rend_SetTextureType(texture_type) {}
void rend_SetWrapType(wrap_type) {}
void rend_SetZBufferState(int8_t) {}
int rend_SetPreferredState(renderer_preferred_state *, bool) { return 1; }

float Render_FOV = 72.0f;
int Render_preferred_bitdepth = 32;
renderer_preferred_state Render_preferred_state{};

void StartFrame(bool) {}
void StartFrame(int, int, int, int, bool, bool) {}
void EndFrame() {}
void DoScreenshot() {}

// text / font stubs
extern "C" {
void grtext_SetParameters(int, int, int, int, int) {}
void grtext_SetFlags(int) {}
int grtext_GetFont() { return 0; }
int grtext_GetTextHeight(const char *) { return 12; }
int grtext_GetTextLineWidth(const char *) { return 40; }
void grtext_Puts(int, int, const char *) {}
void grtext_CenteredPrintf(int, int, const char *, ...) {}
void grtext_SetColor(ddgr_color) {}
void grtext_SetAlpha(uint8_t) {}
void grtext_SetFont(int) {}
void grtext_Flush() {}
int grfont_GetHeight(int) { return 12; }
int grfont_KeyToAscii(int, int) { return 'a'; }
}
void textaux_WordWrap(const char *, char *, int, int) {}

// sound / music stubs
hlsSystem::hlsSystem() {}
int hlsSystem::Play2dSound(int, float, float, uint16_t) { return 0; }
void hlsSystem::BeginSoundFrame(bool) {}
void hlsSystem::EndSoundFrame() {}
int hlsSystem::GetLLSoundQuantity() { return 16; }
void hlsSystem::SetLLSoundQuantity(int) {}
float hlsSystem::GetMasterVolume() { return 1.0f; }
void hlsSystem::SetMasterVolume(float) {}
char hlsSystem::GetSoundQuality() { return SQT_NORMAL; }
bool hlsSystem::SetSoundQuality(char) { return true; }
void hlsSystem::StopSoundImmediate(int) {}
void hlsSystem::KillSoundLib(bool) {}
hlsSystem Sound_system;
float D3MusicGetVolume() { return 0.5f; }
void D3MusicSetVolume(float) {}
void D3MusicDoFrame(tMusicSeqInfo *) {}
int FindSoundName(const char *) { return 777; }

// SDL assert hook used by the real ui library
SDL_AssertState SDL_ReportAssertion(SDL_AssertData *, const char *, const char *,
                                    int) {
  return SDL_ASSERTION_IGNORE;
}

// cfile stubs
CFILE *cfopen(const std::filesystem::path &, const char *) { return nullptr; }
void cfclose(CFILE *) {}
int cf_ReadBytes(uint8_t *, int, CFILE *) { return 0; }
int32_t cf_ReadInt(CFILE *, bool) { return 0; }
int16_t cf_ReadShort(CFILE *, bool) { return 0; }
int8_t cf_ReadByte(CFILE *) { return 0; }
std::vector<std::filesystem::path> cf_LocateMultiplePaths(const std::filesystem::path &) {
  std::vector<std::filesystem::path> out;
  for (const auto &entry : g_dir_files)
    out.push_back(entry.first);
  return out;
}

// inventory stub
Inventory::Inventory() {}
Inventory::~Inventory() {}

// pilot global with recordable hud accessors
pilot::pilot() {}
pilot::~pilot() {}
void pilot::get_hud_data(uint8_t *hmode, uint16_t *hstat,
                         uint16_t *hgraphicalstat, int *gw_w, int *gw_h) {
  if (hmode)
    *hmode = 0;
  if (hstat)
    *hstat = 0;
  if (hgraphicalstat)
    *hgraphicalstat = 0;
  if (gw_w)
    *gw_w = 640;
  if (gw_h)
    *gw_h = 480;
}
void pilot::set_hud_data(uint8_t *, uint16_t *, uint16_t *, int *, int *) {}
pilot Current_pilot;

static void ResetRecorderState() {
  g_screenmode.clear();
  g_gamemode.clear();
  g_funcmode.clear();
  g_flip.clear();
  g_loadmission.clear();
  g_updatemsns.clear();
  g_pilotselect.clear();
  g_pltread.clear();
  g_msgbox.clear();
  g_waitmsg.clear();
  g_menuscene.clear();
  g_calldll.clear();
  g_loadmultidll.clear();
  g_openlib.clear();
  g_closelib.clear();
  g_loadlevelinfo.clear();
  g_setshipperm.clear();
  g_resetperm.clear();
  s_doui_seq.clear();
  s_additems.clear();
  s_music_regions.clear();
}

// ==== game-hook stubs ====
void SetScreenMode(int sm, bool force_res_change) {
  REC(g_screenmode, sm, ",", force_res_change ? "force" : "");
}
void SetGameMode(int mode) { REC(g_gamemode, mode); }
void rend_Flip() {
  REC(g_flip, "flip");
  // end-of-frame escape for real newuiTiledWindow::DoUI loops (the frame
  // itself clears UI_frame_result, so force it again here)
  if (s_defer_force_exit)
    UI_frame_result = s_defer_escape;
}

bool LoadMission(const char *msn) {
  REC(g_loadmission, msn);
  s_last_loaded_mission = msn;
  return s_loadmission_ok;
}
void CurrentPilotUpdateMissionStatus(bool just_add_data) {
  REC(g_updatemsns, just_add_data ? 1 : 0);
}
void PilotSelect() { REC(g_pilotselect, "select"); }
void PltReadFile(pilot *plt, bool keyconfig, bool missiondata) {
  REC(g_pltread, plt == &Current_pilot ? "cur" : "?", ",", keyconfig ? 1 : 0,
      ",", missiondata ? 1 : 0);
}
int PilotGetHighestLevelAchieved(pilot *, char *) { return s_highest_level; }
int GetPilotShipPermissions(pilot *, const char *) { return s_ship_permission; }
bool PlayerSetShipPermission(int pnum, char *ship_name, bool allowed) {
  REC(g_setshipperm, pnum, "|", ship_name, "|", allowed ? 1 : 0);
  return true;
}
bool PlayerResetShipPermissions(int pnum, bool set_default) {
  REC(g_resetperm, pnum, "|", set_default ? 1 : 0);
  return true;
}

bool GetMissionInfo(const std::filesystem::path &msnfile, tMissionInfo *msn) {
  s_getmsn_calls++;
  std::string key = msnfile.filename().u8string();
  *msn = tMissionInfo{};
  auto n = s_msn_names.find(key);
  if (n != s_msn_names.end())
    strncpy(msn->name, n->second.c_str(), MSN_NAMELEN - 1);
  msn->single = s_msn_single[key];
  return !s_msn_names[key].empty();
}
bool LoadLevelInfo(const std::filesystem::path &filename, level_info &info) {
  REC(g_loadlevelinfo, filename.u8string());
  strncpy(info.name, s_level_names[filename.u8string()].c_str(),
          sizeof(info.name) - 1);
  info.name[sizeof(info.name) - 1] = '\0';
  return true;
}

int LoadMultiDLL(const char *dllname) {
  REC(g_loadmultidll, dllname);
  return s_loadmultidll_result;
}
void CallMultiDLL(int action) { REC(g_calldll, action); }
int AutoConnectPXO() { return s_autopxo_result; }
int AutoConnectLANIP() { return s_lanip_result; }
int AutoConnectHeat() { return s_heat_result; }
int MainMultiplayerMenu() { return s_mainmp_result; }
int ReturnMultiplayerGameMenu() { return s_returnmp_result; }
bool LoadGameDialog() { return s_loadgame_result; }
bool LoadDemoDialog() { return s_loaddemo_result; }
void MenuScene() { REC(g_menuscene, "scene"); }
void OptionsMenu() {}

int DoMessageBox(const char *title, const char *msg, int type, ddgr_color,
                 ddgr_color) {
  REC(g_msgbox, title ? title : "", "|", msg ? msg : "", "|", type);
  return s_msgbox_answer;
}
void DoWaitMessage(bool enable, const char *message) {
  REC(g_waitmsg, enable ? 1 : 0, "|", message ? message : "");
}

int cf_OpenLibrary(const std::filesystem::path &libname) {
  REC(g_openlib, libname.u8string());
  return kOpenLibHandle;
}
void cf_CloseLibrary(int handle) { REC(g_closelib, handle); }

void ddio_DoForeachFile(
    const std::filesystem::path &ext, const std::regex &,
    const std::function<void(std::filesystem::path)> &fp) {
  auto it = g_dir_files.find(ext.u8string());
  if (it == g_dir_files.end())
    return;
  for (const auto &f : it->second)
    fp(f);
}

const char *GetStringFromTable(int index) {
  static char bufs[64][24];
  static int rot = 0;
  char *b = bufs[rot % 64];
  rot++;
  snprintf(b, sizeof(bufs[0]), "STR%d", index);
  return b;
}

// pilot members normally living in pilot.cpp
std::string pilot::get_filename() { return s_pilot_filename; }
void pilot::set_filename(const std::string &fname) { s_pilot_filename = fname; }
int pilot::find_mission_data(const char *) { return s_find_mission_data; }

// mmItem / mmInterface members normally living in mmItem.cpp / menuman.cpp
mmInterface *mmItem::m_window = nullptr;
mmItem::mmItem() {}
mmItem::~mmItem() {}
void mmItem::AttachInterface(mmInterface *wnd) { m_window = wnd; }
void mmItem::Create(int, int, int, int, const char *, int, tmmItemFX) {}
void mmItem::AddEffect(tmmItemFX) {}
void mmItem::ClearEffects() {}
void mmItem::OnMouseBtnDown(int) {}
void mmItem::OnMouseBtnUp(int) {}
void mmItem::OnFormat() {}
void mmItem::OnDraw() {}
void mmItem::OnLostFocus() {}
void mmItem::OnGainFocus() {}
void mmItem::OnDestroy() {}
void mmItem::OnUserProcess() {}
void mmItem::OnSelect() {}

bool mmInterface::AddItem(int id, int key, const char *text, int type) {
  s_additems.emplace_back(id, key, text ? text : "", type);
  return true;
}
void mmInterface::Create() {}
int mmInterface::DoUI() {
  if (s_doui_seq.empty())
    return UID_CANCEL; // safety valve so UI loops always terminate
  int v = s_doui_seq.front();
  s_doui_seq.pop_front();
  return v;
}
void mmInterface::SetMusicRegion(int region) {
  s_music_regions.push_back(region);
}
void mmInterface::OnDestroy() {}
void mmInterface::CopyrightText() {}

// ==== fixture ====
/**
 * @brief GTest fixture for MenuTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class MenuTest : public ::testing::Test {
protected:
  void SetUp() override {
    ResetRecorderState();
    Auto_connected = false;
    Demo_looping = Demo_restart = false;
    TCP_active = false;
    Mem_quick_exit = false;
    IsCheater = false;
    FirstGame = false;
    IsRestoredGame = false;
    Directplay_lobby_launched_game = false;
    Auto_login_addr[0] = Auto_login_port[0] = '\0';
    s_loadmultidll_result = false;
    s_autopxo_result = s_lanip_result = s_heat_result = false;
    s_mainmp_result = s_returnmp_result = false;
    s_loadgame_result = s_loaddemo_result = false;
    s_defer_force_exit = false;
    s_defer_escape = UID_CANCEL;
    s_loadmission_ok = true;
    MultiDLLGameStarting = 0;
    s_msgbox_answer = 0;
    s_highest_level = 0;
    s_ship_permission = 0;
    s_find_mission_data = 0;
    s_pilot_filename.clear();
    s_last_loaded_mission.clear();
    s_msn_single.clear();
    s_msn_names.clear();
    s_level_names.clear();
    s_getmsn_calls = 0;
    g_dir_files.clear();

    for (char *f : level_name_bufs)
      delete[] f;
    level_name_bufs.clear();
    Current_mission = tMission{};
    strncpy(mission_filename_buf, "test.mn3", sizeof(mission_filename_buf));
    Current_mission.filename = mission_filename_buf;
    strncpy(Current_mission.name, "TestMission", MSN_NAMELEN - 1);

    TestSetArgs({});
  }

  void SetMissionLevels(int n) {
    Current_mission.levels.clear();
    Current_mission.num_levels = n;
    for (int i = 0; i < n; i++) {
      auto *fn = new char[16];
      snprintf(fn, 16, "lvl%d.d3l", i);
      level_name_bufs.push_back(fn);
      tLevelNode node{};
      node.filename = fn;
      Current_mission.levels.push_back(node);
      s_level_names[fn] = std::string("L") + std::to_string(i);
    }
  }

  static void TestSetArgs(std::initializer_list<const char *> args) {
    GameArgc = 1;
    strcpy(GameArgs[0], "descent3");
    for (const char *a : args) {
      ASSERT_LT(GameArgc, MAX_ARGS);
      strncpy(GameArgs[GameArgc], a, MAX_CHARS_PER_ARG - 1);
      GameArgs[GameArgc][MAX_CHARS_PER_ARG - 1] = '\0';
      GameArgc++;
    }
  }

  static void ScriptDoUI(std::initializer_list<int> seq) {
    s_doui_seq.assign(seq);
  }

  static void ScriptDialog(int escape = UID_CANCEL) {
    s_defer_escape = escape;
    s_defer_force_exit = true;
  }

  void FeedMissions(const std::initializer_list<const char *> &names) {
    std::vector<std::filesystem::path> v;
    for (auto *n : names)
      v.emplace_back(n);
    g_dir_files["missions"] = v;
  }

  char mission_filename_buf[64];
  std::vector<char *> level_name_bufs;
};

// ==== ProcessCommandLine tests ====

/**
 * @test MenuTest.UrlIpAutoConnectStartsGame
 * @brief Verifies url Ip Auto Connect Starts Game.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, UrlIpAutoConnectStartsGame) {
  TCP_active = true;
  s_loadmultidll_result = true;
  MultiDLLGameStarting = 1;
  TestSetArgs({"-url", "descent3://ip/192.168.1.5"});

  EXPECT_TRUE(ProcessCommandLine());
  EXPECT_TRUE(Auto_connected);
  ASSERT_EQ(g_loadmultidll.size(), 1u);
  EXPECT_NE(g_loadmultidll.strings[0].find("Direct TCP~IP"), std::string::npos);
  ASSERT_EQ(g_calldll.size(), 1u);
  EXPECT_EQ(g_calldll.strings[0], std::to_string(MT_AUTO_LOGIN));
  EXPECT_STREQ(Auto_login_addr, "192.168.1.5");
  EXPECT_EQ(Auto_login_port[0], '\0');
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));
}

/**
 * @test MenuTest.UrlDllFailureStaysInMenu
 * @brief Verifies url Dll Failure Stays In Menu.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, UrlDllFailureStaysInMenu) {
  TCP_active = true;
  s_loadmultidll_result = false;
  TestSetArgs({"-url", "descent3://ip/10.0.0.1"});

  EXPECT_FALSE(ProcessCommandLine());
  EXPECT_TRUE(Auto_connected); // consumed the attempt
  EXPECT_EQ(g_calldll.size(), 0u);
  EXPECT_EQ(g_funcmode.size(), 0u);
}

/**
 * @test MenuTest.UrlServerNotStartingStaysInMenu
 * @brief Verifies url Server Not Starting Stays In Menu.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, UrlServerNotStartingStaysInMenu) {
  TCP_active = true;
  s_loadmultidll_result = true;
  MultiDLLGameStarting = 0;
  TestSetArgs({"-url", "descent3://ip/10.0.0.2"});

  EXPECT_FALSE(ProcessCommandLine());
  EXPECT_EQ(g_calldll.size(), 1u);
  EXPECT_EQ(g_funcmode.size(), 0u);
}

/**
 * @test MenuTest.UrlIgnoredWithoutTcpActive
 * @brief Verifies url Ignored Without Tcp Active.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, UrlIgnoredWithoutTcpActive) {
  TCP_active = false;
  TestSetArgs({"-url", "descent3://ip/10.0.0.3"});

  EXPECT_FALSE(ProcessCommandLine());
  EXPECT_FALSE(Auto_connected);
  EXPECT_EQ(g_loadmultidll.size(), 0u);
}

/**
 * @test MenuTest.PxoFlagExitsToGame
 * @brief Verifies pxo Flag Exits To Game.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, PxoFlagExitsToGame) {
  s_autopxo_result = true;
  TestSetArgs({"-pxo"});

  EXPECT_TRUE(ProcessCommandLine());
  EXPECT_TRUE(Auto_connected);
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));
}

/**
 * @test MenuTest.DirectIpFlagExitsToGame
 * @brief Verifies direct Ip Flag Exits To Game.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, DirectIpFlagExitsToGame) {
  s_lanip_result = true;
  TestSetArgs({"-directip"});

  EXPECT_TRUE(ProcessCommandLine());
  EXPECT_TRUE(Auto_connected);
}

/**
 * @test MenuTest.HeatFlagExitsToGame
 * @brief Verifies heat Flag Exits To Game.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, HeatFlagExitsToGame) {
  s_heat_result = true;
  TestSetArgs({"-heat"});

  EXPECT_TRUE(ProcessCommandLine());
  EXPECT_TRUE(Auto_connected);
}

/**
 * @test MenuTest.ConnectSplitsHostPortAndUsesLan
 * @brief Verifies connect Splits Host Port And Uses Lan.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, ConnectSplitsHostPortAndUsesLan) {
  s_lanip_result = true;
  TestSetArgs({"+connect", "myhost.example:2099", "+cl_pxotrack", "0"});

  EXPECT_TRUE(ProcessCommandLine());
  EXPECT_TRUE(Auto_connected);
  EXPECT_STREQ(Auto_login_addr, "myhost.example");
  EXPECT_STREQ(Auto_login_port, "2099");
}

/**
 * @test MenuTest.ConnectWithTrackOneUsesPxo
 * @brief Verifies connect With Track One Uses Pxo.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, ConnectWithTrackOneUsesPxo) {
  s_autopxo_result = true;
  TestSetArgs({"+connect", "pxohost:1234", "+cl_pxotrack", "1"});

  EXPECT_TRUE(ProcessCommandLine());
  EXPECT_STREQ(Auto_login_addr, "pxohost");
  EXPECT_STREQ(Auto_login_port, "1234");
}

/**
 * @test MenuTest.LobbyLaunchExitsToGame
 * @brief Verifies lobby Launch Exits To Game.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, LobbyLaunchExitsToGame) {
  Directplay_lobby_launched_game = true;

  EXPECT_TRUE(ProcessCommandLine());
  EXPECT_TRUE(Auto_connected);
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));
}

/**
 * @test MenuTest.NoArgumentsReturnsFalse
 * @brief Verifies no Arguments Returns False.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, NoArgumentsReturnsFalse) {
  EXPECT_FALSE(ProcessCommandLine());
  EXPECT_FALSE(Auto_connected);
  EXPECT_EQ(g_funcmode.size(), 0u);
}

// Runs last among PCL tests: -mission flips a function-local static.
/**
 * @test MenuTest.MissionArgExitsToGameOnce
 * @brief Verifies mission Arg Exits To Game Once.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, MissionArgExitsToGameOnce) {
  TestSetArgs({"-mission"});

  EXPECT_TRUE(ProcessCommandLine());
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));

  // second invocation: static flag suppresses the mission entry
  g_funcmode.clear();
  EXPECT_FALSE(ProcessCommandLine());
  EXPECT_EQ(g_funcmode.size(), 0u);
}

// ==== DisplayLevelWarpDlg tests ====
// Empty script == immediate UID_CANCEL from the DoUI safety valve.

/**
 * @test MenuTest.WarpCancelReturnsMinusOneListsAllLevels
 * @brief Verifies warp Cancel Returns Minus One Lists All Levels.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, WarpCancelReturnsMinusOneListsAllLevels) {
  SetMissionLevels(3);

  ScriptDialog();
  EXPECT_EQ(DisplayLevelWarpDlg(-1), -1);
  // max_level -1 -> every mission level gets listed
  ASSERT_EQ(g_loadlevelinfo.size(), 3u);
  EXPECT_EQ(g_loadlevelinfo.strings[0], "lvl0.d3l");
  EXPECT_EQ(g_openlib.size(), 0u); // not a split mission
}

/**
 * @test MenuTest.WarpMaxBoundsListedLevels
 * @brief Verifies warp Max Bounds Listed Levels.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, WarpMaxBoundsListedLevels) {
  SetMissionLevels(5);

  ScriptDialog(UID_CANCEL);
  DisplayLevelWarpDlg(3);
  ASSERT_EQ(g_loadlevelinfo.size(), 3u);
  EXPECT_EQ(g_loadlevelinfo.strings[2], "lvl2.d3l"); // lvl3/lvl4 excluded
}

/**
 * @test MenuTest.SplitMissionOpensSiblingLibrary
 * @brief Verifies split Mission Opens Sibling Library.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, SplitMissionOpensSiblingLibrary) {
  SetMissionLevels(2);

  strncpy(Current_mission.filename, "d3.mn3", sizeof(mission_filename_buf));
  ScriptDialog();
  DisplayLevelWarpDlg(2);
  ASSERT_EQ(g_openlib.size(), 1u);
  EXPECT_NE(g_openlib.strings[0].find("d3_2.mn3"), std::string::npos);
  ASSERT_EQ(g_closelib.size(), 1u);
  EXPECT_EQ(g_closelib.strings[0], std::to_string(kOpenLibHandle));

  // reverse split direction
  g_openlib.clear();
  g_closelib.clear();
  strncpy(Current_mission.filename, "d3_2.mn3", sizeof(mission_filename_buf));
  ScriptDialog();
  DisplayLevelWarpDlg(2);
  ASSERT_EQ(g_openlib.size(), 1u);
  EXPECT_NE(g_openlib.strings[0].find("/d3.mn3"), std::string::npos);
  EXPECT_EQ(g_closelib.size(), 1u);
}

// ==== MenuNewGame tests ====

/**
 * @test MenuTest.NewGameFirstRunLaunchesTraining
 * @brief Verifies new Game First Run Launches Training.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, NewGameFirstRunLaunchesTraining) {
  s_find_mission_data = -1; // no mission progress on this pilot yet

  EXPECT_TRUE(MenuNewGame());
  EXPECT_TRUE(FirstGame); // set by the training branch
  EXPECT_EQ(s_last_loaded_mission, "training.mn3");
  ASSERT_EQ(g_updatemsns.size(), 1u);
  ASSERT_EQ(g_gamemode.size(), 1u);
  EXPECT_EQ(g_gamemode.strings[0], std::to_string(GM_NORMAL));
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));
  EXPECT_EQ(s_doui_seq.size(), 0u); // never opened a dialog
}

/**
 * @test MenuTest.NewGameFirstGameSecondCallLoadsMainMission
 * @brief Verifies new Game First Game Second Call Loads Main Mission.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, NewGameFirstGameSecondCallLoadsMainMission) {
  FirstGame = true; // training was already offered previously
  s_find_mission_data = 0;

  EXPECT_TRUE(MenuNewGame());
  EXPECT_FALSE(FirstGame); // consumed
  EXPECT_EQ(s_last_loaded_mission, "d3.mn3");
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));
}

/**
 * @test MenuTest.NewGameTrainingFailureShowsError
 * @brief Verifies new Game Training Failure Shows Error.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, NewGameTrainingFailureShowsError) {
  s_find_mission_data = -1;
  s_loadmission_ok = false;

  EXPECT_FALSE(MenuNewGame());
  ASSERT_EQ(g_msgbox.size(), 1u);
  EXPECT_EQ(g_funcmode.size(), 0u);
}

/**
 * @test MenuTest.NewGameDialogFilterAndCancelPath
 * @brief Verifies new Game Dialog Filter And Cancel Path.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, NewGameDialogFilterAndCancelPath) {
  s_find_mission_data = 0;
  FeedMissions({"good.mn3", "mp_only.mn3"});
  s_msn_names["good.mn3"] = "Good Mission";
  s_msn_single["good.mn3"] = true;
  s_msn_names["mp_only.mn3"] = "MP Mission";
  s_msn_single["mp_only.mn3"] = false; // filtered out (multiplayer)
  ScriptDialog(UID_CANCEL);

  EXPECT_FALSE(MenuNewGame());
  EXPECT_GE(s_getmsn_calls, 1); // filter evaluated the directory feed
  EXPECT_EQ(g_funcmode.size(), 0u);
  EXPECT_EQ(g_gamemode.size(), 0u);
}

// Engine quirk: OK without a listbox click leaves GetCurrentIndex() at -1,
// so the dialog shows the load-error message box instead of starting.
/**
 * @test MenuTest.NewGameOkWithoutSelectionShowsError
 * @brief Verifies new Game Ok Without Selection Shows Error.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, NewGameOkWithoutSelectionShowsError) {
  s_find_mission_data = 0;
  FeedMissions({"good.mn3"});
  s_msn_names["good.mn3"] = "Good Mission";
  s_msn_single["good.mn3"] = true;
  ScriptDialog(UID_OK);

  EXPECT_FALSE(MenuNewGame());
  ASSERT_GE(g_msgbox.size(), 1u);
  EXPECT_EQ(g_funcmode.size(), 0u);
}

// No legal mission in any directory -> error box before the dialog opens.
/**
 * @test MenuTest.NewGameZeroMissionsShowsError
 * @brief Verifies new Game Zero Missions Shows Error.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, NewGameZeroMissionsShowsError) {
  s_find_mission_data = 0;
  FeedMissions({"mp_only.mn3"});
  s_msn_names["mp_only.mn3"] = "MP Mission";
  s_msn_single["mp_only.mn3"] = false;

  EXPECT_FALSE(MenuNewGame());
  ASSERT_GE(g_msgbox.size(), 1u);
  EXPECT_EQ(g_funcmode.size(), 0u);
}

// ==== MainMenu tests ====
// NOTE: PilotArgLoadsPilotFile depends on MainMenu's function-local
// `first_time` static and must therefore stay the FIRST MainMenu test.

/**
 * @test MenuTest.PilotArgLoadsPilotFileOnce
 * @brief Verifies pilot Arg Loads Pilot File Once.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, PilotArgLoadsPilotFileOnce) {
  TestSetArgs({"-pilot", "ace"});
  ScriptDoUI({IDV_CREDITS}); // leave via credits right after opening

  EXPECT_EQ(MainMenu(), 0);
  EXPECT_EQ(s_pilot_filename, "ace.plt");
  ASSERT_EQ(g_pltread.size(), 1u);
  EXPECT_EQ(g_pltread.strings[0], "cur,1,0");
  EXPECT_EQ(g_pilotselect.size(), 0u); // filename non-empty -> no prompt
}

/**
 * @test MenuTest.EmptyPilotTriggersSelectThenQuitWorks
 * @brief Verifies empty Pilot Triggers Select Then Quit Works.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, EmptyPilotTriggersSelectThenQuitWorks) {
  ScriptDoUI({IDV_QUIT});
  s_msgbox_answer = 1; // confirm quit

  EXPECT_EQ(MainMenu(), 1);
  EXPECT_EQ(g_pilotselect.size(), 1u);
  EXPECT_TRUE(Mem_quick_exit);
  EXPECT_FALSE(Directplay_lobby_launched_game);
}

/**
 * @test MenuTest.QuitDeclinedFallsThroughToCredits
 * @brief Verifies quit Declined Falls Through To Credits.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, QuitDeclinedFallsThroughToCredits) {
  ScriptDoUI({IDV_QUIT, IDV_CREDITS});
  s_msgbox_answer = 0; // decline quit

  EXPECT_EQ(MainMenu(), 0);
  ASSERT_EQ(g_msgbox.size(), 1u);
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(CREDITS_MODE));
}

/**
 * @test MenuTest.DemoLoopingSwitchesToLoadDemo
 * @brief Verifies demo Looping Switches To Load Demo.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, DemoLoopingSwitchesToLoadDemo) {
  Demo_looping = true;
  Demo_restart = true;

  EXPECT_EQ(MainMenu(), 0);
  EXPECT_FALSE(Demo_restart); // consumed
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(LOADDEMO_MODE));
  EXPECT_EQ(g_gamemode.strings.back(), std::to_string(GM_NORMAL));
}

/**
 * @test MenuTest.MultiplayerEnablesOnlyUsedShips
 * @brief Verifies multiplayer Enables Only Used Ships.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, MultiplayerEnablesOnlyUsedShips) {
  strncpy(Ships[0].name, "Phoenix", sizeof(Ships[0].name));
  Ships[0].used = 1;
  Ships[1].used = 0;
  strncpy(Ships[2].name, "Titan", sizeof(Ships[2].name));
  Ships[2].used = 1;
  s_mainmp_result = true;
  ScriptDoUI({IDV_MULTIPLAYER});

  EXPECT_EQ(MainMenu(), 0);
  ASSERT_EQ(g_setshipperm.size(), 2u);
  EXPECT_NE(g_setshipperm.strings[0].find("Phoenix"), std::string::npos);
  EXPECT_NE(g_setshipperm.strings[0].find("|1"), std::string::npos);
  EXPECT_NE(g_setshipperm.strings[1].find("Titan"), std::string::npos);
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));
  // multiplayer music region requested before the menu ran
  EXPECT_NE(std::find(s_music_regions.begin(), s_music_regions.end(),
                      MULTI_MUSIC_REGION),
            s_music_regions.end());
}

/**
 * @test MenuTest.MultiDLLStartingReturnsToMultiplayerGame
 * @brief Verifies multi DLLStarting Returns To Multiplayer Game.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, MultiDLLStartingReturnsToMultiplayerGame) {
  MultiDLLGameStarting = 1;
  s_returnmp_result = true;

  EXPECT_EQ(MainMenu(), 0); // exits before any DoUI
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));
}

/**
 * @test MenuTest.FirstGameShortCircuitLaunchesTrainingDirectly
 * @brief Verifies first Game Short Circuit Launches Training Directly.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, FirstGameShortCircuitLaunchesTrainingDirectly) {
  FirstGame = true;
  s_find_mission_data = -1; // training path inside MenuNewGame
  s_loadmission_ok = true;

  EXPECT_EQ(MainMenu(), 0);
  // short-circuit returns before the interface: no scene, no flip, no items
  EXPECT_EQ(g_menuscene.size(), 0u);
  EXPECT_EQ(g_flip.size(), 0u);
  EXPECT_EQ(s_additems.size(), 0u);
  ASSERT_EQ(g_gamemode.size(), 1u);
  EXPECT_EQ(g_gamemode.strings[0], std::to_string(GM_NORMAL));
  ASSERT_EQ(g_funcmode.size(), 1u);
  EXPECT_EQ(g_funcmode.strings[0], std::to_string(GAME_MODE));
}

/**
 * @test MenuTest.MainMenuBuildsEightItemsWithMarkers
 * @brief Verifies main Menu Builds Eight Items With Markers.
 *
 * @details
 * Exercises the MenuTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/menu.cpp
 * @ingroup descent3_tests
 */
TEST_F(MenuTest, MainMenuBuildsEightItemsWithMarkers) {
  ScriptDoUI({IDV_CREDITS});

  EXPECT_EQ(MainMenu(), 0);
  // menu screen was selected before the interface opened
  ASSERT_EQ(g_screenmode.size(), 1u);
  EXPECT_EQ(g_screenmode.strings[0], std::to_string(SM_MENU) + ",force");
  ASSERT_EQ(s_additems.size(), 8u);
  EXPECT_EQ(std::get<0>(s_additems[0]), IDV_NEWGAME);
  EXPECT_EQ(std::get<3>(s_additems[0]), MM_STARTMENU_TYPE);
  EXPECT_EQ(std::get<0>(s_additems[7]), IDV_QUIT);
  EXPECT_EQ(std::get<3>(s_additems[7]), MM_ENDMENU_TYPE);
  for (size_t i = 1; i < 7; i++)
    EXPECT_EQ(std::get<3>(s_additems[i]), 0);
}
