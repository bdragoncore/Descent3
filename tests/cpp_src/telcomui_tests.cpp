/**
 * @file telcomui_tests.cpp
 * @brief Unit tests for the TelCom UI art-space scaling helpers (BUGFIX #685).
 *
 * @details
 * The TelCom UI (briefing screens, goals, cargo, automap) is authored for a
 * 640x480 screen. Descent3/telcomui.h provides the art <-> window coordinate
 * conversions applied at the draw and input boundaries. These tests pin the
 * conversion math: identity at the reference resolution, proportional scaling
 * at high resolutions, and the round-trip property of the inverse.
 *
 * @par Source
 * Descent3/telcomui.h
 * @par Harness
 * telcomui_tests.cpp
 * @par Framework
 * GoogleTest (gtest) — TEST macros
 *
 * @ingroup descent3_tests
 */

#include <gtest/gtest.h>

#include "grdefs.h"
#include "telcomui.h"

/**
 * @test TelcomUI.IdentityAtReferenceResolution
 * @brief At the 640x480 reference resolution the conversions are identity.
 *
 * @ingroup descent3_tests
 */
TEST(TelcomUI, IdentityAtReferenceResolution) {
  for (int v = -10; v <= 700; v += 17) {
    EXPECT_EQ(TelcomScaledX(v, FIXED_SCREEN_WIDTH), v);
    EXPECT_EQ(TelcomScaledY(v, FIXED_SCREEN_HEIGHT), v);
    EXPECT_EQ(TelcomUnscaledX(v, FIXED_SCREEN_WIDTH), v);
    EXPECT_EQ(TelcomUnscaledY(v, FIXED_SCREEN_HEIGHT), v);
  }
}

/**
 * @test TelcomUI.ScalesProportionally
 * @brief Art coordinates scale proportionally with the window size.
 *
 * @details
 * At 1280x960 (2x) an art coordinate doubles; at 1920x1440 (3x) it triples.
 * @ingroup descent3_tests
 */
TEST(TelcomUI, ScalesProportionally) {
  // 2x window
  EXPECT_EQ(TelcomScaledX(320, 1280), 640);
  EXPECT_EQ(TelcomScaledY(240, 960), 480);
  EXPECT_EQ(TelcomScaledX(640, 1280), 1280);
  EXPECT_EQ(TelcomScaledY(480, 960), 960);
  // 3x window
  EXPECT_EQ(TelcomScaledX(100, 1920), 300);
  EXPECT_EQ(TelcomScaledY(175, 1440), 525);
  // 16:9 widescreen window: x and y scale independently
  EXPECT_EQ(TelcomScaledX(640, 3440), 3440);
  EXPECT_EQ(TelcomScaledY(480, 1440), 1440);
  EXPECT_EQ(TelcomScaledY(175, 1440), 175 * 3);
}

/**
 * @test TelcomUI.UnscaleIsInverseOfScale
 * @brief Unscaling a scaled window coordinate returns the art coordinate.
 *
 * @details
 * The round trip is exact when the art coordinate is a multiple of the scale
 * factor; the mouse hit-testing relies on this within a pixel.
 * @ingroup descent3_tests
 */
TEST(TelcomUI, UnscaleIsInverseOfScale) {
  // exact round trip for multiples of the scale factor
  for (int art = 0; art <= FIXED_SCREEN_WIDTH; art += 10) {
    int window = TelcomScaledX(art, 1280);
    EXPECT_EQ(TelcomUnscaledX(window, 1280), art);
  }
  for (int art = 0; art <= FIXED_SCREEN_HEIGHT; art += 10) {
    int window = TelcomScaledY(art, 1440);
    EXPECT_EQ(TelcomUnscaledY(window, 1440), art);
  }
}

/**
 * @test TelcomUI.UnscaleMapsWindowCornersToArtBounds
 * @brief Window corners map to the art-space bounds.
 *
 * @ingroup descent3_tests
 */
TEST(TelcomUI, UnscaleMapsWindowCornersToArtBounds) {
  EXPECT_EQ(TelcomUnscaledX(0, 3440), 0);
  EXPECT_EQ(TelcomUnscaledX(3439, 3440), FIXED_SCREEN_WIDTH - 1);
  EXPECT_EQ(TelcomUnscaledY(1439, 1440), FIXED_SCREEN_HEIGHT - 1);
}
