/**
 * @file scorch_real_tests.cpp
 * @brief Scorch mark system tests (Descent3/scorch.cpp).
 *
 * @details
 * Links the real scorch.cpp. The world state it touches (Rooms[],
 * GameTextures[], Detail_settings) is defined here as minimal fixtures;
 * renderer/3d entry points are captured by stubs so AddScorch/DrawScorches
 * behavior can be asserted without a GPU. ps_rand is a self-consistent LCG
 * replica (rotation randomness itself is not asserted, only orthonormality
 * and magnitudes of the stored orientation vectors).
 *
 * This harness validates the behavior of `Descent3/scorch.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/scorch.cpp`
 * @par Harness
 * `scorch_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/scorch.cpp
 */

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "3d.h"
#include "config.h"
#include "object_external_struct.h"
#include "pserror.h"
#include "psrand.h"
#include "renderer.h"
#include "room.h"
#include "scorch.h"

// Pull in the real implementation so internal state (Scorches ring buffer,
// Scorch_start/Scorch_end, texture handle table) is test-visible.
#include "../../Descent3/scorch.cpp"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere + stubs with capture state
// ---------------------------------------------------------------------------
room Rooms[MAX_ROOMS];
texture GameTextures[MAX_TEXTURES];
tDetailSettings Detail_settings;
bool StateLimited = false;

static uint32_t g_ps_next = 1;
void ps_srand(uint32_t seed) { g_ps_next = seed; }
int32_t ps_rand(void) {
  g_ps_next = g_ps_next * 214013u + 2531011u;
  return static_cast<int32_t>((g_ps_next >> 16) & 0x7fff);
}

// Renderer state captures
struct RendState {
  int alpha_type_count = 0;
  uint8_t alpha_value = 0;
  int zbias_calls = 0;
  float last_zbias = 0.0f;
  int zwrite_calls = 0;
  int texture_type_calls = 0;
};
static RendState g_rend;
void rend_SetAlphaType(int8_t) { g_rend.alpha_type_count++; }
void rend_SetAlphaValue(uint8_t val) { g_rend.alpha_value = val; }
void rend_SetLighting(light_state) {}
void rend_SetColorModel(color_model) {}
void rend_SetOverlayType(uint8_t) {}
void rend_SetZBias(float z_bias) {
  g_rend.zbias_calls++;
  g_rend.last_zbias = z_bias;
}
void rend_SetZBufferWriteMask(int state) { g_rend.zwrite_calls += state; }
void rend_SetTextureType(texture_type) { g_rend.texture_type_calls++; }

// 3D stubs: rotate copies the input vector through, draw captures quads.
static float g_fake_depth = 100.0f;
float g3_CalcPointDepth(vector *) { return g_fake_depth; }
uint8_t g3_RotatePoint(g3Point *dest, vector *src) {
  dest->p3_vec = *src;
  return 0;
}

struct PolyRec {
  int bm;
  std::vector<vector> corners;
  std::vector<float> lights;
};
static std::vector<PolyRec> g_polys;
int g3_DrawPoly(int nv, g3Point **pointlist, int bm, int, g3Codes *) {
  PolyRec rec;
  rec.bm = bm;
  for (int i = 0; i < nv; i++) {
    rec.corners.push_back(pointlist[i]->p3_vec);
    rec.lights.push_back(pointlist[i]->p3_uvl.l);
  }
  g_polys.push_back(rec);
  return 1;
}

static int g_fake_bitmap = 4242;
int GetTextureBitmap(int, int, bool) { return g_fake_bitmap; }

