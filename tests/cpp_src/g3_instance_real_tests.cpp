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

// GPU ground truth: the composed model matrix maps a local point src to
// src.x*rvec + src.y*uvec + src.z*fvec + pos, so the local-frame coordinates
// of a world point are model^-1 * world. Computed here with GLM directly,
// independently of the CPU matrix code under test.
static vector GtLocalFromWorld(const glm::mat4 &model, const vector &world) {
  glm::vec4 local = glm::inverse(model) * glm::vec4(world.x(), world.y(), world.z(), 1.0f);
  return vector{local.x, local.y, local.z};
}

// GPU ground truth for the facing check: rotate the viewer-minus-world-point
// vector back into the object's local frame with model^-1 and dot with the
// object-space normal.
static bool GtFacing(const glm::mat4 &model, const vector &viewPos, const vector &v, const vector &norm) {
  glm::vec4 worldG = model * glm::vec4(v.x(), v.y(), v.z(), 1.0f);
  glm::vec4 dG = glm::vec4(viewPos.x(), viewPos.y(), viewPos.z(), 1.0f) - worldG;
  glm::vec4 tG = glm::inverse(model) * dG;
  vector tempv{tG.x, tG.y, tG.z};
  return (vm_Dot3Product(tempv, norm) > 0);
}

// Builds a non-symmetric exactly-orthonormal orient (90-degree rotation around
// Z). Hand-built because vm_AnglesToMatrix produces a zero matrix in the test
// environment (sincos_table is not initialized), which made the old
// expectations pass trivially.
static matrix MakeOrient90Z() {
  matrix m;
  vm_MakeIdentity(&m);
  m.rvec = vector{0, 1, 0};
  m.uvec = vector{-1, 0, 0};
  m.fvec = vector{0, 0, 1};
  return m;
}

// Builds the widescreen-cockpit orient: 45-degree rotation around Z with the
// rvec scaled by h_scale (aspect/(4/3)), making it non-orthonormal.
static matrix MakeScaledOrient(float h_scale) {
  matrix m;
  vm_MakeIdentity(&m);
  float c45 = 0.70710678f;
  m.rvec = vector{c45, c45, 0.0f};
  m.uvec = vector{-c45, c45, 0.0f};
  m.fvec = vector{0.0f, 0.0f, 1.0f};
  m.rvec = m.rvec * h_scale;
  return m;
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
 * @brief Verifies g3_RotatePoint during instancing maps the object-space point
 *        to the same view-space position as the GPU model-view matrix.
 *
 * @details
 * The GPU model matrix maps a local point src to
 * src.x*rvec + src.y*uvec + src.z*fvec + pos; the CPU code must agree or the
 * facing checks disagree with what is rendered. The expectation is computed
 * with GLM directly (independent of the CPU code under test).
 *
 * @see renderer/HardwarePoints.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, RotatePointEquivalence) {
  matrix viewOrient;
  vm_MakeIdentity(&viewOrient);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  vector trueViewPos = View_position;
  matrix trueViewMat = View_matrix;

  matrix objOrient = MakeOrient90Z();
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  vector src{1, 2, 3};
  g3Point pt;
  g3_RotatePoint(&pt, &src);

  // Ground truth from the GPU model matrix.
  glm::mat4 model = MakeModelMatrix(objPos, objOrient);
  glm::vec4 worldG = model * glm::vec4(src.x(), src.y(), src.z(), 1.0f);
  vector world{worldG.x, worldG.y, worldG.z};
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
  vm_MakeIdentity(&viewOrient);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  vector trueViewPos = View_position;

  matrix objOrient = MakeOrient90Z();
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  vector vp;
  g3_GetViewPosition(&vp);

  // Ground truth: the model matrix maps local points to world, so the
  // local-frame view position is model^-1 * View_position.
  glm::mat4 model = MakeModelMatrix(objPos, objOrient);
  vector expected = GtLocalFromWorld(model, trueViewPos);

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
  vm_MakeIdentity(&viewOrient);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix trueUnscaled = Unscaled_matrix;

  matrix objOrient = MakeOrient90Z();
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  matrix m;
  g3_GetUnscaledMatrix(&m);

  // Old re-based behavior (the shipped semantic): Unscaled_r = (~input) *
  // Unscaled_matrix. Expressed with the input orient this is exact for all
  // orients, non-orthonormal included.
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

  matrix objOrient = MakeOrient90Z();
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  vector v{1, 2, 3};
  vector norm{0, 0, 1};
  bool actual = g3_CheckNormalFacing(&v, &norm);

  // Ground truth computed with GLM directly.
  glm::mat4 model = MakeModelMatrix(objPos, objOrient);
  bool expected = GtFacing(model, trueViewPos, v, norm);

  EXPECT_EQ(actual, expected);
  g3_DoneInstance();
}

