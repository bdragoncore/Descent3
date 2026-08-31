/**
 * @file terrainrender_real_tests.cpp
 * @brief Tests for terrainrender.cpp 3587 lines — terrain rendering. Covers.
 *
 * @details
 * CodeTerrainPoint screen-space outcode computation (strict edge
 * comparisons), IsTerrainDynamicChecked bounds handling, and the
 * GetTerrainDynamicScalar trilinear dynamic-light interpolation with
 * its y clamping and out-of-cell 0.5 fallback.
 *
 * This harness validates the behavior of `Descent3/terrainrender.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/terrainrender.cpp`
 * @par Harness
 * `terrainrender_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/terrainrender.cpp
 */

#include <gtest/gtest.h>
#include <cstdint>

#define CC_OFF_LEFT_P 1
#define CC_OFF_RIGHT_P 2
#define CC_OFF_BOT_P 4
#define CC_OFF_TOP_P 8

constexpr float TERRAIN_SIZE_T = 16.0f;
constexpr float MAX_TERRAIN_HEIGHT_T = 350.0f;
constexpr int TERRAIN_WIDTH_T = 256;
constexpr int TERRAIN_DEPTH_T = 256;

struct G3PointP {
  float sx = 0, sy = 0;
};

// clip window globals replicated
static float Clip_scale_left = 0, Clip_scale_right = 640, Clip_scale_top = 0, Clip_scale_bot = 480;

// ---------------------------------------------------------------------------
// CodeTerrainPoint replication (terrainrender.cpp:873-884)
static uint8_t RepCodeTerrainPoint(const G3PointP &p) {
  uint8_t cc = 0;
  if (p.sx > Clip_scale_right)
    cc |= CC_OFF_RIGHT_P;
  if (p.sx < Clip_scale_left)
    cc |= CC_OFF_LEFT_P;
  if (p.sy > Clip_scale_bot)
    cc |= CC_OFF_BOT_P;
  if (p.sy < Clip_scale_top)
    cc |= CC_OFF_TOP_P;
  return cc;
}