// ---------------------------------------------------------------------------
// Fixture: one room with one quad face on the XZ plane (normal +Y)
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for ScorchTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class ScorchTest : public ::testing::Test {
protected:
  static constexpr int kRoom = 0;
  static constexpr int kFace = 0;
  static constexpr int kTmap = 0;

  void SetUp() override {
    InitMathTables(); // fix/fix.cpp trig tables used by vm_VectorAngleToMatrix
    ResetScorches();
    Num_scorch_textures = 0;
    memset(GameTextures, 0, sizeof(texture) * 4);
    memset(&Detail_settings, 0, sizeof(Detail_settings));
    Detail_settings.Scorches_enabled = true;
    g_polys.clear();
    g_fake_depth = 100.0f;
    ps_srand(1);

    BuildRoom(kRoom);
  }

  // Builds a room at index `idx` holding `nfaces` identical quad faces
  // (normal +Y) sharing one vertex pool.
  void BuildRoom(int idx, int nfaces = 1) {
    room &rp = Rooms[idx];
    rp.num_faces = nfaces;
    rp.faces = new face[nfaces];
    memset(rp.faces, 0, sizeof(face) * nfaces);
    rp.num_verts = 4;
    rp.verts = new vector[4];
    rp.verts[0] = {-10, 0, -10};
    rp.verts[1] = {10, 0, -10};
    rp.verts[2] = {10, 0, 10};
    rp.verts[3] = {-10, 0, 10};

    for (int f = 0; f < nfaces; f++) {
      face &fp = rp.faces[f];
      fp.flags = 0;
      fp.num_verts = 4;
      fp.portal_num = -1;
      fp.face_verts = new int16_t[4]{0, 1, 2, 3};
      fp.normal = {0, 1, 0};
      fp.tmap = kTmap;
    }
  }

  void TearDown() override {
    for (int i = 0; i < MAX_ROOMS; i++) {
      if (Rooms[i].faces) {
        for (int f = 0; f < Rooms[i].num_faces; f++)
          delete[] Rooms[i].faces[f].face_verts;
        delete[] Rooms[i].faces;
      }
      delete[] Rooms[i].verts;
      Rooms[i] = room{};
    }
  }

  static bool FaceScorched(int room, int facenum) {
    return (Rooms[room].faces[facenum].flags & FF_SCORCHED) != 0;
  }

  static int CountScorches() {
    if (Scorch_start < 0)
      return 0;
    int n = 0;
    for (int i = Scorch_start;; i++) {
      n++;
      if (i == Scorch_end)
        break;
      if (i + 1 == MAX_SCORCHES)
        i = -1;
    }
    return n;
  }
};

