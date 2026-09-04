/**
 * @file g3_drawpath_real_tests.cpp
 * @brief Unit tests for the g3 3D polygon draw path in the renderer.
 *
 * @details
 * Compiles the real `renderer/HardwareDraw.cpp` and `renderer/HardwareGlobalVars.cpp`
 * against stubbed renderer entry points and asserts that the 3D polygon draw path
 * (`g3_DrawPoly` / `g3_DrawPolyList`) submits **world-space** vertices straight to
 * `rend_DrawPolygon3D` / `rend_DrawPolygonList3D` without any CPU-side projection
 * or clipping.
 *
 * These tests are the regression guard for docs/g3_replacement.md, Phase 2: the
 * GPU performs projection and clipping, so the draw path must never call
 * `g3_ProjectPoint` or `g3_ClipPolygon`, and must submit polygons even when they
 * are fully off-screen (the old software path would have clipped them away and
 * returned 0 without drawing).
 *
 * @par Source
 * `renderer/HardwareDraw.cpp`, `renderer/HardwareGlobalVars.cpp`
 * @par Harness
 * `g3_drawpath_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Real renderer sources compiled directly into the test executable with stubbed
 * `rend_*` entry points and g3 point helpers.
 *
 * @ingroup descent3_tests
 */

#include <gtest/gtest.h>
#include <cstdint>

#include "pstypes.h"
#include "vecmat.h"
#include "3d.h"
#include "renderer.h"
#include "HardwareInternal.h"

// ---- Call counters for the g3 helpers the draw path must NOT use ----

static int g_project_calls = 0;
static int g_clip_calls = 0;
static int g_rotate_calls = 0;
static int g_clipline_calls = 0;

// g3_ProjectPoint is used by the 2D/overlay paths (g3_DrawLine, g3_DrawSphere,
// g3_DrawBox) but must never be reached from the 3D polygon draw path.
void g3_ProjectPoint(g3Point *point) {
  g_project_calls++;
  point->p3_flags |= PF_PROJECTED;
}

// g3_RotatePoint is used by the bitmap/line helpers to build world-space
// vertices; it must never be reached from g3_DrawPoly / g3_DrawPolyList.
uint8_t g3_RotatePoint(g3Point *dest, vector *src) {
  g_rotate_calls++;
  dest->p3_vec = *src;
  return 0;
}

// g3_ClipPolygon is used by portal/mirror culling and the editor, but must
// never be reached from the 3D polygon draw path.
g3Point **g3_ClipPolygon(g3Point **pointlist, int *nv, g3Codes *cc) {
  g_clip_calls++;
  return pointlist;
}

// ClipLine is used by the 2D line helpers, never the polygon draw path.
void ClipLine(g3Point **p0, g3Point **p1, uint8_t codes_or) {
  g_clipline_calls++;
}

void FreeTempPoint(g3Point *p) { (void)p; }

// g3_GetUnscaledMatrix is defined in renderer/HardwareSetup.cpp (not compiled
// here); stub it for the bitmap helpers.
void g3_GetUnscaledMatrix(matrix *mat) { *mat = IDENTITY_MATRIX; }

// ---- Stubs for renderer entry points used by HardwareDraw.cpp ----

static int g_poly3d_calls = 0;
static int g_poly3d_handle = -1;
static int g_poly3d_nv = 0;
static int g_poly3d_map_type = -1;
static g3Point **g_poly3d_points = nullptr;

void rend_DrawPolygon3D(int handle, g3Point **p, int nv, int map_type) {
  g_poly3d_calls++;
  g_poly3d_handle = handle;
  g_poly3d_nv = nv;
  g_poly3d_map_type = map_type;
  g_poly3d_points = p;
}

static int g_polylist_calls = 0;
static int g_polylist_handle = -1;
static int g_polylist_ntri = 0;
static int g_polylist_map_type = -1;
static g3Point **g_polylist_points = nullptr;

void rend_DrawPolygonList3D(int handle, g3Point **p, int ntri, int map_type) {
  g_polylist_calls++;
  g_polylist_handle = handle;
  g_polylist_ntri = ntri;
  g_polylist_map_type = map_type;
  g_polylist_points = p;
}

