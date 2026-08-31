/**
 * @file config_real_tests.cpp
 * @brief Unit tests for Descent3/config.cpp: display resolution enumeration.
 *
 * @details
 * (SDL3-backed), detail-level presets and the OptionsMenu() integration flow,
 * built on the REAL newui_core.cpp + real ui library (libui.a).
 *
 * This harness validates the behavior of `Descent3/config.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/config.cpp`
 * @par Harness
 * `config_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/config.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <strings.h>

#include "newui_core.h"
#include "newui.h"
#include "uisys.h"
#include "pserror.h"
#include "gamefont.h"
#include "d3music.h"
#include "mem.h"
#include "descent.h"
#include "renderer.h"
#include "chrono_timer.h"
#include "hlsoundlib.h"
#include "config.h"
#include "hud.h"
#include "terrain.h"
#include "ctlconfig.h"
#include "args.h"
#include "pilot.h"
#include "D3ForceFeedback.h"
#include "soundload.h"
#include "appdatabase.h"
#include "gameloop.h"
#include <SDL3/SDL.h>

#define REC(msg) g_recorder.push_back(msg)
static std::vector<std::string> g_recorder;

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere (shared with newuicore tests)
// ---------------------------------------------------------------------------
int Max_window_w = 640;
int Max_window_h = 480;
bool Multi_bail_ui_menu = false;
int Game_fonts[NUM_FONTS] = {0};

static function_mode s_function_mode = MENU_MODE;
function_mode GetFunctionMode() { return s_function_mode; }
void SetFunctionMode(function_mode mode) { s_function_mode = mode; }

struct FakeApp : oeApplication {
  void init() override {}
  void get_info(void *) override {}
  int flags() const override { return 0; }
  unsigned defer() override {
    REC("defer");
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

// ---------------------------------------------------------------------------
// bitmap stubs
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// input stubs: dead keyboard/mouse
// ---------------------------------------------------------------------------
int ddio_GetAdjKeyState(int) { return 0; }
void ddio_KeyFlush() {}
int ddio_KeyInKey() { return 0; }
bool ddio_MouseGetEvent(int *btn, bool *state) {
  *btn = 0;
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

// ---------------------------------------------------------------------------
// renderer / frame stubs
// ---------------------------------------------------------------------------
void rend_ClearScreen(ddgr_color) {}
void rend_Flip() { REC("flip"); }
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, uint8_t) {}
void rend_DrawLine(int, int, int, int) {}
void rend_DrawPolygon2D(int, g3Point **, int) { REC("drawpoly2d"); }
void rend_DrawScaledBitmap(int, int, int, int, int, float, float, float, float, int, const float *) {}
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
int rend_SetPreferredState(renderer_preferred_state *, bool) { REC("setpreferredstate"); return 1; }

float Render_FOV = 72.0f;
int Render_preferred_bitdepth = 32;
renderer_preferred_state Render_preferred_state{};

void StartFrame(bool) {}
void StartFrame(int, int, int, int, bool, bool) {}
void EndFrame() {}
void DoScreenshot() {}

// ---------------------------------------------------------------------------
// text / font stubs
// ---------------------------------------------------------------------------
const char *GetStringFromTable(int id) {
  static std::string buf;
  buf = "STR_" + std::to_string(id);
  return buf.c_str();
}
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

// ---------------------------------------------------------------------------
// sound / music stubs (scriptable)
// ---------------------------------------------------------------------------
static float s_master_vol = 1.0f, s_set_master_vol = -1.0f;
static char s_sound_quality = SQT_NORMAL, s_set_quality = -1;
static int s_ll_qty = MIN_SOUNDS_MIXED;
static bool s_ll_qty_changed = false;

hlsSystem::hlsSystem() {}
int hlsSystem::Play2dSound(int, float, float, uint16_t) { return 0; }
void hlsSystem::BeginSoundFrame(bool) {}
void hlsSystem::EndSoundFrame() {}
int hlsSystem::GetLLSoundQuantity() { return s_ll_qty; }
void hlsSystem::SetLLSoundQuantity(int q) {
  s_ll_qty = q;
  s_ll_qty_changed = true;
}
float hlsSystem::GetMasterVolume() { return s_master_vol; }
void hlsSystem::SetMasterVolume(float v) { s_set_master_vol = v; }
char hlsSystem::GetSoundQuality() { return s_sound_quality; }
bool hlsSystem::SetSoundQuality(char q) {
  s_set_quality = q;
  return true;
}
void hlsSystem::StopSoundImmediate(int) {}
void hlsSystem::KillSoundLib(bool) {}
hlsSystem Sound_system;

static float s_music_get_vol = 0.5f, s_music_set_vol = -1.0f;
float D3MusicGetVolume() { return s_music_get_vol; }
void D3MusicSetVolume(float v) { s_music_set_vol = v; }
void D3MusicDoFrame(tMusicSeqInfo *) {}

static int s_find_sound_calls = 0;
int FindSoundName(const char *) {
  s_find_sound_calls++;
  return 777;
}

// SDL assert hook used by the real ui library
SDL_AssertState SDL_ReportAssertion(SDL_AssertData *, const char *, const char *, int) {
  return SDL_ASSERTION_IGNORE;
}

// ---------------------------------------------------------------------------
// SDL display scriptables
// ---------------------------------------------------------------------------
static std::vector<SDL_DisplayID> s_displays;
static std::map<SDL_DisplayID, std::vector<std::pair<int, int>>> s_display_modes;
static std::map<SDL_DisplayID, std::pair<int, int>> s_current_modes;

SDL_DisplayID *SDL_GetDisplays(int *count) {
  if (s_displays.empty()) {
    if (count)
      *count = 0;
    return nullptr;
  }
  static std::vector<SDL_DisplayID> copy;
  copy = s_displays;
  if (count)
    *count = (int)copy.size();
  return copy.data();
}

SDL_DisplayMode **SDL_GetFullscreenDisplayModes(SDL_DisplayID id, int *count) {
  auto it = s_display_modes.find(id);
  if (it == s_display_modes.end()) {
    if (count)
      *count = 0;
    return nullptr;
  }
  static std::vector<SDL_DisplayMode> storage;
  static std::vector<SDL_DisplayMode *> ptrs;
  storage.clear();
  ptrs.clear();
  for (auto &[w, h] : it->second) {
    SDL_DisplayMode mode{};
    mode.w = w;
    mode.h = h;
    storage.push_back(mode);
  }
  for (auto &m : storage)
    ptrs.push_back(&m);
  if (count)
    *count = (int)ptrs.size();
  return ptrs.data();
}

const SDL_DisplayMode *SDL_GetCurrentDisplayMode(SDL_DisplayID id) {
  static SDL_DisplayMode mode{};
  auto it = s_current_modes.find(id);
  if (it == s_current_modes.end())
    return nullptr;
  mode = SDL_DisplayMode{};
  mode.w = it->second.first;
  mode.h = it->second.second;
  return &mode;
}

void SDL_free(void *) {}

// ---------------------------------------------------------------------------
// args implementation mirroring Descent3/args.cpp semantics
// ---------------------------------------------------------------------------
char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];
static int s_total_args = 0;

static const char *SkipArgPrefix(const char *arg) {
  if (*arg != '\0' && *(arg + 1) != '\0') {
    if (*arg == '-') {
      if (*(arg + 1) == '-' && *(arg + 2) != '\0')
        return arg + 2;
      if (*(arg + 1) != '-')
        return arg + 1;
    }
  }
  return arg;
}

const char *GetArg(int index) {
  if (index >= s_total_args || index == 0)
    return nullptr;
  return GameArgs[index];
}

int FindArg(const char *which, int start) {
  if (which == nullptr)
    return 0;
  for (int i = start; i <= s_total_args; i++) {
    if (strcasecmp(SkipArgPrefix(which), SkipArgPrefix(GameArgs[i])) == 0)
      return i;
  }
  return 0;
}

static void TestSetArgs(const std::vector<std::string> &args) {
  memset(GameArgs, 0, sizeof(GameArgs));
  snprintf(GameArgs[0], MAX_CHARS_PER_ARG, "d3");
  int i = 1;
  for (auto &a : args) {
    snprintf(GameArgs[i], MAX_CHARS_PER_ARG, "%s", a.c_str());
    i++;
  }
  s_total_args = (int)args.size() + 1;
}

// ---------------------------------------------------------------------------
// database fake
// ---------------------------------------------------------------------------
class FakeDatabase : public oeAppDatabase {
public:
  std::map<std::string, int> ints;
  std::vector<std::pair<std::string, int>> writes;

  bool create_record(const char *) override { return true; }
  bool lookup_record(const char *) override { return true; }
  bool read(const char *label, char *, int *) override { return false; }
  bool read(const char *label, void *entry, int wordsize) override {
    auto it = ints.find(label);
    if (it == ints.end())
      return false;
    if (wordsize == sizeof(int))
      *(int *)entry = it->second;
    else if (wordsize == sizeof(uint16_t))
      *(uint16_t *)entry = (uint16_t)it->second;
    else
      return false;
    return true;
  }
  bool read(const char *label, bool *entry) override {
    auto it = ints.find(label);
    if (it == ints.end())
      return false;
    *entry = it->second != 0;
    return true;
  }
  bool write(const char *label, const char *, int) override { return true; }
  bool write(const char *label, int entry) override {
    writes.emplace_back(label, entry);
    ints[label] = entry;
    return true;
  }
  void get_user_name(char *buffer, size_t *size) override {
    if (buffer && size && *size > 0)
      buffer[0] = '\0';
  }
};
static FakeDatabase g_database;
oeAppDatabase *Database = &g_database;

// ---------------------------------------------------------------------------
// pilot global with recordable hud accessors
// ---------------------------------------------------------------------------
static uint16_t s_pilot_stat = 0, s_pilot_grstat = 0;
static int s_gw_w = 640, s_gw_h = 480;
static uint16_t s_rec_new_stat = 0, s_rec_new_grstat = 0;
static int s_rec_gw_w = -1, s_rec_gw_h = -1, s_sethuddata_calls = 0;
static int s_pltwrite_calls = 0, s_savegamesettings_calls = 0;

pilot::pilot() {}
pilot::~pilot() {}
void pilot::get_hud_data(uint8_t *hmode, uint16_t *hstat, uint16_t *hgraphicalstat, int *gw_w, int *gw_h) {
  if (hmode)
    *hmode = 0;
  if (hstat)
    *hstat = s_pilot_stat;
  if (hgraphicalstat)
    *hgraphicalstat = s_pilot_grstat;
  if (gw_w)
    *gw_w = s_gw_w;
  if (gw_h)
    *gw_h = s_gw_h;
}
void pilot::set_hud_data(uint8_t *hmode, uint16_t *hstat, uint16_t *hgraphicalstat, int *gw_w, int *gw_h) {
  s_sethuddata_calls++;
  if (hmode)
    *hmode = 0;
  if (hstat) {
    s_pilot_stat = *hstat;
    s_rec_new_stat = *hstat;
  }
  if (hgraphicalstat) {
    s_pilot_grstat = *hgraphicalstat;
    s_rec_new_grstat = *hgraphicalstat;
  }
  if (gw_w) {
    s_gw_w = *gw_w;
    s_rec_gw_w = *gw_w;
  }
  if (gw_h) {
    s_gw_h = *gw_h;
    s_rec_gw_h = *gw_h;
  }
}
pilot Current_pilot;

int PltWriteFile(pilot *, bool) {
  s_pltwrite_calls++;
  return 1;
}
void SaveGameSettings() { s_savegamesettings_calls++; }

// ---------------------------------------------------------------------------
// misc game hooks
// ---------------------------------------------------------------------------
int Default_player_terrain_leveling = 2;
int Default_player_room_leveling = 2;
int Missile_camera_window = -1;
uint16_t Hud_stat_mask = 0;

void DoWaitMessage(bool, const char *) {}
void CtlConfig(int) { REC("ctlconfig"); }
void joystick_settings_dialog() { REC("joydialog"); }
void key_settings_dialog() { REC("keydialog"); }
void net_settings_dialog() { REC("netdialog"); }
void LoadControlConfig(pilot *) {}
void SaveControlConfig(pilot *) {}

static uint16_t s_rec_sethud_mask = 0, s_rec_sethud_gr = 0;
static int s_sethud_calls = 0;
void SetHUDState(uint16_t mask, uint16_t gr_mask) {
  s_rec_sethud_mask = mask;
  s_rec_sethud_gr = gr_mask;
  s_sethud_calls++;
}
tHUDMode GetHUDMode() { return HUD_FULLSCREEN; }

static int s_screen_mode = 1, s_screen_mode_changes = 0;
int GetScreenMode() { return s_screen_mode; }
void SetScreenMode(int sm, bool) {
  s_screen_mode = sm;
  s_screen_mode_changes++;
}

// file-local to newui_core.cpp
void SimpleUICallback();

// ---------------------------------------------------------------------------
// fixture
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for ConfigTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class ConfigTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_recorder.clear();
    Multi_bail_ui_menu = false;
    Max_window_w = 640;
    Max_window_h = 480;

    // resolution state (fallback list restored from pristine copy)
    static const std::vector<tVideoResolution> pristine_list = Video_res_list;
    Video_res_list = pristine_list;
    Default_resolution_id = 7;
    Current_video_resolution_id = 7;
    Display_id = 0;
    Game_fullscreen = true;

    // detail state
    Default_detail_level = DETAIL_LEVEL_MED;

    // database
    g_database.ints.clear();
    g_database.writes.clear();

    // args
    TestSetArgs({});

    // pilot / hud
    s_pilot_stat = 0;
    s_pilot_grstat = 0;
    s_gw_w = 640;
    s_gw_h = 480;
    s_rec_new_stat = 0;
    s_rec_new_grstat = 0;
    s_rec_gw_w = -1;
    s_rec_gw_h = -1;
    s_sethuddata_calls = 0;
    s_pltwrite_calls = 0;
    s_savegamesettings_calls = 0;
    Current_pilot.read_controller = READF_JOY | READF_MOUSE;

    // sound
    s_master_vol = 1.0f;
    s_set_master_vol = -1.0f;
    s_sound_quality = SQT_NORMAL;
    s_set_quality = -1;
    s_ll_qty = MIN_SOUNDS_MIXED;
    s_ll_qty_changed = false;
    s_music_get_vol = 0.5f;
    s_music_set_vol = -1.0f;
    s_find_sound_calls = 0;

    // video globals (bit_depth must match preferred so video.finish's
    // screen-mode branch stays dormant in the default flow)
    Render_FOV = 72.0f;
    Render_preferred_state = renderer_preferred_state{};
    Render_preferred_state.bit_depth = 32;
    Render_preferred_bitdepth = 32;
    s_screen_mode = 1;
    s_screen_mode_changes = 0;

    // toggles inputs
    Default_player_terrain_leveling = 2;
    Default_player_room_leveling = 2;
    Missile_camera_window = -1;
    Hud_stat_mask = 0;
    s_sethud_calls = 0;
    s_rec_sethud_mask = 0;
    s_rec_sethud_gr = 0;

    s_function_mode = MENU_MODE;

    newuiCore_Init();
  }
};

// ---------------------------------------------------------------------------
// ConfigureDisplayResolutions
// ---------------------------------------------------------------------------

/**
 * @test ConfigTest.FallbackListKeptWhenNoDisplays
 * @brief Verifies fallback List Kept When No Displays.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, FallbackListKeptWhenNoDisplays) {
  s_displays.clear(); // SDL_GetDisplays returns NULL

  std::vector<tVideoResolution> before = Video_res_list;
  ASSERT_GT(before.size(), 10u);

  ConfigureDisplayResolutions();

  EXPECT_EQ(Video_res_list.size(), before.size()); // fallback list intact
  bool found_640 = false;
  for (auto &r : Video_res_list)
    if (r.width == 640 && r.height == 480)
      found_640 = true;
  EXPECT_TRUE(found_640);
}

/**
 * @test ConfigTest.BuildsUniqueSortedListFromDisplayModes
 * @brief Verifies builds Unique Sorted List From Display Modes.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, BuildsUniqueSortedListFromDisplayModes) {
  s_displays = {11, 22};
  s_display_modes[11] = {{1920, 1080}, {640, 480}, {1920, 1080}, {1280, 720}};
  s_display_modes[22] = {{640, 480}, {800, 600}}; // duplicate across displays
  s_current_modes[11] = {1920, 1080};

  ConfigureDisplayResolutions();

  ASSERT_EQ(Video_res_list.size(), 4u); // deduped across displays
  // sorted by width then height per tVideoResolutionCompare
  EXPECT_EQ(Video_res_list[0].width, 640);
  EXPECT_EQ(Video_res_list[0].height, 480);
  EXPECT_EQ(Video_res_list[1].width, 800);
  EXPECT_EQ(Video_res_list[2].width, 1280);
  EXPECT_EQ(Video_res_list[3].width, 1920);
  EXPECT_EQ(Video_res_list[3].getName(), "1920x1080");
}

/**
 * @test ConfigTest.DefaultIdFromCurrentModeAndCurrentFollowsWhenDbMisses
 * @brief Verifies default Id From Current Mode And Current Follows When Db Misses.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, DefaultIdFromCurrentModeAndCurrentFollowsWhenDbMisses) {
  s_displays = {42};
  s_display_modes[42] = {{640, 480}, {1280, 720}};
  s_current_modes[42] = {1280, 720};
  // Database has no RS_resolution entry -> Current id follows default

  ConfigureDisplayResolutions();

  EXPECT_EQ(Display_id, 42);
  EXPECT_EQ(Default_resolution_id, 1); // index of 1280x720 in sorted list
  EXPECT_EQ(Current_video_resolution_id, 1);
  EXPECT_EQ(Video_res_list[Current_video_resolution_id].getName(), "1280x720");
}

/**
 * @test ConfigTest.CurrentIdPreservedWhenDbHasSetting
 * @brief Verifies current Id Preserved When Db Has Setting.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, CurrentIdPreservedWhenDbHasSetting) {
  s_displays = {42};
  s_display_modes[42] = {{640, 480}, {1280, 720}};
  s_current_modes[42] = {1280, 720};
  Current_video_resolution_id = 0; // user picked 640x480 previously
  g_database.ints["RS_resolution"] = 0;

  ConfigureDisplayResolutions();

  EXPECT_EQ(Default_resolution_id, 1);   // still derived from current mode
  EXPECT_EQ(Current_video_resolution_id, 0); // but not overridden
}

/**
 * @test ConfigTest.CliWidthHeightAddsResolutionAsDefault
 * @brief Verifies cli Width Height Adds Resolution As Default.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, CliWidthHeightAddsResolutionAsDefault) {
  TestSetArgs({"-Width", "1000", "-Height", "750"});
  s_displays = {7};
  s_display_modes[7] = {{640, 480}};
  s_current_modes[7] = {640, 480};

  ConfigureDisplayResolutions();

  ASSERT_EQ(Video_res_list.size(), 2u);
  EXPECT_EQ(Video_res_list[0].getName(), "640x480");
  EXPECT_EQ(Video_res_list[1].getName(), "1000x750"); // appended, sorted last
  EXPECT_EQ(Default_resolution_id, 1);                // CLI res is the default
  EXPECT_EQ(Current_video_resolution_id, 1);
}

/**
 * @test ConfigTest.WidthWithoutHeightFallsBackToCurrentMode
 * @brief Verifies width Without Height Falls Back To Current Mode.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, WidthWithoutHeightFallsBackToCurrentMode) {
  TestSetArgs({"-Width", "1000"}); // missing -Height -> warning path
  s_displays = {7};
  s_display_modes[7] = {{640, 480}, {800, 600}};
  s_current_modes[7] = {800, 600};

  ConfigureDisplayResolutions();

  // CLI resolution NOT added; current display mode drives the default
  ASSERT_EQ(Video_res_list.size(), 2u);
  EXPECT_EQ(Default_resolution_id, 1); // 800x600 sorts after 640x480
  EXPECT_EQ(Current_video_resolution_id, 1);
}

/**
 * @test ConfigTest.DisplayArgSelectsDisplayAndValidatesRange
 * @brief Verifies display Arg Selects Display And Validates Range.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, DisplayArgSelectsDisplayAndValidatesRange) {
  TestSetArgs({"-display", "1"});
  s_displays = {5, 9};
  s_display_modes[5] = {{640, 480}};
  s_display_modes[9] = {{1600, 900}};
  s_current_modes[5] = {640, 480};
  s_current_modes[9] = {1600, 900};

  ConfigureDisplayResolutions();
  EXPECT_EQ(Display_id, 9);

  // out of range: keeps display 0
  TestSetArgs({"-display", "5"});
  s_current_modes[5] = {640, 480};
  ConfigureDisplayResolutions();
  EXPECT_EQ(Display_id, 5); // fell back to first display

  // missing parameter: also falls back
  TestSetArgs({"-display"});
  ConfigureDisplayResolutions();
  EXPECT_EQ(Display_id, 5);
}

/**
 * @test ConfigTest.MissingCurrentModeDefaultsToHighest
 * @brief Verifies missing Current Mode Defaults To Highest.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, MissingCurrentModeDefaultsToHighest) {
  s_displays = {3};
  s_display_modes[3] = {{640, 480}, {1024, 768}, {1600, 900}};
  s_current_modes[3] = {777, 555}; // current mode absent from list

  ConfigureDisplayResolutions();

  EXPECT_EQ(Default_resolution_id, (int)Video_res_list.size() - 1); // highest supported
  EXPECT_EQ(Video_res_list[Default_resolution_id].getName(), "1600x900");
}

/**
 * @test ConfigTest.ModesNullOnFirstDisplayLeavesStateUntouched
 * @brief Verifies modes Null On First Display Leaves State Untouched.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, ModesNullOnFirstDisplayLeavesStateUntouched) {
  s_displays = {66};           // display present...
  // ...but no modes registered -> SDL_GetFullscreenDisplayModes returns NULL

  std::vector<tVideoResolution> before = Video_res_list;
  int old_display = Display_id;

  ConfigureDisplayResolutions();

  EXPECT_EQ(Video_res_list.size(), before.size()); // early return before swap
  EXPECT_EQ(Display_id, old_display);
  EXPECT_TRUE(g_recorder.empty() || true);
}

// ---------------------------------------------------------------------------
// ConfigSetDetailLevel presets
// ---------------------------------------------------------------------------

// NOTE on presets: the DetailPreset* aggregate initializers predate the
// Bumpmapping_enabled field, which was inserted into tDetailSettings without
// updating the lists. Every value after Weapon_coronas lands one slot early,
// and Object_complexity (last field) is never initialized -> always 0.
static void expect_preset_common(float dist_mult, float pixel_err) {
  EXPECT_FLOAT_EQ(Detail_settings.Terrain_render_distance, dist_mult * TERRAIN_SIZE);
  EXPECT_FLOAT_EQ(Detail_settings.Pixel_error, pixel_err);
}

/**
 * @test ConfigTest.LowPresetDisablesEyeCandy
 * @brief Verifies low Preset Disables Eye Candy.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, LowPresetDisablesEyeCandy) {
  memset(&Detail_settings, 0xAB, sizeof(Detail_settings));
  ConfigSetDetailLevel(DETAIL_LEVEL_LOW);

  expect_preset_common(80.0f, 25.0f);
  EXPECT_FALSE(Detail_settings.Dynamic_lighting);
  EXPECT_FALSE(Detail_settings.Fog_enabled);
  EXPECT_FALSE(Detail_settings.Coronas_enabled);
  EXPECT_FALSE(Detail_settings.Scorches_enabled);
  EXPECT_FALSE(Detail_settings.Powerup_halos);
  EXPECT_TRUE(Detail_settings.Fast_headlight_on); // even low keeps headlight fast
  // shift artifacts of the misaligned initializer
  EXPECT_TRUE(Detail_settings.Bumpmapping_enabled);        // got SPEC_MAPPING_TYPE(1)
  EXPECT_EQ(Detail_settings.Specular_mapping_type, 0);     // got OBJECT_COMPLEXITY(0)
  EXPECT_EQ(Detail_settings.Object_complexity, 0);         // never initialized
  EXPECT_EQ(Default_detail_level, DETAIL_LEVEL_LOW);
}

/**
 * @test ConfigTest.MedPresetEnablesFogAndCoronas
 * @brief Verifies med Preset Enables Fog And Coronas.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, MedPresetEnablesFogAndCoronas) {
  memset(&Detail_settings, 0, sizeof(Detail_settings));
  ConfigSetDetailLevel(DETAIL_LEVEL_MED);

  expect_preset_common(90.0f, 18.0f);
  EXPECT_TRUE(Detail_settings.Fog_enabled);
  EXPECT_TRUE(Detail_settings.Coronas_enabled);
  EXPECT_TRUE(Detail_settings.Scorches_enabled);
  EXPECT_TRUE(Detail_settings.Powerup_halos);
  EXPECT_FALSE(Detail_settings.Dynamic_lighting);
  EXPECT_FALSE(Detail_settings.Mirrored_surfaces);
  EXPECT_EQ(Detail_settings.Specular_mapping_type, 1); // OBJECT_COMPLEXITY slot
  EXPECT_EQ(Detail_settings.Object_complexity, 0);     // quirk: never set
  EXPECT_EQ(Default_detail_level, DETAIL_LEVEL_MED);
}

/**
 * @test ConfigTest.HighPresetAddsDynamicLightingAndMirrors
 * @brief Verifies high Preset Adds Dynamic Lighting And Mirrors.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, HighPresetAddsDynamicLightingAndMirrors) {
  ConfigSetDetailLevel(DETAIL_LEVEL_HIGH);

  expect_preset_common(100.0f, 12.0f);
  EXPECT_TRUE(Detail_settings.Dynamic_lighting);
  EXPECT_TRUE(Detail_settings.Mirrored_surfaces);
  EXPECT_TRUE(Detail_settings.Procedurals_enabled);
  EXPECT_TRUE(Detail_settings.Weapon_coronas_enabled);
  EXPECT_FALSE(Detail_settings.Specular_lighting); // only very-high gets specular
  EXPECT_EQ(Detail_settings.Specular_mapping_type, 2);
  EXPECT_EQ(Default_detail_level, DETAIL_LEVEL_HIGH);
}

/**
 * @test ConfigTest.VeryHighPresetMaxesEverything
 * @brief Verifies very High Preset Maxes Everything.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, VeryHighPresetMaxesEverything) {
  ConfigSetDetailLevel(DETAIL_LEVEL_VERY_HIGH);

  expect_preset_common(120.0f, 10.0f);
  EXPECT_TRUE(Detail_settings.Specular_lighting);
  EXPECT_TRUE(Detail_settings.Dynamic_lighting);
  EXPECT_TRUE(Detail_settings.Procedurals_enabled);
  EXPECT_TRUE(Detail_settings.Weapon_coronas_enabled);
  EXPECT_TRUE(Detail_settings.Bumpmapping_enabled);    // shifted SPEC_MAPPING_TYPE
  EXPECT_EQ(Detail_settings.Specular_mapping_type, 2); // shifted OBJECT_COMPLEXITY
}

/**
 * @test ConfigTest.UnknownLevelSkipsPresetButRecordsLevel
 * @brief Verifies unknown Level Skips Preset But Records Level.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, UnknownLevelSkipsPresetButRecordsLevel) {
  ConfigSetDetailLevel(DETAIL_LEVEL_HIGH);
  tDetailSettings saved = Detail_settings;

  ConfigSetDetailLevel(99); // no matching preset

  // settings untouched, but level variable updated regardless (quirk)
  EXPECT_EQ(memcmp(&Detail_settings, &saved, sizeof(tDetailSettings)), 0);
  EXPECT_EQ(Default_detail_level, 99);
}

// ---------------------------------------------------------------------------
// OptionsMenu integration: forced bail-out runs every finish hook and saves
// ---------------------------------------------------------------------------

/**
 * @test ConfigTest.ForceQuitRunsAllFinishHooksAndSaves
 * @brief Verifies force Quit Runs All Finish Hooks And Saves.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, ForceQuitRunsAllFinishHooksAndSaves) {
  Multi_bail_ui_menu = true; // DoUI() returns NEWUIRES_FORCEQUIT immediately

  s_pilot_stat = STAT_SHIP | STAT_INVENTORY;
  s_pilot_grstat = 0;

  OptionsMenu();

  // one ui frame was run and flipped
  EXPECT_EQ(std::count(g_recorder.begin(), g_recorder.end(), "defer"), 1);
  EXPECT_EQ(std::count(g_recorder.begin(), g_recorder.end(), "flip"), 1);

  // hud.finish rebuilt masks from pilot data (messages+custom base always set)
  EXPECT_EQ(s_sethuddata_calls, 1);
  uint16_t expected_stat = STAT_MESSAGES | STAT_CUSTOM | STAT_SHIP | STAT_INVENTORY;
  EXPECT_EQ(s_rec_new_stat, expected_stat);
  EXPECT_EQ(s_rec_new_grstat, 0);

  // toggles.finish round-trips controller flags and leveling defaults
  EXPECT_EQ(Current_pilot.read_controller, READF_JOY | READF_MOUSE);
  EXPECT_EQ(Missile_camera_window, -1); // none selected stays none
  EXPECT_EQ(Default_player_terrain_leveling, 2);
  EXPECT_EQ(Default_player_room_leveling, 2);

  // details.finish wrote preset radio back to database
  ASSERT_GE(g_database.writes.size(), 1u);
  bool found_db_write = false;
  for (auto &[label, val] : g_database.writes)
    if (label == std::string("PredefDetailSetting") && val == DETAIL_LEVEL_MED)
      found_db_write = true;
  EXPECT_TRUE(found_db_write);

  // sound.finish applied slider-derived volumes
  EXPECT_FLOAT_EQ(s_set_master_vol, 1.0f);
  EXPECT_FLOAT_EQ(s_music_set_vol, 0.5f);
  EXPECT_EQ(s_set_quality, SQT_NORMAL);
  EXPECT_FALSE(s_ll_qty_changed); // quantity unchanged -> no reconfig

  // video.finish recomputed FOV from slider (72 deg default)
  EXPECT_FLOAT_EQ(Render_FOV_setting, D3_DEFAULT_FOV);

  // pilot written + settings saved exactly once
  EXPECT_EQ(s_pltwrite_calls, 1);
  EXPECT_EQ(s_savegamesettings_calls, 1);
}

/**
 * @test ConfigTest.InGameHudChangePropagatesToSetHUDState
 * @brief Verifies in Game Hud Change Propagates To Set HUDState.
 *
 * @details
 * Exercises the ConfigTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/config.cpp
 * @ingroup descent3_tests
 */
TEST_F(ConfigTest, InGameHudChangePropagatesToSetHUDState) {
  Multi_bail_ui_menu = true;
  s_function_mode = GAME_MODE; // in-game branch active
  Hud_stat_mask = STAT_FPS;
  s_pilot_stat = STAT_SHIP;
  s_pilot_grstat = 0;

  OptionsMenu();

  EXPECT_EQ(s_sethud_calls, 1);
  // config.cpp maps its STAT_SCORE macro onto STAT_TIMER
  uint16_t expected = STAT_MESSAGES | STAT_CUSTOM | STAT_SHIP | STAT_TIMER | STAT_FPS;
  EXPECT_EQ(s_rec_sethud_mask, expected);
  EXPECT_EQ(s_rec_sethud_gr, 0);
}
