/**
 * @file credits_real_tests.cpp
 * @brief Credits system tests (Descent3/credits.cpp).
 *
 * @details
 * credits.cpp's interesting logic lives in two functions:
 *   - Credits_LoadCredits(): static parser for GameCredits.txt-style files
 *   - Credits_Render(): scrolling renderer with top/bottom alpha fade
 * Neither is reachable through a public symbol with an exposed data type
 * (creditline is internal), so this TU #includes the real .cpp directly to
 * bring the statics into test scope. Everything credits.cpp references from
 * other translation units (bitmap/music/renderer/text/key/timer/sound) is
 * stubbed here or in fake_hlsystem.cpp; cfile and logger are real.
 *
 * This harness validates the behavior of `Descent3/credits.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/credits.cpp`
 * @par Harness
 * `credits_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/credits.cpp
 */

#include <gtest/gtest.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "bitmap.h"
#include "crossplat.h"
#include "d3music.h"
#include "ddio.h"
#include "descent.h"
#include "gamefont.h"
#include "grtext.h"
#include "hlsoundlib.h"
#include "log.h"
#include "pserror.h"
#include "psrand.h"
#include "renderer.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
int Game_fonts[NUM_FONTS] = {0, 0, 0, 0, 0, 0};
int Game_window_w = 640;
int Game_window_h = 480;

struct FakeApp : oeApplication {
  void init() override {}
  void get_info(void *) override {}
  int flags() const override { return 0; }
  unsigned defer() override { return 0; }
  void delay(float) override {}
  void set_defer_handler(void (*)(bool)) override {}
};
static FakeApp g_fake_app;
oeApplication *Descent = &g_fake_app;

namespace D3 {
static float g_fake_time = 0.0f;
float ChronoTimer::GetTime() { return g_fake_time; }
int64_t ChronoTimer::GetTimeMS() { return static_cast<int64_t>(g_fake_time * 1000.0f); }
} // namespace D3

// Deterministic replica of misc/psrand.cpp LCG (self-consistent is enough:
// the easter-egg test picks a seed by consuming one roll first).
static uint32_t g_ps_next = 1;
void ps_srand(uint32_t seed) { g_ps_next = seed; }
int32_t ps_rand(void) {
  g_ps_next = g_ps_next * 214013u + 2531011u;
  return static_cast<int32_t>((g_ps_next >> 16) & 0x7fff);
}

// hlsSystem members referenced by credits.cpp but not provided by
// fake_hlsystem.cpp (Sound_system itself + ctor/KillSoundLib live there).
void hlsSystem::BeginSoundFrame(bool) {}
void hlsSystem::EndSoundFrame() {}

// ---------------------------------------------------------------------------
// Stubs with capture state
// ---------------------------------------------------------------------------
struct PrintRec {
  std::string text;
  int xoff;
  int y;
  uint8_t alpha;
  ddgr_color color;
};
static std::vector<PrintRec> g_prints;
static ddgr_color g_cur_color = GR_RGB(0, 0, 0);
static uint8_t g_cur_alpha = 255;
static int g_flush_count = 0;
static int g_font_height = 20;

void grtext_Reset() {
  g_prints.clear();
  g_flush_count = 0;
}
void grtext_SetColor(ddgr_color col) { g_cur_color = col; }
void grtext_SetAlpha(uint8_t alpha) { g_cur_alpha = alpha; }
void grtext_SetFont(int) {}
void grtext_SetFontScale(float) {}
void grtext_CenteredPrintf(int xoff, int y, const char *fmt, ...) {
  char buf[512];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  g_prints.push_back({buf, xoff, y, g_cur_alpha, g_cur_color});
}
void grtext_Flush() { g_flush_count++; }
int grfont_GetHeight(int) { return g_font_height; }

static std::vector<std::string> g_music_started;
static bool g_music_stopped = false;
void D3MusicStart(const char *theme_file) { g_music_started.push_back(theme_file); }
void D3MusicStop() { g_music_stopped = true; }
void D3MusicDoFrame(tMusicSeqInfo *) {}
void D3MusicSetRegion(int16_t, bool) {}