void rend_SetFlatColor(ddgr_color color) { (void)color; }
void rend_DrawLine(int x1, int y1, int x2, int y2) {
  (void)x1; (void)y1; (void)x2; (void)y2;
}
void rend_DrawSpecialLine(g3Point *p0, g3Point *p1) { (void)p0; (void)p1; }
void rend_FillCircle(ddgr_color col, int x, int y, int rad) {
  (void)col; (void)x; (void)y; (void)rad;
}
void rend_SetTextureType(texture_type t) { (void)t; }
void rend_SetLighting(light_state s) { (void)s; }

// ---- Stubs for the transform pipeline (HardwareTransforms.cpp / HardwareInstance.cpp) ----

// Z_bias applied to the model-view translation (extern in HardwareTransforms.cpp).
float Z_bias = 0.0f;

void rend_TransformSetToPassthru(void) {}
void rend_TransformSetViewport(int lx, int ty, int width, int height) {
  (void)lx; (void)ty; (void)width; (void)height;
}
void rend_TransformSetProjection(float trans[4][4]) { (void)trans; }
void rend_TransformSetModelView(float trans[4][4]) { (void)trans; }
void rend_GetProjectionScreenParameters(int &screenLX, int &screenTY, int &screenW, int &screenH) {
  screenLX = 0; screenTY = 0; screenW = 800; screenH = 600;
}

class G3DrawPathTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_project_calls = 0;
    g_clip_calls = 0;
    g_rotate_calls = 0;
    g_clipline_calls = 0;
    g_poly3d_calls = 0;
    g_poly3d_handle = -1;
    g_poly3d_nv = 0;
    g_poly3d_map_type = -1;
    g_poly3d_points = nullptr;
    g_polylist_calls = 0;
    g_polylist_handle = -1;
    g_polylist_ntri = 0;
    g_polylist_map_type = -1;
    g_polylist_points = nullptr;
  }
};

// Builds a triangle of world-space points. p3_vecPreRot holds the original
// world-space position; p3_vec holds the view-space position. The draw path
// must submit p3_vecPreRot (world space) and let the GPU project + clip.
static void MakeWorldSpaceTriangle(g3Point pts[3]) {
  for (int i = 0; i < 3; i++) {
    pts[i].p3_sx = 0.0f;
    pts[i].p3_sy = 0.0f;
    pts[i].p3_codes = 0;
    pts[i].p3_flags = PF_UV | PF_L;
    pts[i].p3_vec = vector{float(i), 0.0f, 10.0f};
    pts[i].p3_vecPreRot = vector{float(i), 0.0f, 10.0f};
    pts[i].p3_uvl.u = float(i) * 0.5f;
    pts[i].p3_uvl.v = 0.0f;
    pts[i].p3_uvl.l = 1.0f;
  }
}

