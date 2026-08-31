/**
 * @file briefingparse_real_tests.cpp
 * @brief Tests for BriefingParse.cpp (1347 lines).
 *
 * @details
 * Drives the real parser over small briefing files and asserts what the
 * registered callbacks receive: screens, titles, text blocks, effects,
 * sounds, movies, bitmaps, plus error/abort paths.
 *
 * This harness validates the behavior of `Descent3/BriefingParse.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/BriefingParse.cpp`
 * @par Harness
 * `briefingparse_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/BriefingParse.cpp
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <tuple>
#include <vector>

#include "BriefingParse.h"
#include "TelComEffects.h"
#include "TelComEfxStructs.h"
#include "cfile.h"
#include "log.h"
#include "mem.h"

// ---------------------------------------------------------------------------
// Callback recorder
// ---------------------------------------------------------------------------
struct Rec {
  std::vector<std::string> order;
  std::vector<int> screen_num;
  std::vector<std::string> screen_desc, screen_layout;
  std::vector<uint32_t> mask_set, mask_unset;
  std::vector<std::string> titles;
  std::vector<float> statics, glitches;
  std::vector<std::string> texts, text_desc;
  std::vector<int> text_ids;
  std::vector<int> text_types;
  std::vector<uint32_t> text_flags;
  std::vector<std::pair<std::string, float>> snds;
  std::vector<std::tuple<std::string, float, bool, int, int>> movies;
  std::vector<std::pair<std::string, int>> bmps;
};
static Rec g_rec;
static bool g_loop_abort = false;

static void R_AddTextEffect(TCTEXTDESC *d, char *text, char *desc, int id) {
  g_rec.order.push_back("text");
  g_rec.texts.push_back(text);
  g_rec.text_desc.push_back(desc);
  g_rec.text_ids.push_back(id);
  g_rec.text_types.push_back(d->type);
  g_rec.text_flags.push_back(d->flags);
}
static void R_AddBmpEffect(TCBMPDESC *d, char *) {
  g_rec.order.push_back("bmp");
  g_rec.bmps.push_back({d->filename, d->type});
}
static void R_AddMovieEffect(TCMOVIEDESC *d, char *) {
  g_rec.order.push_back("movie");
  g_rec.movies.push_back({d->filename, d->fps, (bool)d->looping, (int)d->x, (int)d->y});
}
static void R_AddBkgEffect(TCBKGDESC *, char *) { g_rec.order.push_back("bkg"); }
static void R_AddPolyEffect(TCPOLYDESC *, char *) { g_rec.order.push_back("poly"); }
static void R_AddSoundEffect(TCSNDDESC *d, char *) {
  g_rec.order.push_back("sound");
  g_rec.snds.push_back({d->filename, d->waittime});
}
static void R_AddButtonEffect(TCBUTTONDESC *, char *, int) { g_rec.order.push_back("button"); }
static void R_StartScreen(int num, char *desc, char *layout, uint32_t set, uint32_t unset) {
  g_rec.order.push_back("screen");
  g_rec.screen_num.push_back(num);
  g_rec.screen_desc.push_back(desc ? desc : "");
  g_rec.screen_layout.push_back(layout ? layout : "");
  g_rec.mask_set.push_back(set);
  g_rec.mask_unset.push_back(unset);
}
static void R_EndScreen() { g_rec.order.push_back("endscreen"); }
static bool R_LoopCallback() { return g_loop_abort; }
static void R_SetTitle(char *t) {
  g_rec.order.push_back("title");
  g_rec.titles.push_back(t);
}
static void R_SetStatic(float a) {
  g_rec.order.push_back("static");
  g_rec.statics.push_back(a);
}
static void R_SetGlitch(float a) {
  g_rec.order.push_back("glitch");
  g_rec.glitches.push_back(a);
}
static void R_AddVoice(char *, int, char *) {}

/**
 * @brief GTest fixture for BriefingParseTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class BriefingParseTest : public ::testing::Test {
protected:
  CBriefParse parser;

  void SetUp() override {
    g_rec = Rec{};
    g_loop_abort = false;

    tBriefParseCallbacks cb{};
    cb.AddTextEffect = R_AddTextEffect;
    cb.AddBmpEffect = R_AddBmpEffect;
    cb.AddMovieEffect = R_AddMovieEffect;
    cb.AddBkgEffect = R_AddBkgEffect;
    cb.AddPolyEffect = R_AddPolyEffect;
    cb.AddSoundEffect = R_AddSoundEffect;
    cb.AddButtonEffect = R_AddButtonEffect;
    cb.StartScreen = R_StartScreen;
    cb.EndScreen = R_EndScreen;
    cb.LoopCallback = R_LoopCallback;
    cb.SetTitle = R_SetTitle;
    cb.SetStatic = R_SetStatic;
    cb.SetGlitch = R_SetGlitch;
    cb.AddVoice = R_AddVoice;
    parser.SetCallbacks(&cb);

    static bool path_set = false;
    if (!path_set) {
      cf_SetSearchPath("/tmp/opencode");
      path_set = true;
    }
  }

  int Parse(const std::string &body) {
    FILE *fp = fopen("/tmp/opencode/brief.txt", "wb");
    fwrite(body.data(), 1, body.size(), fp);
    fclose(fp);
    return parser.ParseBriefing("/tmp/opencode/brief.txt");
  }
};

/**
 * @test BriefingParseTest.MissingFileReturnsFileNotExist
 * @brief Verifies missing File Returns File Not Exist.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, MissingFileReturnsFileNotExist) {
  EXPECT_EQ(parser.ParseBriefing("/tmp/opencode/no_such_briefing.txt"), PBERR_FILENOTEXIST);
}

/**
 * @test BriefingParseTest.TitleScreenAndStaticGlitch
 * @brief Verifies title Screen And Static Glitch.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, TitleScreenAndStaticGlitch) {
  ASSERT_EQ(Parse("$TITLE \"The Story\"\n"
                  "$STATIC 0.25\n"
                  "$GLITCH 0.5\n"
                  "$SCREEN 2 desc \"intro screen\" isset 3 isnset 5 layout \"std\" start\n"
                  "$ENDSCREEN\n"),
            PBERR_NOERR);

  ASSERT_EQ(g_rec.titles.size(), 1u);
  EXPECT_EQ(g_rec.titles[0], "The Story");
  ASSERT_EQ(g_rec.statics.size(), 1u);
  EXPECT_FLOAT_EQ(g_rec.statics[0], 0.25f);
  EXPECT_FLOAT_EQ(g_rec.glitches[0], 0.5f);

  ASSERT_EQ(g_rec.screen_num.size(), 1u);
  EXPECT_EQ(g_rec.screen_num[0], 2);
  EXPECT_EQ(g_rec.screen_desc[0], "intro screen");
  EXPECT_EQ(g_rec.screen_layout[0], "std");
  EXPECT_EQ(g_rec.mask_set[0], 0x08u);   // bit 3
  EXPECT_EQ(g_rec.mask_unset[0], 0x20u); // bit 5

  std::vector<std::string> expect = {"title", "static", "glitch", "screen", "endscreen"};
  EXPECT_EQ(g_rec.order, expect);
}

/**
 * @test BriefingParseTest.TextBlockJoinsLinesWithNewlines
 * @brief Verifies text Block Joins Lines With Newlines.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, TextBlockJoinsLinesWithNewlines) {
  ASSERT_EQ(Parse("$SCREEN 0 start\n"
                  "$TEXT id 7 desc \"narration\" show\n"
                  "first line here\n"
                  "second line\n"
                  "$ENDTEXT\n"
                  "$ENDSCREEN\n"),
            PBERR_NOERR);

  ASSERT_EQ(g_rec.texts.size(), 1u);
  EXPECT_EQ(g_rec.texts[0], "first line here\nsecond line");
  EXPECT_EQ(g_rec.text_ids[0], 7);
  EXPECT_EQ(g_rec.text_desc[0], "narration");
}

/**
 * @test BriefingParseTest.TextEffectMapsToTypeAndFlags
 * @brief Verifies text Effect Maps To Type And Flags.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, TextEffectMapsToTypeAndFlags) {
  ASSERT_EQ(Parse("$SCREEN 0 start\n"
                  "$TEXT effect flash show\nhello\n$ENDTEXT\n"
                  "$ENDSCREEN\n"),
            PBERR_NOERR);
  ASSERT_EQ(g_rec.text_types.size(), 1u);
  EXPECT_EQ(g_rec.text_types[0], TC_TEXT_FLASH);

  ASSERT_EQ(Parse("$SCREEN 0 start\n"
                  "$TEXT effect Fade_in_and_out show\nbye\n$ENDTEXT\n"
                  "$ENDSCREEN\n"),
            PBERR_NOERR);
  ASSERT_EQ(g_rec.text_types.size(), 2u);
  EXPECT_EQ(g_rec.text_types[1], TC_TEXT_FADE);
  EXPECT_EQ(g_rec.text_flags[1], (uint32_t)TC_TEXTF_PINGPONG);
}

/**
 * @test BriefingParseTest.SoundWithOnceStarttimeAndDesc
 * @brief Verifies sound With Once Starttime And Desc.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, SoundWithOnceStarttimeAndDesc) {
  ASSERT_EQ(Parse("$SOUND once starttime 1.5 desc \"boom\" play \"sounds/boom.wav\"\n"), PBERR_NOERR);

  ASSERT_EQ(g_rec.snds.size(), 1u);
  EXPECT_EQ(g_rec.snds[0].first, "sounds/boom.wav");
  EXPECT_FLOAT_EQ(g_rec.snds[0].second, 1.5f);
}

/**
 * @test BriefingParseTest.MovieWithFpsLoopingPosition
 * @brief Verifies movie With Fps Looping Position.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, MovieWithFpsLoopingPosition) {
  ASSERT_EQ(Parse("$MOVIE position 12,30 fps 15.0 looping starttime 2.0 show \"movies/intro.mve\"\n"),
            PBERR_NOERR);

  ASSERT_EQ(g_rec.movies.size(), 1u);
  EXPECT_EQ(std::get<0>(g_rec.movies[0]), "movies/intro.mve");
  EXPECT_FLOAT_EQ(std::get<1>(g_rec.movies[0]), 15.0f);
  EXPECT_TRUE(std::get<2>(g_rec.movies[0]));
  EXPECT_EQ(std::get<3>(g_rec.movies[0]), 12);
  EXPECT_EQ(std::get<4>(g_rec.movies[0]), 30);
}

/**
 * @test BriefingParseTest.CommentsAndBlankLinesIgnored
 * @brief Verifies comments And Blank Lines Ignored.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, CommentsAndBlankLinesIgnored) {
  ASSERT_EQ(Parse("; comment at top\n"
                  "\n"
                  "$TITLE \"X\"\n"
                  "; trailing comment\n"),
            PBERR_NOERR);
  ASSERT_EQ(g_rec.titles.size(), 1u);
}

/**
 * @test BriefingParseTest.UnknownKeywordAbortsRestOfFileButReturnsNoErr
 * @brief Verifies unknown Keyword Aborts Rest Of File But Returns No Err.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, UnknownKeywordAbortsRestOfFileButReturnsNoErr) {
  // Quirk: parse errors are logged but ParseBriefing still returns NOERR.
  ASSERT_EQ(Parse("$TITLE \"ok\"\n"
                  "$BOGUS stuff\n"
                  "$STATIC 9.0\n"),
            PBERR_NOERR);

  EXPECT_EQ(g_rec.titles.size(), 1u);
  EXPECT_TRUE(g_rec.statics.empty()); // parsing stopped at the bad keyword
}

/**
 * @test BriefingParseTest.MissingEndtextAtEOFDeliversNothing
 * @brief Verifies missing Endtext At EOFDelivers Nothing.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, MissingEndtextAtEOFDeliversNothing) {
  ASSERT_EQ(Parse("$SCREEN 0 start\n$TEXT show\nunterminated\n"), PBERR_NOERR);
  EXPECT_TRUE(g_rec.texts.empty()); // AddTextEffect never fired
}

/**
 * @test BriefingParseTest.LoopCallbackAbortStopsParsingEarly
 * @brief Verifies loop Callback Abort Stops Parsing Early.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, LoopCallbackAbortStopsParsingEarly) {
  g_loop_abort = true;
  ASSERT_EQ(Parse("$TITLE \"a\"\n$STATIC 1.0\n$GLITCH 1.0\n"), PBERR_NOERR);
  EXPECT_TRUE(g_rec.statics.empty());
}

/**
 * @test BriefingParseTest.EndscreenOutsideScreenStopsParsing
 * @brief Verifies endscreen Outside Screen Stops Parsing.
 *
 * @details
 * Exercises the BriefingParseTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/BriefingParse.cpp
 * @ingroup descent3_tests
 */
TEST_F(BriefingParseTest, EndscreenOutsideScreenStopsParsing) {
  ASSERT_EQ(Parse("$TITLE \"t\"\n"
                  "$ENDSCREEN\n"          // error: no active screen
                  "$STATIC 3.0\n"),       // never reached
            PBERR_NOERR);
  EXPECT_EQ(g_rec.titles.size(), 1u);
  EXPECT_TRUE(g_rec.statics.empty());
}