void StartFrame(int, int, int, int, bool, bool) {}
void EndFrame() {}
void rend_Flip() {}
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, uint8_t) {}

static int g_key_in_key_result = 0;
int ddio_KeyInKey() { return g_key_in_key_result; }
void ddio_KeyFlush() {}

void bm_FreeBitmap(int) {}
int bm_AllocLoadFileBitmap(const char *, int, int) { return 1; }
bool bm_CreateChunkedBitmap(int, chunked_bitmap *) { return true; }
void bm_DestroyChunkedBitmap(chunked_bitmap *) {}

#include "../../Descent3/credits.cpp"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static constexpr const char *kTmpCredits = "/tmp/opencode/d3_test_credits.txt";

static void WriteFile(const char *path, const char *contents) {
  FILE *fp = fopen(path, "wb");
  ASSERT_NE(fp, nullptr);
  fputs(contents, fp);
  fclose(fp);
}

// ---------------------------------------------------------------------------
// Credits_LoadCredits
// ---------------------------------------------------------------------------
/**
 * @test CreditsLoadTest.MissingFileFails
 * @brief Verifies missing File Fails.
 *
 * @details
 * Exercises the CreditsLoadTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST(CreditsLoadTest, MissingFileFails) {
  std::vector<creditline> lines;
  EXPECT_FALSE(Credits_LoadCredits("/tmp/opencode/no_such_credits_file_xyz.txt", lines));
}

/**
 * @test CreditsLoadTest.EmptyFileSucceedsWithNoLines
 * @brief Verifies empty File Succeeds With No Lines.
 *
 * @details
 * Exercises the CreditsLoadTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST(CreditsLoadTest, EmptyFileSucceedsWithNoLines) {
  WriteFile(kTmpCredits, "");
  std::vector<creditline> lines;
  EXPECT_TRUE(Credits_LoadCredits(kTmpCredits, lines));
  EXPECT_TRUE(lines.empty());
}

/**
 * @test CreditsLoadTest.ParsesAllDirectivesInOrder
 * @brief Verifies parses All Directives In Order.
 *
 * @details
 * Exercises the CreditsLoadTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST(CreditsLoadTest, ParsesAllDirectivesInOrder) {
  const char *file =
      "; a comment line\n"
      "^200 0 0\n"
      "$0 255 0\n"
      "!Main Title\n"
      "Hello World\n"
      "\n"
      "*10 5 20 6 1.5\n"
      "Second Line\n"
      "END of credits\n"
      "SHOULD NOT APPEAR\n";
  WriteFile(kTmpCredits, file);

  // Seed chosen so the easter egg does not trigger (first roll % 100 != 0).
  ps_srand(1);

  std::vector<creditline> lines;
  ASSERT_TRUE(Credits_LoadCredits(kTmpCredits, lines));

  ASSERT_EQ(lines.size(), 4u);

  EXPECT_EQ(lines[0].type, CLTYPE_HEADING);
  EXPECT_EQ(lines[0].text, "Main Title");
  EXPECT_EQ(lines[0].color, GR_RGB(200, 0, 0)); // heading color set by '^'

  EXPECT_EQ(lines[1].type, CLTYPE_TEXT);
  EXPECT_EQ(lines[1].text, "Hello World");
  EXPECT_EQ(lines[1].color, GR_RGB(0, 255, 0)); // text color set by '$'
  EXPECT_EQ(lines[1].startx, 0u);
  EXPECT_EQ(lines[1].displaytime, 0.0f);

  EXPECT_EQ(lines[2].type, CLTYPE_BLANK);
  EXPECT_TRUE(lines[2].text.empty());

  EXPECT_EQ(lines[3].type, CLTYPE_TEXT);
  EXPECT_EQ(lines[3].text, "Second Line");
  EXPECT_EQ(lines[3].color, GR_RGB(0, 255, 0));
  // Movement data from '*' applies to the following text line.
  EXPECT_EQ(lines[3].startx, 10u);
  EXPECT_EQ(lines[3].starty, 5u);
  EXPECT_EQ(lines[3].endx, 20u);
  EXPECT_EQ(lines[3].endy, 6u);
  EXPECT_FLOAT_EQ(lines[3].displaytime, 1.5f);
}

/**
 * @test CreditsLoadTest.EndPrefixIsCaseInsensitiveAndTruncates
 * @brief Verifies end Prefix Is Case Insensitive And Truncates.
 *
 * @details
 * Exercises the CreditsLoadTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST(CreditsLoadTest, EndPrefixIsCaseInsensitiveAndTruncates) {
  WriteFile(kTmpCredits, "ender\nIGNORED\n");
  ps_srand(1);
  std::vector<creditline> lines;
  EXPECT_TRUE(Credits_LoadCredits(kTmpCredits, lines));
  EXPECT_TRUE(lines.empty());
}

/**
 * @test CreditsLoadTest.BadMovementKeepsPreviousMovementData
 * @brief Verifies bad Movement Keeps Previous Movement Data.
 *
 * @details
 * Exercises the CreditsLoadTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST(CreditsLoadTest, BadMovementKeepsPreviousMovementData) {
  const char *file =
      "*1 2 3 4 2.0\n"
      "First\n"
      "*not movement data\n"
      "Second\n";
  WriteFile(kTmpCredits, file);
  ps_srand(1);

  std::vector<creditline> lines;
  ASSERT_TRUE(Credits_LoadCredits(kTmpCredits, lines));
  ASSERT_EQ(lines.size(), 2u);

  EXPECT_EQ(lines[0].startx, 1u);
  EXPECT_EQ(lines[0].endy, 4u);
  EXPECT_FLOAT_EQ(lines[0].displaytime, 2.0f);

  // The malformed '*' line logs a warning and leaves cur_credit untouched,
  // so the next text line inherits the previous movement data.
  EXPECT_EQ(lines[1].startx, 1u);
  EXPECT_EQ(lines[1].endy, 4u);
  EXPECT_FLOAT_EQ(lines[1].displaytime, 2.0f);
}

/**
 * @test CreditsLoadTest.BadColorDirectivesKeepPreviousColors
 * @brief Verifies bad Color Directives Keep Previous Colors.
 *
 * @details
 * Exercises the CreditsLoadTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST(CreditsLoadTest, BadColorDirectivesKeepPreviousColors) {
  const char *file =
      "$9 9\n"     // invalid text color: needs 3 components
      "^8 8\n"     // invalid heading color
      "Text A\n"
      "!Head\n";
  WriteFile(kTmpCredits, file);
  ps_srand(1);

  std::vector<creditline> lines;
  ASSERT_TRUE(Credits_LoadCredits(kTmpCredits, lines));
  ASSERT_EQ(lines.size(), 2u);

  EXPECT_EQ(lines[0].color, GR_RGB(0, 128, 255)); // default text color
  EXPECT_EQ(lines[1].color, GR_RGB(255, 255, 255)); // default heading color
}

/**
 * @test CreditsLoadTest.JasonLeightonEasterEggAppendsHiMom
 * @brief Verifies jason Leighton Easter Egg Appends Hi Mom.
 *
 * @details
 * Exercises the CreditsLoadTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST(CreditsLoadTest, JasonLeightonEasterEggAppendsHiMom) {
  WriteFile(kTmpCredits, "Jason Leighton\n");

  // Find a seed whose first roll hits 0 mod 100 so the easter egg fires.
  // The search consumes one roll per candidate, so re-seed afterwards to put
  // the LCG back at its starting point for the parser's own first roll.
  uint32_t seed = 1;
  for (;; ++seed) {
    ps_srand(seed);
    if ((ps_rand() % 100) == 0)
      break;
  }
  ps_srand(seed);

  std::vector<creditline> lines;
  ASSERT_TRUE(Credits_LoadCredits(kTmpCredits, lines));
  ASSERT_EQ(lines.size(), 1u);
  EXPECT_EQ(lines[0].text, "Jason Leighton (Hi mom!)");
}

/**
 * @test CreditsLoadTest.OtherNamesNeverGetTheSuffix
 * @brief Verifies other Names Never Get The Suffix.
 *
 * @details
 * Exercises the CreditsLoadTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST(CreditsLoadTest, OtherNamesNeverGetTheSuffix) {
  WriteFile(kTmpCredits, "Someone Leighton\n");
  ps_srand(1); // any seed: short-circuit means rand is not even consumed

  std::vector<creditline> lines;
  ASSERT_TRUE(Credits_LoadCredits(kTmpCredits, lines));
  ASSERT_EQ(lines.size(), 1u);
  EXPECT_EQ(lines[0].text, "Someone Leighton");
}

// ---------------------------------------------------------------------------
// Credits_Render
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for CreditsRenderTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class CreditsRenderTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_prints.clear();
    g_flush_count = 0;
    g_cur_color = GR_RGB(0, 0, 0);
    g_cur_alpha = 255;
    g_font_height = 20;
  }

  static creditline MakeLine(uint8_t type, const char *text, ddgr_color color) {
    creditline l;
    l.type = type;
    l.text = text;
    l.color = color;
    return l;
  }
};

/**
 * @test CreditsRenderTest.SkipsBlanksStopsAtEndSentinel
 * @brief Verifies skips Blanks Stops At End Sentinel.
 *
 * @details
 * Exercises the CreditsRenderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST_F(CreditsRenderTest, SkipsBlanksStopsAtEndSentinel) {
  std::vector<creditline> lines;
  lines.push_back(MakeLine(CLTYPE_BLANK, "", 0));
  lines.push_back(MakeLine(CLTYPE_BLANK, "", 0));
  lines.push_back(MakeLine(CLTYPE_TEXT, "A", GR_RGB(255, 0, 0)));
  lines.push_back(MakeLine(CLTYPE_HEADING, "B", GR_RGB(255, 255, 255)));
  lines.push_back(MakeLine(CLTYPE_END, "", 0));
  lines.push_back(MakeLine(CLTYPE_TEXT, "C", GR_RGB(0, 255, 0)));

  Credits_Render(lines.begin(), lines.end(), 0.0f);

  ASSERT_EQ(g_prints.size(), 2u);
  EXPECT_EQ(g_prints[0].text, "A");
  EXPECT_EQ(g_prints[0].y, 40); // after two blank lines at 20px each
  EXPECT_EQ(g_prints[0].alpha, 255);
  EXPECT_EQ(g_prints[0].color, GR_RGB(255, 0, 0));

  EXPECT_EQ(g_prints[1].text, "B");
  EXPECT_EQ(g_prints[1].y, 60);
  EXPECT_EQ(g_prints[1].alpha, 255);
  EXPECT_EQ(g_prints[1].color, GR_RGB(255, 255, 255));

  EXPECT_EQ(g_flush_count, 1);
}

/**
 * @test CreditsRenderTest.TopExitFadeOutAlpha
 * @brief Verifies top Exit Fade Out Alpha.
 *
 * @details
 * Exercises the CreditsRenderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST_F(CreditsRenderTest, TopExitFadeOutAlpha) {
  std::vector<creditline> lines;
  lines.push_back(MakeLine(CLTYPE_TEXT, "Top", GR_RGB(0, 255, 0)));
  lines.push_back(MakeLine(CLTYPE_END, "", 0));

  // cur_pixel starts at -pixels_in: the current top line slides UP and out
  // of the viewport as pixels_in grows toward one line height, fading out
  // linearly. Half a line out => half alpha.
  Credits_Render(lines.begin(), lines.end(), 10.0f);

  ASSERT_EQ(g_prints.size(), 1u);
  EXPECT_EQ(g_prints[0].y, -10);
  EXPECT_EQ(g_prints[0].alpha, 127); // (uint8)(0.5f * 255)
}

/**
 * @test CreditsRenderTest.BottomEntryFadeInAlpha
 * @brief Verifies bottom Entry Fade In Alpha.
 *
 * @details
 * Exercises the CreditsRenderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST_F(CreditsRenderTest, BottomEntryFadeInAlpha) {
  // Lines entering from the bottom fade in over the last line height of
  // travel: a line whose bottom edge is 480+n fades with norm = 1 - n/h.
  // Reach y=470 via 24 blank lines (20px each) at pixels_in=10.
  std::vector<creditline> lines;
  for (int i = 0; i < 24; i++)
    lines.push_back(MakeLine(CLTYPE_BLANK, "", 0));
  lines.push_back(MakeLine(CLTYPE_TEXT, "Bottom", GR_RGB(0, 255, 0)));
  lines.push_back(MakeLine(CLTYPE_END, "", 0));

  Credits_Render(lines.begin(), lines.end(), 10.0f);

  ASSERT_EQ(g_prints.size(), 1u);
  EXPECT_EQ(g_prints[0].y, 470);
  EXPECT_EQ(g_prints[0].alpha, 127); // (uint8)(0.5f * 255)
}

/**
 * @test CreditsRenderTest.FullyVisibleLineHasFullAlpha
 * @brief Verifies fully Visible Line Has Full Alpha.
 *
 * @details
 * Exercises the CreditsRenderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST_F(CreditsRenderTest, FullyVisibleLineHasFullAlpha) {
  std::vector<creditline> lines;
  lines.push_back(MakeLine(CLTYPE_TEXT, "Mid", GR_RGB(1, 2, 3)));
  lines.push_back(MakeLine(CLTYPE_END, "", 0));

  // pixels_in stays in [0, height) in real use (Credits_Display subtracts a
  // line height whenever it crosses), so y=0 means flush against the top
  // edge, fully opaque.
  Credits_Render(lines.begin(), lines.end(), 0.0f);

  ASSERT_EQ(g_prints.size(), 1u);
  EXPECT_EQ(g_prints[0].y, 0);
  EXPECT_EQ(g_prints[0].alpha, 255);
}

/**
 * @test CreditsRenderTest.BlanksOnlyTerminatesAtEndWithoutPrinting
 * @brief Verifies blanks Only Terminates At End Without Printing.
 *
 * @details
 * Exercises the CreditsRenderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST_F(CreditsRenderTest, BlanksOnlyTerminatesAtEndWithoutPrinting) {
  std::vector<creditline> lines;
  lines.push_back(MakeLine(CLTYPE_BLANK, "", 0));
  lines.push_back(MakeLine(CLTYPE_END, "", 0));

  Credits_Render(lines.begin(), lines.end(), 0.0f);

  EXPECT_TRUE(g_prints.empty());
  EXPECT_EQ(g_flush_count, 1);
}

/**
 * @test CreditsRenderTest.LongScrollAdvancesThroughManyLines
 * @brief Verifies long Scroll Advances Through Many Lines.
 *
 * @details
 * Exercises the CreditsRenderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/credits.cpp
 * @ingroup descent3_tests
 */
TEST_F(CreditsRenderTest, LongScrollAdvancesThroughManyLines) {
  std::vector<creditline> lines;
  for (int i = 0; i < 30; i++) {
    lines.push_back(MakeLine(CLTYPE_TEXT, ("L" + std::to_string(i)).c_str(), GR_RGB(9, 9, 9)));
  }
  lines.push_back(MakeLine(CLTYPE_END, "", 0));

  // The per-frame draw budget is 480 + height of travel; lines are drawn at
  // y=0,20,...,480 and the budget runs out right after y=480, so only the
  // first 25 of 30 lines are drawn. The line at y=480 has its bottom edge
  // exactly 20px past the viewport and renders at alpha 0 (invisible).
  Credits_Render(lines.begin(), lines.end(), 0.0f);

  ASSERT_EQ(g_prints.size(), 25u);
  EXPECT_EQ(g_prints[0].y, 0);
  EXPECT_EQ(g_prints[23].y, 460);
  EXPECT_EQ(g_prints[23].alpha, 255);
  EXPECT_EQ(g_prints[24].y, 480);
  EXPECT_EQ(g_prints[24].alpha, 0);
  EXPECT_EQ(g_flush_count, 1);
}
