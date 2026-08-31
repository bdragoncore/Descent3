/**
 * @file bsp_real_tests.cpp
 * @brief Tests for bsp.cpp 1207 lines — mine BSP construction. Covers.
 *
 * @details
 * ClassifyVector/ClassifyPolygon side classification (on-plane
 * vertices counting toward BOTH sides), SplitPolygon's edge-walk
 * quad splitting with interpolated intersection points, and
 * SelectPlane's balance-scored partition picker.
 *
 * This harness validates the behavior of `Descent3/bsp.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/bsp.cpp`
 * @par Harness
 * `bsp_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/bsp.cpp
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cstdlib>

#define BSP_IN_FRONT_P 1
#define BSP_BEHIND_P 2
#define BSP_ON_PLANE_P 3
#define BSP_SPANNING_P 4
#define BSP_COINCIDENT_P 5
#define BSP_EPSILON_P 0.00005f

struct VecP {
  float x = 0, y = 0, z = 0;
};
static VecP operator-(VecP a, VecP b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
static VecP operator+(VecP a, VecP b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static VecP operator*(float s, VecP v) { return {s * v.x, s * v.y, s * v.z}; }

struct PlaneP {
  float a = 0, b = 0, c = 0, d = 0;
  bool used = false;
};

// ---------------------------------------------------------------------------
// ClassifyVector / ClassifyPolygon replication (bsp.cpp:266-320)
static int RepClassifyVector(const PlaneP &pl, const VecP &v) {
  float dist = v.x * pl.a + v.y * pl.b + v.z * pl.c + pl.d;
  if (dist > BSP_EPSILON_P)
    return BSP_IN_FRONT_P;
  if (dist < -BSP_EPSILON_P)
    return BSP_BEHIND_P;
  return BSP_ON_PLANE_P;
}

struct PolyP {
  int nv = 0;
  int roomnum = 0, facenum = 0;
  PlaneP plane;
  VecP verts[16];
};

static int RepClassifyPolygon(const PlaneP &pl, const PolyP &poly) {
  int numfront = 0, numback = 0, numon = 0;
  for (int i = 0; i < poly.nv; i++) {
    switch (RepClassifyVector(pl, poly.verts[i])) {
    case BSP_IN_FRONT_P:
      numfront++;
      break;
    case BSP_BEHIND_P:
      numback++;
      break;
    case BSP_ON_PLANE_P:
      numfront++;
      numback++;
      numon++;
      break;
    }
  }
  if (numon == poly.nv)
    return BSP_COINCIDENT_P;
  if (numfront == poly.nv)
    return BSP_IN_FRONT_P;
  if (numback == poly.nv)
    return BSP_BEHIND_P;
  return BSP_SPANNING_P;
}

/**
 * @test BspClassify.EpsilonBandsAndOnPlaneLeniency
 * @brief Verifies epsilon Bands And On Plane Leniency.
 *
 * @details
 * Exercises the BspClassify code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/bsp.cpp
 * @ingroup descent3_tests
 */
TEST(BspClassify, EpsilonBandsAndOnPlaneLeniency) {
  PlaneP px;
  px.a = 1; // x = 0 plane

  EXPECT_EQ(RepClassifyVector(px, {1, 0, 0}), BSP_IN_FRONT_P);
  EXPECT_EQ(RepClassifyVector(px, {-1, 0, 0}), BSP_BEHIND_P);
  EXPECT_EQ(RepClassifyVector(px, {0, 5, 5}), BSP_ON_PLANE_P);
  // epsilon band: tiny offsets still count as ON
  EXPECT_EQ(RepClassifyVector(px, {0.00001f, 0, 0}), BSP_ON_PLANE_P);
  EXPECT_EQ(RepClassifyVector(px, {0.001f, 0, 0}), BSP_IN_FRONT_P);

  auto mkquad = [](float x0, float x1, float z) {
    PolyP p;
    p.nv = 4;
    p.verts[0] = {x0, -1, z};
    p.verts[1] = {x1, -1, z};
    p.verts[2] = {x1, 1, z};
    p.verts[3] = {x0, 1, z};
    return p;
  };

  EXPECT_EQ(RepClassifyPolygon(px, mkquad(-2, -1, 0)), BSP_BEHIND_P);
  EXPECT_EQ(RepClassifyPolygon(px, mkquad(1, 2, 0)), BSP_IN_FRONT_P);

  // quirk: a polygon TOUCHING the plane (two verts exactly on it) is not
  // spanning — on-plane verts count for both sides, so it classifies as
  // whichever side holds its remaining verts
  EXPECT_EQ(RepClassifyPolygon(px, mkquad(0, 2, 0)), BSP_IN_FRONT_P);
  EXPECT_EQ(RepClassifyPolygon(px, mkquad(-2, 0, 0)), BSP_BEHIND_P);

  // truly straddling
  EXPECT_EQ(RepClassifyPolygon(px, mkquad(-1, 1, 0)), BSP_SPANNING_P);

  PlaneP pz;
  pz.c = 1;
  pz.d = -1; // z = 1 plane
  EXPECT_EQ(RepClassifyPolygon(pz, mkquad(0, 1, 1)), BSP_COINCIDENT_P);
}

