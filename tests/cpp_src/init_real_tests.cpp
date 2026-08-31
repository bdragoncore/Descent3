/**
 * @file init_real_tests.cpp
 * @brief Unit tests for Descent3/init.cpp: settings save/load, PreInit arg handling,.
 *
 * @details
 * boot sequence orchestration, and the ShutdownD3/RestartD3 state machine.
 * Subsystems are stubbed with recorders so ordering and argument plumbing can
 * be asserted without touching real hardware or data files.
 *
 * This harness validates the behavior of `Descent3/init.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/init.cpp`
 * @par Harness
 * `init_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/init.cpp
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "init.h"
#include "application.h"
#include "appdatabase.h"
#include "descent.h"
#include "config.h"
#include "game.h"
#include "gameloop.h"
#include "pilot_class.h"
#include "hlsoundlib.h"
#include "d3music.h"
#include "ddio.h"
#include "cfile.h"
#include "bitmap.h"
#include "renderer.h"
#include "grtext.h"
#include "ui.h"
#include "SmallViews.h"
#include "multi.h"
#include "localization.h"
#include "stringtable.h"
#include "mem.h"
#include "terrain.h"

// module globals without a header home
extern uint16_t Gameport;
extern uint16_t PXOPort;
extern float Mouselook_sensitivity;
extern float Mouse_sensitivity;
extern std::filesystem::path config_base_directory;
extern void LoadGameSettings();

// ==== recorder helpers ====
static std::vector<std::string> g_rec;
static void REC(const std::string &s) { g_rec.push_back(s); }
static size_t Pos(const std::string &s) {
  for (size_t i = 0; i < g_rec.size(); i++)
    if (g_rec[i] == s)
      return i;
  return g_rec.size();
}

static std::vector<std::string> g_dirs;
static std::vector<std::string> g_libs;

// ==== GameArgs plumbing (real layout from args.h) ====
#include "args.h"
char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];
void ResetArgs() {
  memset(GameArgs, 0, sizeof(GameArgs));
  strcpy(GameArgs[0], "d3test"); // argv[0]
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
  unsigned defer() override { REC("defer"); return 0; }
  void delay(float) override {}
  void set_defer_handler(void (*f)(bool)) override { REC(f ? "deferhandler" : "nodeferhandler"); }
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

// ==== game globals normally defined in unlinked modules ====
std::vector<tVideoResolution> Video_res_list;
int Default_resolution_id = 0;
int Current_video_resolution_id = 0;
int Display_id = 0;
bool Game_fullscreen = true;
tDetailSettings Detail_settings{};
int Default_detail_level = DETAIL_LEVEL_MED;
rendering_state Render_state;
renderer_preferred_state Render_preferred_state;
int Render_preferred_bitdepth = 16;
float Render_FOV_setting = D3_DEFAULT_FOV;
float Render_FOV = D3_DEFAULT_FOV;
int Game_window_x = 0, Game_window_y = 0, Game_window_w = 640, Game_window_h = 480;
int Game_mode = 0;
bool Game_paused = false;
bool Katmai = false;
int Use_file_xfer = 1;
int Min_allowed_frametime = 0;
bool Render_powerup_sparkles = false;
bool PlayPowerupVoice = true;
bool PlayVoices = true;
char Sound_quality = SQT_NORMAL;
char Sound_mixer = SOUND_MIXER_SOFTWARE_16;
char Sound_card_name[256] = {};
std::string Default_pilot;
int Num_languages = 1;
int Missile_camera_window = SVW_LEFT;
bool Dedicated_server = false;
netgame_info Netgame{};
int Game_fonts[8] = {7};
bool Mem_low_memory_mode = false;
bool Mem_superlow_memory_mode = false;
std::filesystem::path Descent3_temp_directory;
// error_Init / DebugBreak callbacks come from libddebug.

// function mode plumbing
static function_mode s_fmode = INIT_MODE;
function_mode GetFunctionMode() { return s_fmode; }
void SetFunctionMode(function_mode fm) {
  s_fmode = fm;
  REC(std::string("fmode:") + std::to_string((int)fm));
}

// screen / ui callback plumbing
static int s_screen_mode = SM_MENU;
static void (*s_ui_cb)() = nullptr;
static bool s_cursor_visible = false;
void SetScreenMode(int sm, bool) {
  s_screen_mode = sm;
  REC(std::string("screenmode:") + std::to_string(sm));
}
int GetScreenMode() { return s_screen_mode; }
void SetUICallback(void (*fn)()) {
  s_ui_cb = fn;
  REC("setuicb");
}
void (*GetUICallback())() { return s_ui_cb; }

// ==== memory CFILE (real FILE* backing for SetupTempDirectory) ====
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
static int s_lib_handle = 0;
int cf_OpenLibrary(const std::filesystem::path &name) {
  g_libs.push_back(name.string());
  return ++s_lib_handle;
}
void cf_AddBaseDirectory(const std::filesystem::path &p) { g_dirs.push_back(p.string()); }
void cf_AddDefaultBaseDirectories() {}
std::filesystem::path cf_GetWritableBaseDirectory() { return "/tmp/opencode/d3writable"; }
std::vector<std::filesystem::path> cf_LocateMultiplePaths(const std::filesystem::path &) { return {}; }

// ==== ddio ====
bool ddio_Init(ddio_init_info *) { REC("ddioinit"); return true; }
void ddio_Close() { REC("ddioclose"); }
void ddio_KeyFlush() {}
void ddio_MouseMode(int mode) { REC(std::string("mousemode:") + std::to_string(mode)); }
void ddio_SetKeyboardLanguage(int lang) { REC(std::string("kblang:") + std::to_string(lang)); }
bool ddio_SetWorkingDir(const char *) { return true; }
bool ddio_CreateLockFile(const std::filesystem::path &) { REC("lockfile"); return true; }
void ddio_DoForeachFile(const std::filesystem::path &, const std::regex &,
                        const std::function<void(std::filesystem::path)> &) {}
std::filesystem::path ddio_GetBasePath() { return "/tmp/opencode"; }
std::filesystem::path ddio_GetPrefPath(const char *, const char *) { return "/tmp/opencode/d3pref"; }
static int s_tmp_counter = 0;
std::filesystem::path ddio_GetTmpFileName(const std::filesystem::path &basedir, const char *prefix) {
  return basedir / (std::string(prefix) + "test" + std::to_string(++s_tmp_counter));
}

// ==== bitmap / renderer / grtext ====
void bm_InitBitmaps() { REC("bm_InitBitmaps"); }
void bm_FreeBitmap(int) {}
int bm_AllocLoadFileBitmap(const char *, int, int) { return 33; }
bool bm_CreateChunkedBitmap(int, chunked_bitmap *) { return true; }
void bm_DestroyChunkedBitmap(chunked_bitmap *) {}
void rend_ClearScreen(ddgr_color) {}
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, uint8_t) {}
void rend_DrawPolygon2D(int, g3Point **, int) {}
bool rend_InitWindowMode() { REC("windowmode"); return true; }
void rend_SetAlphaType(int8_t) {}
void rend_SetAlphaValue(uint8_t) {}
void rend_SetFlatColor(ddgr_color) {}
void rend_SetLighting(light_state) {}
void rend_SetTextureType(texture_type) {}
void rend_SetZBufferState(int8_t) {}
void rend_Flip() {}
void grtext_Init() { REC("grtext_Init"); }
int grfont_GetHeight(int) { return 16; }
void grtext_SetFont(int) {}
void grtext_SetColor(ddgr_color) {}
void grtext_SetAlpha(uint8_t) {}
void grtext_Puts(int, int, const char *) {}
void grtext_Flush() {}

// ==== sound system ====
static float s_master_volume = 1.0f;
static int s_ll_quantity = 0;
int hlsSystem::InitSoundLib(oeApplication *, char mixer_type, char quality, bool) {
  REC(std::string("initsound:") + std::to_string((int)mixer_type) + ":" + std::to_string((int)quality));
  return 1;
}
void hlsSystem::SetLLSoundQuantity(int n) { s_ll_quantity = n; }
int hlsSystem::GetLLSoundQuantity() { return s_ll_quantity; }
void hlsSystem::PauseSounds(bool) { REC("pausesounds"); }
void hlsSystem::ResumeSounds() { REC("resumesounds"); }
void hlsSystem::SetMasterVolume(float v) { s_master_volume = v; }
float hlsSystem::GetMasterVolume() { return s_master_volume; }
hlsSystem::hlsSystem() {}
void hlsSystem::KillSoundLib(bool) {}
hlsSystem Sound_system;

// ==== music ====
static float s_music_volume = 0.5f;
void InitD3Music(bool allow) { REC(std::string("music:") + (allow ? "on" : "off")); }
void D3MusicPause() { REC("musicpause"); }
void D3MusicResume() { REC("musicresume"); }
float D3MusicGetVolume() { return s_music_volume; }
void D3MusicSetVolume(float v) { s_music_volume = v; }

// ==== force feedback ====
bool D3Use_force_feedback = true;
bool D3Force_auto_center = true;
float D3Force_gain = 1.0f;
uint8_t Default_player_terrain_leveling = 2;
uint8_t Default_player_room_leveling = 2;
void ForceInit() { REC("forceinit"); }
void ForceRestart() { REC("forcerestart"); }
void ForceShutdown() { REC("forceshutdown"); }

// ==== controls / pilot ====
bool Control_poll_flag = false;
void InitControls() { REC("initcontrols"); }
void CloseControls() { REC("closecontrols"); }
void ResumeControls() { REC("resumecontrols"); }
void LoadControlConfig(pilot *) { REC("loadctlcfg"); }
void SaveControlConfig(pilot *) { REC("savectlcfg"); }
pilot::pilot() {}
pilot::~pilot() {}
void ConfigSetDetailLevel(int level) { REC(std::string("detaillevel:") + std::to_string(level)); }

// ==== subsystem init recorders ====
void InitObjectInfo() { REC("objinfo"); }
void LoadAllFonts() { REC("fonts"); }
renderer_type PreferredRenderer = RENDERER_OPENGL;
void g3_SetAspectRatio(float a) { REC(std::string("aspect:") + std::to_string(a)); }
void InitVClips() { REC("vclips"); }
void InitRooms() { REC("rooms"); }
void InitLightmapInfo(int) { REC("lminfo"); }
void InitSpecialFaces() { REC("specfaces"); }
void InitDynamicLighting() { REC("dynlight"); }
void InitMission() { REC("mission"); }
void InitDefaultMissionFromCLI() { REC("climission"); }
void InitShips() { REC("ships"); }
void InitFVI() { REC("fvi"); }
void InitMatcens() { REC("matcens"); }
void InitMathTables() { REC("math"); }
void InitSounds() { REC("sounds"); }
void InitTerrain() { REC("terrain"); }
void InitModels() { REC("models"); }
void InitDoors() { REC("doors"); }
void InitGamefiles() { REC("gamefiles"); }
void InitFrameTime() { REC("frametime"); }
void InitObjects() { REC("objects"); }
void InitFireballs() { REC("fireballs"); }
void InitTriggers() { REC("triggers"); }
bool InitVoices() { REC("voices"); return true; }
void InitPlayers() { REC("players"); }
void InitMarkers() { REC("markers"); }
int InitTextures() { REC("textures"); return 1; }
void TelComInit() { REC("telcom"); }
void Cinematic_Init() { REC("cinematic2"); }
bool InitCinematics() { REC("cinematic1"); return true; }
void NewUIInit() { REC("newui"); }
void InitAmbientSoundSystem() { REC("ambient"); }
void PilotInit() { REC("pilotinit"); }
void Localize_ctl_bindings() { REC("localizectl"); }
int LoadServerConfigFile() { REC("loadservercfg"); return 1; }
bool PPic_InitDatabase() { REC("ppic"); return true; }
int gspy_Init() { REC("gamespy"); return 1; }

// ==== network / osiris / manage ====
void nw_InitNetworking(int) { REC("nw_net"); }
void nw_InitSockets(uint16_t port) { REC(std::string("nw_sockets:") + std::to_string(port)); }
void Osiris_InitModuleLoader() { REC("osiris_modloader"); }
int Osiris_ExtractScriptsFromHog(int hid, bool) {
  REC(std::string("osiris_extract:") + std::to_string(hid));
  return 1;
}
int mng_InitTableFiles() { REC("mng_init"); return 1; }
int mng_LoadTableFiles(int) { REC("mng_load"); return 42; }

// ==== localization / strings ====
int Localization_language_recorded = -1;
void Localization_SetLanguage(int lang) { Localization_language_recorded = lang; }
int LoadStringTables() { REC("strings"); return 555; }

const char *GetStringFromTable(int id) {
  switch (id) {
  case 123:
    return "Loading data...";
  case 124:
    return "tableerr";
  case 125:
    return "Collating...";
  case 558:
    return "Shutting down!";
  default:
    return "str";
  }
}

// ==== misc ====
// error_Init / DebugBreak callbacks come from libddebug.

// ==== misc ====
void mem_Init() { REC("meminit"); }
void DebugGraph_Initialize() { REC("debuggraph"); }
void rtp_Init() {}
void PrintDedicatedMessage(const char *fmt, ...) {
  char buf[256];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  std::string s(buf);
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
    s.pop_back();
  REC(std::string("dedmsg:") + s);
}
void D3DeferHandler(bool active) { REC(std::string("d3defer:") + (active ? "on" : "off")); }
void ui_Init(oeApplication *, tUIInitInfo *info) {
  REC(std::string("uiinit:") + std::to_string(info->window_font) + ":" + std::to_string(info->w) + ":" +
      std::to_string(info->h));
}
void ui_Close() { REC("uiclose"); }
void ui_UseCursor(const char *) { REC("cursor"); }
bool ui_ShowCursor() { s_cursor_visible = true; return true; }
bool ui_IsCursorVisible() { return s_cursor_visible; }
void PauseGame() { REC("pausegame"); }
void ResumeGame() { REC("resumegame"); }
void StartFrame(bool) {}
void EndFrame() {}

namespace D3 {
float ChronoTimer::GetTime() { return 0.0f; }
} // namespace D3

// ==== TESTS ====

/**
 * @brief GTest fixture for InitTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class InitTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_rec.clear();
    g_dirs.clear();
    g_libs.clear();
    s_lib_handle = 0;
    ResetArgs();
    Descent = new TestApp();
    s_db.store.clear();
    s_fmode = INIT_MODE;
    Game_mode = 0;
    Game_paused = false;
    Control_poll_flag = false;
    Dedicated_server = false;
    Use_file_xfer = 1;
    Mem_low_memory_mode = Mem_superlow_memory_mode = false;
    Min_allowed_frametime = 0;
    Mouselook_sensitivity = 0;
    Mouse_sensitivity = 0;
    Video_res_list.clear();
    for (int i = 0; i < 3; i++)
      Video_res_list.push_back({(uint16_t)(640 + i * 320), (uint16_t)(480 + i * 240)});
    Default_resolution_id = 0;
  }
  void TearDown() override { delete Descent; }
};

// -superlowmem sets both flags; -himem wins last; -nomultibmp kills file xfer
/**
 * @test InitTest.PreInitMemoryArgPrecedence
 * @brief Verifies pre Init Memory Arg Precedence.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, PreInitMemoryArgPrecedence) {
  PushArg("-superlowmem");
  PushArg("-nomultibmp");
  PreInitD3Systems();
  EXPECT_TRUE(Mem_low_memory_mode);
  EXPECT_TRUE(Mem_superlow_memory_mode);
  EXPECT_EQ(Use_file_xfer, 0);

  Mem_low_memory_mode = Mem_superlow_memory_mode = false;
  ResetArgs();
  PushArg("-superlowmem");
  PushArg("-himem");
  PreInitD3Systems();
  EXPECT_FALSE(Mem_low_memory_mode);
  EXPECT_FALSE(Mem_superlow_memory_mode);
}

// -framecap always wins: even a preceding -limitframe is overridden by the default cap of 60
/**
 * @test InitTest.FrameCapAlwaysWinsOverLimitFrame
 * @brief Verifies frame Cap Always Wins Over Limit Frame.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, FrameCapAlwaysWinsOverLimitFrame) {
  PushArg("-limitframe");
  PushArg("5");
  PreInitD3Systems();
  EXPECT_EQ(Min_allowed_frametime, (int)((1.0 / 60.0) * 1000)); // quirk: 5 ignored

  ResetArgs();
  PushArg("-framecap");
  PushArg("120");
  PreInitD3Systems();
  EXPECT_EQ(Min_allowed_frametime, (int)((1.0 / 120.0) * 1000)); // 8
}

// sensitivity args scale through the fixed-point angle conversion
/**
 * @test InitTest.SensitivityArgsAreScaledAndParsed
 * @brief Verifies sensitivity Args Are Scaled And Parsed.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, SensitivityArgsAreScaledAndParsed) {
  const float kAPD = 65536.0f / 360.0f;
  PushArg("-mlooksens");
  PushArg("45");
  PushArg("-mousesens");
  PushArg("2.5");
  PreInitD3Systems();
  EXPECT_FLOAT_EQ(Mouselook_sensitivity, kAPD * 45.0f);
  EXPECT_FLOAT_EQ(Mouse_sensitivity, 2.5f);
}

static void SeedSettings() {
  Render_preferred_state.gamma = 2.25f;
  s_master_volume = 0.75f;
  D3MusicSetVolume(0.4f);
  Detail_settings.Pixel_error = 3.0f;
  Detail_settings.Terrain_render_distance = 55.0f * TERRAIN_SIZE;
  Detail_settings.Dynamic_lighting = false;
  Default_player_terrain_leveling = 1;
  Default_player_room_leveling = 0;
  Detail_settings.Specular_lighting = true;
  Detail_settings.Fast_headlight_on = true;
  Detail_settings.Mirrored_surfaces = false;
  Missile_camera_window = SVW_LEFT;
  Render_preferred_state.vsync_on = false;
  Detail_settings.Scorches_enabled = false;
  Detail_settings.Weapon_coronas_enabled = true;
  Detail_settings.Fog_enabled = false;
  Detail_settings.Coronas_enabled = false;
  Detail_settings.Procedurals_enabled = false;
  Detail_settings.Object_complexity = 2;
  Detail_settings.Powerup_halos = false;
  Current_video_resolution_id = 2;
  Render_FOV_setting = 85.0f;
  Game_fullscreen = false;
  Render_preferred_bitdepth = 32;
  Render_preferred_state.filtering = false;
  Render_preferred_state.mipping = false;
  Render_state.cur_color_model = CM_RGB;
  PlayPowerupVoice = false;
  PlayVoices = false;
  D3Use_force_feedback = false;
  D3Force_auto_center = false;
  D3Force_gain = 0.35f;
  Sound_quality = 1;
  s_ll_quantity = 27;
  Default_pilot = "emmy";
  config_base_directory = "/tmp/opencode/cfgdir";
}

// full save -> load round trip of every setting
/**
 * @test InitTest.SaveThenLoadRoundTripsSettings
 * @brief Verifies save Then Load Round Trips Settings.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, SaveThenLoadRoundTripsSettings) {
  SeedSettings();
  SaveGameSettings();

  // scramble everything
  Render_preferred_state.gamma = 0.0f;
  s_master_volume = 1.0f;
  D3MusicSetVolume(1.0f);
  Detail_settings.Pixel_error = 99;
  Detail_settings.Terrain_render_distance = 1;
  Detail_settings.Dynamic_lighting = true;
  Default_player_terrain_leveling = Default_player_room_leveling = 3;
  Detail_settings.Specular_lighting = false;
  Detail_settings.Fast_headlight_on = false;
  Detail_settings.Mirrored_surfaces = true;
  Missile_camera_window = SVW_CENTER;
  Render_preferred_state.vsync_on = true;
  Detail_settings.Scorches_enabled = true;
  Detail_settings.Weapon_coronas_enabled = false;
  Detail_settings.Fog_enabled = true;
  Detail_settings.Coronas_enabled = true;
  Detail_settings.Procedurals_enabled = true;
  Detail_settings.Object_complexity = 0;
  Detail_settings.Powerup_halos = true;
  Current_video_resolution_id = 0;
  Render_FOV_setting = D3_DEFAULT_FOV;
  Game_fullscreen = true;
  Render_preferred_bitdepth = 16;
  Render_preferred_state.filtering = true;
  Render_preferred_state.mipping = true;
  PlayPowerupVoice = PlayVoices = true;
  D3Use_force_feedback = D3Force_auto_center = true;
  D3Force_gain = 0.0f;
  Sound_quality = 0;
  s_ll_quantity = 5;
  Default_pilot.clear();
  config_base_directory.clear();

  LoadGameSettings();

  EXPECT_FLOAT_EQ(Render_preferred_state.gamma, 2.25f);
  EXPECT_FLOAT_EQ(Sound_system.GetMasterVolume(), 0.75f);
  EXPECT_FLOAT_EQ(D3MusicGetVolume(), 0.4f);
  EXPECT_FLOAT_EQ(Detail_settings.Pixel_error, 3.0f);
  EXPECT_NEAR(Detail_settings.Terrain_render_distance, 55.0f * TERRAIN_SIZE, 0.01f);
  EXPECT_FALSE(Detail_settings.Dynamic_lighting);
  EXPECT_EQ(Default_player_terrain_leveling, 1);
  EXPECT_EQ(Default_player_room_leveling, 0);
  EXPECT_TRUE(Detail_settings.Specular_lighting);
  EXPECT_TRUE(Detail_settings.Fast_headlight_on);
  EXPECT_FALSE(Detail_settings.Mirrored_surfaces);
  EXPECT_EQ(Missile_camera_window, SVW_LEFT);
  EXPECT_FALSE(Render_preferred_state.vsync_on != 0); // stored/read as int
  EXPECT_TRUE(Render_preferred_state.vsync_on == 0);
  EXPECT_FALSE(Detail_settings.Scorches_enabled);
  EXPECT_TRUE(Detail_settings.Weapon_coronas_enabled);
  EXPECT_FALSE(Detail_settings.Fog_enabled);
  EXPECT_FALSE(Detail_settings.Coronas_enabled);
  EXPECT_FALSE(Detail_settings.Procedurals_enabled);
  EXPECT_EQ(Detail_settings.Object_complexity, 2);
  EXPECT_FALSE(Detail_settings.Powerup_halos);
  EXPECT_EQ(Current_video_resolution_id, 2);
  EXPECT_FLOAT_EQ(Render_FOV_setting, 85.0f);
  EXPECT_FLOAT_EQ(Render_FOV, 85.0f);
  EXPECT_FALSE(Game_fullscreen);
  EXPECT_EQ(Render_preferred_bitdepth, 32);
  EXPECT_FALSE(Render_preferred_state.filtering != 0);
  EXPECT_FALSE(Render_preferred_state.mipping != 0);
  EXPECT_EQ(Render_state.cur_color_model, CM_RGB);
  EXPECT_FALSE(PlayPowerupVoice);
  EXPECT_FALSE(PlayVoices);
  EXPECT_FALSE(D3Use_force_feedback);
  EXPECT_FALSE(D3Force_auto_center);
  EXPECT_FLOAT_EQ(D3Force_gain, 0.35f);
  EXPECT_EQ(Sound_quality, 1);
  EXPECT_EQ(Sound_system.GetLLSoundQuantity(), 27);
  EXPECT_EQ(Default_pilot, "emmy");
  EXPECT_EQ(config_base_directory.string(), "/tmp/opencode/cfgdir");
}

// out-of-range resolution index keeps the default instead of indexing OOB
/**
 * @test InitTest.LoadClampsResolutionIndex
 * @brief Verifies load Clamps Resolution Index.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, LoadClampsResolutionIndex) {
  s_db.store["RS_resolution"] = {99, 0, 0, 0};
  Current_video_resolution_id = 1;
  LoadGameSettings();
  EXPECT_EQ(Current_video_resolution_id, Default_resolution_id);

  s_db.store["RS_resolution"] = {255, 255, 255, 255}; // -1
  LoadGameSettings();
  EXPECT_EQ(Current_video_resolution_id, Default_resolution_id);
}

// FOV is clamped to [D3_DEFAULT_FOV, 90]
/**
 * @test InitTest.LoadClampsFieldOfView
 * @brief Verifies load Clamps Field Of View.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, LoadClampsFieldOfView) {
  s_db.store["RS_fov"] = {120, 0, 0, 0};
  LoadGameSettings();
  EXPECT_FLOAT_EQ(Render_FOV_setting, 90.0f);

  s_db.store["RS_fov"] = {10, 0, 0, 0};
  LoadGameSettings();
  EXPECT_FLOAT_EQ(Render_FOV_setting, D3_DEFAULT_FOV);
}

// force feedback gain clamps in place on save, then again via the stored byte
/**
 * @test InitTest.ForceGainClampsOnSaveAndLoad
 * @brief Verifies force Gain Clamps On Save And Load.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, ForceGainClampsOnSaveAndLoad) {
  D3Force_gain = 1.7f;
  SaveGameSettings();
  EXPECT_FLOAT_EQ(D3Force_gain, 1.0f); // save clamps the live value too
  LoadGameSettings();
  EXPECT_FLOAT_EQ(D3Force_gain, 1.0f); // byte 100 -> 1.0

  D3Force_gain = -0.4f;
  SaveGameSettings();
  EXPECT_FLOAT_EQ(D3Force_gain, 0.0f);
  LoadGameSettings();
  EXPECT_FLOAT_EQ(D3Force_gain, 0.0f);
}

/**
 * @test InitTest.ObjectComplexityClampOnLoad
 * @brief Verifies object Complexity Clamp On Load.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, ObjectComplexityClampOnLoad) {
  s_db.store["DetailObjectComp"] = {9, 0, 0, 0};
  LoadGameSettings();
  EXPECT_EQ(Detail_settings.Object_complexity, 1); // out of [0,2] resets to medium

  s_db.store["DetailObjectComp"] = {2, 0, 0, 0};
  LoadGameSettings();
  EXPECT_EQ(Detail_settings.Object_complexity, 2);
}

// keyboard language is picked from the database string
/**
 * @test InitTest.KeyboardLanguageFromDatabase
 * @brief Verifies keyboard Language From Database.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, KeyboardLanguageFromDatabase) {
  std::string fr = "French";
  s_db.store["KeyboardType"] = std::vector<uint8_t>(fr.begin(), fr.end());
  g_rec.clear();
  LoadGameSettings();
  bool saw_french = false;
  for (auto &r : g_rec)
    if (r == "kblang:2") // KBLANG_FRENCH
      saw_french = true;
  EXPECT_TRUE(saw_french);

  ResetArgs();
  g_rec.clear();
  LoadGameSettings(); // no KeyboardType entry
  bool saw_american_last = false;
  for (auto &r : g_rec)
    if (r == "kblang:0") // KBLANG_AMERICAN default
      saw_american_last = true;
  EXPECT_TRUE(saw_american_last);
}

// an empty default pilot is saved as a single space placeholder
/**
 * @test InitTest.EmptyPilotSavesPlaceholder
 * @brief Verifies empty Pilot Saves Placeholder.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, EmptyPilotSavesPlaceholder) {
  Default_pilot.clear();
  SaveGameSettings();
  auto it = s_db.store.find("Default_pilot");
  ASSERT_NE(it, s_db.store.end());
  ASSERT_EQ(it->second.size(), 2u);
  EXPECT_EQ(it->second[0], ' ');
  EXPECT_EQ(it->second[1], '\0');
}

// the full boot sequence runs end to end and hits every milestone in order
/**
 * @test InitTest.FullBootSequenceRunsInOrder
 * @brief Verifies full Boot Sequence Runs In Order.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, FullBootSequenceRunsInOrder) {
  InitD3Systems1(false);
  InitD3Systems2(false);

  // settings load (via detail preset) happens inside IO init, before ddio
  EXPECT_LT(Pos("detaillevel:1"), Pos("ddioinit"));
  // table files come up before string tables and graphics
  EXPECT_LT(Pos("mng_init"), Pos("strings"));
  EXPECT_LT(Pos("bm_InitBitmaps"), Pos("textures"));
  EXPECT_LT(Pos("strings"), Pos("fonts"));
  // subsystem order within systems1
  EXPECT_LT(Pos("objinfo"), Pos("sounds"));
  EXPECT_LT(Pos("nw_net"), Pos("gamespy"));
  EXPECT_EQ(std::count(g_rec.begin(), g_rec.end(), "nw_sockets:2092"), 1); // D3_DEFAULT_PORT
  // sound lib got the configured mixer/quality
  bool saw_sound = false;
  for (auto &r : g_rec)
    if (r == "initsound:0:0")
      saw_sound = true;
  EXPECT_TRUE(saw_sound);
  // systems2: tables before pilot before objects
  EXPECT_LT(Pos("mng_load"), Pos("pilotinit"));
  EXPECT_LT(Pos("pilotinit"), Pos("objects"));
  EXPECT_LT(Pos("cinematic2"), Pos("mng_load"));
  // ui init captured the game font and default res
  EXPECT_EQ(std::count(g_rec.begin(), g_rec.end(), "uiinit:7:640:480"), 1);
  // ends in menu mode
  EXPECT_EQ(GetFunctionMode(), MENU_MODE);

  // hog library handles: PRIMARY_HOG opened last, osiris extracts from it
  ASSERT_FALSE(g_libs.empty());
  EXPECT_EQ(g_libs.back(), "d3-linux.hog");
  EXPECT_TRUE(MercInstalled()); // merc.hog "opened" by the stub
  std::string want_extract = "osiris_extract:" + std::to_string((int)g_libs.size());
  bool saw_extract = false;
  for (auto &r : g_rec)
    if (r == want_extract)
      saw_extract = true;
  EXPECT_TRUE(saw_extract);

  // temp directory was really created
  EXPECT_FALSE(Descent3_temp_directory.empty());
  EXPECT_TRUE(std::filesystem::exists(Descent3_temp_directory));
}

// -nonetwork skips net init; +host/-pxoport override ports
/**
 * @test InitTest.BootHonorsNetworkArgsAndPorts
 * @brief Verifies boot Honors Network Args And Ports.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, BootHonorsNetworkArgsAndPorts) {
  PushArg("+host");
  PushArg("3002");
  PushArg("-pxoport");
  PushArg("77");
  PushArg("-nonetwork");
  InitD3Systems1(false);
  EXPECT_EQ(Gameport, 3002);
  EXPECT_EQ(PXOPort, 77);
  EXPECT_TRUE(g_rec.empty() || std::find(g_rec.begin(), g_rec.end(), "nw_net") == g_rec.end());
  EXPECT_TRUE(std::find_if(g_rec.begin(), g_rec.end(), [](const std::string &s) {
                return s.rfind("nw_sockets:", 0) == 0;
              }) == g_rec.end());
}

// shutdown snapshots state; restart restores it (single test: shared statics)
/**
 * @test InitTest.ShutdownThenRestartRestoresState
 * @brief Verifies shutdown Then Restart Restores State.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, ShutdownThenRestartRestoresState) {
  InitD3Systems1(false);
  InitD3Systems2(false); // sets Init_systems_init internally

  SetFunctionMode(GAME_MODE);
  Game_mode = 0; // not multi -> pause path
  Game_paused = false;
  Control_poll_flag = true;
  s_cursor_visible = true;
  s_screen_mode = SM_MENU;
  size_t rec_base = g_rec.size();
  g_rec.clear();

  ShutdownD3();
  EXPECT_EQ(std::count(g_rec.begin(), g_rec.end(), "forceshutdown"), 1);
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "pausegame"), g_rec.end());
  EXPECT_EQ(std::count(g_rec.begin(), g_rec.end(), "musicpause"), 0); // unpaused game path
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "savectlcfg"), g_rec.end());
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "closecontrols"), g_rec.end());
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "screenmode:0"), g_rec.end()); // SM_NULL
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "ddioclose"), g_rec.end());

  g_rec.clear();
  RestartD3();
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "mousemode:2"), g_rec.end()); // EXCLUSIVE, not dedicated
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "ddioinit"), g_rec.end());
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "screenmode:2"), g_rec.end()); // restored SM_MENU
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "initcontrols"), g_rec.end());
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "loadctlcfg"), g_rec.end());
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "resumegame"), g_rec.end());
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "resumecontrols"), g_rec.end()); // poll flag was on
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "forcerestart"), g_rec.end());
}

// dedicated servers route splash text to the console; otherwise silent until gfx is up
/**
 * @test InitTest.InitMessageRoutesForDedicatedServer
 * @brief Verifies init Message Routes For Dedicated Server.
 *
 * @details
 * Exercises the InitTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/init.cpp
 * @ingroup descent3_tests
 */
TEST_F(InitTest, InitMessageRoutesForDedicatedServer) {
  Dedicated_server = true;
  InitMessage("booting", 0.5f);
  EXPECT_NE(std::find(g_rec.begin(), g_rec.end(), "dedmsg:booting"), g_rec.end());

  g_rec.clear();
  Dedicated_server = false;
  InitMessage("silent", -1.0f); // graphics already up from boot tests -> renders, no console
  bool saw_ded = false;
  for (auto &r : g_rec)
    if (r.rfind("dedmsg:", 0) == 0)
      saw_ded = true;
  EXPECT_FALSE(saw_ded);
}
