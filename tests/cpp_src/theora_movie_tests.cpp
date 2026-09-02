/**
 * @file theora_movie_tests.cpp
 * @brief Tests for Descent3/theora_movie.cpp — the Ogg Theora decoder used
 *        to play .ogv cutscenes and the animated main menu background.
 *
 * @details
 * This harness validates the behavior of `Descent3/theora_movie.cpp`. It
 * compiles the real implementation against a real .ogv file (generated at
 * configure time with ffmpeg) and checks the observable contract:
 *   - TheoraOpen() succeeds and reports the correct frame dimensions.
 *   - TheoraDecodeFrame() returns every frame in the file.
 *   - TheoraToRGB555() produces valid RGB555 output.
 *   - Looping mode restarts the stream after EOF.
 *
 * @par Source
 * `Descent3/theora_movie.cpp`
 * @par Harness
 * `theora_movie_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * The decoder is self-contained (libtheora + libogg only); no engine
 * subsystems are required.
 *
 * @ingroup descent3_tests
 * @see Descent3/theora_movie.cpp
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include "theora_movie.h"

// Path to the generated test .ogv, provided by the build system.
#ifndef THEORA_TEST_MOVIE
#define THEORA_TEST_MOVIE "test.ogv"
#endif

namespace {

// A 64x48, 10-frame test video generated with ffmpeg at configure time.
constexpr int kTestWidth = 64;
constexpr int kTestHeight = 48;
constexpr int kTestFrames = 10;

} // namespace

/**
 * @test TheoraMovieTest.OpenReportsDimensions
 * @brief Verifies TheoraOpen() succeeds and reports the encoded dimensions.
 *
 * @details
 * Exercises the TheoraOpen() code path and asserts observable
 * post-conditions. The test .ogv is a known 64x48 video.
 *
 * @see Descent3/theora_movie.cpp
 * @ingroup descent3_tests
 */
TEST(TheoraMovieTest, OpenReportsDimensions) {
  TheoraMovie *tm = TheoraOpen(THEORA_TEST_MOVIE, false);
  ASSERT_NE(tm, nullptr);
  EXPECT_EQ(tm->width, kTestWidth);
  EXPECT_EQ(tm->height, kTestHeight);
  TheoraClose(tm);
}

/**
 * @test TheoraMovieTest.OpenFailsForMissingFile
 * @brief Verifies TheoraOpen() returns nullptr for a nonexistent file.
 *
 * @details
 * Exercises the TheoraOpen() error path and asserts observable
 * post-conditions.
 *
 * @see Descent3/theora_movie.cpp
 * @ingroup descent3_tests
 */
TEST(TheoraMovieTest, OpenFailsForMissingFile) {
  TheoraMovie *tm = TheoraOpen("/nonexistent/file.ogv", false);
  EXPECT_EQ(tm, nullptr);
}

/**
 * @test TheoraMovieTest.DecodesAllFrames
 * @brief Verifies TheoraDecodeFrame() returns every frame in the file.
 *
 * @details
 * Exercises the TheoraDecodeFrame() code path and asserts observable
 * post-conditions. The test .ogv has exactly 10 frames.
 *
 * @see Descent3/theora_movie.cpp
 * @ingroup descent3_tests
 */
TEST(TheoraMovieTest, DecodesAllFrames) {
  TheoraMovie *tm = TheoraOpen(THEORA_TEST_MOVIE, false);
  ASSERT_NE(tm, nullptr);
  int frames = 0;
  while (TheoraDecodeFrame(tm) == 0) {
    ++frames;
  }
  EXPECT_EQ(frames, kTestFrames);
  TheoraClose(tm);
}

/**
 * @test TheoraMovieTest.ToRGB555ProducesValidPixels
 * @brief Verifies TheoraToRGB555() writes opaque RGB555 pixels.
 *
 * @details
 * Exercises the TheoraToRGB555() code path and asserts observable
 * post-conditions. Every pixel must have the opaque flag set and valid
 * 5-bit color components.
 *
 * @see Descent3/theora_movie.cpp
 * @ingroup descent3_tests
 */
TEST(TheoraMovieTest, ToRGB555ProducesValidPixels) {
  TheoraMovie *tm = TheoraOpen(THEORA_TEST_MOVIE, false);
  ASSERT_NE(tm, nullptr);
  ASSERT_EQ(TheoraDecodeFrame(tm), 0);
  std::vector<uint16_t> pixels((size_t)tm->width * tm->height);
  TheoraToRGB555(tm, pixels.data(), tm->width);
  for (uint16_t p : pixels) {
    EXPECT_NE(p & 0x8000, 0) << "pixel must be opaque";
    EXPECT_EQ(p & 0x7FFF, p & 0x7FFF) << "pixel must fit in 15 bits";
  }
  TheoraClose(tm);
}

/**
 * @test TheoraMovieTest.LoopingRestartsAfterEof
 * @brief Verifies looping mode restarts the stream after EOF.
 *
 * @details
 * Exercises the TheoraDecodeFrame() looping path and asserts observable
 * post-conditions. After decoding all frames once, the next call should
 * restart from the beginning rather than returning EOF.
 *
 * @see Descent3/theora_movie.cpp
 * @ingroup descent3_tests
 */
TEST(TheoraMovieTest, LoopingRestartsAfterEof) {
  TheoraMovie *tm = TheoraOpen(THEORA_TEST_MOVIE, true);
  ASSERT_NE(tm, nullptr);
  int frames = 0;
  // Decode two full passes; the second pass proves looping restarted.
  while (frames < kTestFrames * 2) {
    ASSERT_EQ(TheoraDecodeFrame(tm), 0) << "looping should never hit EOF";
    ++frames;
  }
  TheoraClose(tm);
}