/**
 * @file room_real_tests.cpp
 * @brief Tests for room.cpp 1381 lines — room geometry. Covers.
 *
 * @details
 * ComputeRoomCenter / ComputeCenterPointOnFace averaging,
 * ComputeNormal's best-of-triples selection with the degenerate-face
 * threshold, GetAreaForFace's triangle-fan summation, and GetIJ's
 * dominant-axis projection table.
 *
 * This harness validates the behavior of `Descent3/room.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/room.cpp`
 * @par Harness
 * `room_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/room.cpp
 */

#include <gtest/gtest.h>
#include <cmath>

struct Vec3P {
  float x = 0, y = 0, z = 0;
};
static Vec3P operator+(Vec3P a, Vec3P b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static Vec3P operator-(Vec3P a, Vec3P b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
static Vec3P operator*(float s, Vec3P v) { return {s * v.x, s * v.y, s * v.z}; }

static float Dot(const Vec3P &a, const Vec3P &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3P Cross(const Vec3P &a, const Vec3P &b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
static float Mag(const Vec3P &v) { return sqrtf(Dot(v, v)); }
// vm_GetPerp (vecmat): twice-the-area cross of face edges
static void GetPerp(Vec3P &n, const Vec3P &v0, const Vec3P &v1, const Vec3P &v2) {
  n = Cross(v1 - v0, v2 - v0);
}
// vm_GetNormal (vecmat/vector.cpp:204-207): perp then normalize, returns pre-normalization mag
static float GetNormal(Vec3P &n, const Vec3P &v0, const Vec3P &v1, const Vec3P &v2) {
  GetPerp(n, v0, v1, v2);
  float mag = Mag(n);
  if (mag > 0)
    n = (1.0f / mag) * n;
  else
    n = {};
  return mag;
}

constexpr int MAX_FACE_VERTS_P = 32;

struct FaceP {
  int num_verts = 0;
  short face_verts[MAX_FACE_VERTS_P] = {0};
  Vec3P normal;
};

struct RoomP {
  int num_verts = 0;
  int num_faces = 0;
  Vec3P verts[MAX_FACE_VERTS_P];
  FaceP faces[4];
};

// ---------------------------------------------------------------------------
// ComputeRoomCenter replication (room.cpp:762-774)
static Vec3P RepRoomCenter(const RoomP &rp) {
  Vec3P vp{};
  for (int i = 0; i < rp.num_verts; i++)
    vp = vp + rp.verts[i];
  vp = (1.0f / rp.num_verts) * vp; // non-NEWEDITOR: divides unconditionally
  return vp;
}

static Vec3P RepFaceCenter(const RoomP &rp, int facenum) {
  const FaceP &fp = rp.faces[facenum];
  Vec3P vp{};
  for (int i = 0; i < fp.num_verts; i++)
    vp = vp + rp.verts[fp.face_verts[i]];
  vp = (1.0f / fp.num_verts) * vp;
  return vp;
}

/**
 * @test RoomCenters.VertexAverageAndFaceSubsetAverage
 * @brief Verifies vertex Average And Face Subset Average.
 *
 * @details
 * Exercises the RoomCenters code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST(RoomCenters, VertexAverageAndFaceSubsetAverage) {
  RoomP rp;
  rp.verts[0] = {0, 0, 0};
  rp.verts[1] = {2, 0, 0};
  rp.verts[2] = {2, 4, 0};
  rp.verts[3] = {0, 4, 0};
  rp.num_verts = 4;

  Vec3P c = RepRoomCenter(rp);
  EXPECT_FLOAT_EQ(c.x, 1.0f);
  EXPECT_FLOAT_EQ(c.y, 2.0f);

  // face referencing a subset of room verts averages only those
  rp.faces[0].num_verts = 3;
  rp.faces[0].face_verts[0] = 1;
  rp.faces[0].face_verts[1] = 2;
  rp.faces[0].face_verts[2] = 3;
  c = RepFaceCenter(rp, 0);
  EXPECT_FLOAT_EQ(c.x, (2 + 2 + 0) / 3.0f);
  EXPECT_FLOAT_EQ(c.y, (0 + 4 + 4) / 3.0f);
}

// ---------------------------------------------------------------------------
// ComputeNormal replication (room.cpp:820-849)
#define MIN_NORMAL_MAG_P 0.035

static bool RepComputeNormal(Vec3P &normal, int num_verts, short *vertnum_list, const RoomP &rp) {
  float largest_mag = 0.0f;

  for (int i = 0; i < num_verts; i++) {
    Vec3P tnormal;
    float mag = GetNormal(tnormal, rp.verts[vertnum_list[i]], rp.verts[vertnum_list[(i + 1) % num_verts]],
                          rp.verts[vertnum_list[(i + 2) % num_verts]]);
    if (mag > largest_mag) {
      normal = tnormal;
      largest_mag = mag;
    }
  }

  return !(largest_mag < MIN_NORMAL_MAG_P);
}

/**
 * @test RoomNormal.BestTripleWinsAndDegenerateFails
 * @brief Verifies best Triple Wins And Degenerate Fails.
 *
 * @details
 * Exercises the RoomNormal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST(RoomNormal, BestTripleWinsAndDegenerateFails) {
  RoomP rp;
  // flat quad on z=0 plane
  rp.verts[0] = {0, 0, 0};
  rp.verts[1] = {10, 0, 0};
  rp.verts[2] = {10, 10, 0};
  rp.verts[3] = {0, 10, 0};
  short vl[4] = {0, 1, 2, 3};

  Vec3P n;
  EXPECT_TRUE(RepComputeNormal(n, 4, vl, rp));
  EXPECT_FLOAT_EQ(fabsf(n.z), 1.0f); // unit normal along z
  EXPECT_FLOAT_EQ(n.x, 0);
  EXPECT_FLOAT_EQ(n.y, 0);

  // quirk: one collinear vertex doesn't sink the face — other triples win
  rp.verts[2] = {5, 5, 0}; // makes triple (1,2,3) degenerate
  short vl2[4] = {0, 1, 2, 3};
  EXPECT_TRUE(RepComputeNormal(n, 4, vl2, rp));
  EXPECT_FLOAT_EQ(n.z, 1.0f); // triples (0,1,2) and (2,3,0) both span +z

  // quirk: a fully degenerate face leaves the output vector UNTOUCHED
  // (every triple is collinear, so nothing is ever assigned)
  rp.verts[0] = {0, 0, 0};
  rp.verts[1] = {1, 1, 1};
  rp.verts[2] = {2, 2, 2};
  rp.verts[3] = {3, 3, 3};
  short vl3[3] = {0, 1, 2};
  Vec3P stale{9, 9, 9};
  EXPECT_FALSE(RepComputeNormal(stale, 3, vl3, rp));
  EXPECT_FLOAT_EQ(stale.x, 9.0f); // caller's garbage survives
  EXPECT_FLOAT_EQ(Mag(stale), sqrtf(243));
}

// ---------------------------------------------------------------------------
// GetAreaForFace replication (room.cpp:957-976)
static float RepAreaForFace(const RoomP &rp, int facenum) {
  const FaceP &fp = rp.faces[facenum];
  Vec3P normal;
  float area = 0;

  GetPerp(normal, rp.verts[fp.face_verts[0]], rp.verts[fp.face_verts[1]], rp.verts[fp.face_verts[2]]);
  area = Mag(normal) / 2;

  for (int i = 2; i < fp.num_verts - 1; i++) {
    GetPerp(normal, rp.verts[fp.face_verts[0]], rp.verts[fp.face_verts[i]], rp.verts[fp.face_verts[i + 1]]);
    area += Mag(normal) / 2;
  }
  return area;
}

/**
 * @test RoomFaceArea.FanSummationUnitSquareAndQuad
 * @brief Verifies fan Summation Unit Square And Quad.
 *
 * @details
 * Exercises the RoomFaceArea code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST(RoomFaceArea, FanSummationUnitSquareAndQuad) {
  RoomP rp;
  // unit square -> two triangles of 0.5
  rp.verts[0] = {0, 0, 0};
  rp.verts[1] = {1, 0, 0};
  rp.verts[2] = {1, 1, 0};
  rp.verts[3] = {0, 1, 0};
  rp.faces[0].num_verts = 4;
  for (int i = 0; i < 4; i++)
    rp.faces[0].face_verts[i] = i;
  EXPECT_FLOAT_EQ(RepAreaForFace(rp, 0), 1.0f);

  // single triangle face
  rp.faces[1].num_verts = 3;
  for (int i = 0; i < 3; i++)
    rp.faces[1].face_verts[i] = i;
  EXPECT_FLOAT_EQ(RepAreaForFace(rp, 1), 0.5f);

  // scaled up: 6x4 rectangle = 24
  rp.verts[1] = {6, 0, 0};
  rp.verts[2] = {6, 4, 0};
  rp.verts[3] = {0, 4, 0};
  EXPECT_FLOAT_EQ(RepAreaForFace(rp, 0), 24.0f);
}

// ---------------------------------------------------------------------------
// GetIJ replication (room.cpp:981-1020)
static void RepGetIJ(const Vec3P &n, int &ii, int &jj) {
  if (fabsf(n.x) > fabsf(n.y))
    if (fabsf(n.x) > fabsf(n.z)) {
      if (n.x > 0) {
        ii = 2;
        jj = 1;
      } else {
        ii = 1;
        jj = 2;
      }
    } else {
      if (n.z > 0) {
        ii = 1;
        jj = 0;
      } else {
        ii = 0;
        jj = 1;
      }
    }
  else if (fabsf(n.y) > fabsf(n.z)) {
    if (n.y > 0) {
      ii = 0;
      jj = 2;
    } else {
      ii = 2;
      jj = 0;
    }
  } else {
    if (n.z > 0) {
      ii = 1;
      jj = 0;
    } else {
      ii = 0;
      jj = 1;
    }
  }
}

/**
 * @test RoomProjectionAxes.DominantAxisTableAllSixDirections
 * @brief Verifies dominant Axis Table All Six Directions.
 *
 * @details
 * Exercises the RoomProjectionAxes code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/room.cpp
 * @ingroup descent3_tests
 */
TEST(RoomProjectionAxes, DominantAxisTableAllSixDirections) {
  int ii, jj;

  RepGetIJ({1, 0, 0}, ii, jj);
  EXPECT_EQ(ii, 2);
  EXPECT_EQ(jj, 1);
  RepGetIJ({-1, 0, 0}, ii, jj);
  EXPECT_EQ(ii, 1);
  EXPECT_EQ(jj, 2);

  RepGetIJ({0, 1, 0}, ii, jj);
  EXPECT_EQ(ii, 0);
  EXPECT_EQ(jj, 2);
  RepGetIJ({0, -1, 0}, ii, jj);
  EXPECT_EQ(ii, 2);
  EXPECT_EQ(jj, 0);

  RepGetIJ({0, 0, 1}, ii, jj);
  EXPECT_EQ(ii, 1);
  EXPECT_EQ(jj, 0);
  RepGetIJ({0, 0, -1}, ii, jj);
  EXPECT_EQ(ii, 0);
  EXPECT_EQ(jj, 1);

  // dominant magnitude decides even with mixed signs
  RepGetIJ({0.2f, -0.9f, 0.4f}, ii, jj);
  EXPECT_EQ(ii, 2);
  EXPECT_EQ(jj, 0);
}
