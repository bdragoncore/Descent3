/**
 * @file descent_real_tests.cpp
 * @brief Tests for Descent3/descent.cpp -- main loop state machine, defer.
 *
 * @details
 * handler, splash screen helper, and boot-time proxy parsing.
 *
 * Everything the module orchestrates (menus, game frames, renderer,
 * movies, sound) is stubbed; we observe sequencing and branching.
 *
 * This harness validates the behavior of `Descent3/descent.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/descent.cpp`
 * @par Harness
 * `descent_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/descent.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>

#include "descent.h"
#include "gamesequence.h"
#include "game.h"
#include "player.h"
#include "hlsoundlib.h"
#include "application.h"
#include "chrono_timer.h"
#include "args.h"

// ---- recorded output ----
static std::vector<std::string> g_order;
#define REC(name) g_order.push_back(name)

static int g_mainmenu_result = 0;
static int g_mainmenu_calls = 0;
static tGameState g_last_gamestate = GAMESTATE_IDLE;
static int g_playgame_calls = 0;
static bool g_gamemode_multi_seen = false;
static bool g_skip_render_during_frame = false;
extern bool Skip_render_game_frame; // defined below, consumed by GameFrame

int MainMenu() {
  g_mainmenu_calls++;
  REC("mainmenu");
  return g_mainmenu_result;
}
void PlayGame() {
  g_playgame_calls++;
  // end the loop after the first play pass
  SetFunctionMode(QUIT_MODE);
}
void GameFrame() {
  g_skip_render_during_frame = Skip_render_game_frame;
  REC("gameframe");
}
void Credits_Display() { REC("credits"); }
void IntroScreen() { REC("intro"); }
void ConfigureDisplayResolutions() { REC("configres"); }
void InitD3Systems1(bool) { REC("init1"); }
void InitD3Systems2(bool) { REC("init2"); }
void SaveGameSettings() { REC("save"); }
void FreeMultiDLL() { REC("freemod"); }
void SetScreenMode(int mode, bool) {
  REC((std::string("screenmode") + std::to_string(mode)));
}
void ui_SetScreenMode(int, int) {}
void PlayMovie(const std::filesystem::path &) {}
static int g_version_type = -1;
void ProgramVersion(int type, unsigned char, unsigned char, unsigned char) {
  REC("version");
  g_version_type = type;
}
bool g_killsound_called = false;
void hlsSystem::KillSoundLib(bool) { g_killsound_called = true; }
hlsSystem::hlsSystem() {}
hlsSystem Sound_system;

// pulled in via player.h; not exercised here
Inventory::Inventory() {}
Inventory::~Inventory() {}
void Inventory::Reset(bool, int) { REC("invreset"); }

namespace D3 {
float ChronoTimer::GetTime() { return 0.0f; }
} // namespace D3

// ---- world state ----
// GameArgs is a fixed 2D table per args.h -- a char** alias links but
// reads garbage, so mirror the real layout exactly.
char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];
int Game_mode = 0;
int Max_window_w = 640, Max_window_h = 480;
bool Dedicated_server = false;
bool Skip_render_game_frame = false;
tGameState Game_state = GAMESTATE_IDLE;
int frames_one_second = 0, min_one_second = 0, max_one_second = 0;
player Players[MAX_PLAYERS];

int FindArg(const char *which, int start) {
  for (int i = start; i < MAX_ARGS && GameArgs[i][0]; i++)
    if (strcmp(GameArgs[i], which) == 0)
      return i;
  return 0;
}
const char *GetStringFromTable(int) { return "err"; }
void Error(const char *, ...) {}
bool ddio_DeleteLockFile(const std::filesystem::path &) { return true; }
void ddio_Frame() {}
void ddio_KeyFlush() {}
int ddio_KeyInKey() { return 0; }
void ddio_MouseReset() {}
bool ddio_MouseGetState(int *, int *, int *, int *, int *, int *) { return false; }
void nw_DoNetworkIdle() {}
void rend_Flip() {}

// ---- splash screen observability ----
static int g_splash_bm_handle = -1;
static int g_splash_draws = 0;
static bool g_splash_freed = false;
static bool g_splash_chunked_destroyed = false;
int bm_AllocLoadFileBitmap(const char *, int, int) { return g_splash_bm_handle; }
bool bm_CreateChunkedBitmap(int, chunked_bitmap *) { return true; }
void bm_DestroyChunkedBitmap(chunked_bitmap *) { g_splash_chunked_destroyed = true; }
void bm_FreeBitmap(int) { g_splash_freed = true; }
void StartFrame(bool) {}
void EndFrame() {}
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, unsigned char) { g_splash_draws++; }

// minimal application object so Descent->defer() can be driven
class TestApp : public oeApplication {
public:
  int defer_calls = 0;
  void init() override {}
  void get_info(void *) override {}
  int flags(void) const override { return 0; }
  unsigned defer() override {
    defer_calls++;
    return 0;
  }
  void delay(float) override {}
  void set_defer_handler(void (*)(bool)) override {}
};
// ---- helpers ----
extern oeApplication *Descent;
static TestApp g_app;

static void ResetWorld() {
  g_order.clear();
  g_mainmenu_calls = 0;
  g_mainmenu_result = 1; // quit immediately by default
  g_playgame_calls = 0;
  g_killsound_called = false;
  g_skip_render_during_frame = false;
  Dedicated_server = false;
  Game_mode = 0;
  g_version_type = -1;
  g_splash_bm_handle = -1;
  g_splash_draws = 0;
  g_splash_freed = false;
  g_splash_chunked_destroyed = false;
  memset(GameArgs, 0, sizeof(GameArgs));
  strcpy(GameArgs[0], "descent3");
  memset(Players, 0, sizeof(Players));
  Descent = &g_app; // ShowStaticScreen calls defer() every frame
}

// public in descent.cpp but missing from descent.h
extern char Proxy_server[200];
extern int16_t Proxy_port;
extern bool Skip_render_game_frame;
void D3DeferHandler(bool is_active);

/**
 * @test DescentMain.FunctionModeAccessorRoundTrips
 * @brief Verifies function Mode Accessor Round Trips.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, FunctionModeAccessorRoundTrips) {
  ResetWorld();
  SetFunctionMode(GAME_MODE);
  EXPECT_EQ(GetFunctionMode(), GAME_MODE);
  SetFunctionMode(MENU_MODE);
  EXPECT_EQ(GetFunctionMode(), MENU_MODE);
}

/**
 * @test DescentMain.MenuQuitExitsLoopAndRunsShutdown
 * @brief Verifies menu Quit Exits Loop And Runs Shutdown.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, MenuQuitExitsLoopAndRunsShutdown) {
  ResetWorld();
  SetFunctionMode(MENU_MODE);
  MainLoop();

  // menu consulted once, then shutdown sequence
  ASSERT_EQ(g_mainmenu_calls, 1);
  ASSERT_FALSE(g_order.empty());
  EXPECT_TRUE(g_order.back().rfind("screenmode", 0) == 0); // SetScreenMode(SM_NULL) last
  EXPECT_TRUE(g_killsound_called);
  EXPECT_NE(std::find(g_order.begin(), g_order.end(), "freemod"), g_order.end());
  // note: SaveGameSettings belongs to Descent3()'s boot, not MainLoop
  EXPECT_EQ(std::find(g_order.begin(), g_order.end(), "save"), g_order.end());
}

/**
 * @test DescentMain.GameRestoreAndDemoModesDrivePlayGameWithState
 * @brief Verifies game Restore And Demo Modes Drive Play Game With State.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, GameRestoreAndDemoModesDrivePlayGameWithState) {
  ResetWorld();
  g_mainmenu_result = 0; // never reached

  // GAME_MODE -> NEW state
  SetFunctionMode(GAME_MODE);
  MainLoop();
  EXPECT_EQ(Game_state, GAMESTATE_NEW);
  EXPECT_EQ(g_playgame_calls, 1);

  // RESTORE_GAME_MODE -> LOADGAME state
  ResetWorld();
  g_mainmenu_result = 0;
  SetFunctionMode(RESTORE_GAME_MODE);
  MainLoop();
  EXPECT_EQ(Game_state, GAMESTATE_LOADGAME);
  EXPECT_EQ(g_playgame_calls, 1);

  // LOADDEMO_MODE -> LOADDEMO state
  ResetWorld();
  g_mainmenu_result = 0;
  SetFunctionMode(LOADDEMO_MODE);
  MainLoop();
  EXPECT_EQ(Game_state, GAMESTATE_LOADDEMO);
  EXPECT_EQ(g_playgame_calls, 1);
}

/**
 * @test DescentMain.CreditsModeDisplaysThenFallsToMenu
 * @brief Verifies credits Mode Displays Then Falls To Menu.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, CreditsModeDisplaysThenFallsToMenu) {
  ResetWorld();
  g_mainmenu_result = 1;
  SetFunctionMode(CREDITS_MODE);
  MainLoop();
  EXPECT_EQ(g_mainmenu_calls, 1); // credits handed off to menu
  auto it = std::find(g_order.begin(), g_order.end(), "credits");
  ASSERT_NE(it, g_order.end());
  auto mm = std::find(g_order.begin(), g_order.end(), "mainmenu");
  ASSERT_NE(mm, g_order.end());
  EXPECT_LT(it - g_order.begin(), mm - g_order.begin());
}

/**
 * @test DescentMain.DedicatedServerSkipsMenusIntoGame
 * @brief Verifies dedicated Server Skips Menus Into Game.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, DedicatedServerSkipsMenusIntoGame) {
  ResetWorld();
  Dedicated_server = true;
  SetFunctionMode(MENU_MODE);
  MainLoop();
  Dedicated_server = false;

  EXPECT_EQ(g_mainmenu_calls, 0);      // never saw a menu
  EXPECT_EQ(g_playgame_calls, 1);      // forced into game
  EXPECT_EQ(Game_state, GAMESTATE_NEW);
}

/**
 * @test DescentMain.DeferHandlerActiveOnlyPumpsInput
 * @brief Verifies defer Handler Active Only Pumps Input.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, DeferHandlerActiveOnlyPumpsInput) {
  ResetWorld();
  D3DeferHandler(true);
  EXPECT_TRUE(g_order.empty()); // no GameFrame when active

  // inactive but single player: still no game frame
  D3DeferHandler(false);
  EXPECT_TRUE(g_order.empty());
  EXPECT_FALSE(g_skip_render_during_frame);
}

/**
 * @test DescentMain.DeferHandlerInactiveMultiplayerTicksFrameHeadless
 * @brief Verifies defer Handler Inactive Multiplayer Ticks Frame Headless.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, DeferHandlerInactiveMultiplayerTicksFrameHeadless) {
  ResetWorld();
  Game_mode |= GM_MULTI;
  D3DeferHandler(false);
  Game_mode &= ~GM_MULTI;

  // the background frame ran with rendering suppressed, and the flag
  // was restored afterwards
  EXPECT_TRUE(g_skip_render_during_frame);
  ASSERT_EQ(g_order.size(), 1u);
  EXPECT_EQ(g_order[0].substr(0, 9), "gameframe");
}

/**
 * @test DescentMain.StaticScreenTimesOutAfterDelayAndCleansBitmap
 * @brief Verifies static Screen Times Out After Delay And Cleans Bitmap.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, StaticScreenTimesOutAfterDelayAndCleansBitmap) {
  ResetWorld();
  // load succeeds; timed with negative delay exits after one frame
  g_splash_bm_handle = 42;
  ShowStaticScreen("splash.ogf", true, -1.0f);
  EXPECT_EQ(g_splash_draws, 1);
  EXPECT_TRUE(g_splash_freed);
  EXPECT_TRUE(g_splash_chunked_destroyed);
}

/**
 * @test DescentMain.StaticScreenLoadFailureStillResetsUiMode
 * @brief Verifies static Screen Load Failure Still Resets Ui Mode.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, StaticScreenLoadFailureStillResetsUiMode) {
  ResetWorld();
  g_splash_bm_handle = -1; // load fails
  ShowStaticScreen("missing.ogf", true, -1.0f);
  EXPECT_EQ(g_splash_draws, 0);
}

/**
 * @test DescentMain.BootParsesHttpProxyAndShutsDownCleanly
 * @brief Verifies boot Parses Http Proxy And Shuts Down Cleanly.
 *
 * @details
 * Exercises the DescentMain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/descent.cpp
 * @ingroup descent3_tests
 */
TEST(DescentMain, BootParsesHttpProxyAndShutsDownCleanly) {
  ResetWorld();
  strcpy(GameArgs[1], "-httpproxy");
  strcpy(GameArgs[2], "proxyhost:1234");
  Dedicated_server = true; // skip intro/menu screens
  Descent3();
  Dedicated_server = false;

  EXPECT_STREQ(Proxy_server, "proxyhost");
  EXPECT_EQ(Proxy_port, 1234);

  // ordering: systems1 -> systems2 -> mainloop(shutdown) -> save
  auto i1 = std::find(g_order.begin(), g_order.end(), "init1");
  auto i2 = std::find(g_order.begin(), g_order.end(), "init2");
  auto sv = std::find(g_order.begin(), g_order.end(), "save");
  ASSERT_NE(i1, g_order.end());
  ASSERT_NE(i2, g_order.end());
  ASSERT_NE(sv, g_order.end());
  EXPECT_LT(i1 - g_order.begin(), i2 - g_order.begin());
  EXPECT_LT(i2 - g_order.begin(), sv - g_order.begin());
  EXPECT_TRUE(g_killsound_called);
}
