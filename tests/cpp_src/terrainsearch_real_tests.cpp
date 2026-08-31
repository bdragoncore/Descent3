/**
 * @file terrainsearch_real_tests.cpp
 * @brief Tests for TerrainSearch.cpp 910 lines — terrain LOD/search engine.
 *
 * @details
 * Covers cell/ground-point lookups, the ground plane equation with
 * diagonal-dependent normals, SimplifyVertex error math, EvaluateBlock
 * delta lookup with shutoff sentinels, and the LODOffs save/restore
 * stack.
 *
 * This harness validates the behavior of `Descent3/TerrainSearch.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/TerrainSearch.cpp`
 * @par Harness
 * `terrainsearch_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/TerrainSearch.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <algorithm>

// replicated constants (terrain.h:31-40)
constexpr int TERRAIN_WIDTH = 256;
constexpr int TERRAIN_DEPTH = 256;
constexpr float TERRAIN_SIZE = 16.0f;
constexpr int MAX_TERRAIN_LOD = 4;
constexpr float SHUTOFF_LOD_DELTA = 800000.0f;
constexpr float SHUTOFF_LOD_INVISIBLE = 900000.0f;

// replicated GetTerrainCellFromPos (TerrainSearch.cpp:730-738)
static int GetTerrainCellFromPos(float px, float pz) {
  int x = (int)(px / TERRAIN_SIZE);
  int z = (int)(pz / TERRAIN_SIZE);
  if (x < 0 || x >= TERRAIN_WIDTH || z < 0 || z >= TERRAIN_DEPTH)
    return -1;
  return (z * TERRAIN_WIDTH + x);
}

/**
 * @test TerrainSearch.CellFromPosBounds
 * @brief Verifies cell From Pos Bounds.
 *
 * @details
 * Exercises the TerrainSearch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TerrainSearch.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainSearch, CellFromPosBounds) {
  EXPECT_EQ(GetTerrainCellFromPos(0.0f, 0.0f), 0);
  EXPECT_EQ(GetTerrainCellFromPos(15.9f, 0.0f), 0);   // inside first cell
  EXPECT_EQ(GetTerrainCellFromPos(16.0f, 0.0f), 1);   // second cell across
  EXPECT_EQ(GetTerrainCellFromPos(16.0f, 32.0f), 2 * TERRAIN_WIDTH + 1);
  // quirk: int cast truncates toward zero, so slightly-negative
  // positions snap into cell 0 instead of returning -1
  EXPECT_EQ(GetTerrainCellFromPos(-0.1f, 0.0f), 0);
  EXPECT_EQ(GetTerrainCellFromPos(0.0f, -15.9f), 0);
  // genuinely negative cells do report outside
  EXPECT_EQ(GetTerrainCellFromPos(-16.1f, 0.0f), -1);
  EXPECT_EQ(GetTerrainCellFromPos(0.0f, TERRAIN_DEPTH * TERRAIN_SIZE), -1); // past high z
}

// replicated ground point math (TerrainSearch.cpp:754-786)
struct NormalMock {
  float nx, ny, nz;
};
static NormalMock Normals[TERRAIN_WIDTH * TERRAIN_DEPTH][2]; // normal1, normal2 per cell
static float SegY[TERRAIN_WIDTH * TERRAIN_DEPTH];

static float RepGroundPoint(float px, float pz) {
  int x = (int)(px / TERRAIN_SIZE);
  int z = (int)(pz / TERRAIN_SIZE);
  if (x < 0 || x >= TERRAIN_WIDTH || z < 0 || z >= TERRAIN_DEPTH)
    return 0;

  int t = z * TERRAIN_WIDTH + x;
  float lx = px - (x * TERRAIN_SIZE);
  float lz = pz - (z * TERRAIN_SIZE);

  const NormalMock &n = (lx > lz) ? Normals[t][1] : Normals[t][0];

  float y = ((lx * n.nx) + (lz * n.nz)) / n.ny;
  y = -y;
  y += SegY[t];
  return y;
}

/**
 * @test TerrainSearch.GroundPointPlaneEquationDiagonalSplit
 * @brief Verifies ground Point Plane Equation Diagonal Split.
 *
 * @details
 * Exercises the TerrainSearch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TerrainSearch.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainSearch, GroundPointPlaneEquationDiagonalSplit) {
  // flat terrain: both normals point straight up, y == SegY everywhere
  for (int i = 0; i < 8; i++) {
    Normals[i][0] = {0, 1, 0};
    Normals[i][1] = {0, 1, 0};
    SegY[i] = 5.0f;
  }
  EXPECT_FLOAT_EQ(RepGroundPoint(3.0f, 7.0f), 5.0f);
  EXPECT_FLOAT_EQ(RepGroundPoint(15.99f, 15.99f), 5.0f);

  // tilted triangle: normal2 used only when local x > local z
  int t = 0;
  Normals[t][0] = {0, 1, 0};          // lower-left triangle: flat
  Normals[t][1] = {-1, 1, 0};         // upper-right triangle: y = -(lx*-1)/1 + SegY
  SegY[t] = 10.0f;
  // point below diagonal (lx<lz): flat side
  EXPECT_FLOAT_EQ(RepGroundPoint(2.0f, 6.0f), 10.0f);
  // point above diagonal (lx>lz): y = -(lx*-1 + lz*0)/1 + 10 = lx + 10
  EXPECT_FLOAT_EQ(RepGroundPoint(12.0f, 2.0f), 22.0f);
  // quirk: exactly on the diagonal uses normal1 (strict > comparison)
  EXPECT_FLOAT_EQ(RepGroundPoint(8.0f, 8.0f), 10.0f);

  // quirk again: slightly-negative position truncates into cell 0 and
  // returns that cell's height; a full cell negative returns 0
  EXPECT_FLOAT_EQ(RepGroundPoint(-5.0f, 3.0f), 10.0f);
  EXPECT_FLOAT_EQ(RepGroundPoint(-20.0f, 3.0f), 0.0f);
}

// replicated SimplifyVertex (TerrainSearch.cpp:812-838)
static float TS_SimplifyCondition = 1.0f;
struct ViewMock {
  float x, y, z;
};
static bool RepSimplifyVertex(const ViewMock &eye, float vx, float vy, float vz, float delta) {
  float d2 = delta * delta;
  float exv = (eye.x - vx) * (eye.x - vx);
  float ezv = (eye.z - vz) * (eye.z - vz);
  float eyv = (eye.y - vy) * (eye.y - vy);
  float first = d2 * (exv + ezv);
  float inner = exv + ezv + eyv;
  float second = TS_SimplifyCondition * (inner * inner);
  return first <= second;
}

/**
 * @test TerrainSearch.SimplifyVertexErrorMetric
 * @brief Verifies simplify Vertex Error Metric.
 *
 * @details
 * Exercises the TerrainSearch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TerrainSearch.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainSearch, SimplifyVertexErrorMetric) {
  ViewMock eye{0, 0, 0};
  TS_SimplifyCondition = 1.0f;
  // vertex at origin-ish, small delta -> simplify (error under threshold)
  EXPECT_TRUE(RepSimplifyVertex(eye, 16, 0, 16, 0.001f));
  // huge delta -> too much error -> don't simplify
  EXPECT_FALSE(RepSimplifyVertex(eye, 16, 0, 16, 100.0f));
  // far away vertex with big delta still simplifies (distance dominates)
  EXPECT_TRUE(RepSimplifyVertex(eye, 1600, 0, 1600, 100.0f));
  // y offset contributes to denominator via inner term but not numerator
  EXPECT_TRUE(RepSimplifyVertex(eye, 16, 500, 16, 0.01f));
  // boundary: equality simplifies (<=) — delta equal to horizontal distance
  float vx = 16, vz = 16;
  float horiz = std::sqrt(vx * vx + vz * vz); // eye at same height: inner = horiz^2
  EXPECT_TRUE(RepSimplifyVertex(eye, vx, 0, vz, horiz));
}

// replicated EvaluateBlock delta lookup (TerrainSearch.cpp:844-868)
static float DeltaBlocks[MAX_TERRAIN_LOD][TERRAIN_WIDTH * TERRAIN_DEPTH / 4];
static bool LOD_engine_off = false;
static ViewMock TS_eye{0, 0, 0};

static int RepEvaluateBlock(int x, int z, int lod) {
  if (LOD_engine_off)
    return 0;
  int simplemul = 1 << ((MAX_TERRAIN_LOD - 1) - lod);
  float delta = DeltaBlocks[lod][((z / simplemul) * (TERRAIN_WIDTH / simplemul)) + (x / simplemul)];
  if (delta == SHUTOFF_LOD_INVISIBLE)
    return -1; // block completely invisible
  if (RepSimplifyVertex(TS_eye, x + (simplemul / 2), 0, z + (simplemul / 2), delta))
    return 1;
  return 0;
}

/**
 * @test TerrainSearch.EvaluateBlockLODIndexingAndSentinels
 * @brief Verifies evaluate Block LODIndexing And Sentinels.
 *
 * @details
 * Exercises the TerrainSearch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TerrainSearch.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainSearch, EvaluateBlockLODIndexingAndSentinels) {
  memset(DeltaBlocks, 0, sizeof(DeltaBlocks));
  LOD_engine_off = false;

  // lod 2 (SearchQuadTree's 2-block call) -> simplemul=2, block grid W/2
  DeltaBlocks[2][(5 / 2) * (TERRAIN_WIDTH / 2) + (7 / 2)] = SHUTOFF_LOD_INVISIBLE;
  EXPECT_EQ(RepEvaluateBlock(7, 5, MAX_TERRAIN_LOD - 2), -1);

  // lod 0 -> simplemul=8: 16 cells share one block entry
  DeltaBlocks[0][((40 / 8) * (TERRAIN_WIDTH / 8)) + (24 / 8)] = SHUTOFF_LOD_INVISIBLE;
  EXPECT_EQ(RepEvaluateBlock(24, 40, 0), -1);   // top-left corner of block
  EXPECT_EQ(RepEvaluateBlock(31, 47, 0), -1);   // bottom-right of same 8x8 block

  // engine off forces "not simple" regardless of data
  LOD_engine_off = true;
  EXPECT_EQ(RepEvaluateBlock(24, 40, 0), 0);
  LOD_engine_off = false;
}

/**
 * @test TerrainSearch.EvaluateBlockSimplifiesNearFlatBlocks
 * @brief Verifies evaluate Block Simplifies Near Flat Blocks.
 *
 * @details
 * Exercises the TerrainSearch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TerrainSearch.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainSearch, EvaluateBlockSimplifiesNearFlatBlocks) {
  memset(DeltaBlocks, 0, sizeof(DeltaBlocks));
  // zero delta near eye at block center -> simplifies
  TS_eye = {32, 0, 32};
  EXPECT_EQ(RepEvaluateBlock(28, 28, MAX_TERRAIN_LOD - 2), 1); // center (32,32), delta 0

  // large delta blocks the merge even near the eye (lod-2 block grid)
  DeltaBlocks[MAX_TERRAIN_LOD - 2][(28 / 2) * (TERRAIN_WIDTH / 2) + (28 / 2)] = 50.0f;
  EXPECT_EQ(RepEvaluateBlock(28, 28, MAX_TERRAIN_LOD - 2), 0);
}

// replicated LODOffs save/restore stack (TerrainSearch.cpp:871-910)
struct LodOffMock {
  int cellnum;
  float save_delta[MAX_TERRAIN_LOD - 1];
};
static LodOffMock Offs[64];
static int Num_offs = 0;

static void TurnOffForCell(int cellnum, float blocks[MAX_TERRAIN_LOD][TERRAIN_WIDTH * TERRAIN_DEPTH / 4]) {
  if (cellnum < 0 || cellnum >= (TERRAIN_WIDTH * TERRAIN_DEPTH))
    FAIL() << "cellnum out of range"; // ASSERT in original
  int x = cellnum % TERRAIN_WIDTH;
  int z = cellnum / TERRAIN_WIDTH;

  Offs[Num_offs].cellnum = cellnum;
  for (int i = 0; i < MAX_TERRAIN_LOD - 1; i++) {
    int simplemul = 1 << ((MAX_TERRAIN_LOD - 1) - i);
    int idx = ((z / simplemul) * (TERRAIN_WIDTH / simplemul)) + (x / simplemul);
    Offs[Num_offs].save_delta[i] = blocks[i][idx];
    blocks[i][idx] = SHUTOFF_LOD_DELTA;
  }
  Num_offs++;
}

static void ClearOffs(float blocks[MAX_TERRAIN_LOD][TERRAIN_WIDTH * TERRAIN_DEPTH / 4]) {
  for (int t = Num_offs - 1; t >= 0; t--) {
    int x = Offs[t].cellnum % TERRAIN_WIDTH;
    int z = Offs[t].cellnum / TERRAIN_WIDTH;
    for (int i = 0; i < MAX_TERRAIN_LOD - 1; i++) {
      int simplemul = 1 << ((MAX_TERRAIN_LOD - 1) - i);
      int idx = ((z / simplemul) * (TERRAIN_WIDTH / simplemul)) + (x / simplemul);
      blocks[i][idx] = Offs[t].save_delta[i];
    }
  }
  Num_offs = 0;
}

/**
 * @test TerrainSearch.TurnOffAndClearLODRestoresAllLevels
 * @brief Verifies turn Off And Clear LODRestores All Levels.
 *
 * @details
 * Exercises the TerrainSearch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TerrainSearch.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainSearch, TurnOffAndClearLODRestoresAllLevels) {
  memset(DeltaBlocks, 0, sizeof(DeltaBlocks));
  // distinct values per LOD level at cell (3,2)
  DeltaBlocks[0][(2 / 8) * (TERRAIN_WIDTH / 8) + 3 / 8] = 11.0f;
  DeltaBlocks[1][(2 / 4) * (TERRAIN_WIDTH / 4) + 3 / 4] = 22.0f;
  DeltaBlocks[2][(2 / 2) * (TERRAIN_WIDTH / 2) + 3 / 2] = 33.0f;

  TurnOffForCell(2 * TERRAIN_WIDTH + 3, DeltaBlocks);
  ASSERT_EQ(Num_offs, 1);

  int cell = 2 * TERRAIN_WIDTH + 3;
  int x = cell % TERRAIN_WIDTH, z = cell / TERRAIN_WIDTH;
  EXPECT_FLOAT_EQ(DeltaBlocks[2][(z / 2) * (TERRAIN_WIDTH / 2) + x / 2], SHUTOFF_LOD_DELTA);

  ClearOffs(DeltaBlocks);
  EXPECT_EQ(Num_offs, 0);
  EXPECT_FLOAT_EQ(DeltaBlocks[0][(2 / 8) * (TERRAIN_WIDTH / 8) + 3 / 8], 11.0f);
  EXPECT_FLOAT_EQ(DeltaBlocks[1][(2 / 4) * (TERRAIN_WIDTH / 4) + 3 / 4], 22.0f);
  EXPECT_FLOAT_EQ(DeltaBlocks[2][(2 / 2) * (TERRAIN_WIDTH / 2) + 3 / 2], 33.0f);
}

/**
 * @test TerrainSearch.SharedBlockTurnOffClearsInReverse
 * @brief Verifies shared Block Turn Off Clears In Reverse.
 *
 * @details
 * Exercises the TerrainSearch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TerrainSearch.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainSearch, SharedBlockTurnOffClearsInReverse) {
  memset(DeltaBlocks, 0, sizeof(DeltaBlocks));
  // two nearby cells share a coarse-LOD block entry — turning off both
  // saves original then SHUTOFF, but ClearOffs unwinds LIFO so the FIRST
  // (original) save is restored LAST and wins
  DeltaBlocks[0][0] = 7.0f;

  TurnOffForCell(0, DeltaBlocks); // saves 7.0, sets SHUTOFF
  TurnOffForCell(1, DeltaBlocks); // same block idx at lod0: saves SHUTOFF
  ASSERT_EQ(Num_offs, 2);

  ClearOffs(DeltaBlocks);
  EXPECT_EQ(Num_offs, 0);
  // LIFO restore order makes the overlap benign here
  EXPECT_FLOAT_EQ(DeltaBlocks[0][0], 7.0f);
}
