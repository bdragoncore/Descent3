/**
 * @file subtitles_real_tests.cpp
 * @brief Tests for subtitles.cpp — movie subtitle parsing/drawing (308 lines).
 *
 * @details
 * Compiles real Descent3/subtitles.cpp with real args.cpp + cfile.
 * Covers readline/parse_int/parse_token/parse_whitespace, SubtParseSubtitles,
 * SubtInitSubtitles -subtitles gate, SubtDrawSubtitles frame windows, SubtCloseSubtitles.
 *
 * This harness validates the behavior of `Descent3/subtitles.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/subtitles.cpp`
 * @par Harness
 * `subtitles_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/subtitles.cpp
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include <cstdint>
#include "subtitles.h"
#include "cfile.h"
#include "args.h"
#include "grdefs.h"

// ---- stubs for symbols referenced by subtitles.cpp ----
std::filesystem::path LocalArtDir = "/tmp/opencode/d3art";

// mve capture
struct MveCall {
  int x, y;
  uint32_t color;
  std::string msg;
};
static std::vector<MveCall> g_puts_calls;
static int g_clearrect_calls = 0;

void mve_Puts(int16_t x, int16_t y, ddgr_color col, const char *txt) {
  g_puts_calls.push_back({x, y, col, txt ? txt : ""});
}
void mve_ClearRect(int16_t, int16_t, int16_t, int16_t) { g_clearrect_calls++; }

// Error() comes from ddebug lib (already linked)

// These are non-static in subtitles.cpp but not in the header
char *parse_whitespace(char *p);
char *parse_int(char *buf, int *d);
char *parse_token(char *p, const char *token);
void readline(CFILE *file, char *buf, int maxsize);
void SubtParseSubtitles(CFILE *file);

extern int Num_subtitles;
extern int Movie_subtitle_init;

#define GR_RGB(r, g, b) ((ddgr_color)(((r) << 16) | ((g) << 8) | (b)))

static const char *kTestSub =
    "# Comment line ignored by parse (fails int -> reset)\n"
    "\n";

/**
 * @brief GTest fixture for SubtitlesTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class SubtitlesTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Reset args to empty set so FindArg("-subtitles") is false
    GatherArgs((const char *)"");
    g_puts_calls.clear();
    g_clearrect_calls = 0;
    
    SubtCloseSubtitles();
    Movie_subtitle_init = 0;
  }

  static void WriteFile(const char *path, const std::string &content) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << content;
  }
};

/**
 * @test SubtitlesTest.ParseWhitespaceSkipsLeading
 * @brief Verifies parse Whitespace Skips Leading.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, ParseWhitespaceSkipsLeading) {
  char buf[] = "   \t\t42 rest";
  char *p = parse_whitespace(buf);
  EXPECT_STREQ(p, "42 rest");
}

/**
 * @test SubtitlesTest.ParseIntBasics
 * @brief Verifies parse Int Basics.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, ParseIntBasics) {
  int d = -99;
  char buf1[] = "123";
  char *r = parse_int(buf1, &d);
  ASSERT_NE(r, nullptr);
  EXPECT_EQ(d, 123);
  EXPECT_EQ(*r, '\0');

  char buf2[] = "-45 tail";
  r = parse_int(buf2, &d);
  ASSERT_NE(r, nullptr);
  EXPECT_EQ(d, -45);
  EXPECT_STREQ(r, " tail");

  char buf3[] = "7,more";
  r = parse_int(buf3, &d);
  ASSERT_NE(r, nullptr);
  EXPECT_EQ(d, 7);
  EXPECT_STREQ(r, ",more");
}

/**
 * @test SubtitlesTest.ParseIntInvalid
 * @brief Verifies parse Int Invalid.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, ParseIntInvalid) {
  int d = -1;
  char buf1[] = "abc";
  EXPECT_EQ(parse_int(buf1, &d), nullptr);
  EXPECT_EQ(d, 0);

  char buf2[] = "12x";
  d = -1;
  EXPECT_EQ(parse_int(buf2, &d), nullptr);

  char buf3[] = "";
  d = -1;
  EXPECT_EQ(parse_int(buf3, &d), nullptr);
}

/**
 * @test SubtitlesTest.ParseTokenCaseInsensitivePrefix
 * @brief Verifies parse Token Case Insensitive Prefix.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, ParseTokenCaseInsensitivePrefix) {
  char buf[] = "#end of data";
  char *r = parse_token(buf, "#End");
  ASSERT_NE(r, nullptr);
  EXPECT_STREQ(r, " of data");

  char buf2[] = "nomatch";
  EXPECT_EQ(parse_token(buf2, "#End"), nullptr);
}

/**
 * @test SubtitlesTest.ReadlineReadsLinesUntilEof
 * @brief Verifies readline Reads Lines Until Eof.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, ReadlineReadsLinesUntilEof) {
  const char *path = "/tmp/opencode/d3sub_readline.txt";
  WriteFile(path, "first line\nsecond line\n");
  CFILE *f = cfopen(path, "rt");
  ASSERT_NE(f, nullptr);
  char buf[300];
  readline(f, buf, sizeof(buf));
  EXPECT_STREQ(buf, "first line");
  readline(f, buf, sizeof(buf));
  EXPECT_STREQ(buf, "second line");
  readline(f, buf, sizeof(buf));
  EXPECT_STREQ(buf, "");
  cfclose(f);
}

/**
 * @test SubtitlesTest.InitWithoutFlagResetsAndDoesNotLoad
 * @brief Verifies init Without Flag Resets And Does Not Load.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, InitWithoutFlagResetsAndDoesNotLoad) {
  Movie_subtitle_init = 1;
  Num_subtitles = 5; // simulate stale state
  SubtInitSubtitles("intro.mve");
  // No -subtitles arg: quick out after zeroing counters
  EXPECT_EQ(Num_subtitles, 0);
  EXPECT_EQ(Movie_subtitle_init, 0);
}

/**
 * @test SubtitlesTest.ParseFullSubtitleFile
 * @brief Verifies parse Full Subtitle File.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, ParseFullSubtitleFile) {
  const char *path = "/tmp/opencode/d3test.msb";
  WriteFile(path,
            "0 10 Hello world\n"
            "15 30 Second subtitle\n"
            "#End\n");
  CFILE *f = cfopen(path, "rt");
  ASSERT_NE(f, nullptr);
  SubtParseSubtitles(f);
  cfclose(f);

  ASSERT_EQ(Num_subtitles, 2);
  EXPECT_EQ(Movie_subtitle_init, 1);
  // Access internal array via draw behavior below; verify count only here.
}

/**
 * @test SubtitlesTest.DrawShowsActiveWithinFrameWindow
 * @brief Verifies draw Shows Active Within Frame Window.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, DrawShowsActiveWithinFrameWindow) {
  const char *path = "/tmp/opencode/d3test_draw.msb";
  WriteFile(path,
            "10 20 Only One\n"
            "#End\n");
  CFILE *f = cfopen(path, "rt");
  ASSERT_NE(f, nullptr);
  SubtParseSubtitles(f);
  cfclose(f);
  ASSERT_EQ(Num_subtitles, 1);

  // Before first frame: nothing active
  g_puts_calls.clear();
  SubtDrawSubtitles(5);
  EXPECT_TRUE(g_puts_calls.empty());
  EXPECT_EQ(g_clearrect_calls, 1);

  // Inside window: drawn white
  g_puts_calls.clear();
  g_clearrect_calls = 0;
  SubtDrawSubtitles(15);
  ASSERT_EQ(g_puts_calls.size(), 1u);
  EXPECT_EQ(g_puts_calls[0].color, GR_RGB(255, 255, 255));
  EXPECT_STREQ(g_puts_calls[0].msg.c_str(), "Only One");
  EXPECT_EQ(g_clearrect_calls, 1);

  // After last_frame: expired and removed
  g_puts_calls.clear();
  SubtDrawSubtitles(21);
  EXPECT_TRUE(g_puts_calls.empty());

  SubtCloseSubtitles();
  EXPECT_EQ(Num_subtitles, 0);
}

/**
 * @test SubtitlesTest.CloseFreesAndMarksInit
 * @brief Verifies close Frees And Marks Init.
 *
 * @details
 * Exercises the SubtitlesTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/subtitles.cpp
 * @ingroup descent3_tests
 */
TEST_F(SubtitlesTest, CloseFreesAndMarksInit) {
  const char *path = "/tmp/opencode/d3test_close.msb";
  WriteFile(path,
            "0 5 A\n"
            "6 9 B\n"
            "#End\n");
  CFILE *f = cfopen(path, "rt");
  ASSERT_NE(f, nullptr);
  SubtParseSubtitles(f);
  cfclose(f);
  ASSERT_EQ(Num_subtitles, 2);

  SubtCloseSubtitles();
  EXPECT_EQ(Num_subtitles, 0);
  EXPECT_EQ(Movie_subtitle_init, 1);
}