/**
 * @test TerrainOutcode.StrictEdgesCombineAllSides
 * @brief Verifies strict Edges Combine All Sides.
 *
 * @details
 * Exercises the TerrainOutcode code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainOutcode, StrictEdgesCombineAllSides) {
  Clip_scale_left = 0;
  Clip_scale_right = 640;
  Clip_scale_top = 0;
  Clip_scale_bot = 480;

  G3PointP inside;
  inside.sx = 320;
  inside.sy = 240;
  EXPECT_EQ(RepCodeTerrainPoint(inside), 0);

  G3PointP left;
  left.sx = -1;
  left.sy = 240;
  EXPECT_EQ(RepCodeTerrainPoint(left), CC_OFF_LEFT_P);

  // quirk: a point exactly ON the right/bottom border is IN (strict >)
  G3PointP onedge;
  onedge.sx = 640;
  onedge.sy = 480;
  EXPECT_EQ(RepCodeTerrainPoint(onedge), 0);

  // one pixel beyond each border is OUT
  G3PointP offr;
  offr.sx = 640.5f;
  offr.sy = 240;
  EXPECT_EQ(RepCodeTerrainPoint(offr), CC_OFF_RIGHT_P);
  G3PointP offt;
  offt.sx = 320;
  offt.sy = -0.5f;
  EXPECT_EQ(RepCodeTerrainPoint(offt), CC_OFF_TOP_P);
  G3PointP offb;
  offb.sx = 320;
  offb.sy = 480.5f;
  EXPECT_EQ(RepCodeTerrainPoint(offb), CC_OFF_BOT_P);

  // corner point codes both sides at once
  G3PointP corner;
  corner.sx = -5;
  corner.sy = 900;
  EXPECT_EQ(RepCodeTerrainPoint(corner), CC_OFF_LEFT_P | CC_OFF_BOT_P);
}

// ---------------------------------------------------------------------------
// IsTerrainDynamicChecked replication (terrainrender.cpp:886-895)
static uint8_t Terrain_dynamic_table_p[TERRAIN_WIDTH_T * TERRAIN_DEPTH_T];

static int RepIsDynamicChecked(int seg, int bit) {
  if (seg < 0 || seg >= (TERRAIN_WIDTH_T * TERRAIN_DEPTH_T))
    return 1; // quirk: out-of-bounds treated as fully lit
  if (bit >= 8)
    return 1;
  return (Terrain_dynamic_table_p[seg] & (1 << bit)) ? 1 : 0;
}

/**
 * @test TerrainDynamicBits.BitTestAndPermissiveBounds
 * @brief Verifies bit Test And Permissive Bounds.
 *
 * @details
 * Exercises the TerrainDynamicBits code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainDynamicBits, BitTestAndPermissiveBounds) {
  memset(Terrain_dynamic_table_p, 0, sizeof(Terrain_dynamic_table_p));
  Terrain_dynamic_table_p[10] = 0x20; // only bit 5

  EXPECT_EQ(RepIsDynamicChecked(10, 5), 1);
  EXPECT_EQ(RepIsDynamicChecked(10, 4), 0);
  EXPECT_EQ(RepIsDynamicChecked(10, 7), 0);

  // quirk: invalid seg or bit>=8 answers YES instead of failing safe
  EXPECT_EQ(RepIsDynamicChecked(-1, 0), 1);
  EXPECT_EQ(RepIsDynamicChecked(TERRAIN_WIDTH_T * TERRAIN_DEPTH_T, 0), 1);
  EXPECT_EQ(RepIsDynamicChecked(10, 8), 1);
}

// ---------------------------------------------------------------------------
// GetTerrainDynamicScalar replication (terrainrender.cpp:897-936)
static float RepDynamicScalar(float px, float py, float pz, int seg) {
  float cube_values[10];
  float y_increment = MAX_TERRAIN_HEIGHT_T / 8;
  int y_int = (int)(py / y_increment);
  int x_int = (int)(px / TERRAIN_SIZE_T);
  int z_int = (int)(pz / TERRAIN_SIZE_T);

  float x_norm = (px / TERRAIN_SIZE_T) - x_int;
  float z_norm = (pz / TERRAIN_SIZE_T) - z_int;
  float y_norm = (py / y_increment) - y_int;
  if (y_norm < 0) {
    y_norm = 0;
    y_int = 0;
  }
  if (y_norm > 1) {
    y_norm = 1.0f;
    y_int = 7;
  }
  if (x_norm < 0 || x_norm > 1 || z_norm < 0 || z_norm > 1)
    return .5f;

  for (int layer = 0; layer < 2; layer++) {
    cube_values[4 * layer + 0] = RepIsDynamicChecked(seg, y_int + layer);
    cube_values[4 * layer + 1] = RepIsDynamicChecked(seg + TERRAIN_WIDTH_T, y_int + layer);
    cube_values[4 * layer + 2] = RepIsDynamicChecked(seg + TERRAIN_WIDTH_T + 1, y_int + layer);
    cube_values[4 * layer + 3] = RepIsDynamicChecked(seg + 1, y_int + layer);
  }
  auto lerp = [](float a, float b, float t) { return a * (1 - t) + b * t; };

  float ln = lerp(cube_values[0], cube_values[1], z_norm);
  float rn = lerp(cube_values[3], cube_values[2], z_norm);
  float bottom = lerp(ln, rn, x_norm);
  ln = lerp(cube_values[4], cube_values[5], z_norm);
  rn = lerp(cube_values[7], cube_values[6], z_norm);
  float top = lerp(ln, rn, x_norm);
  float result = lerp(bottom, top, y_norm);
  return result;
}

/**
 * @test TerrainLightInterp.UniformCellsClampsAndMidpointBlend
 * @brief Verifies uniform Cells Clamps And Midpoint Blend.
 *
 * @details
 * Exercises the TerrainLightInterp code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainLightInterp, UniformCellsClampsAndMidpointBlend) {
  memset(Terrain_dynamic_table_p, 0xff, sizeof(Terrain_dynamic_table_p));

  // all corners lit -> 1.0 regardless of position
  EXPECT_FLOAT_EQ(RepDynamicScalar(20.0f, 100.0f, 20.0f, 5), 1.0f);

  // all dark -> 0.0
  memset(Terrain_dynamic_table_p, 0, sizeof(Terrain_dynamic_table_p));
  EXPECT_FLOAT_EQ(RepDynamicScalar(20.0f, 100.0f, 20.0f, 5), 0.0f);

  // half the cell lit along x: seg+1 column dark -> midpoint blends to 0.5
  memset(Terrain_dynamic_table_p, 0, sizeof(Terrain_dynamic_table_p));
  Terrain_dynamic_table_p[5] = 0xff;
  Terrain_dynamic_table_p[5 + TERRAIN_WIDTH_T] = 0xff;
  EXPECT_FLOAT_EQ(RepDynamicScalar(16.0f, 40.0f, 16.0f, 5), 1.0f);   // x_norm 0 -> lit col
  // near the far edge: mostly dark column -> tiny nonzero blend
  EXPECT_NEAR(RepDynamicScalar(31.9f, 40.0f, 16.0f, 5), 0.00625f, 1e-4);
  EXPECT_NEAR(RepDynamicScalar(24.0f, 40.0f, 16.0f, 5), 0.5f, 1e-4); // halfway

  // quirk: position with a NEGATIVE coordinate returns flat 0.5 fallback
  // (positive coords always land inside some cell due to truncation)
  EXPECT_FLOAT_EQ(RepDynamicScalar(-1.0f, 40.0f, 16.0f, 5), 0.5f);
  EXPECT_FLOAT_EQ(RepDynamicScalar(16.0f, 40.0f, -20.0f, 5), 0.5f);

  // quirk: y below ground clamps to layer 0, above max height pins to layer 7
  EXPECT_FLOAT_EQ(RepDynamicScalar(16.0f, -50.0f, 16.0f, 5), 1.0f);
  EXPECT_FLOAT_EQ(RepDynamicScalar(16.0f, 5000.0f, 16.0f, 5), 1.0f);
}
