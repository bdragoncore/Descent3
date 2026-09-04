/**
 * @file g3_instance_real_tests.cpp
 * @brief Unit tests for the g3 instance model-matrix stack (Phase 3).
 *
 * @details
 * Compiles the real `renderer/HardwareInstance.cpp`, `renderer/HardwareSetup.cpp`,
 * `renderer/HardwareTransforms.cpp`, `renderer/HardwareGlobalVars.cpp`,
 * `renderer/HardwarePoints.cpp`, `renderer/HardwareClipper.cpp` and
 * `renderer/HardwareDraw.cpp` against stubbed renderer entry points and asserts
 * the behavior of the instance transform stack.
 *
 * These tests are the regression guard for the GLM refactor (docs/g3_replacement.md,
 * Phase 3): the old view-rebasing trick in `g3_StartInstanceMatrix` is replaced by
 * a GLM model-matrix stack. The equivalence tests pin that the new behavior
 * produces the same results as the old re-based state, and the refactor-pinning
 * tests assert the new invariant that the global view state is never re-based.
 *
 * @par Source
 * `renderer/HardwareInstance.cpp`, `renderer/HardwareSetup.cpp`,
 * `renderer/HardwareTransforms.cpp`, `renderer/HardwareGlobalVars.cpp`,
 * `renderer/HardwarePoints.cpp`, `renderer/HardwareClipper.cpp`,
 * `renderer/HardwareDraw.cpp`
 * @par Harness
 * `g3_instance_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Real renderer sources compiled directly into the test executable with stubbed
 * `rend_*` entry points.
 *
 * @ingroup descent3_tests
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "pstypes.h"
#include "vecmat.h"
#include "3d.h"
#include "renderer.h"
#include "HardwareInternal.h"

// ---- Stubs for renderer entry points used by the real g3 code ----

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

void rend_TransformSetToPassthru(void) {}
void rend_TransformSetViewport(int lx, int ty, int width, int height) {
  (void)lx; (void)ty; (void)width; (void)height;
}
void rend_TransformSetProjection(float trans[4][4]) { (void)trans; }
void rend_TransformSetModelView(float trans[4][4]) { (void)trans; }

// Draw entry points used by HardwareDraw.cpp.
void rend_SetFlatColor(ddgr_color color) { (void)color; }
void rend_DrawLine(int x1, int y1, int x2, int y2) { (void)x1; (void)y1; (void)x2; (void)y2; }
void rend_DrawSpecialLine(g3Point *p0, g3Point *p1) { (void)p0; (void)p1; }
void rend_FillCircle(ddgr_color col, int x, int y, int rad) { (void)col; (void)x; (void)y; (void)rad; }
void rend_SetTextureType(texture_type tt) { (void)tt; }
void rend_SetLighting(light_state ls) { (void)ls; }
void rend_DrawPolygon3D(int handle, g3Point **p, int nv, int map_type) {
  (void)handle; (void)p; (void)nv; (void)map_type;
}
void rend_DrawPolygonList3D(int handle, g3Point **p, int ntri, int map_type) {
  (void)handle; (void)p; (void)ntri; (void)map_type;
}

// Z_bias applied to the model-view translation (extern in HardwareTransforms.cpp;
// defined in HardwareOpenGL.cpp in the real build).
float Z_bias = 0.0f;

// Internal g3 helpers defined in renderer/HardwareSetup.cpp but not declared
// in any public header; forward-declared here for the tests.
void g3_GetViewPortMatrix(float *viewMat);
void g3_GetProjectionMatrix(float zoom, float *projMat);

// ---- Test helpers ----

// Builds the GLM model matrix for a g3 instance transform:
// model = translate(pos) * mat4(orient), matching the Phase 1 column-major
// convention (column 0 = rvec, column 1 = uvec, column 2 = fvec).
static glm::mat4 MakeModelMatrix(const vector &pos, const matrix &orient) {
  return glm::translate(glm::mat4(1.0f), glm::vec3(pos.x(), pos.y(), pos.z())) *
         glm::mat4(glm::vec4(orient.rvec.x(), orient.rvec.y(), orient.rvec.z(), 0.0f),
                   glm::vec4(orient.uvec.x(), orient.uvec.y(), orient.uvec.z(), 0.0f),
                   glm::vec4(orient.fvec.x(), orient.fvec.y(), orient.fvec.z(), 0.0f),
                   glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

// Returns the current base model-view matrix (from the true view state) as GLM.
static glm::mat4 GetBaseModelViewGLM() {
  float view[16];
  g3_GetModelViewMatrix(&View_position, &Unscaled_matrix, view);
  return glm::make_mat4x4(view);
}

// Asserts gTransformModelView equals the given GLM matrix (column-major).
static void ExpectModelViewEquals(const glm::mat4 &expected) {
  for (int c = 0; c < 4; c++) {
    for (int r = 0; r < 4; r++) {
      EXPECT_FLOAT_EQ(gTransformModelView[c][r], expected[c][r]);
    }
  }
}

static void ExpectVectorEq(const vector &a, const vector &b) {
  EXPECT_FLOAT_EQ(a.x(), b.x());
  EXPECT_FLOAT_EQ(a.y(), b.y());
  EXPECT_FLOAT_EQ(a.z(), b.z());
}

static void ExpectMatrixEq(const matrix &a, const matrix &b) {
  ExpectVectorEq(a.rvec, b.rvec);
  ExpectVectorEq(a.uvec, b.uvec);
  ExpectVectorEq(a.fvec, b.fvec);
}

class G3InstanceTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_test_proj_w = 800;
    g_test_proj_h = 600;
    g_test_screen_x = 0;
    g_test_screen_y = 0;
    g_test_screen_w = 800;
    g_test_screen_h = 600;
    Z_bias = 0.0f;
    g3_ForceTransformRefresh();
  }
};

// ---- Equivalence tests (pass against both the old re-based and new GLM code) ----

/**
 * @test G3InstanceTest.ModelViewEqualsViewTimesModel
 * @brief Verifies gTransformModelView after g3_StartInstanceMatrix equals
 *        view * model (the GLM product of the base view and the instance model).
 *
 * @details
 * The old code re-based the global view state and rebuilt the model-view from
 * it; the new code composes the base model-view with the instance model matrix.
 * Both must produce the same matrix.
 *
 * @see renderer/HardwareInstance.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, ModelViewEqualsViewTimesModel) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};

  glm::mat4 expected = GetBaseModelViewGLM() * MakeModelMatrix(objPos, objOrient);

  g3_StartInstanceMatrix(&objPos, &objOrient);
  ExpectModelViewEquals(expected);
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.ModelViewEqualsViewTimesModelNested
 * @brief Verifies the composed model-view for nested instances equals
 *        view * model1 * model2.
 *
 * @see renderer/HardwareInstance.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, ModelViewEqualsViewTimesModelNested) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix o1;
  vm_AnglesToMatrix(&o1, 0x1000, 0x2000, 0x0500);
  vector p1{3, -2, 8};
  matrix o2;
  vm_AnglesToMatrix(&o2, 0x0800, 0x0400, 0x0C00);
  vector p2{-1, 4, 2};

  glm::mat4 expected = GetBaseModelViewGLM() * MakeModelMatrix(p1, o1) * MakeModelMatrix(p2, o2);

  g3_StartInstanceMatrix(&p1, &o1);
  g3_StartInstanceMatrix(&p2, &o2);
  ExpectModelViewEquals(expected);
  g3_DoneInstance();
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.RotatePointEquivalence
 * @brief Verifies g3_RotatePoint during instancing produces the same view-space
 *        position as the old re-based formula.
 *
 * @details
 * The old code rotated object-space vertices against the re-based view state;
 * the new code transforms the vertex to world space first. Both must agree.
 *
 * @see renderer/HardwarePoints.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, RotatePointEquivalence) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  vector trueViewPos = View_position;
  matrix trueViewMat = View_matrix;

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  vector src{1, 2, 3};
  g3Point pt;
  g3_RotatePoint(&pt, &src);

  vector world = (src * ~objOrient) + objPos;
  vector expected = (world - trueViewPos) * trueViewMat;

  ExpectVectorEq(pt.p3_vec, expected);
  ExpectVectorEq(pt.p3_vecPreRot, src);
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.GetViewPositionEquivalence
 * @brief Verifies g3_GetViewPosition during instancing returns the view position
 *        in the object's local frame.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, GetViewPositionEquivalence) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  vector trueViewPos = View_position;

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  vector vp;
  g3_GetViewPosition(&vp);
  vector expected = (trueViewPos - objPos) * objOrient;

  ExpectVectorEq(vp, expected);
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.GetUnscaledMatrixEquivalence
 * @brief Verifies g3_GetUnscaledMatrix during instancing returns the unscaled
 *        matrix in the object's local frame.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, GetUnscaledMatrixEquivalence) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix trueUnscaled = Unscaled_matrix;

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  matrix m;
  g3_GetUnscaledMatrix(&m);
  matrix expected = ~objOrient * trueUnscaled;

  ExpectMatrixEq(m, expected);
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.CheckNormalFacingEquivalence
 * @brief Verifies g3_CheckNormalFacing during instancing agrees with the old
 *        re-based formula for an object-space point and normal.
 *
 * @see renderer/HardwareDraw.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, CheckNormalFacingEquivalence) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  vector trueViewPos = View_position;

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  vector v{1, 2, 3};
  vector norm{0, 0, 1};
  bool actual = g3_CheckNormalFacing(&v, &norm);

  vector world = (v * ~objOrient) + objPos;
  vector tempv = (trueViewPos - world) * objOrient;
  bool expected = (vm_Dot3Product(tempv, norm) > 0);

  EXPECT_EQ(actual, expected);
  g3_DoneInstance();
}

// ---- Refactor-pinning tests (fail against the old re-based code) ----

/**
 * @test G3InstanceTest.ViewStateUnchangedDuringInstance
 * @brief Verifies the global view state is NOT re-based during instancing.
 *
 * @details
 * The old code overwrote View_position/View_matrix/Unscaled_matrix with the
 * re-based values; the new code keeps them true and applies the model transform
 * in the consumers. This test fails on the old code and passes on the new.
 *
 * @see renderer/HardwareInstance.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, ViewStateUnchangedDuringInstance) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  vector vp0 = View_position;
  matrix vm0 = View_matrix;
  matrix um0 = Unscaled_matrix;

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  ExpectVectorEq(View_position, vp0);
  ExpectMatrixEq(View_matrix, vm0);
  ExpectMatrixEq(Unscaled_matrix, um0);
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.DoneInstanceRestoresViewState
 * @brief Verifies g3_DoneInstance restores the pre-instance view state and
 *        model-view matrix.
 *
 * @see renderer/HardwareInstance.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, DoneInstanceRestoresViewState) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  vector vp0 = View_position;
  matrix vm0 = View_matrix;
  matrix um0 = Unscaled_matrix;
  float mv0[4][4];
  memcpy(mv0, gTransformModelView, sizeof(mv0));

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);
  g3_DoneInstance();

  ExpectVectorEq(View_position, vp0);
  ExpectMatrixEq(View_matrix, vm0);
  ExpectMatrixEq(Unscaled_matrix, um0);
  for (int i = 0; i < 16; i++) {
    EXPECT_FLOAT_EQ(((float *)gTransformModelView)[i], ((float *)mv0)[i]);
  }
}

// ---- Phase 3 helper tests (g3_GetInstanceTransform / g3_UpdateModelViewMatrix) ----

/**
 * @test G3InstanceTest.GetInstanceTransformIdentity
 * @brief Verifies g3_GetInstanceTransform returns identity/zero when no
 *        instance is active.
 *
 * @see renderer/HardwareInstance.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, GetInstanceTransformIdentity) {
  matrix orient;
  vector pos;
  g3_GetInstanceTransform(&orient, &pos);
  EXPECT_FLOAT_EQ(orient.rvec.x(), 1.0f);
  EXPECT_FLOAT_EQ(orient.uvec.y(), 1.0f);
  EXPECT_FLOAT_EQ(orient.fvec.z(), 1.0f);
  EXPECT_FLOAT_EQ(orient.rvec.y(), 0.0f);
  EXPECT_FLOAT_EQ(orient.rvec.z(), 0.0f);
  EXPECT_FLOAT_EQ(orient.uvec.x(), 0.0f);
  EXPECT_FLOAT_EQ(orient.uvec.z(), 0.0f);
  EXPECT_FLOAT_EQ(orient.fvec.x(), 0.0f);
  EXPECT_FLOAT_EQ(orient.fvec.y(), 0.0f);
  EXPECT_FLOAT_EQ(pos.x(), 0.0f);
  EXPECT_FLOAT_EQ(pos.y(), 0.0f);
  EXPECT_FLOAT_EQ(pos.z(), 0.0f);
}

/**
 * @test G3InstanceTest.GetInstanceTransformSingle
 * @brief Verifies g3_GetInstanceTransform returns the instance orient/pos.
 *
 * @see renderer/HardwareInstance.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, GetInstanceTransformSingle) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  matrix orient;
  vector pos;
  g3_GetInstanceTransform(&orient, &pos);
  ExpectMatrixEq(orient, objOrient);
  ExpectVectorEq(pos, objPos);
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.GetInstanceTransformNested
 * @brief Verifies g3_GetInstanceTransform composes nested instance transforms.
 *
 * @details
 * For nested instances the composed transform is
 * orient_total = orient1 * orient2 and pos_total = pos1 + orient1 * pos2
 * (column convention), so world = src * ~orient_total + pos_total.
 *
 * @see renderer/HardwareInstance.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, GetInstanceTransformNested) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix o1;
  vm_AnglesToMatrix(&o1, 0x1000, 0x2000, 0x0500);
  vector p1{3, -2, 8};
  matrix o2;
  vm_AnglesToMatrix(&o2, 0x0800, 0x0400, 0x0C00);
  vector p2{-1, 4, 2};
  g3_StartInstanceMatrix(&p1, &o1);
  g3_StartInstanceMatrix(&p2, &o2);

  matrix orient;
  vector pos;
  g3_GetInstanceTransform(&orient, &pos);

  matrix expectedOrient = o1 * o2;
  vector expectedPos;
  vm_MatrixMulVector(&expectedPos, &p2, &o1);
  expectedPos += p1;
  ExpectMatrixEq(orient, expectedOrient);
  ExpectVectorEq(pos, expectedPos);
  g3_DoneInstance();
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.UpdateModelViewMatrixZBias
 * @brief Verifies g3_UpdateModelViewMatrix preserves the instance model when
 *        Z_bias changes (the path rend_SetZBias uses during instancing).
 *
 * @details
 * The old rend_SetZBias rebuilt the model-view from the re-based globals; the
 * new code recomputes view * model so the instance transform is preserved.
 *
 * @see renderer/HardwareInstance.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, UpdateModelViewMatrixZBias) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  Z_bias = 0.5f;
  g3_UpdateModelViewMatrix();

  glm::mat4 expected = GetBaseModelViewGLM() * MakeModelMatrix(objPos, objOrient);
  ExpectModelViewEquals(expected);

  Z_bias = 0.0f;
  g3_DoneInstance();
}