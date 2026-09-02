/**
 * @file g3_transform_real_tests.cpp
 * @brief Unit tests for the g3 transform matrix code in the renderer.
 *
 * @details
 * Compiles the real `renderer/HardwareSetup.cpp`, `renderer/HardwareTransforms.cpp`
 * and `renderer/HardwareGlobalVars.cpp` against stubbed renderer entry points and
 * asserts the exact matrix values produced by the g3 transform pipeline.
 *
 * These tests are the regression guard for the GLM refactor (docs/g3_replacement.md,
 * Phase 1): the hand-rolled matrix code and the GLM-based replacement must produce
 * byte-identical values, so every assertion here pins down the current behavior.
 *
 * @par Source
 * `renderer/HardwareSetup.cpp`, `renderer/HardwareTransforms.cpp`, `renderer/HardwareGlobalVars.cpp`
 * @par Harness
 * `g3_transform_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Real renderer sources compiled directly into the test executable with stubbed
 * `rend_*` entry points and point-list helpers.
 *
 * @ingroup descent3_tests
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <cmath>

#include "pstypes.h"
#include "vecmat.h"
#include "3d.h"
#include "renderer.h"
#include "HardwareInternal.h"

// ---- Stubs for renderer entry points used by the real g3 transform code ----

// Configurable viewport state for the stubs.
static int g_test_proj_w = 800;
static int g_test_proj_h = 600;
static int g_test_screen_x = 0;
static int g_test_screen_y = 0;
static int g_test_screen_w = 800;
static int g_test_screen_h = 600;

void rend_GetProjectionParameters(int *width, int *height) {
  *width = g_test_proj_w;
  *height = g_test_proj_h;
}

void rend_GetProjectionScreenParameters(int &screenLX, int &screenTY, int &screenW, int &screenH) {
  screenLX = g_test_screen_x;
  screenTY = g_test_screen_y;
  screenW = g_test_screen_w;
  screenH = g_test_screen_h;
}

// Call counters for the transform-set entry points.
static int g_passthru_calls = 0;
static int g_viewport_calls = 0;
static int g_projection_calls = 0;
static int g_modelview_calls = 0;

void rend_TransformSetToPassthru(void) { g_passthru_calls++; }
void rend_TransformSetViewport(int lx, int ty, int width, int height) {
  (void)lx; (void)ty; (void)width; (void)height;
  g_viewport_calls++;
}
void rend_TransformSetProjection(float trans[4][4]) { (void)trans; g_projection_calls++; }
void rend_TransformSetModelView(float trans[4][4]) { (void)trans; g_modelview_calls++; }

// Internal g3 helpers defined in renderer/HardwareSetup.cpp but not declared
// in any public header; forward-declared here for the tests.
void g3_GetViewPortMatrix(float *viewMat);
void g3_GetProjectionMatrix(float zoom, float *projMat);

// Z_bias applied to the model-view translation (extern in HardwareTransforms.cpp).
float Z_bias = 0.0f;

// Point-list helpers used by g3_StartFrame / g3_EndFrame.
void InitFreePoints(void) {}
#ifdef _DEBUG
void CheckTempPoints(void) {}
#endif
void g3_SetFarClipZ(float z) { Far_clip_z = z; }

class G3TransformTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_test_proj_w = 800;
    g_test_proj_h = 600;
    g_test_screen_x = 0;
    g_test_screen_y = 0;
    g_test_screen_w = 800;
    g_test_screen_h = 600;
    g_passthru_calls = 0;
    g_viewport_calls = 0;
    g_projection_calls = 0;
    g_modelview_calls = 0;
    Z_bias = 0.0f;
    g3_ForceTransformRefresh();
  }
};

// ---- g3_GetProjectionMatrix ----

/**
 * @test G3TransformTest.ProjectionMatrixLandscape
 * @brief Verifies the projection matrix for a landscape viewport.
 *
 * @details
 * 800x600 with zoom 1.0: aspect s = 4/3 > 1, so the horizontal scale is
 * oOT/s and the vertical scale is oOT, with oOT = 1/(zoom*3/4) = 4/3.
 * The near=0/far=infinity convention is pinned by [10]=1, [11]=1, [14]=-1.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ProjectionMatrixLandscape) {
  float m[16];
  g3_GetProjectionMatrix(1.0f, m);
  EXPECT_FLOAT_EQ(m[0], 1.0f);
  EXPECT_FLOAT_EQ(m[5], 4.0f / 3.0f);
  EXPECT_FLOAT_EQ(m[10], 1.0f);
  EXPECT_FLOAT_EQ(m[11], 1.0f);
  EXPECT_FLOAT_EQ(m[14], -1.0f);
  for (int i = 0; i < 16; i++) {
    if (i == 0 || i == 5 || i == 10 || i == 11 || i == 14) continue;
    EXPECT_FLOAT_EQ(m[i], 0.0f);
  }
}

/**
 * @test G3TransformTest.ProjectionMatrixZoom
 * @brief Verifies the projection matrix scales with zoom.
 *
 * @details
 * 800x600 with zoom 2.0: vertical_fov = 1.5, oOT = 2/3, so the horizontal
 * scale is (2/3)/(4/3) = 1/2 and the vertical scale is 2/3.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ProjectionMatrixZoom) {
  float m[16];
  g3_GetProjectionMatrix(2.0f, m);
  EXPECT_FLOAT_EQ(m[0], 0.5f);
  EXPECT_FLOAT_EQ(m[5], 2.0f / 3.0f);
  EXPECT_FLOAT_EQ(m[10], 1.0f);
  EXPECT_FLOAT_EQ(m[11], 1.0f);
  EXPECT_FLOAT_EQ(m[14], -1.0f);
}

/**
 * @test G3TransformTest.ProjectionMatrixPortrait
 * @brief Verifies the projection matrix for a portrait viewport.
 *
 * @details
 * 600x800 with zoom 1.0: aspect s = 3/4 <= 1, so the horizontal scale is
 * oOT and the vertical scale is oOT*s.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ProjectionMatrixPortrait) {
  g_test_proj_w = 600;
  g_test_proj_h = 800;
  float m[16];
  g3_GetProjectionMatrix(1.0f, m);
  EXPECT_FLOAT_EQ(m[0], 4.0f / 3.0f);
  EXPECT_FLOAT_EQ(m[5], 1.0f);
  EXPECT_FLOAT_EQ(m[10], 1.0f);
  EXPECT_FLOAT_EQ(m[11], 1.0f);
  EXPECT_FLOAT_EQ(m[14], -1.0f);
}

/**
 * @test G3TransformTest.ProjectionMatrixSquare
 * @brief Verifies the projection matrix for a square viewport.
 *
 * @details
 * 640x640 with zoom 1.0: aspect s = 1, both scales equal oOT = 4/3.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ProjectionMatrixSquare) {
  g_test_proj_w = 640;
  g_test_proj_h = 640;
  float m[16];
  g3_GetProjectionMatrix(1.0f, m);
  EXPECT_FLOAT_EQ(m[0], 4.0f / 3.0f);
  EXPECT_FLOAT_EQ(m[5], 4.0f / 3.0f);
  EXPECT_FLOAT_EQ(m[10], 1.0f);
  EXPECT_FLOAT_EQ(m[11], 1.0f);
  EXPECT_FLOAT_EQ(m[14], -1.0f);
}

// ---- g3_GetModelViewMatrix ----

/**
 * @test G3TransformTest.ModelViewIdentity
 * @brief Verifies the model-view matrix for an identity view.
 *
 * @details
 * Identity orientation at the origin produces the identity matrix.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ModelViewIdentity) {
  matrix id = IDENTITY_MATRIX;
  vector pos{0, 0, 0};
  float m[16];
  g3_GetModelViewMatrix(&pos, &id, m);
  EXPECT_FLOAT_EQ(m[0], 1.0f); EXPECT_FLOAT_EQ(m[1], 0.0f); EXPECT_FLOAT_EQ(m[2], 0.0f); EXPECT_FLOAT_EQ(m[3], 0.0f);
  EXPECT_FLOAT_EQ(m[4], 0.0f); EXPECT_FLOAT_EQ(m[5], 1.0f); EXPECT_FLOAT_EQ(m[6], 0.0f); EXPECT_FLOAT_EQ(m[7], 0.0f);
  EXPECT_FLOAT_EQ(m[8], 0.0f); EXPECT_FLOAT_EQ(m[9], 0.0f); EXPECT_FLOAT_EQ(m[10], 1.0f); EXPECT_FLOAT_EQ(m[11], 0.0f);
  EXPECT_FLOAT_EQ(m[12], 0.0f); EXPECT_FLOAT_EQ(m[13], 0.0f); EXPECT_FLOAT_EQ(m[14], 0.0f); EXPECT_FLOAT_EQ(m[15], 1.0f);
}

/**
 * @test G3TransformTest.ModelViewTranslation
 * @brief Verifies the model-view translation is the negated view position.
 *
 * @details
 * With an identity orientation the translation row is -viewPos.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ModelViewTranslation) {
  matrix id = IDENTITY_MATRIX;
  vector pos{10, 20, 30};
  float m[16];
  g3_GetModelViewMatrix(&pos, &id, m);
  EXPECT_FLOAT_EQ(m[12], -10.0f);
  EXPECT_FLOAT_EQ(m[13], -20.0f);
  EXPECT_FLOAT_EQ(m[14], -30.0f);
  EXPECT_FLOAT_EQ(m[0], 1.0f);
  EXPECT_FLOAT_EQ(m[5], 1.0f);
  EXPECT_FLOAT_EQ(m[10], 1.0f);
  EXPECT_FLOAT_EQ(m[15], 1.0f);
}

/**
 * @test G3TransformTest.ModelViewZBias
 * @brief Verifies Z_bias is NOT baked into the model-view matrix.
 *
 * @details
 * Z_bias is applied per-vertex in the vertex shader (u_z_bias), not in the
 * matrix: baking it into view-space Z shifted screen-space X/Y after the
 * perspective divide. The matrix holds the pure translation.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ModelViewZBias) {
  matrix id = IDENTITY_MATRIX;
  vector pos{10, 20, 30};
  Z_bias = 0.5f;
  float m[16];
  g3_GetModelViewMatrix(&pos, &id, m);
  EXPECT_FLOAT_EQ(m[14], -30.0f);
  EXPECT_FLOAT_EQ(m[12], -10.0f);
  EXPECT_FLOAT_EQ(m[13], -20.0f);
}

/**
 * @test G3TransformTest.ModelViewRotated
 * @brief Verifies the model-view matrix layout for a rotated view.
 *
 * @details
 * The rotation part is stored column-major (OpenGL convention): the first
 * column holds the x components of rvec/uvec/fvec, etc. The translation row
 * holds dot(-viewPos, basis) with Z_bias on the forward component.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ModelViewRotated) {
  matrix rot;
  vm_AnglesToMatrix(&rot, 0x2000, 0x1000, 0x3000);
  vector pos{5, -7, 11};
  float m[16];
  g3_GetModelViewMatrix(&pos, &rot, m);
  EXPECT_FLOAT_EQ(m[0], rot.rvec.x()); EXPECT_FLOAT_EQ(m[1], rot.uvec.x()); EXPECT_FLOAT_EQ(m[2], rot.fvec.x());
  EXPECT_FLOAT_EQ(m[4], rot.rvec.y()); EXPECT_FLOAT_EQ(m[5], rot.uvec.y()); EXPECT_FLOAT_EQ(m[6], rot.fvec.y());
  EXPECT_FLOAT_EQ(m[8], rot.rvec.z()); EXPECT_FLOAT_EQ(m[9], rot.uvec.z()); EXPECT_FLOAT_EQ(m[10], rot.fvec.z());
  vector neg{-5, 7, -11};
  EXPECT_FLOAT_EQ(m[12], vm_Dot3Product(neg, rot.rvec));
  EXPECT_FLOAT_EQ(m[13], vm_Dot3Product(neg, rot.uvec));
  EXPECT_FLOAT_EQ(m[14], vm_Dot3Product(neg, rot.fvec));
  EXPECT_FLOAT_EQ(m[3], 0.0f);
  EXPECT_FLOAT_EQ(m[7], 0.0f);
  EXPECT_FLOAT_EQ(m[11], 0.0f);
  EXPECT_FLOAT_EQ(m[15], 1.0f);
}

// ---- g3_GetViewPortMatrix ----

/**
 * @test G3TransformTest.ViewPortMatrix
 * @brief Verifies the viewport matrix for a full-screen viewport.
 *
 * @details
 * 800x600 at (0,0): scale x by w/2, flip y by -h/2, translate to the center.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ViewPortMatrix) {
  float m[16];
  g3_GetViewPortMatrix(m);
  EXPECT_FLOAT_EQ(m[0], 400.0f);
  EXPECT_FLOAT_EQ(m[5], -300.0f);
  EXPECT_FLOAT_EQ(m[12], 400.0f);
  EXPECT_FLOAT_EQ(m[13], 300.0f);
  EXPECT_FLOAT_EQ(m[10], 1.0f);
  EXPECT_FLOAT_EQ(m[15], 1.0f);
  for (int i = 0; i < 16; i++) {
    if (i == 0 || i == 5 || i == 10 || i == 12 || i == 13 || i == 15) continue;
    EXPECT_FLOAT_EQ(m[i], 0.0f);
  }
}

/**
 * @test G3TransformTest.ViewPortMatrixOffset
 * @brief Verifies the viewport matrix honors the viewport origin.
 *
 * @details
 * 800x600 at (100,50): the translation is the center plus the origin.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, ViewPortMatrixOffset) {
  g_test_screen_x = 100;
  g_test_screen_y = 50;
  float m[16];
  g3_GetViewPortMatrix(m);
  EXPECT_FLOAT_EQ(m[12], 500.0f);
  EXPECT_FLOAT_EQ(m[13], 350.0f);
  EXPECT_FLOAT_EQ(m[0], 400.0f);
  EXPECT_FLOAT_EQ(m[5], -300.0f);
}

// ---- g3_TransformMult / g3_TransformTrans ----

/**
 * @test G3TransformTest.TransformMult
 * @brief Verifies 4x4 matrix multiplication.
 *
 * @details
 * res[y][x] = sum_k a[y][k] * b[k][x] for two known matrices.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, TransformMult) {
  float a[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  float b[4][4] = {{16, 15, 14, 13}, {12, 11, 10, 9}, {8, 7, 6, 5}, {4, 3, 2, 1}};
  float res[4][4];
  g3_TransformMult(res, a, b);
  EXPECT_FLOAT_EQ(res[0][0], 80.0f);  EXPECT_FLOAT_EQ(res[0][1], 70.0f);
  EXPECT_FLOAT_EQ(res[0][2], 60.0f);  EXPECT_FLOAT_EQ(res[0][3], 50.0f);
  EXPECT_FLOAT_EQ(res[1][0], 240.0f); EXPECT_FLOAT_EQ(res[1][1], 214.0f);
  EXPECT_FLOAT_EQ(res[1][2], 188.0f); EXPECT_FLOAT_EQ(res[1][3], 162.0f);
  EXPECT_FLOAT_EQ(res[2][0], 400.0f); EXPECT_FLOAT_EQ(res[2][1], 358.0f);
  EXPECT_FLOAT_EQ(res[2][2], 316.0f); EXPECT_FLOAT_EQ(res[2][3], 274.0f);
  EXPECT_FLOAT_EQ(res[3][0], 560.0f); EXPECT_FLOAT_EQ(res[3][1], 502.0f);
  EXPECT_FLOAT_EQ(res[3][2], 444.0f); EXPECT_FLOAT_EQ(res[3][3], 386.0f);
}

/**
 * @test G3TransformTest.TransformTrans
 * @brief Verifies 4x4 matrix transpose.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, TransformTrans) {
  float t[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  float res[4][4];
  g3_TransformTrans(res, t);
  EXPECT_FLOAT_EQ(res[0][0], 1.0f);  EXPECT_FLOAT_EQ(res[0][1], 5.0f);
  EXPECT_FLOAT_EQ(res[0][2], 9.0f);  EXPECT_FLOAT_EQ(res[0][3], 13.0f);
  EXPECT_FLOAT_EQ(res[1][0], 2.0f);  EXPECT_FLOAT_EQ(res[1][1], 6.0f);
  EXPECT_FLOAT_EQ(res[1][2], 10.0f); EXPECT_FLOAT_EQ(res[1][3], 14.0f);
  EXPECT_FLOAT_EQ(res[2][0], 3.0f);  EXPECT_FLOAT_EQ(res[2][1], 7.0f);
  EXPECT_FLOAT_EQ(res[2][2], 11.0f); EXPECT_FLOAT_EQ(res[2][3], 15.0f);
  EXPECT_FLOAT_EQ(res[3][0], 4.0f);  EXPECT_FLOAT_EQ(res[3][1], 8.0f);
  EXPECT_FLOAT_EQ(res[3][2], 12.0f); EXPECT_FLOAT_EQ(res[3][3], 16.0f);
}

// ---- g3_StartFrame ----

/**
 * @test G3TransformTest.StartFrameSetsGlobals
 * @brief Verifies the globals set by g3_StartFrame.
 *
 * @details
 * 800x600, identity view at (10,20,30), zoom 1.0. The window aspect is
 * 600/800 = 3/4, and zoom is converted to a vertical FOV of 3/4, so the
 * resulting Matrix_scale is {1, 4/3, 1} and View_zoom is 3/4.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, StartFrameSetsGlobals) {
  matrix id = IDENTITY_MATRIX;
  vector pos{10, 20, 30};
  g3_StartFrame(&pos, &id, 1.0f);

  EXPECT_EQ(Window_width, 800);
  EXPECT_EQ(Window_height, 600);
  EXPECT_FLOAT_EQ(Window_w2, 400.0f);
  EXPECT_FLOAT_EQ(Window_h2, 300.0f);
  EXPECT_FLOAT_EQ(View_zoom, 0.75f);
  EXPECT_FLOAT_EQ(View_position.x(), 10.0f);
  EXPECT_FLOAT_EQ(View_position.y(), 20.0f);
  EXPECT_FLOAT_EQ(View_position.z(), 30.0f);
  EXPECT_FLOAT_EQ(Matrix_scale.x(), 1.0f);
  EXPECT_FLOAT_EQ(Matrix_scale.y(), 4.0f / 3.0f);
  EXPECT_FLOAT_EQ(Matrix_scale.z(), 1.0f);
  EXPECT_FLOAT_EQ(View_matrix.rvec.x(), 1.0f);
  EXPECT_FLOAT_EQ(View_matrix.uvec.y(), 4.0f / 3.0f);
  EXPECT_FLOAT_EQ(View_matrix.fvec.z(), 1.0f);
  EXPECT_FLOAT_EQ(Far_clip_z, FLT_MAX);
}

/**
 * @test G3TransformTest.StartFrameFullTransform
 * @brief Verifies gTransformFull is ModelView * Projection * ViewPort.
 *
 * @details
 * After g3_StartFrame the full transform must equal the product of the three
 * component matrices, computed independently with g3_TransformMult.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, StartFrameFullTransform) {
  matrix id = IDENTITY_MATRIX;
  vector pos{10, 20, 30};
  g3_StartFrame(&pos, &id, 1.0f);

  float expected[4][4];
  g3_TransformMult(expected, gTransformModelView, gTransformProjection);
  g3_TransformMult(expected, expected, gTransformViewPort);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      EXPECT_FLOAT_EQ(gTransformFull[y][x], expected[y][x]);
    }
  }
}

/**
 * @test G3TransformTest.StartFrameComponentMatrices
 * @brief Verifies the component matrices set by g3_StartFrame.
 *
 * @details
 * The viewport, projection and model-view matrices must match the values
 * produced by the individual getters for the same viewport/view state.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, StartFrameComponentMatrices) {
  matrix id = IDENTITY_MATRIX;
  vector pos{10, 20, 30};
  g3_StartFrame(&pos, &id, 1.0f);

  float vp[16];
  g3_GetViewPortMatrix(vp);
  for (int i = 0; i < 16; i++) EXPECT_FLOAT_EQ(((float *)gTransformViewPort)[i], vp[i]);

  float proj[16];
  g3_GetProjectionMatrix(1.0f, proj);
  for (int i = 0; i < 16; i++) EXPECT_FLOAT_EQ(((float *)gTransformProjection)[i], proj[i]);

  float mv[16];
  g3_GetModelViewMatrix(&pos, &id, mv);
  for (int i = 0; i < 16; i++) EXPECT_FLOAT_EQ(((float *)gTransformModelView)[i], mv[i]);
}

/**
 * @test G3TransformTest.GetViewState
 * @brief Verifies the g3_Get* accessors return the frame state.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, GetViewState) {
  matrix id = IDENTITY_MATRIX;
  vector pos{10, 20, 30};
  g3_StartFrame(&pos, &id, 1.0f);

  vector vp;
  g3_GetViewPosition(&vp);
  EXPECT_FLOAT_EQ(vp.x(), 10.0f);
  EXPECT_FLOAT_EQ(vp.y(), 20.0f);
  EXPECT_FLOAT_EQ(vp.z(), 30.0f);

  matrix m;
  g3_GetUnscaledMatrix(&m);
  EXPECT_FLOAT_EQ(m.rvec.x(), 1.0f);
  EXPECT_FLOAT_EQ(m.uvec.y(), 1.0f);
  EXPECT_FLOAT_EQ(m.fvec.z(), 1.0f);

  g3_GetViewMatrix(&m);
  EXPECT_FLOAT_EQ(m.rvec.x(), 1.0f);
  EXPECT_FLOAT_EQ(m.uvec.y(), 4.0f / 3.0f);
  EXPECT_FLOAT_EQ(m.fvec.z(), 1.0f);

  vector ms;
  g3_GetMatrixScale(&ms);
  EXPECT_FLOAT_EQ(ms.x(), 1.0f);
  EXPECT_FLOAT_EQ(ms.y(), 4.0f / 3.0f);
  EXPECT_FLOAT_EQ(ms.z(), 1.0f);
}

// ---- g3_UpdateFullTransform ----

/**
 * @test G3TransformTest.UpdateFullTransform
 * @brief Verifies g3_UpdateFullTransform composes the three matrices.
 *
 * @details
 * Sets the component matrices directly and checks the full transform equals
 * ModelView * Projection * ViewPort.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, UpdateFullTransform) {
  float mv[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
  float proj[4][4] = {{2, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 1, 1}, {0, 0, -1, 0}};
  float vp[4][4] = {{400, 0, 0, 0}, {0, -300, 0, 0}, {0, 0, 1, 0}, {400, 300, 0, 1}};
  memcpy(gTransformModelView, mv, sizeof(mv));
  memcpy(gTransformProjection, proj, sizeof(proj));
  memcpy(gTransformViewPort, vp, sizeof(vp));

  g3_UpdateFullTransform();

  float expected[4][4];
  g3_TransformMult(expected, mv, proj);
  g3_TransformMult(expected, expected, vp);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      EXPECT_FLOAT_EQ(gTransformFull[y][x], expected[y][x]);
    }
  }
}

// ---- g3_RefreshTransforms ----

/**
 * @test G3TransformTest.RefreshTransformsPassthru
 * @brief Verifies passthru mode calls rend_TransformSetToPassthru once.
 *
 * @details
 * The passthru state is cached, so a second call with the same mode is a no-op.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, RefreshTransformsPassthru) {
  g3_RefreshTransforms(true);
  EXPECT_EQ(g_passthru_calls, 1);
  EXPECT_EQ(g_viewport_calls, 0);
  EXPECT_EQ(g_projection_calls, 0);
  EXPECT_EQ(g_modelview_calls, 0);

  g3_RefreshTransforms(true);
  EXPECT_EQ(g_passthru_calls, 1);
}

/**
 * @test G3TransformTest.RefreshTransformsFull
 * @brief Verifies full mode sets viewport, projection and model-view.
 *
 * @details
 * Only the passthru state is cached; full-transform mode re-sets the
 * viewport, projection and model-view matrices on every call.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, RefreshTransformsFull) {
  g3_RefreshTransforms(false);
  EXPECT_EQ(g_passthru_calls, 0);
  EXPECT_EQ(g_viewport_calls, 1);
  EXPECT_EQ(g_projection_calls, 1);
  EXPECT_EQ(g_modelview_calls, 1);

  g3_RefreshTransforms(false);
  EXPECT_EQ(g_viewport_calls, 2);
  EXPECT_EQ(g_projection_calls, 2);
  EXPECT_EQ(g_modelview_calls, 2);
}

/**
 * @test G3TransformTest.RefreshTransformsSwitch
 * @brief Verifies switching between passthru and full modes re-sets state.
 *
 * @see renderer/HardwareTransforms.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, RefreshTransformsSwitch) {
  g3_RefreshTransforms(true);
  g3_RefreshTransforms(false);
  EXPECT_EQ(g_passthru_calls, 1);
  EXPECT_EQ(g_viewport_calls, 1);
  EXPECT_EQ(g_projection_calls, 1);
  EXPECT_EQ(g_modelview_calls, 1);

  g3_RefreshTransforms(true);
  EXPECT_EQ(g_passthru_calls, 2);
}

// ---- g3_SetAspectRatio / g3_GetAspectRatio ----

/**
 * @test G3TransformTest.AspectRatio
 * @brief Verifies the user-specified aspect ratio is stored and returned.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3TransformTest, AspectRatio) {
  g3_SetAspectRatio(1.5f);
  EXPECT_FLOAT_EQ(g3_GetAspectRatio(), 1.5f);
  g3_SetAspectRatio(0.0f);
  EXPECT_FLOAT_EQ(g3_GetAspectRatio(), 0.0f);
}