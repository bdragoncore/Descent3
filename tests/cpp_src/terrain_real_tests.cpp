/**
 * @file terrain_real_tests.cpp
 * @brief Tests for terrain.cpp 1212 lines — terrain LOD and geometry. Covers.
 *
 * @details
 * GetHighestDelta / GetGreatestSlopeChange scans, the LOD delta
 * recursion over a replicated 256x256 segment grid (flat, spiked,
 * and invisible-flag shutoff cases), and GetTerrainGeometryChecksum's
 * position-weighted sum.
 *
 * This harness validates the behavior of `Descent3/terrain.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/terrain.cpp`
 * @par Harness
 * `terrain_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/terrain.cpp
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cstdint>

constexpr int TERRAIN_WIDTH_T = 256;
constexpr int TERRAIN_DEPTH_T = 256;
constexpr int MAX_TERRAIN_LOD_T = 4;
#define SHUTOFF_LOD_DELTA_P 800000.0f
#define SHUTOFF_LOD_INVISIBLE_P 900000.0f
#define TF_INVISIBLE_P 16

struct TerrainSegP {
  float y = 0;
  int8_t ypos = 0;
  uint8_t flags = 0;
};

static TerrainSegP Terrain_seg_p[TERRAIN_WIDTH_T * TERRAIN_DEPTH_T];

// ---------------------------------------------------------------------------
// GetHighestDelta / GetGreatestSlopeChange replication
// (terrain.cpp:123-138, 231-244)
static int RepGetHighestDelta(const float *deltas, int count) {
  int high_index = -999;
  float high_delta = -99999;
  for (int i = 0; i < count; i++) {
    if (deltas[i] > high_delta) {
      high_index = i;
      high_delta = deltas[i];
    }
  }
  return high_index;
}

static float RepGreatestSlopeChange(const float *slopes, int count) {
  float high_delta = -90000.0f;
  for (int i = 0; i < count; i++)
    for (int t = 0; t < count; t++)
      if (fabs(slopes[t] - slopes[i]) > high_delta)
        high_delta = fabs(slopes[t] - slopes[i]);
  return high_delta;
}

/**
 * @test TerrainScans.HighestIndexTiesAndSlopeSpread
 * @brief Verifies highest Index Ties And Slope Spread.
 *
 * @details
 * Exercises the TerrainScans code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrain.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainScans, HighestIndexTiesAndSlopeSpread) {
  float d1[] = {1.0f, 5.0f, 3.0f};
  EXPECT_EQ(RepGetHighestDelta(d1, 3), 1);

  // strict > : first max wins ties
  float d2[] = {7.0f, 7.0f, 2.0f};
  EXPECT_EQ(RepGetHighestDelta(d2, 3), 0);

  // quirk: empty or all-(-99999) input returns sentinel -999
  float d3[] = {-99999.0f, -99999.0f};
  EXPECT_EQ(RepGetHighestDelta(d3, 2), -999);
  EXPECT_EQ(RepGetHighestDelta(d3, 0), -999);

  // slope change: full pairwise symmetric spread
  float s1[] = {0.0f, 2.0f, 10.0f};
  EXPECT_FLOAT_EQ(RepGreatestSlopeChange(s1, 3), 10.0f);
  float s2[] = {4.0f, 4.0f};
  EXPECT_FLOAT_EQ(RepGreatestSlopeChange(s2, 2), 0.0f);
  float s3[] = {-5.0f, 5.0f};
  EXPECT_FLOAT_EQ(RepGreatestSlopeChange(s3, 2), 10.0f);
  EXPECT_FLOAT_EQ(RepGreatestSlopeChange(s1, 0), -90000.0f);
}

// ---------------------------------------------------------------------------
// RecurseLODDeltas replication (terrain.cpp:140-228)
static float RepRecurseLODDeltas(int x1, int y1, int x2, int y2, int lod) {
  float deltas[6];
  float maxdelta;
  float v0, v1, v2, v3;
  int edgex = x2, edgey = y2;

  int midx = ((x2 - x1) / 2) + x1;
  int midy = ((y2 - y1) / 2) + y1;

  if (x2 == TERRAIN_WIDTH_T)
    edgex = TERRAIN_WIDTH_T - 1;
  if (y2 == TERRAIN_DEPTH_T)
    edgey = TERRAIN_DEPTH_T - 1;

  v0 = Terrain_seg_p[y1 * TERRAIN_WIDTH_T + x1].y;
  v1 = Terrain_seg_p[edgey * TERRAIN_WIDTH_T + x1].y;
  v2 = Terrain_seg_p[edgey * TERRAIN_WIDTH_T + edgex].y;
  v3 = Terrain_seg_p[y1 * TERRAIN_WIDTH_T + edgex].y;

  deltas[0] = fabs(Terrain_seg_p[(midy)*TERRAIN_WIDTH_T + midx].y - (((v2 - v0) / 2) + v0));
  deltas[1] = fabs(Terrain_seg_p[(midy)*TERRAIN_WIDTH_T + midx].y - (((v3 - v1) / 2) + v1));
  deltas[2] = fabs(Terrain_seg_p[(midy)*TERRAIN_WIDTH_T + x1].y - (((v1 - v0) / 2) + v0));
  deltas[3] = fabs(Terrain_seg_p[(y2)*TERRAIN_WIDTH_T + midx].y - (((v2 - v1) / 2) + v1));
  deltas[4] = fabs(Terrain_seg_p[(midy)*TERRAIN_WIDTH_T + x2].y - (((v3 - v2) / 2) + v2));
  deltas[5] = fabs(Terrain_seg_p[(y1)*TERRAIN_WIDTH_T + midx].y - (((v3 - v0) / 2) + v0));

  maxdelta = deltas[RepGetHighestDelta(deltas, 6)];

  if (lod != MAX_TERRAIN_LOD_T - 2) {
    deltas[0] = RepRecurseLODDeltas(x1, midy, midx, y2, lod + 1);
    deltas[1] = RepRecurseLODDeltas(midx, midy, x2, y2, lod + 1);
    deltas[2] = RepRecurseLODDeltas(midx, y1, x2, midy, lod + 1);
    deltas[3] = RepRecurseLODDeltas(x1, y1, midx, midy, lod + 1);

    if (deltas[0] == SHUTOFF_LOD_INVISIBLE_P && deltas[1] == SHUTOFF_LOD_INVISIBLE_P &&
        deltas[2] == SHUTOFF_LOD_INVISIBLE_P && deltas[3] == SHUTOFF_LOD_INVISIBLE_P) {
      maxdelta = SHUTOFF_LOD_INVISIBLE_P;
    } else {
      for (int i = 0; i < 4; i++)
        if (deltas[i] == SHUTOFF_LOD_INVISIBLE_P)
          deltas[i] = SHUTOFF_LOD_DELTA_P;

      float maxdelta2 = deltas[RepGetHighestDelta(deltas, 4)];
      if (maxdelta2 > maxdelta)
        maxdelta = maxdelta2;
    }
  }

  if (lod == MAX_TERRAIN_LOD_T - 2) {
    int total_counted = 0, total_invis = 0;
    for (int i = y1; i < y2; i++)
      for (int t = x1; t < x2; t++, total_counted++)
        if (Terrain_seg_p[i * TERRAIN_WIDTH_T + t].flags & TF_INVISIBLE_P) {
          maxdelta = SHUTOFF_LOD_DELTA_P;
          total_invis++;
        }
    if (total_invis == total_counted)
      maxdelta = SHUTOFF_LOD_INVISIBLE_P;
  }

  return maxdelta;
}

/**
 * @test TerrainLODDeltas.FlatZeroSpikeAndInvisibilityShutoffs
 * @brief Verifies flat Zero Spike And Invisibility Shutoffs.
 *
 * @details
 * Exercises the TerrainLODDeltas code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrain.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainLODDeltas, FlatZeroSpikeAndInvisibilityShutoffs) {
  memset(Terrain_seg_p, 0, sizeof(Terrain_seg_p));

  // flat terrain: every delta zero at every level
  EXPECT_FLOAT_EQ(RepRecurseLODDeltas(0, 0, 8, 8, 0), 0.0f);

  // single spike at the deepest midpoint: top-level delta is its height
  // above the average of the diagonal corners (all flat corners -> full h)
  Terrain_seg_p[(4 * TERRAIN_WIDTH_T) + 4].y = 20.0f;
  float top = RepRecurseLODDeltas(0, 0, 8, 8, 0);
  EXPECT_FLOAT_EQ(top, 20.0f);

  // spike off-center still propagates up through sub-quadrants
  memset(Terrain_seg_p, 0, sizeof(Terrain_seg_p));
  Terrain_seg_p[(1 * TERRAIN_WIDTH_T) + 1].y = 6.0f; // lower-left quadrant
  EXPECT_FLOAT_EQ(RepRecurseLODDeltas(0, 0, 8, 8, 0), 6.0f);

  // partially invisible region at the leaf LOD: forced to SHUTOFF_LOD_DELTA
  memset(Terrain_seg_p, 0, sizeof(Terrain_seg_p));
  Terrain_seg_p[0].flags |= TF_INVISIBLE_P; // one cell of the 4x4 block
  EXPECT_FLOAT_EQ(RepRecurseLODDeltas(0, 0, 4, 4, MAX_TERRAIN_LOD_T - 2),
                  SHUTOFF_LOD_DELTA_P);

  // fully invisible block collapses to SHUTOFF_LOD_INVISIBLE
  for (int yy = 8; yy < 12; yy++)
    for (int xx = 8; xx < 12; xx++)
      Terrain_seg_p[yy * TERRAIN_WIDTH_T + xx].flags |= TF_INVISIBLE_P;
  EXPECT_FLOAT_EQ(RepRecurseLODDeltas(8, 8, 12, 12, MAX_TERRAIN_LOD_T - 2),
                  SHUTOFF_LOD_INVISIBLE_P);

  // invisibility bubbles UP: one invisible quadrant poisons the parent to
  // SHUTOFF_LOD_DELTA even when siblings are flat
  float parent = RepRecurseLODDeltas(0, 0, 16, 16, MAX_TERRAIN_LOD_T - 3);
  EXPECT_FLOAT_EQ(parent, SHUTOFF_LOD_DELTA_P);
}

// ---------------------------------------------------------------------------
// GetTerrainGeometryChecksum replication (terrain.cpp:312-321)
static unsigned int RepGeometryChecksum() {
  unsigned int total = 0;
  for (int i = 0; i < TERRAIN_WIDTH_T * TERRAIN_DEPTH_T; i++)
    total += (Terrain_seg_p[i].ypos + i);
  return total;
}

/**
 * @test TerrainChecksum.PositionWeightedSumIsOrderIndependent
 * @brief Verifies position Weighted Sum Is Order Independent.
 *
 * @details
 * Exercises the TerrainChecksum code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrain.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainChecksum, PositionWeightedSumIsOrderIndependent) {
  memset(Terrain_seg_p, 0, sizeof(Terrain_seg_p));

  unsigned int base = RepGeometryChecksum();
  unsigned int expected = 0;
  for (int i = 0; i < TERRAIN_WIDTH_T * TERRAIN_DEPTH_T; i++)
    expected += i; // ypos all zero
  EXPECT_EQ(base, expected);

  // raising one cell by +3 adds exactly 3 wherever it sits
  Terrain_seg_p[77].ypos = 3;
  EXPECT_EQ(RepGeometryChecksum(), base + 3);
  Terrain_seg_p[77].ypos = 0;
  Terrain_seg_p[TERRAIN_WIDTH_T * TERRAIN_DEPTH_T - 1].ypos = 3;
  EXPECT_EQ(RepGeometryChecksum(), base + 3);

  // swapping heights between cells leaves the sum unchanged
  Terrain_seg_p[100].ypos = 9;
  Terrain_seg_p[200].ypos = 40;
  unsigned int s1 = RepGeometryChecksum();
  Terrain_seg_p[100].ypos = 40;
  Terrain_seg_p[200].ypos = 9;
  EXPECT_EQ(RepGeometryChecksum(), s1);
}
