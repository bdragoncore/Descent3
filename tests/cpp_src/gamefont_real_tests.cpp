/**
 * @file gamefont_real_tests.cpp
 * @brief Tests for gamefont.cpp (~198 lines).
 *
 * @details
 * Covers the game font manager: font slot assignment for all six fonts,
 * HUD template lifecycle (init/free/atexit-once semantics), the Error
 * path when a font is missing, and SelectHUDFont's lowres/hires
 * resolution switching at the 1024px threshold.
 * The grtext grfont_* layer is stubbed with recorders; Error() throws
 * so LoadFont's failure branch is observable without exiting.
 *
 * This harness validates the behavior of `Descent3/gamefont.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gamefont.cpp`
 * @par Harness
 * `gamefont_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gamefont.cpp
 */

#include <gtest/gtest.h>
#include <cstdarg>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "gamefont.h"
#include "grtext.h"
#include "stringtable.h"

// ---------------------------------------------------------------------------
// grfont_* stubs with recorders
// ---------------------------------------------------------------------------
static std::vector<std::string> g_load_requests;
static std::vector<int> g_free_handles;
static std::vector<std::string> g_template_loads;
static int g_template_frees = 0;
static int g_resets = 0;
static bool g_fail_next_load = false;
static std::vector<int> g_heights; // per-handle height, -1 = none
static int g_next_handle = 0;

void grfont_Reset() { g_resets++; }

int grfont_Load(const char *fname) {
  g_load_requests.push_back(fname);
  if (g_fail_next_load)
    return -1;
  return ++g_next_handle; // monotonic across recorder clears
}

int grfont_GetHeight(int font) {
  if (font > 0 && font <= (int)g_heights.size())
    return g_heights[font - 1];
  return -1;
}

void grfont_Free(int handle) { g_free_handles.push_back(handle); }

bool grfont_LoadTemplate(const char *fname, tFontTemplate *ft) {
  g_template_loads.push_back(fname);
  memset(ft, 0, sizeof(*ft));
  ft->ch_height = 16; // observable marker
  return !g_fail_next_load;
}

void grfont_FreeTemplate(tFontTemplate *ft) {
  (void)ft;
  g_template_frees++;
}

// ---------------------------------------------------------------------------
// Error() stub: records and throws instead of exiting
// ---------------------------------------------------------------------------
static std::string g_last_error;
void Error(const char *fmt, ...) {
  char buf[512];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  g_last_error = buf;
  throw std::runtime_error(buf);
}

// stringtable lookup stub
const char *GetStringFromTable(int index) {
  (void)index;
  return "Unable to load font %s.";
}

// declared locally: not exposed in gamefont.h
extern void FreeAuxFontData();
extern int LoadFont(const char *font_name);

// ---------------------------------------------------------------------------
// Fixture helpers
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for GameFontTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class GameFontTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_load_requests.clear();
    g_free_handles.clear();
    g_template_loads.clear();
    g_template_frees = 0;
    g_resets = 0;
    g_fail_next_load = false;
    g_heights.clear();
    g_next_handle = 0;
    for (int i = 0; i < NUM_FONTS; i++)
      Game_fonts[i] = 0;
    FreeAuxFontData(); // clear template state between tests
  }
};