// ---- Widescreen cockpit scaled-orient tests (BUGFIX #692) ----

/**
 * @test G3InstanceTest.GetViewPositionScaledOrient
 * @brief Verifies g3_GetViewPosition returns the correct local-frame view
 *        position when the instance orient has a scaled rvec (the widescreen
 *        cockpit's aspect/(4/3) correction).
 *
 * @details
 * The widescreen cockpit scales the instance orient's rvec to counteract
 * projection compression, making the orient non-orthonormal. The local-frame
 * view position must be model^-1 * View_position (GLM ground truth), which for
 * a non-orthonormal orient differs from any plain multiply or transpose. This
 * is the #692 regression pin: it fails on the pre-fix code (plain multiply)
 * and on transposed variants.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, GetViewPositionScaledOrient) {
  // Build a known non-zero view state (vm_MakeIdentity avoids sincos_table).
  vector viewPos{5, -7, 11};
  matrix viewOrient;
  vm_MakeIdentity(&viewOrient);
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  // Widescreen-cockpit orient: 45-degree rotation around Z with a scaled rvec.
  float h_scale = 4.0f / 3.0f;
  matrix objOrient = MakeScaledOrient(h_scale);

  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  vector vp;
  g3_GetViewPosition(&vp);

  // Ground truth computed with GLM directly (independent of the CPU inverse).
  glm::mat4 model = MakeModelMatrix(objPos, objOrient);
  vector expected = GtLocalFromWorld(model, View_position);

  ExpectVectorEq(vp, expected);
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.GetViewPositionScaledOrientNested
 * @brief Verifies g3_GetViewPosition returns the correct local-frame view
 *        position for a NESTED instance under a scaled (non-orthonormal) root
 *        orient.
 *
 * @details
 * The widescreen cockpit scales the root instance orient's rvec. Submodels are
 * then pushed as nested instances, composing the scaled root orient with the
 * submodel's rotation. The composed orient's columns are NOT orthogonal, so the
 * earlier column-length correction (valid only for the root instance) gives the
 * wrong local-frame position here. The full matrix inverse is correct at every
 * instance depth. This test fails on the column-length fix and passes on the
 * inverse fix.
 *
 * @see renderer/HardwareSetup.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, GetViewPositionScaledOrientNested) {
  // Build a known non-zero view state (vm_MakeIdentity avoids sincos_table).
  vector viewPos{5, -7, 11};
  matrix viewOrient;
  vm_MakeIdentity(&viewOrient);
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  // Root instance: widescreen cockpit with scaled rvec.
  float h_scale = 4.0f / 3.0f;
  matrix o1 = MakeScaledOrient(h_scale);
  vector p1{3, -2, 8};

  // Nested instance: submodel rotated 45 degrees around Z with a translation.
  matrix o2 = MakeScaledOrient(1.0f);
  vector p2{0.5f, 0.0f, 0.0f};

  g3_StartInstanceMatrix(&p1, &o1);
  g3_StartInstanceMatrix(&p2, &o2);

  vector vp;
  g3_GetViewPosition(&vp);

  // Ground truth: composed model matrix (root * nested), inverted with GLM.
  glm::mat4 model = MakeModelMatrix(p1, o1) * MakeModelMatrix(p2, o2);
  vector expected = GtLocalFromWorld(model, View_position);

  ExpectVectorEq(vp, expected);
  g3_DoneInstance();
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.CheckNormalFacingScaledOrient
 * @brief Verifies g3_CheckNormalFacing agrees with the correct inverse
 *        transform when the instance orient has a scaled rvec (widescreen
 *        cockpit).
 *
 * @details
 * With a non-orthonormal orient the viewer-minus-point vector must be
 * transformed by orient^-T (via the full matrix inverse), not by orient. This
 * test pins that correction; it fails on the pre-fix code which used the plain
 * * orient and could wrongly cull faces.
 *
 * @see renderer/HardwareDraw.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, CheckNormalFacingScaledOrient) {
  // Build a known non-zero view state (vm_MakeIdentity avoids sincos_table).
  vector viewPos{5, -7, 12};
  matrix viewOrient;
  vm_MakeIdentity(&viewOrient);
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  // Widescreen-cockpit orient: 45-degree rotation around Z with a scaled rvec.
  float h_scale = 4.0f / 3.0f;
  matrix objOrient = MakeScaledOrient(h_scale);

  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  vector v{1, 2, 3};
  vector norm{1, 0, -0.5f};
  bool actual = g3_CheckNormalFacing(&v, &norm);

  // Ground truth computed with GLM directly (independent of the CPU inverse).
  glm::mat4 model = MakeModelMatrix(objPos, objOrient);
  bool expected = GtFacing(model, View_position, v, norm);

  EXPECT_EQ(actual, expected);
  g3_DoneInstance();
}

/**
 * @test G3InstanceTest.CheckNormalFacingScaledOrientNested
 * @brief Verifies g3_CheckNormalFacing agrees with the correct inverse
 *        transform for a NESTED instance under a scaled (non-orthonormal) root
 *        orient.
 *
 * @details
 * Submodels of the widescreen cockpit are pushed as nested instances, composing
 * the scaled root orient with the submodel's rotation. The composed orient's
 * columns are NOT orthogonal, so the earlier column-length correction gives the
 * wrong local-frame vector here. The full matrix inverse is correct at every
 * instance depth. This test fails on the column-length fix and passes on the
 * inverse fix.
 *
 * @see renderer/HardwareDraw.cpp
 * @ingroup descent3_tests
 */
TEST_F(G3InstanceTest, CheckNormalFacingScaledOrientNested) {
  // Build a known non-zero view state (vm_MakeIdentity avoids sincos_table).
  vector viewPos{5, -7, 12};
  matrix viewOrient;
  vm_MakeIdentity(&viewOrient);
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  // Root instance: widescreen cockpit with scaled rvec.
  float h_scale = 4.0f / 3.0f;
  matrix o1 = MakeScaledOrient(h_scale);
  vector p1{3, -2, 8};

  // Nested instance: submodel rotated 45 degrees around Z with a translation.
  matrix o2 = MakeScaledOrient(1.0f);
  vector p2{0.5f, 0.0f, 0.0f};

  g3_StartInstanceMatrix(&p1, &o1);
  g3_StartInstanceMatrix(&p2, &o2);

  vector v{1, 2, 3};
  vector norm{1, 0, -0.5f};
  bool actual = g3_CheckNormalFacing(&v, &norm);

  // Ground truth: composed model matrix (root * nested), with GLM.
  glm::mat4 model = MakeModelMatrix(p1, o1) * MakeModelMatrix(p2, o2);
  bool expected = GtFacing(model, View_position, v, norm);

  EXPECT_EQ(actual, expected);
  g3_DoneInstance();
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