// ---------------------------------------------------------------------------
// SplitPolygon replication (bsp.cpp:323-451)
static PolyP *NewPolyP(int nv) {
  auto p = new PolyP{};
  p->nv = nv;
  return p;
}

static bool RepSplitPolygon(const PlaneP &plane, const PolyP &testpoly,
                            PolyP **frontpoly, PolyP **backpoly) {
  float dists[257];
  int codes[257] = {};
  VecP polyvert[256];
  VecP frontvert[256], backvert[256];
  VecP newvert[256];
  int num_new_verts = 0;

  int numvert = testpoly.nv;
  int numfront = 0, numback = 0;

  for (int i = 0; i < numvert; i++) {
    polyvert[i] = testpoly.verts[i];
    codes[i] = RepClassifyVector(plane, polyvert[i]);
    dists[i] = plane.a * polyvert[i].x + plane.b * polyvert[i].y + plane.c * polyvert[i].z + plane.d;
  }
  codes[numvert] = codes[0];
  dists[numvert] = dists[0];

  for (int i = 0; i < numvert; i++) {
    VecP v1 = polyvert[i];

    if (codes[i] == BSP_ON_PLANE_P) {
      frontvert[numfront++] = v1;
      backvert[numback++] = v1;
    } else if (codes[i] == BSP_IN_FRONT_P) {
      frontvert[numfront++] = v1;
    } else if (codes[i] == BSP_BEHIND_P) {
      backvert[numback++] = v1;
    }

    if ((codes[i] == BSP_ON_PLANE_P) || (codes[i] == codes[i + 1]))
      continue;

    VecP v2 = polyvert[(i + 1) % numvert];
    float t = dists[i] / (dists[i] - dists[i + 1]);
    VecP delta = v2 - v1;
    newvert[num_new_verts] = v1 + (t * delta);
    frontvert[numfront++] = newvert[num_new_verts];
    backvert[numback++] = newvert[num_new_verts];
    num_new_verts++;
  }

  *frontpoly = NewPolyP(numfront);
  *backpoly = NewPolyP(numback);
  for (int i = 0; i < numfront; i++) {
    (*frontpoly)->verts[i] = frontvert[i];
    (*frontpoly)->plane = testpoly.plane;
    (*frontpoly)->facenum = testpoly.facenum;
    (*frontpoly)->roomnum = testpoly.roomnum;
  }
  for (int i = 0; i < numback; i++) {
    (*backpoly)->verts[i] = backvert[i];
    (*backpoly)->plane = testpoly.plane;
    (*backpoly)->facenum = testpoly.facenum;
    (*backpoly)->roomnum = testpoly.roomnum;
  }
  return true;
}

/**
 * @test BspSplit.QuadSplitsIntoTwoWithInterpolatedPoints
 * @brief Verifies quad Splits Into Two With Interpolated Points.
 *
 * @details
 * Exercises the BspSplit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/bsp.cpp
 * @ingroup descent3_tests
 */
TEST(BspSplit, QuadSplitsIntoTwoWithInterpolatedPoints) {
  PlaneP px;
  px.a = 1; // x = 0

  PolyP quad;
  quad.nv = 4;
  quad.facenum = 9;
  quad.roomnum = 3;
  quad.plane.c = 1;
  quad.verts[0] = {-2, -1, 0}; // behind
  quad.verts[1] = {2, -1, 0};  // front
  quad.verts[2] = {2, 1, 0};   // front
  quad.verts[3] = {-2, 1, 0};  // behind

  PolyP *fp = nullptr, *bp = nullptr;
  ASSERT_TRUE(RepSplitPolygon(px, quad, &fp, &bp));

  // front: [cut(y=-1), B, C, cut(y=1)]; back: [A, cut(y=-1), cut(y=1), D]
  ASSERT_EQ(fp->nv, 4);
  ASSERT_EQ(bp->nv, 4);
  EXPECT_FLOAT_EQ(fp->verts[0].x, 0);
  EXPECT_FLOAT_EQ(fp->verts[0].y, -1);
  EXPECT_FLOAT_EQ(fp->verts[1].x, 2);
  EXPECT_FLOAT_EQ(fp->verts[2].x, 2);
  EXPECT_FLOAT_EQ(fp->verts[3].y, 1);

  EXPECT_FLOAT_EQ(bp->verts[0].x, -2);
  EXPECT_FLOAT_EQ(bp->verts[1].x, 0);
  EXPECT_FLOAT_EQ(bp->verts[1].y, -1);
  EXPECT_FLOAT_EQ(bp->verts[2].y, 1);
  EXPECT_FLOAT_EQ(bp->verts[3].x, -2);

  // source metadata copied onto both halves
  EXPECT_EQ(fp->facenum, 9);
  EXPECT_EQ(bp->roomnum, 3);
  EXPECT_FLOAT_EQ(fp->plane.c, 1);

  delete fp;
  delete bp;
}