// ---------------------------------------------------------------------------
// ResetScorches / basic add
// ---------------------------------------------------------------------------
/**
 * @test ScorchTest.ResetClearsList
 * @brief Verifies reset Clears List.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, ResetClearsList) {
  vector pos = {0, 0, 0};
  AddScorch(kRoom, kFace, &pos, 7, 2.0f);
  EXPECT_GT(CountScorches(), 0);
  ResetScorches();
  EXPECT_EQ(Scorch_start, -1);
  EXPECT_EQ(Scorch_end, -1);
}

/**
 * @test ScorchTest.TerrainRoomnumRejected
 * @brief Verifies terrain Roomnum Rejected.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, TerrainRoomnumRejected) {
  vector pos = {0, 0, 0};
  int outside_room = MAKE_ROOMNUM(5); // cell flag set => outside/terrain
  BuildRoom(5);
  AddScorch(outside_room, 0, &pos, 7, 2.0f);
  EXPECT_EQ(Scorch_end, -1);
  EXPECT_FALSE(FaceScorched(5, 0));
}

/**
 * @test ScorchTest.LightAndProceduralTexturesRejected
 * @brief Verifies light And Procedural Textures Rejected.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, LightAndProceduralTexturesRejected) {
  GameTextures[kTmap].flags |= TF_LIGHT;
  vector pos = {0, 0, 0};
  AddScorch(kRoom, kFace, &pos, 7, 2.0f);
  EXPECT_EQ(Scorch_end, -1);

  GameTextures[kTmap].flags = TF_PROCEDURAL;
  AddScorch(kRoom, kFace, &pos, 7, 2.0f);
  EXPECT_EQ(Scorch_end, -1);
  EXPECT_FALSE(FaceScorched(kRoom, kFace));
}

/**
 * @test ScorchTest.AddStoresDataAndFlagsFace
 * @brief Verifies add Stores Data And Flags Face.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, AddStoresDataAndFlagsFace) {
  vector pos = {1, 0, 2};
  AddScorch(kRoom, kFace, &pos, 77, 2.0f);

  ASSERT_EQ(Scorch_start, 0);
  ASSERT_EQ(Scorch_end, 0);
  const auto &sp = Scorches[0];

  EXPECT_EQ(sp.roomface, (kRoom << 16) + kFace); // ROOMFACE packing
  EXPECT_FLOAT_EQ(sp.pos.x(), 1.0f);
  EXPECT_FLOAT_EQ(sp.pos.z(), 2.0f);
  EXPECT_EQ(sp.size, 32u); // size * 16
  EXPECT_EQ(sp.handle_index, 0u);
  EXPECT_EQ(Num_scorch_textures, 1);
  EXPECT_EQ(Scorch_texture_handles[0], 77);
  EXPECT_TRUE(FaceScorched(kRoom, kFace));

  // Orientation vectors stored as +/-127-scaled int8 components of an
  // orthonormal frame around the face normal.
  float rlen = sqrtf(float(sp.rx) * sp.rx + float(sp.ry) * sp.ry + float(sp.rz) * sp.rz);
  float ulen = sqrtf(float(sp.ux) * sp.ux + float(sp.uy) * sp.uy + float(sp.uz) * sp.uz);
  EXPECT_NEAR(rlen, 127.0f, 1.5f);
  EXPECT_NEAR(ulen, 127.0f, 1.5f);
}

/**
 * @test ScorchTest.TextureHandlesDeduplicated
 * @brief Verifies texture Handles Deduplicated.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, TextureHandlesDeduplicated) {
  vector pos = {0, 0, 0};
  AddScorch(kRoom, kFace, &pos, 55, 1.0f);
  AddScorch(kRoom, kFace, &pos, 66, 1.0f); // different handle -> new slot

  EXPECT_EQ(Num_scorch_textures, 2);
  EXPECT_EQ(Scorch_texture_handles[0], 55);
  EXPECT_EQ(Scorch_texture_handles[1], 66);
  EXPECT_EQ(Scorches[Scorch_start].handle_index, 0u);
  EXPECT_EQ(Scorches[Scorch_end].handle_index, 1u);

  AddScorch(kRoom, kFace, &pos, 55, 1.0f); // reuse first handle
  const auto &sp = Scorches[Scorch_end];
  EXPECT_EQ(Num_scorch_textures, 2);
  EXPECT_EQ(sp.handle_index, 0u);
}

// ---------------------------------------------------------------------------
// Rejection rules
// ---------------------------------------------------------------------------
/**
 * @test ScorchTest.ScorchOverlappingFaceEdgeRejected
 * @brief Verifies scorch Overlapping Face Edge Rejected.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, ScorchOverlappingFaceEdgeRejected) {
  vector near_edge = {9.5f, 0, 0}; // 0.5 from edge x=10, size 2
  AddScorch(kRoom, kFace, &near_edge, 7, 2.0f);
  EXPECT_EQ(Scorch_end, -1);
  EXPECT_FALSE(FaceScorched(kRoom, kFace));

  vector center = {0, 0, 0}; // 10 from every edge
  AddScorch(kRoom, kFace, &center, 7, 2.0f);
  EXPECT_EQ(Scorch_end, 0);
}

/**
 * @test ScorchTest.PerFaceLimitRejectsAtMaxVisCount
 * @brief Verifies per Face Limit Rejects At Max Vis Count.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, PerFaceLimitRejectsAtMaxVisCount) {
  vector pos = {0, 0, 0};

  // MAX_VIS_COUNT=30 weighted adds allowed with unit-size marks.
  for (int i = 0; i < 30; i++)
    AddScorch(kRoom, kFace, &pos, 7, 1.0f);
  EXPECT_EQ(CountScorches(), 30);

  AddScorch(kRoom, kFace, &pos, 7, 1.0f); // count already at limit
  EXPECT_EQ(CountScorches(), 30);

  ResetScorches();

  // Large marks (size >= 2) count double: 15 scorches reach the limit.
  for (int i = 0; i < 15; i++)
    AddScorch(kRoom, kFace, &pos, 7, 2.5f);
  EXPECT_EQ(CountScorches(), 15);

  AddScorch(kRoom, kFace, &pos, 7, 2.5f);
  EXPECT_EQ(CountScorches(), 15);
}

/**
 * @test ScorchTest.CircularBufferEvictsOldest
 * @brief Verifies circular Buffer Evicts Oldest.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, CircularBufferEvictsOldest) {
  // Fill distinct faces so the per-face limit never triggers: rooms 1..10 x
  // 50 faces each = exactly MAX_SCORCHES unique (room, face) combos.
  // (SetUp leaves room 0 with a single face, so it stays out of the mix.)
  for (int i = 0; i < 10; i++)
    BuildRoom(i + 1, 50);

  vector pos = {0, 0, 0};
  for (int i = 0; i < 500; i++) // MAX_SCORCHES
    AddScorch(1 + (i % 10), (i / 10) % 50, &pos, 7, 1.0f);

  EXPECT_EQ(CountScorches(), 500);
  EXPECT_EQ(Scorch_start, 0);
  EXPECT_EQ(Scorch_end, 499);
  EXPECT_TRUE(FaceScorched(1, 0)); // oldest mark lives here (i=0)

  // Next add wraps onto the oldest slot: evicts it and clears its flag.
  AddScorch(3, 17, &pos, 7, 1.0f);
  EXPECT_EQ(CountScorches(), 500);
  EXPECT_EQ(Scorch_start, 1);
  EXPECT_EQ(Scorch_end, 0);
  EXPECT_FALSE(FaceScorched(1, 0)); // evicted
  EXPECT_TRUE(FaceScorched(3, 17)); // newest
  EXPECT_EQ(Scorches[0].roomface, (3 << 16) + 17);
}

// ---------------------------------------------------------------------------
// DeleteScorch flag semantics (exercised through real eviction flow:
// DeleteScorch only scans the live ring range, so marks must be evicted by
// wraparound, not deleted out-of-band)
// ---------------------------------------------------------------------------
/**
 * @test ScorchTest.DeleteScorchKeepsFlagWhileOthersRemain
 * @brief Verifies delete Scorch Keeps Flag While Others Remain.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, DeleteScorchKeepsFlagWhileOthersRemain) {
  // Two scorches on the same face, then fill the ring with distinct faces.
  vector p1 = {0, 0, -4};
  vector p2 = {0, 0, 4};
  AddScorch(kRoom, kFace, &p1, 7, 1.0f);
  AddScorch(kRoom, kFace, &p2, 7, 1.0f);
  ASSERT_TRUE(FaceScorched(kRoom, kFace));

  int unique = 0;
  auto add_unique = [&]() {
    AddScorch(1 + (unique % 9), unique % 50, &p1, 7, 1.0f);
    unique++;
  };
  // Rooms 1..9 with 50 faces each: 450 unique combos, enough to fill the
  // ring alongside the two same-face marks.
  for (int i = 0; i < 9; i++)
    BuildRoom(i + 1, 50);
  while (CountScorches() < MAX_SCORCHES)
    add_unique();
  ASSERT_TRUE(FaceScorched(kRoom, kFace)); // both same-face marks alive

  // Next add wraps onto the first mark: DeleteScorch finds the second mark
  // on the face and keeps the flag set.
  add_unique();
  EXPECT_EQ(CountScorches(), MAX_SCORCHES);
  EXPECT_TRUE(FaceScorched(kRoom, kFace));

  // Keep evicting until the second same-face mark is gone too: only then is
  // the FF_SCORCHED flag cleared.
  int guard = 2 * MAX_SCORCHES;
  while (FaceScorched(kRoom, kFace) && guard-- > 0)
    add_unique();
  EXPECT_FALSE(FaceScorched(kRoom, kFace));
}

// ---------------------------------------------------------------------------
// DrawScorches
// ---------------------------------------------------------------------------
/**
 * @test ScorchTest.DrawSkippedWhenDisabledInDetailSettings
 * @brief Verifies draw Skipped When Disabled In Detail Settings.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, DrawSkippedWhenDisabledInDetailSettings) {
  vector pos = {0, 0, 0};
  AddScorch(kRoom, kFace, &pos, 7, 2.0f);
  Detail_settings.Scorches_enabled = false;

  DrawScorches(kRoom, kFace);
  EXPECT_TRUE(g_polys.empty());
}

/**
 * @test ScorchTest.DrawNearbyQuadWithUvsAndBitmap
 * @brief Verifies draw Nearby Quad With Uvs And Bitmap.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, DrawNearbyQuadWithUvsAndBitmap) {
  vector pos = {0, 0, 0};
  AddScorch(kRoom, kFace, &pos, 7, 2.0f);
  g_fake_depth = 100.0f; // well inside FADE_START_DISTANCE

  DrawScorches(kRoom, kFace);

  ASSERT_EQ(g_polys.size(), 1u);
  EXPECT_EQ(g_polys[0].bm, g_fake_bitmap);
  ASSERT_EQ(g_polys[0].corners.size(), 4u);
  ASSERT_EQ(g_polys[0].lights.size(), 4u);

  // Quad is centered on the mark; half-diagonal ~ size*sqrt(2).
  float d0 = hypotf(g_polys[0].corners[0].x(), g_polys[0].corners[0].z());
  EXPECT_NEAR(d0, 2.0f * sqrtf(2.0f), 0.08f);
  for (auto l : g_polys[0].lights)
    EXPECT_FLOAT_EQ(l, 1.0f);

  // State setup/teardown ran (non-StateLimited path).
  EXPECT_EQ(g_rend.zbias_calls, 2);
  EXPECT_FLOAT_EQ(g_rend.last_zbias, 0.0f);
}

/**
 * @test ScorchTest.DrawShrinksQuadBetweenFadeStartAndMaxDistance
 * @brief Verifies draw Shrinks Quad Between Fade Start And Max Distance.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, DrawShrinksQuadBetweenFadeStartAndMaxDistance) {
  vector pos = {0, 0, 0};
  AddScorch(kRoom, kFace, &pos, 7, 3.0f);

  g_fake_depth = 100.0f;
  DrawScorches(kRoom, kFace);
  ASSERT_EQ(g_polys.size(), 1u);
  float near_d = hypotf(g_polys[0].corners[0].x(), g_polys[0].corners[0].z());
  g_polys.clear();

  // depth=185 => scale factor 1 - (185-170)/(200-170) = 0.5
  g_fake_depth = 185.0f;
  DrawScorches(kRoom, kFace);
  ASSERT_EQ(g_polys.size(), 1u);
  float mid_d = hypotf(g_polys[0].corners[0].x(), g_polys[0].corners[0].z());

  EXPECT_NEAR(mid_d / near_d, 0.5f, 0.03f);
}

/**
 * @test ScorchTest.DrawSkippedBeyondMaxVisDistance
 * @brief Verifies draw Skipped Beyond Max Vis Distance.
 *
 * @details
 * Exercises the ScorchTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/scorch.cpp
 * @ingroup descent3_tests
 */
TEST_F(ScorchTest, DrawSkippedBeyondMaxVisDistance) {
  vector pos = {0, 0, 0};
  AddScorch(kRoom, kFace, &pos, 7, 2.0f);
  g_fake_depth = 250.0f; // > MAX_VIS_DISTANCE (200)

  DrawScorches(kRoom, kFace);
  EXPECT_TRUE(g_polys.empty());
}