/**
 * @test G3DrawPathTest.DrawPolySubmitsWorldSpaceVerts
 * @brief Verifies g3_DrawPoly forwards world-space vertices to the GPU.
 *
 * @details
 * The polygon is fully behind the camera (all p3_codes = CC_BEHIND). The old
 * software path would have clipped it away and returned 0 without drawing; the
 * GPU path must submit it unchanged. g3_ProjectPoint and g3_ClipPolygon must
 * not be called.
 *
 * @see renderer/HardwareDraw.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3DrawPathTest, DrawPolySubmitsWorldSpaceVerts) {
  g3Point pts[3];
  MakeWorldSpaceTriangle(pts);
  for (int i = 0; i < 3; i++) {
    pts[i].p3_codes = CC_BEHIND;
  }
  g3Point *pointlist[3] = {&pts[0], &pts[1], &pts[2]};

  int drew = g3_DrawPoly(3, pointlist, 42, MAP_TYPE_BITMAP);

  EXPECT_EQ(drew, 1);
  EXPECT_EQ(g_poly3d_calls, 1);
  EXPECT_EQ(g_poly3d_handle, 42);
  EXPECT_EQ(g_poly3d_nv, 3);
  EXPECT_EQ(g_poly3d_map_type, MAP_TYPE_BITMAP);
  EXPECT_EQ(g_poly3d_points, pointlist);
  EXPECT_EQ(g_project_calls, 0);
  EXPECT_EQ(g_clip_calls, 0);
  EXPECT_EQ(g_rotate_calls, 0);
}

/**
 * @test G3DrawPathTest.DrawPolyListSubmitsWorldSpaceVerts
 * @brief Verifies g3_DrawPolyList forwards world-space vertices to the GPU.
 *
 * @details
 * Same guarantee as g3_DrawPoly for the batched triangle-list path used by the
 * terrain renderer.
 *
 * @see renderer/HardwareDraw.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3DrawPathTest, DrawPolyListSubmitsWorldSpaceVerts) {
  g3Point pts[3];
  MakeWorldSpaceTriangle(pts);
  g3Point *pointlist[3] = {&pts[0], &pts[1], &pts[2]};

  int drew = g3_DrawPolyList(1, pointlist, 7, MAP_TYPE_LIGHTMAP);

  EXPECT_EQ(drew, 1);
  EXPECT_EQ(g_polylist_calls, 1);
  EXPECT_EQ(g_polylist_handle, 7);
  EXPECT_EQ(g_polylist_ntri, 1);
  EXPECT_EQ(g_polylist_map_type, MAP_TYPE_LIGHTMAP);
  EXPECT_EQ(g_polylist_points, pointlist);
  EXPECT_EQ(g_project_calls, 0);
  EXPECT_EQ(g_clip_calls, 0);
  EXPECT_EQ(g_rotate_calls, 0);
}

/**
 * @test G3DrawPathTest.DrawPolyPreservesPreRotAndFlags
 * @brief Verifies the submitted points keep their world-space data.
 *
 * @details
 * The points passed to rend_DrawPolygon3D must still carry p3_vecPreRot (the
 * world-space position the GPU shader consumes) and must not be marked
 * PF_PROJECTED by any CPU-side projection.
 *
 * @see renderer/HardwareDraw.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3DrawPathTest, DrawPolyPreservesPreRotAndFlags) {
  g3Point pts[3];
  MakeWorldSpaceTriangle(pts);
  g3Point *pointlist[3] = {&pts[0], &pts[1], &pts[2]};

  g3_DrawPoly(3, pointlist, 0, MAP_TYPE_BITMAP);

  ASSERT_EQ(g_poly3d_calls, 1);
  for (int i = 0; i < 3; i++) {
    EXPECT_EQ(g_poly3d_points[i], &pts[i]);
    EXPECT_FLOAT_EQ(g_poly3d_points[i]->p3_vecPreRot.x(), float(i));
    EXPECT_FLOAT_EQ(g_poly3d_points[i]->p3_vecPreRot.z(), 10.0f);
    EXPECT_EQ(g_poly3d_points[i]->p3_flags & PF_PROJECTED, 0);
    EXPECT_NE(g_poly3d_points[i]->p3_flags & PF_UV, 0);
    EXPECT_NE(g_poly3d_points[i]->p3_flags & PF_L, 0);
  }
}

/**
 * @test G3DrawPathTest.DrawPolyIgnoresClipCodes
 * @brief Verifies the clip_codes parameter is ignored by the draw path.
 *
 * @details
 * The old software path used clip_codes to decide whether to clip; the GPU
 * path ignores it entirely and always submits the polygon.
 *
 * @see renderer/HardwareDraw.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3DrawPathTest, DrawPolyIgnoresClipCodes) {
  g3Point pts[3];
  MakeWorldSpaceTriangle(pts);
  g3Point *pointlist[3] = {&pts[0], &pts[1], &pts[2]};
  g3Codes cc;
  cc.cc_or = CC_BEHIND;
  cc.cc_and = CC_BEHIND;

  g3_DrawPoly(3, pointlist, 0, MAP_TYPE_BITMAP, &cc);

  EXPECT_EQ(g_poly3d_calls, 1);
  EXPECT_EQ(g_clip_calls, 0);
  EXPECT_EQ(g_project_calls, 0);
}