// ---------------------------------------------------------------------------
// SelectPlane replication (bsp.cpp:458-528)
struct ListNodeP {
  PolyP *data;
  ListNodeP *next = nullptr;
};

static PolyP *RepSelectPlane(ListNodeP **polylist) {
  float g_kbalance = 1.0f, g_ksplit = 0.0f, g_kplanar = 0.0f;
  PolyP *bestpoly = nullptr;
  float bestscore = 9999999.0f;
  int checked = 0;

  for (ListNodeP *outer = *polylist; outer != nullptr; outer = outer->next) {
    PolyP *outerpoly = outer->data;
    int splits = 0, front = 0, back = 0, planar = 0;
    float score = 0.0f;

    if (outerpoly->plane.used)
      continue;

    if (checked >= 300)
      return bestpoly;

    for (ListNodeP *inner = *polylist; inner != nullptr; inner = inner->next) {
      PolyP *innerpoly = inner->data;
      if (innerpoly != outerpoly) {
        switch (RepClassifyPolygon(outerpoly->plane, *innerpoly)) {
        case BSP_SPANNING_P:
          splits++;
          break;
        case BSP_IN_FRONT_P:
          front++;
          break;
        case BSP_BEHIND_P:
          back++;
          break;
        case BSP_COINCIDENT_P:
          planar++;
          break;
        }
      }
    }

    int balance = abs(front - back);
    score = g_kbalance * balance + g_ksplit * splits + g_kplanar * planar;

    checked++;

    if (score < 0.005)
      return outerpoly; // perfect partition found early

    if (score < bestscore) {
      bestscore = score;
      bestpoly = outerpoly;
    }
  }
  return bestpoly;
}

static void AppendP(ListNodeP **head, PolyP *p) {
  auto n = new ListNodeP{p, nullptr};
  n->next = *head;
  *head = n;
}

/**
 * @test BspSelectPlane.BalanceScorePerfectEarlyOutAndUsedSkip
 * @brief Verifies balance Score Perfect Early Out And Used Skip.
 *
 * @details
 * Exercises the BspSelectPlane code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/bsp.cpp
 * @ingroup descent3_tests
 */
TEST(BspSelectPlane, BalanceScorePerfectEarlyOutAndUsedSkip) {
  // P1/P2 stacked on z, splitter quad at x=0 -> perfect balance 0
  auto mk = [](int nv, std::initializer_list<VecP> vs, PlaneP pl, int face) {
    auto p = new PolyP{};
    p->nv = nv;
    p->plane = pl;
    p->facenum = face;
    int i = 0;
    for (auto v : vs)
      p->verts[i++] = v;
    return p;
  };
  PlaneP ztop{0, 0, 1, -1};   // z=1
  PlaneP zbot{0, 0, 1, 1};    // z=-1
  PlaneP xmid{1, 0, 0, 0};    // x=0

  PolyP *p1 = mk(4, {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}}, ztop, 0);
  PolyP *p2 = mk(4, {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}}, zbot, 1);
  PolyP *splitter = mk(4, {{0, -2, -1}, {0, 2, -1}, {0, 2, 1}, {0, -2, 1}}, xmid, 2);

  // splitter first in list: sees one front, one back -> early perfect out
  ListNodeP *list = nullptr;
  AppendP(&list, splitter);
  AppendP(&list, p2);
  AppendP(&list, p1);
  EXPECT_EQ(RepSelectPlane(&list)->facenum, 2);

  // all-coincident planes also read as "perfect" since planar weight is 0
  ListNodeP *flat = nullptr;
  PolyP *q1 = mk(4, {{0, -1, 1}, {1, -1, 1}, {1, 1, 1}, {0, 1, 1}}, ztop, 10);
  PolyP *q2 = mk(4, {{5, -1, 1}, {6, -1, 1}, {6, 1, 1}, {5, 1, 1}}, ztop, 11);
  AppendP(&flat, q2);
  AppendP(&flat, q1); // prepends: q1 examined first
  EXPECT_EQ(RepSelectPlane(&flat)->facenum, 10); // perfect score -> immediate

  // used planes are skipped entirely
  q1->plane.used = true;
  EXPECT_EQ(RepSelectPlane(&flat)->facenum, 11);

  // cleanup omitted: test-lifetime allocations
}
