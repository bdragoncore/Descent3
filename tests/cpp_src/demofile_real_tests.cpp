/**
 * @file demofile_real_tests.cpp
 * @brief Tests for Descent3/demofile.cpp -- the demo record/replay event.
 *
 * @details
 * stream: header format, frame dispatch loop, and teardown paths.
 *
 * cf_* file I/O is replaced with an in-memory byte stream so record ->
 * playback round trips run entirely headless. The save-game helpers
 * (SGS* / LGS*) are no-ops that consume no bytes, keeping the protocol
 * offsets deterministic for the events under test.
 *
 * This harness validates the behavior of `Descent3/demofile.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/demofile.cpp`
 * @par Harness
 * `demofile_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/demofile.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "demofile.h"
#include "gamesave.h"
#include "object.h"
#include "objinfo.h"
#include "player.h"
#include "weapon.h"
#include "ship.h"
#include "Mission.h"
#include "game.h"
#include "hud.h"
#include "hlsoundlib.h"
#include "multi.h"
#include "args.h"
#include "chrono_timer.h"
#include "newui.h"
#include "newui_core.h"
#include "findintersection.h"
#include "gamesequence.h"
#include "psrand.h"
#include "damage.h"
#include "viseffect.h"

#define MAX_COOP_TURRETS 400 // matches the module-local define

// module globals without a header home
extern bool Demo_looping;
extern std::filesystem::path Old_demo_fname;

// ---- recorded output ----
struct KillRec {
  int victim;
  int killer;
  float damage;
  float delay;
};
static std::vector<KillRec> g_kills;
static std::vector<std::string> g_hud;
struct SoundRec {
  bool is3d;
  int idx;
  int obj;
  float vol;
};
static std::vector<SoundRec> g_sounds;
struct AttachRec {
  int parent, child, kind;
};
static std::vector<AttachRec> g_attach;
static std::vector<std::string> g_order;
static std::vector<std::string> g_msgboxes;
#define REC(name) g_order.push_back(name)

static bool g_dialog_ok = true;
static const char *g_dialog_name = "roundtrip";
static bool g_mission_loads = true;

// ---- memory-backed CFILE (stream stashed in the FILE* slot) ----
struct MemFile {
  std::vector<uint8_t> bytes;
  size_t pos = 0;
};
static std::map<std::string, MemFile *> g_store;

CFILE *cfopen(const std::filesystem::path &filename, const char *mode) {
  std::string key = filename.string();
  MemFile *mf;
  if (strchr(mode, 'w')) {
    mf = new MemFile;
    g_store[key] = mf;
  } else {
    auto it = g_store.find(key);
    if (it == g_store.end())
      return nullptr;
    it->second->pos = 0;
    mf = it->second;
  }
  CFILE *cfp = new CFILE;
  cfp->name = strdup(key.c_str());
  cfp->file = (FILE *)mf;
  cfp->lib_handle = -1;
  cfp->size = mf->bytes.size();
  cfp->lib_offset = 0;
  cfp->position = 0;
  cfp->flags = 0;
  return cfp;
}
void cfclose(CFILE *cfp) {
  free(cfp->name);
  delete cfp;
}
static MemFile *MF(CFILE *cfp) { return (MemFile *)cfp->file; }
static void mf_need(MemFile *mf, size_t n) {
  if (mf->pos + n > mf->bytes.size())
    throw 1; // mirrors the real cfile EOF throw
}

int cf_ReadBytes(uint8_t *buf, int count, CFILE *cfp) {
  MemFile *mf = MF(cfp);
  mf_need(mf, count);
  memcpy(buf, &mf->bytes[mf->pos], count);
  mf->pos += count;
  return count;
}
int32_t cf_ReadInt(CFILE *cfp, bool) {
  int32_t v;
  cf_ReadBytes((uint8_t *)&v, 4, cfp);
  return v;
}
int16_t cf_ReadShort(CFILE *cfp, bool) {
  int16_t v;
  cf_ReadBytes((uint8_t *)&v, 2, cfp);
  return v;
}
int8_t cf_ReadByte(CFILE *cfp) {
  int8_t v;
  cf_ReadBytes((uint8_t *)&v, 1, cfp);
  return v;
}
float cf_ReadFloat(CFILE *cfp) {
  float v;
  cf_ReadBytes((uint8_t *)&v, 4, cfp);
  return v;
}
int cf_ReadString(char *buf, size_t n, CFILE *cfp) {
  MemFile *mf = MF(cfp);
  size_t count = 0;
  for (;; count++) {
    mf_need(mf, 1);
    uint8_t c = mf->bytes[mf->pos++];
    if (c == 0)
      break;
    if (count < n - 1)
      buf[count] = c;
  }
  buf[count < n ? count : n - 1] = '\0';
  return count;
}

int cf_WriteBytes(const uint8_t *buf, int count, CFILE *cfp) {
  MemFile *mf = MF(cfp);
  mf->bytes.insert(mf->bytes.end(), buf, buf + count);
  return count;
}
int cf_WriteString(CFILE *cfp, const char *buf) {
  int len = strlen(buf);
  if (len)
    cf_WriteBytes((const uint8_t *)buf, len, cfp);
  cf_WriteByte(cfp, 0);
  return len + 1;
}
void cf_WriteInt(CFILE *cfp, int32_t i) { cf_WriteBytes((uint8_t *)&i, 4, cfp); }
void cf_WriteShort(CFILE *cfp, int16_t s) { cf_WriteBytes((uint8_t *)&s, 2, cfp); }
void cf_WriteByte(CFILE *cfp, int8_t b) { cf_WriteBytes((uint8_t *)&b, 1, cfp); }
void cf_WriteFloat(CFILE *cfp, float f) { cf_WriteBytes((uint8_t *)&f, 4, cfp); }

std::filesystem::path cf_GetWritableBaseDirectory() { return "/tmp/opencode"; }

// ---- world state ----
object Objects[MAX_OBJECTS];
object_info Object_info[MAX_OBJECT_IDS];
player Players[MAX_PLAYERS];
int Player_num = 0;
object *Viewer_object = nullptr;
object *Player_object = nullptr;
int Highest_object_index = 0;
uint16_t Local_object_list[MAX_OBJECTS];
uint16_t Server_object_list[MAX_OBJECTS];
float Gametime = 0, Frametime = 0;
float Min_frametime = 0, Max_frametime = 0, Avg_frametime = 0;
uint32_t Frames_counted = 0;
int FrameCount = 0;
int Game_mode = 0;
int Game_interface_mode = 0;
tMission Current_mission;
ship Ships[MAX_SHIPS];
weapon Weapons[MAX_WEAPONS];
poly_model Poly_models[MAX_POLY_MODELS];
float turret_holder[MAX_COOP_TURRETS];
gs_tables *gs_Xlates = nullptr;
bool is_multi_demo = false;
bool IsRestoredGame = false;
bool Game_paused = false;
float Render_FOV = 1.0f;
int Camera_view_mode[4] = {0, 0, 0, 0};

char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];
int FindArg(const char *which, int start) {
  for (int i = start; i < MAX_ARGS && GameArgs[i][0]; i++)
    if (strcasecmp(GameArgs[i], which) == 0)
      return i;
  return 0;
}

const char *GetStringFromTable(int id) {
  switch (id) {
  case 134:
    return "Error";
  case 407:
    return "cantload";
  case 408:
    return "baddemofile";
  case 404:
    return "Demo file saved!";
  default:
    return "str";
  }
}

Inventory::Inventory() {}
Inventory::~Inventory() {}
void Inventory::Reset(bool, int) {}

namespace D3 {
float g_timer_now = 0.0f;
float ChronoTimer::GetTime() { return g_timer_now; }
} // namespace D3

void *mem_malloc_sub(int size, const char *, int) { return malloc(size); }
void mem_free_sub(void *p) { free(p); }

hlsSystem::hlsSystem() {}
void hlsSystem::KillSoundLib(bool) {}
int hlsSystem::Play2dSound(int idx, float vol, float, unsigned short) {
  g_sounds.push_back({false, idx, -1, vol});
  return 0;
}
int hlsSystem::Play3dSound(int idx, object *obj, float vol, int, float) {
  g_sounds.push_back({true, idx, (int)OBJNUM(obj), vol});
  return 0;
}
void hlsSystem::ResumeSounds() { REC("resumesounds"); }

// save-game helpers: consume nothing, keeping stream offsets deterministic
void SGSXlateTables(CFILE *) {}
void SGSRooms(CFILE *) {}
void SGSTriggers(CFILE *) {}
void SGSObjects(CFILE *) {}
void SGSPlayers(CFILE *) {}
void SGSVisEffects(CFILE *) {}
void SGSSpew(CFILE *) {}
int LGSXlateTables(CFILE *) { REC("lgsxlate"); return 1; }
int LGSRooms(CFILE *) { REC("lgsrooms"); return 1; }
int LGSTriggers(CFILE *) { REC("lgstriggers"); return 1; }
int LGSObjects(CFILE *, int) { REC("lgsobjects"); return 1; }
int LGSPlayers(CFILE *) { REC("lgsplayers"); return 1; }
int LGSVisEffects(CFILE *) { REC("lgsviseffects"); return 1; }
int LGSSpew(CFILE *) { REC("lgsspew"); return 1; }

bool Osiris_SaveSystemState(CFILE *) { return true; }
bool Osiris_RestoreSystemState(CFILE *) { REC("osirestore"); return true; }
void Osiris_DisableCreateEvents() { REC("osioff"); }
void Osiris_EnableCreateEvents() { REC("osion"); }
void mng_LoadAddonPages() {}

bool LoadMission(const char *) { return g_mission_loads; }
void SetCurrentLevel(int level) { REC("setlevel" + std::to_string(level)); }
bool LoadAndStartCurrentLevel() { REC("startlevel"); return true; }
void PageInAllData() {}
int CreateAndFireWeapon(vector *, vector *, object *, int) { return 0; }
hlsSystem Sound_system;
vis_effect *VisEffects = nullptr;

bool AddHUDMessage(const char *fmt, ...) { g_hud.push_back(fmt); return true; }
bool AddBlinkingHUDMessage(const char *fmt, ...) { g_hud.push_back(std::string("blink:") + fmt); return true; }
bool AddColoredHUDMessage(ddgr_color, const char *fmt, ...) {
  g_hud.push_back(std::string("color:") + fmt);
  return true;
}
void ResetPersistentHUDMessage() { REC("resethud"); }
void AddPersistentHUDMessage(ddgr_color, int, int, float, int, int, const char *fmt, ...) {
  g_hud.push_back(std::string("persist:") + fmt);
}
void InitShipHUD(int ship) { REC("shiphud" + std::to_string(ship)); }
tHUDMode GetHUDMode() { return HUD_COCKPIT; }
void SetHUDMode(tHUDMode mode) { REC("hudmode" + std::to_string((int)mode)); }
void InitCockpit(int ship_index) { REC("cockpit" + std::to_string(ship_index)); }
void RestoreCameraRearviews() { REC("rearview"); }
void DoScreenshot() {}

void SetFunctionMode(function_mode) {}
void KillObject(object *victim, object *killer, float damage, int death_flags, float delay) {
  g_kills.push_back({(int)OBJNUM(victim), killer ? (int)OBJNUM(killer) : -1, damage, delay});
  (void)death_flags;
}
void InitiatePlayerDeath(object *, bool melee, int fate) {
  REC(std::string("death:") + (melee ? "melee" : "norm") + ":" + std::to_string(fate));
}
void EndPlayerDeath(int) {}
void ObjSetPos(object *, vector *, int roomnum, matrix *, bool) { REC("setpos" + std::to_string(roomnum)); }
int ObjCreate(uint8_t type, uint16_t id, int roomnum, vector *, const matrix *, int) {
  REC("objcreate:" + std::to_string(type) + ":" + std::to_string(id) + ":" + std::to_string(roomnum));
  return 60;
}
void InitObjectScripts(object *, bool) { REC("initscripts"); }
void AttachUpdateSubObjects(object *) { REC("attachsub"); }
bool AttachObject(object *parent, int8_t, object *child, char, bool) {
  g_attach.push_back({(int)OBJNUM(parent), (int)OBJNUM(child), 0});
  return true;
}
bool AttachObject(object *parent, int8_t, object *child, float rad) {
  g_attach.push_back({(int)OBJNUM(parent), (int)OBJNUM(child), 1000 + (int)rad});
  return true;
}
bool UnattachFromParent(object *child) {
  g_attach.push_back({-1, (int)OBJNUM(child), 2});
  return true;
}
void SetObjectControlType(object *, int) {}
void SetObjectDeadFlag(object *, bool, bool) { REC("setdead"); }
bool ObjGetAnimUpdate(uint16_t, custom_anim *) { return false; }
void ObjSetAnimUpdate(uint16_t, custom_anim *) { REC("animset"); }
void ObjGetTurretUpdate(uint16_t, multi_turret *) {}
void ObjSetTurretUpdate(uint16_t, multi_turret *) { REC("turretset"); }
int VisEffectCreate(uint8_t, uint8_t, int, vector *) { return -1; }
void Cinematic_DoDemoFileData(uint8_t *) { REC("cinedata"); }
int FindTextureName(const char *) { return 3; }
int FindWeaponName(const char *) { return 7; }
uint32_t MultiGetMatchChecksum(int type, int id) { return (uint32_t)(type * 1000 + id); }
int MultiMatchWeapon(uint32_t unique_id) { return (int)(unique_id % 1000); }
void MultiBuildMatchTables() { REC("matchtables"); }
void MultiDoMSafeFunction(unsigned char *) { REC("msafefn"); }
void MultiDoMSafePowerup(unsigned char *) { REC("msafepow"); }
void MultiMakePlayerGhost(int pnum) { REC("ghost" + std::to_string(pnum)); }
void MultiMakePlayerReal(int pnum) { REC("real" + std::to_string(pnum)); }
void PlayerSetRotatingBall(int, int, float, float *, float *, float *) { REC("balls"); }
void PlayerSwitchToObserver(int slot, int mode, int piggy) {
  REC("observe" + std::to_string(slot) + ":" + std::to_string(mode) + ":" + std::to_string(piggy));
}
void PlayerStopObserving(int slot) { REC("stopobserve" + std::to_string(slot)); }
int fvi_FindIntersection(fvi_query *, fvi_info *, bool) { return 0; }
// vm_VectorDistanceQuick comes from libvecmat.
int collide_player_and_weapon(object *, object *, vector *, vector *, bool, fvi_info *) { return 0; }
int collide_generic_and_weapon(object *, object *, vector *, vector *, bool, fvi_info *) { return 0; }
// ps_srand comes from libmisc; nothing to stub here.

// dialogs
bool DoEditDialog(const char *, char *buffer, int, bool) {
  strcpy(buffer, g_dialog_name);
  return g_dialog_ok;
}
int DoMessageBox(const char *title, const char *msg, int type, ddgr_color, ddgr_color) {
  g_msgboxes.push_back(std::string(title) + "/" + msg + "/" + std::to_string(type));
  return 1;
}
bool DoPathFileDialog(bool, std::filesystem::path &, const char *, const std::vector<std::string> &, int) {
  return false;
}
int DoUI() { return 0; }

// ---- UI class stubs (emit vtables for the demo-end menu references) ----
void UIGadget::OnFormat() {}
void UIGadget::OnSelect() {}
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
UITextItem::~UITextItem() {}
void UIText::Create(UIWindow *, UITextItem *, int, int, int) {}
void UIText::OnFormat() {}
newuiSheet::newuiSheet() {}
newuiTiledWindow::newuiTiledWindow() {}
void newuiTiledWindow::OnDraw() {}
void newuiTiledWindow::OnDestroy() {}
NewUIGameWindow::NewUIGameWindow() {}
void NewUIButton::Create(UIWindow *, int, UIItem *, int, int, int, int, int) {}
void NewUIButton::OnDraw() {}
void NewUIButton::OnFormat() {}
void NewUIGameWindow::Create(int, int, int, int, int) {}

// ---- helpers ----
static void ResetWorld() {
  memset(Objects, 0, sizeof(Objects));
  memset(Object_info, 0, sizeof(Object_info));
  memset(Players, 0, sizeof(Players));
  memset(Ships, 0, sizeof(Ships));
  memset(Weapons, 0, sizeof(Weapons));
  memset(Poly_models, 0, sizeof(Poly_models));
  memset(GameArgs, 0, sizeof(GameArgs));
  strcpy(GameArgs[0], "descent3");
  for (int i = 0; i < MAX_OBJECTS; i++)
    Objects[i].type = OBJ_NONE;
  Highest_object_index = 0;
  Viewer_object = Player_object = nullptr;
  Player_num = 0;
  Gametime = Frametime = Min_frametime = Max_frametime = Avg_frametime = 0;
  Frames_counted = 0;
  FrameCount = 0;
  Game_mode = 0;
  Game_interface_mode = 0;
  gs_Xlates = nullptr;
  is_multi_demo = false;
  IsRestoredGame = false;
  Render_FOV = 1.0f;
  Camera_view_mode[0] = Camera_view_mode[2] = 0;
  static char mission_name[] = "test.mn3";
  Current_mission.filename = mission_name;
  Current_mission.cur_level = 1;
  g_kills.clear();
  g_hud.clear();
  g_sounds.clear();
  g_attach.clear();
  g_order.clear();
  g_msgboxes.clear();
  g_dialog_ok = true;
  g_dialog_name = "roundtrip";
  g_mission_loads = true;
  for (auto &kv : g_store)
    delete kv.second;
  g_store.clear();
}

static void MakeRobot(int slot, float x) {
  Objects[slot].type = OBJ_ROBOT;
  Objects[slot].id = 5;
  Objects[slot].roomnum = 42;
  Objects[slot].pos = {x, 0, 0};
  vm_MakeIdentity(&Objects[slot].orient);
  Objects[slot].rtype.pobj_info.model_num = 9;
  Objects[slot].flags |= OF_POLYGON_OBJECT;
  if (slot > Highest_object_index)
    Highest_object_index = slot;
}

// stream builder mirroring the demo wire format
struct Buf {
  std::vector<uint8_t> b;
  void u8(uint8_t v) { b.push_back(v); }
  void i16(int v) {
    b.push_back(v & 0xff);
    b.push_back((v >> 8) & 0xff);
  }
  void i32(int32_t v) {
    for (int i = 0; i < 4; i++)
      b.push_back((v >> (8 * i)) & 0xff);
  }
  void f(float v) { i32(*(int32_t *)&v); }
  void str(const char *s) {
    while (*s)
      u8(*s++);
    u8(0);
  }
};

static void StoreForRead(const char *path, std::vector<uint8_t> bytes) {
  auto *mf = new MemFile;
  mf->bytes = std::move(bytes);
  g_store[path] = mf;
}

// craft a valid header so DemoPlaybackFile/DemoFrame get past setup
static Buf HeaderBytes(int level, float gametime, int frames) {
  Buf h;
  h.str(D3_DEMO_SIG_NEW);
  h.i16(GAMESAVE_VERSION);
  h.str("test.mn3");
  h.i32(level);
  h.f(gametime);
  h.i32(frames);
  h.i16(0); // Player_num restored at the end of the header
  return h;
}

// start recording through the public entry point and hand back the stream
static MemFile *StartRecording(const char *name) {
  g_dialog_name = name;
  Demo_flags = DF_NONE;
  DemoToggleRecording();
  auto it = g_store.find("/tmp/opencode/demo/" + std::string(name) + ".dem");
  return it == g_store.end() ? nullptr : it->second;
}

/**
 * @test DemoFile.ToggleRecordingWritesHeaderFrameAndStops
 * @brief Verifies toggle Recording Writes Header Frame And Stops.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, ToggleRecordingWritesHeaderFrameAndStops) {
  ResetWorld();
  MemFile *mf = StartRecording("hdr");
  ASSERT_NE(mf, nullptr);
  ASSERT_TRUE(Demo_flags & DF_RECORDING);

  size_t pos = 0;
  auto rd = [&](void *dst, size_t n) {
    memcpy(dst, &mf->bytes[pos], n);
    pos += n;
  };
  char sig[10] = {};
  rd(sig, strlen(D3_DEMO_SIG_NEW) + 1);
  EXPECT_STREQ(sig, D3_DEMO_SIG_NEW);
  int16_t ver;
  rd(&ver, 2);
  EXPECT_EQ(ver, GAMESAVE_VERSION);
  char mission[64] = {};
  while (mf->bytes[pos] != 0)
    strncat(mission, (const char *)&mf->bytes[pos++], 1);
  pos++;
  EXPECT_STREQ(mission, "test.mn3");
  int32_t level;
  rd(&level, 4);
  EXPECT_EQ(level, 1);
  float hgametime;
  rd(&hgametime, 4);
  int32_t frames;
  rd(&frames, 4);
  int16_t playernum;
  rd(&playernum, 2);

  // header ends with Player_num; the first frame marker follows
  // sig(6)+ver(2)+mission(9)+level(4)+gametime(4)+frames(4)+playernum(2)=31
  EXPECT_EQ(pos, 31u);
  ASSERT_LT(pos, mf->bytes.size());
  EXPECT_EQ(mf->bytes[pos], DT_NEW_FRAME);

  // stopping resets state and announces the save
  DemoToggleRecording();
  EXPECT_EQ(Demo_flags, DF_NONE);
  ASSERT_FALSE(g_hud.empty());
  EXPECT_EQ(g_hud.back(), "blink:Demo file saved!"); // TXT_DEMOSAVED(404)
  EXPECT_FALSE(Demo_fname.has_filename());
}

/**
 * @test DemoFile.ToggleDuringPlaybackIsIgnored
 * @brief Verifies toggle During Playback Is Ignored.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, ToggleDuringPlaybackIsIgnored) {
  ResetWorld();
  Demo_flags = DF_PLAYBACK;
  DemoToggleRecording();
  EXPECT_EQ(Demo_flags, DF_PLAYBACK);
  Demo_flags = DF_NONE;
}

/**
 * @test DemoFile.ChangedObjectsOnlyWritesMovedRenderables
 * @brief Verifies changed Objects Only Writes Moved Renderables.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, ChangedObjectsOnlyWritesMovedRenderables) {
  ResetWorld();
  MemFile *mf = StartRecording("moved");
  ASSERT_NE(mf, nullptr);
  mf->bytes.clear(); // drop header/frame, watch just this call

  MakeRobot(3, 10.0f);
  Objects[3].flags |= OF_MOVED_THIS_FRAME;
  MakeRobot(4, 20.0f); // not moved -> skipped
  Objects[6].type = OBJ_WEAPON;
  Objects[6].flags |= OF_MOVED_THIS_FRAME; // weapons never recorded

  DemoWriteChangedObjects();

  // one robot record: opcode+objnum+roomnum+vector+matrix
  ASSERT_EQ(mf->bytes.size(), 1u + 2 + 4 + 12 + 36);
  EXPECT_EQ(mf->bytes[0], DT_OBJ);

  // a moved player additionally records the player flag word
  mf->bytes.clear();
  Objects[3].type = OBJ_PLAYER;
  Objects[3].id = 0;
  Players[0].flags = 0x77;
  DemoWriteChangedObj(&Objects[3]);
  EXPECT_EQ(mf->bytes.size(), 1u + 2 + 4 + 12 + 36 + 4);

  DemoToggleRecording(); // close out
}

/**
 * @test DemoFile.FramePeriodicallyWritesPlayerInfo
 * @brief Verifies frame Periodically Writes Player Info.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, FramePeriodicallyWritesPlayerInfo) {
  ResetWorld();
  MemFile *mf = StartRecording("pinfo");
  ASSERT_NE(mf, nullptr);
  mf->bytes.clear();

  Players[0].objnum = 5;
  Objects[5].type = OBJ_PLAYER;
  Objects[5].shields = 80;
  Players[0].energy = 55.9f; // stored as a short: truncated to 55
  Players[0].weapon_flags = 0x21;
  Players[0].afterburn_time_left = 1.5f;

  Gametime = 7.0f;
  Frametime = 0.25f;
  D3::g_timer_now = 1.0f; // > DEMO_PINFO_UPDATE since recording started
  DemoStartNewFrame();

  // frame marker + gametime + frametime + player info record
  size_t base = 1 + 4 + 4;
  size_t pinfo = 1 + 2 + 2 + MAX_PLAYER_WEAPONS * 2 + 4 + 4 + 4 + 4 + 4;
  ASSERT_EQ(mf->bytes.size(), base + pinfo);
  EXPECT_EQ(mf->bytes[0], DT_NEW_FRAME);
  EXPECT_EQ(mf->bytes[base], DT_PLAYER_INFO);

  DemoToggleRecording();
}

/**
 * @test DemoFile.KillExplodeRoundTrip
 * @brief Verifies kill Explode Round Trip.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, KillExplodeRoundTrip) {
  ResetWorld();
  MakeRobot(8, 0);
  MakeRobot(9, 0);

  Buf b = HeaderBytes(1, -1.0f, 0);
  b.u8(DT_OBJ_EXPLODE);
  b.i16(8);    // victim
  b.i16(9);    // killer
  b.f(33.5f);  // damage
  b.i32(0x11); // death flags
  b.f(2.5f);   // delay
  b.i16(1234); // random seed
  b.u8(DT_NEW_FRAME);
  b.f(-1.0f); // next gametime (keeps the pacing loop inert)
  b.f(0.1f);
  StoreForRead("/tmp/opencode/kill.dem", std::move(b.b));

  ASSERT_TRUE(DemoPlaybackFile("/tmp/opencode/kill.dem"));
  DemoFrame();
  ASSERT_EQ(g_kills.size(), 1u);
  EXPECT_EQ(g_kills[0].victim, 8);
  EXPECT_EQ(g_kills[0].killer, 9);
  EXPECT_FLOAT_EQ(g_kills[0].damage, 33.5f);
  EXPECT_FLOAT_EQ(g_kills[0].delay, 2.5f);
  ASSERT_FALSE(g_order.empty());
  int replayed = ps_rand();
  ps_srand(1234);
  EXPECT_EQ(ps_rand(), replayed); // seed was replayed deterministically

  Demo_flags = DF_NONE;
}

/**
 * @test DemoFile.HudMessagesRouteByColorAndBlink
 * @brief Verifies hud Messages Route By Color And Blink.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, HudMessagesRouteByColorAndBlink) {
  ResetWorld();
  Buf b = HeaderBytes(1, -1.0f, 0);
  b.u8(DT_HUD_MESSAGE);
  b.i32(0x00FF00); // colored
  b.u8(0);
  b.str("colored!");
  b.u8(DT_HUD_MESSAGE);
  b.i32(0);
  b.u8(1); // blinking
  b.str("blinkmsg");
  b.u8(DT_HUD_MESSAGE);
  b.i32(0);
  b.u8(0); // plain
  b.str("plainmsg");
  b.u8(DT_NEW_FRAME);
  b.f(-1.0f);
  b.f(0.1f);
  StoreForRead("/tmp/opencode/hud.dem", std::move(b.b));

  ASSERT_TRUE(DemoPlaybackFile("/tmp/opencode/hud.dem"));
  DemoFrame();
  ASSERT_EQ(g_hud.size(), 3u);
  EXPECT_EQ(g_hud[0], "color:colored!"); // color routes to AddColoredHUDMessage
  EXPECT_EQ(g_hud[1], "blink:blinkmsg"); // blink flag routes to AddBlinkingHUDMessage
  EXPECT_EQ(g_hud[2], "plainmsg");       // default goes to AddHUDMessage

  Demo_flags = DF_NONE;
}

/**
 * @test DemoFile.SoundEventsRoundTripThroughObjects
 * @brief Verifies sound Events Round Trip Through Objects.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, SoundEventsRoundTripThroughObjects) {
  ResetWorld();
  MakeRobot(11, 0);
  Buf b = HeaderBytes(1, -1.0f, 0);
  b.u8(DT_2D_SOUND);
  b.i16(44);
  b.f(0.75f);
  b.u8(DT_3D_SOUND);
  b.i16(11); // object emitting
  b.i16(45);
  b.f(0.5f);
  b.u8(DT_NEW_FRAME);
  b.f(-1.0f);
  b.f(0.1f);
  StoreForRead("/tmp/opencode/snd.dem", std::move(b.b));

  ASSERT_TRUE(DemoPlaybackFile("/tmp/opencode/snd.dem"));
  DemoFrame();
  ASSERT_EQ(g_sounds.size(), 2u);
  EXPECT_FALSE(g_sounds[0].is3d);
  EXPECT_EQ(g_sounds[0].idx, 44);
  EXPECT_FLOAT_EQ(g_sounds[0].vol, 0.75f);
  EXPECT_TRUE(g_sounds[1].is3d);
  EXPECT_EQ(g_sounds[1].obj, 11);
  EXPECT_EQ(g_sounds[1].idx, 45);

  Demo_flags = DF_NONE;
}

/**
 * @test DemoFile.PlayerDeathRoundTrip
 * @brief Verifies player Death Round Trip.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, PlayerDeathRoundTrip) {
  ResetWorld();
  MakeRobot(12, 0);
  Buf b = HeaderBytes(1, -1.0f, 0);
  b.u8(DT_PLAYER_DEATH);
  b.i16(12);
  b.u8(1); // melee
  b.i32(5);
  b.u8(DT_NEW_FRAME);
  b.f(-1.0f);
  b.f(0.1f);
  StoreForRead("/tmp/opencode/death.dem", std::move(b.b));

  ASSERT_TRUE(DemoPlaybackFile("/tmp/opencode/death.dem"));
  DemoFrame();
  bool saw_death = false;
  for (auto &r : g_order)
    if (r == "death:melee:5")
      saw_death = true;
  EXPECT_TRUE(saw_death);

  Demo_flags = DF_NONE;
}

/**
 * @test DemoFile.AttachVariantsAndUnattachReplay
 * @brief Verifies attach Variants And Unattach Replay.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, AttachVariantsAndUnattachReplay) {
  ResetWorld();
  MakeRobot(13, 0);
  MakeRobot(14, 0);
  // QUIRK: DemoWriteAttachObjRad actually emits DT_ATTACH (not
  // DT_ATTACH_RAD), so radius records replay through the wrong reader.
  // The DT_ATTACH_RAD reader still exists -- feed it directly.
  Buf b = HeaderBytes(1, -1.0f, 0);
  b.u8(DT_ATTACH);
  b.i16(13); // parent
  b.u8(3);   // parent attach point
  b.i16(14); // child
  b.u8(1);   // child attach point
  b.u8(1);   // aligned
  b.u8(DT_ATTACH_RAD);
  b.i16(13);
  b.u8(2);
  b.i16(14);
  b.f(0.25f);
  b.u8(DT_UNATTACH);
  b.i16(14);
  b.u8(DT_NEW_FRAME);
  b.f(-1.0f);
  b.f(0.1f);
  StoreForRead("/tmp/opencode/attach.dem", std::move(b.b));

  ASSERT_TRUE(DemoPlaybackFile("/tmp/opencode/attach.dem"));
  DemoFrame();
  ASSERT_EQ(g_attach.size(), 3u);
  EXPECT_EQ(g_attach[0].parent, 13);
  EXPECT_EQ(g_attach[0].child, 14);
  EXPECT_EQ(g_attach[0].kind, 0);
  EXPECT_EQ(g_attach[1].kind, 1000 + 0); // rad record lands in float overload
  EXPECT_EQ(g_attach[2].parent, -1);     // unattach marker
  EXPECT_EQ(g_attach[2].child, 14);

  Demo_flags = DF_NONE;
}

/**
 * @test DemoFile.ObjCreateAppliesTranslationTablesAndMaps
 * @brief Verifies obj Create Applies Translation Tables And Maps.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, ObjCreateAppliesTranslationTablesAndMaps) {
  ResetWorld();
  MakeRobot(15, 0);
  Buf b = HeaderBytes(1, -1.0f, 0);
  b.u8(DT_OBJ_CREATE);
  b.u8(OBJ_ROBOT);
  b.u8(1); // has orientation
  b.i16(7);
  b.i32(77);
  b.f(1); b.f(2); b.f(3);
  b.i32(0); // parent handle
  for (int i = 0; i < 9; i++)
    b.f(0); // identity-ish matrix slots
  b.i16(99); // old object number
  b.u8(DT_NEW_FRAME);
  b.f(-1.0f);
  b.f(0.1f);
  StoreForRead("/tmp/opencode/create.dem", std::move(b.b));

  // xlate table comes back from the header load
  gs_Xlates = new gs_tables;
  memset(gs_Xlates, 0, sizeof(gs_tables));
  gs_Xlates->obji_indices[7] = 77;

  ASSERT_TRUE(DemoPlaybackFile("/tmp/opencode/create.dem"));
  DemoFrame();

  bool saw_create = false;
  for (auto &r : g_order)
    if (r == "objcreate:2:77:77") // OBJ_ROBOT, xlated id, roomnum
      saw_create = true;
  EXPECT_TRUE(saw_create);

  EXPECT_EQ(Demo_obj_map[99], 60);
  EXPECT_EQ(Server_object_list[99], 60);
  EXPECT_TRUE(Objects[60].flags & OF_SERVER_OBJECT);
  bool saw_scripts = false;
  for (auto &r : g_order)
    if (r == "initscripts")
      saw_scripts = true;
  EXPECT_TRUE(saw_scripts);

  Demo_flags = DF_NONE;
}

/**
 * @test DemoFile.EndOfFileTearsDownAndPostsMenu
 * @brief Verifies end Of File Tears Down And Posts Menu.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, EndOfFileTearsDownAndPostsMenu) {
  ResetWorld();
  StoreForRead("/tmp/opencode/eof.dem", HeaderBytes(1, -1.0f, 0).b);
  Demo_fname = "eof.dem";
  Demo_looping = false;

  ASSERT_TRUE(DemoPlaybackFile("/tmp/opencode/eof.dem"));
  DemoFrame(); // hits EOF immediately

  EXPECT_EQ(Demo_flags, DF_NONE);
  EXPECT_EQ(gs_Xlates, nullptr); // freed by DemoAbort
  EXPECT_EQ(Game_interface_mode, GAME_POST_DEMO);
  EXPECT_FALSE(Demo_fname.has_filename());

  // looping demos reload the same file instead of exiting
  ResetWorld();
  StoreForRead("/tmp/opencode/eof2.dem", HeaderBytes(1, -1.0f, 0).b);
  Demo_fname = "eof2.dem";
  Demo_looping = true;
  ASSERT_TRUE(DemoPlaybackFile("/tmp/opencode/eof2.dem"));
  DemoFrame();
  EXPECT_EQ(Game_interface_mode, GAME_DEMO_LOOP);
  EXPECT_EQ(Demo_fname, Old_demo_fname);
  EXPECT_STREQ(Demo_fname.string().c_str(), "eof2.dem");
  Demo_looping = false;
  Demo_flags = DF_NONE;
}

/**
 * @test DemoFile.ReadHeaderRejectsUnknownSignature
 * @brief Verifies read Header Rejects Unknown Signature.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, ReadHeaderRejectsUnknownSignature) {
  ResetWorld();
  Buf b;
  b.str("JUNK!");
  b.i16(GAMESAVE_VERSION);
  b.str("whatever.mn3");
  b.i32(1);
  b.f(0);
  b.i32(0);
  b.i16(0);
  StoreForRead("/tmp/opencode/bad.dem", std::move(b.b));

  EXPECT_EQ(DemoPlaybackFile("/tmp/opencode/bad.dem"), 0);
  ASSERT_EQ(g_msgboxes.size(), 1u);
  EXPECT_NE(g_msgboxes[0].find("baddemofile"), std::string::npos);
  EXPECT_EQ(Demo_flags, DF_NONE);
}

/**
 * @test DemoFile.MissingDemoFileReportsErrorWithoutStateChange
 * @brief Verifies missing Demo File Reports Error Without State Change.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, MissingDemoFileReportsErrorWithoutStateChange) {
  ResetWorld();
  EXPECT_EQ(DemoPlaybackFile("/tmp/opencode/nope.dem"), 0);
  ASSERT_EQ(g_msgboxes.size(), 1u);
  EXPECT_NE(g_msgboxes[0].find("cantload"), std::string::npos);
  EXPECT_EQ(Demo_flags, DF_NONE);
}

/**
 * @test DemoFile.PlaybackHonorsMakeMovieFlag
 * @brief Verifies playback Honors Make Movie Flag.
 *
 * @details
 * Exercises the DemoFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/demofile.cpp
 * @ingroup descent3_tests
 */
TEST(DemoFile, PlaybackHonorsMakeMovieFlag) {
  ResetWorld();
  strcpy(GameArgs[1], "-makemovie");
  StoreForRead("/tmp/opencode/movie.dem", HeaderBytes(1, -1.0f, 42).b);

  EXPECT_TRUE(DemoPlaybackFile("/tmp/opencode/movie.dem"));
  EXPECT_TRUE(Demo_make_movie);
  EXPECT_EQ(FrameCount, 42); // restored from header

  Demo_flags = DF_NONE;
  Demo_make_movie = false;
}