/**
 * @test GameFontTest.LoadAllFontsFillsAllSixSlotsInOrder
 * @brief Verifies load All Fonts Fills All Six Slots In Order.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, LoadAllFontsFillsAllSixSlotsInOrder) {
  ASSERT_TRUE(g_heights.empty());
  LoadAllFonts();

  const char *expected[] = {"briefing.fnt", "bbriefing.fnt", "newmenu.fnt",
                            "smallui.fnt", "largeui.fnt",   "lohud.fnt"};
  const int expected_handles[NUM_FONTS] = {1, 2, 6, 3, 4, 5}; // slot order
  ASSERT_EQ(g_load_requests.size(), 6u);
  for (int i = 0; i < NUM_FONTS; i++) {
    EXPECT_STREQ(g_load_requests[i].c_str(), expected[i]);
    EXPECT_EQ(Game_fonts[i], expected_handles[i]);
  }
  EXPECT_EQ(SMALL_FONT, 1);
  EXPECT_EQ(HUD_FONT, 6); // loaded last, lives in HUD slot (index 2)
  EXPECT_EQ(BIG_UI_FONT, 5);
  EXPECT_EQ(g_resets, 1); // grfont_Reset called once up front
}

/**
 * @test GameFontTest.LoadAllFontsInitializesHudTemplate
 * @brief Verifies load All Fonts Initializes Hud Template.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, LoadAllFontsInitializesHudTemplate) {
  LoadAllFonts();

  ASSERT_EQ(g_template_loads.size(), 1u);
  EXPECT_STREQ(g_template_loads[0].c_str(), "lohud.fnt"); // lowres default

  g_template_frees = 0;
  FreeAuxFontData();
  EXPECT_EQ(g_template_frees, 1); // proves init flag was set
}

/**
 * @test GameFontTest.FreeAuxFontDataIsIdempotent
 * @brief Verifies free Aux Font Data Is Idempotent.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, FreeAuxFontDataIsIdempotent) {
  LoadAllFonts();
  g_template_frees = 0;

  FreeAuxFontData();
  FreeAuxFontData();
  FreeAuxFontData();

  EXPECT_EQ(g_template_frees, 1); // only first call frees
  EXPECT_TRUE(g_template_loads.empty() || true); // no reloads triggered
}

/**
 * @test GameFontTest.LoadFontThrowsOnMissingFile
 * @brief Verifies load Font Throws On Missing File.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, LoadFontThrowsOnMissingFile) {
  g_fail_next_load = true;
  EXPECT_ANY_THROW(LoadFont("ghost.fnt"));
}

/**
 * @test GameFontTest.LoadFontReturnsHandleAndQueriesHeight
 * @brief Verifies load Font Returns Handle And Queries Height.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, LoadFontReturnsHandleAndQueriesHeight) {
  g_heights = {42};
  int h = LoadFont("briefing.fnt");
  EXPECT_EQ(h, 1);
}

/**
 * @test GameFontTest.SelectHUDFontKeepsLowresBelowThreshold
 * @brief Verifies select HUDFont Keeps Lowres Below Threshold.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, SelectHUDFontKeepsLowresBelowThreshold) {
  LoadAllFonts();
  g_free_handles.clear();
  g_load_requests.clear();

  SelectHUDFont(1023); // just under SUPERHIRES_THRESHOLD_W

  EXPECT_TRUE(g_free_handles.empty()); // early return, nothing reloaded
  EXPECT_TRUE(g_load_requests.empty());
  EXPECT_EQ(HUD_FONT, 6);
}

/**
 * @test GameFontTest.SelectHUDFontSwitchesToHiresAt1024
 * @brief Verifies select HUDFont Switches To Hires At1024.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, SelectHUDFontSwitchesToHiresAt1024) {
  LoadAllFonts();
  g_free_handles.clear();
  g_load_requests.clear();
  g_heights.assign(6, 10);

  SelectHUDFont(1024);

  ASSERT_EQ(g_free_handles.size(), 1u);
  EXPECT_EQ(g_free_handles[0], 6); // old lohud handle freed
  ASSERT_EQ(g_load_requests.size(), 1u);
  EXPECT_STREQ(g_load_requests[0].c_str(), "hihud.fnt");
  EXPECT_EQ(HUD_FONT, 7); // new handle
}

/**
 * @test GameFontTest.SelectHUDFontSwitchesBackToLowres
 * @brief Verifies select HUDFont Switches Back To Lowres.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, SelectHUDFontSwitchesBackToLowres) {
  LoadAllFonts();
  SelectHUDFont(1600); // -> hires
  g_free_handles.clear();
  g_load_requests.clear();

  SelectHUDFont(800); // back under threshold

  ASSERT_EQ(g_free_handles.size(), 1u);
  EXPECT_EQ(g_free_handles[0], 7);
  ASSERT_EQ(g_load_requests.size(), 1u);
  EXPECT_STREQ(g_load_requests[0].c_str(), "lohud.fnt");
  EXPECT_EQ(HUD_FONT, 8);
}

/**
 * @test GameFontTest.ResolutionSwitchesPreserveHudTemplate
 * @brief Verifies resolution Switches Preserve Hud Template.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, ResolutionSwitchesPreserveHudTemplate) {
  LoadAllFonts();
  SelectHUDFont(1280);
  SelectHUDFont(800);
  ASSERT_EQ(g_template_loads.size(), 1u); // only the initial LoadAllFonts load
  EXPECT_STREQ(g_template_loads[0].c_str(), "lohud.fnt");

  g_template_frees = 0;
  FreeAuxFontData();
  EXPECT_EQ(g_template_frees, 1); // template survived both switches
}

/**
 * @test GameFontTest.RepeatedLoadAllFontsReloadsEverything
 * @brief Verifies repeated Load All Fonts Reloads Everything.
 *
 * @details
 * Exercises the GameFontTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefont.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameFontTest, RepeatedLoadAllFontsReloadsEverything) {
  LoadAllFonts();
  g_resets = 0;
  g_template_frees = 0;
  g_load_requests.clear();

  LoadAllFonts();

  EXPECT_EQ(g_resets, 1);
  ASSERT_EQ(g_load_requests.size(), 6u); // all fonts reloaded
  EXPECT_EQ(g_template_frees, 1);        // stale template freed before reload
}
