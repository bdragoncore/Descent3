/**
 * @file renderer_state_cache_real_tests.cpp
 * @brief Unit tests for renderer state caching optimizations.
 *
 * @details
 * Tests the following optimizations:
 * - ComputeTextureEnable: pure bitmask helper for setTextureEnabled early-out.
 * - Instance transform decomposition caching: g3_GetInstanceTransform returns
 *   cached values instead of decomposing the 4x4 model matrix per-vertex.
 *
 * @par Source
 * `renderer/HardwareInternal.h` (ComputeTextureEnable),
 * `renderer/HardwareInstance.cpp` (instance transform cache)
 * @par Harness
 * `renderer_state_cache_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — TEST / TEST_F macros
 *
 * @ingroup descent3_tests
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "pstypes.h"
#include "vecmat.h"
#include "3d.h"
#include "renderer.h"
#include "HardwareInternal.h"

// ---- Stubs for renderer entry points ----

static int g_test_proj_w = 800;
static int g_test_proj_h = 600;

void rend_GetProjectionParameters(int *width, int *height) {
  *width = g_test_proj_w;
  *height = g_test_proj_h;
}

void rend_GetProjectionScreenParameters(int &screenLX, int &screenTY, int &screenW, int &screenH) {
  screenLX = 0;
  screenTY = 0;
  screenW = g_test_proj_w;
  screenH = g_test_proj_h;
}

void rend_TransformSetToPassthru(void) {}
void rend_TransformSetViewport(int lx, int ty, int width, int height) {
  (void)lx; (void)ty; (void)width; (void)height;
}
void rend_TransformSetProjection(float trans[4][4]) { (void)trans; }
void rend_TransformSetModelView(float trans[4][4]) { (void)trans; }

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

float Z_bias = 0.0f;

// ---- Test helpers ----

static glm::mat4 MakeModelMatrix(const vector &pos, const matrix &orient) {
  return glm::translate(glm::mat4(1.0f), glm::vec3(pos.x(), pos.y(), pos.z())) *
         glm::mat4(glm::vec4(orient.rvec.x(), orient.rvec.y(), orient.rvec.z(), 0.0f),
                   glm::vec4(orient.uvec.x(), orient.uvec.y(), orient.uvec.z(), 0.0f),
                   glm::vec4(orient.fvec.x(), orient.fvec.y(), orient.fvec.z(), 0.0f),
                   glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
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

class RendererStateCacheTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_test_proj_w = 800;
    g_test_proj_h = 600;
    Z_bias = 0.0f;
    g3_ForceTransformRefresh();
  }
};

// =============================================================================
// ComputeTextureEnable tests
// =============================================================================

/**
 * @test ComputeTextureEnable.SetBit0
 * @brief Verifies setting bit 0 on a zero bitmask.
 */
TEST(ComputeTextureEnableTest, SetBit0) {
  int result = ComputeTextureEnable(0, 0, true);
  EXPECT_EQ(result, 1);
}

/**
 * @test ComputeTextureEnable.SetBit1
 * @brief Verifies setting bit 1 on a zero bitmask.
 */
TEST(ComputeTextureEnableTest, SetBit1) {
  int result = ComputeTextureEnable(0, 1, true);
  EXPECT_EQ(result, 2);
}

/**
 * @test ComputeTextureEnable.ClearBit0
 * @brief Verifies clearing bit 0 when it's set.
 */
TEST(ComputeTextureEnableTest, ClearBit0) {
  int result = ComputeTextureEnable(3, 0, false);
  EXPECT_EQ(result, 2);
}

/**
 * @test ComputeTextureEnable.ClearBit1
 * @brief Verifies clearing bit 1 when it's set.
 */
TEST(ComputeTextureEnableTest, ClearBit1) {
  int result = ComputeTextureEnable(3, 1, false);
  EXPECT_EQ(result, 1);
}

/**
 * @test ComputeTextureEnable.NoChangeSet
 * @brief Verifies setting an already-set bit returns the same value.
 */
TEST(ComputeTextureEnableTest, NoChangeSet) {
  int result = ComputeTextureEnable(1, 0, true);
  EXPECT_EQ(result, 1);
}

/**
 * @test ComputeTextureEnable.NoChangeClear
 * @brief Verifies clearing an already-clear bit returns the same value.
 */
TEST(ComputeTextureEnableTest, NoChangeClear) {
  int result = ComputeTextureEnable(2, 0, false);
  EXPECT_EQ(result, 2);
}

/**
 * @test ComputeTextureEnable.BothBitsSet
 * @brief Verifies both bits can be set independently.
 */
TEST(ComputeTextureEnableTest, BothBitsSet) {
  int val = ComputeTextureEnable(0, 0, true);
  val = ComputeTextureEnable(val, 1, true);
  EXPECT_EQ(val, 3);
}

/**
 * @test ComputeTextureEnable.Toggling
 * @brief Verifies toggling bits on and off returns to original state.
 */
TEST(ComputeTextureEnableTest, Toggling) {
  int val = ComputeTextureEnable(0, 0, true);
  val = ComputeTextureEnable(val, 0, false);
  EXPECT_EQ(val, 0);
}

// =============================================================================
// Instance transform cache tests
// =============================================================================

/**
 * @test RendererStateCacheTest.InstanceTransformCacheSingle
 * @brief Verifies the cached instance transform matches the model matrix
 *        decomposition for a single instance.
 */
TEST_F(RendererStateCacheTest, InstanceTransformCacheSingle) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  // The cached transform should match the decomposed model matrix.
  matrix orient;
  vector pos;
  g3_GetInstanceTransform(&orient, &pos);

  // For a single instance (no parent), the model matrix is
  // translate(objPos) * mat4(objOrient), so the decomposed form
  // should exactly match objOrient and objPos.
  ExpectMatrixEq(orient, objOrient);
  ExpectVectorEq(pos, objPos);

  g3_DoneInstance();
}

/**
 * @test RendererStateCacheTest.InstanceTransformCacheNested
 * @brief Verifies the cached instance transform matches the composed model
 *        matrix decomposition for nested instances.
 */
TEST_F(RendererStateCacheTest, InstanceTransformCacheNested) {
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

  // The cached transform should match the composed model matrix decomposition.
  matrix orient;
  vector pos;
  g3_GetInstanceTransform(&orient, &pos);

  // Composed model = translate(p1) * mat4(o1) * translate(p2) * mat4(o2)
  // Decomposed: orient = o1 * o2, pos = p1 + o1 * p2
  glm::mat4 composedModel = MakeModelMatrix(p1, o1) * MakeModelMatrix(p2, o2);

  // Decompose the GLM matrix and compare.
  matrix expectedOrient;
  expectedOrient.rvec = vector{composedModel[0][0], composedModel[1][0], composedModel[2][0]};
  expectedOrient.uvec = vector{composedModel[0][1], composedModel[1][1], composedModel[2][1]};
  expectedOrient.fvec = vector{composedModel[0][2], composedModel[1][2], composedModel[2][2]};
  vector expectedPos;
  expectedPos.x() = composedModel[3][0];
  expectedPos.y() = composedModel[3][1];
  expectedPos.z() = composedModel[3][2];

  ExpectMatrixEq(orient, expectedOrient);
  ExpectVectorEq(pos, expectedPos);

  g3_DoneInstance();
  g3_DoneInstance();
}

/**
 * @test RendererStateCacheTest.InstanceTransformCacheConsistency
 * @brief Verifies that g3_GetInstanceTransform results are consistent across
 *        multiple calls (the cache doesn't change between calls).
 */
TEST_F(RendererStateCacheTest, InstanceTransformCacheConsistency) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);

  // Call g3_GetInstanceTransform multiple times — the cache should return
  // the same values each time.
  for (int i = 0; i < 10; i++) {
    matrix orient;
    vector pos;
    g3_GetInstanceTransform(&orient, &pos);
    ExpectMatrixEq(orient, objOrient);
    ExpectVectorEq(pos, objPos);
  }

  g3_DoneInstance();
}

/**
 * @test RendererStateCacheTest.InstanceTransformCacheAfterPopPush
 * @brief Verifies the cache is correct after a pop-then-push cycle.
 */
TEST_F(RendererStateCacheTest, InstanceTransformCacheAfterPopPush) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix o1;
  vm_AnglesToMatrix(&o1, 0x1000, 0x2000, 0x0500);
  vector p1{3, -2, 8};
  g3_StartInstanceMatrix(&p1, &o1);

  // Push a different instance
  matrix o2;
  vm_AnglesToMatrix(&o2, 0x0800, 0x0400, 0x0C00);
  vector p2{-1, 4, 2};
  g3_StartInstanceMatrix(&p2, &o2);

  // Pop back to first instance
  g3_DoneInstance();

  // The cache should still return the first instance's transform
  matrix orient;
  vector pos;
  g3_GetInstanceTransform(&orient, &pos);
  ExpectMatrixEq(orient, o1);
  ExpectVectorEq(pos, p1);

  g3_DoneInstance();
}

/**
 * @test RendererStateCacheTest.InstanceTransformCacheIdentityAfterPop
 * @brief Verifies the cache returns identity after popping all instances.
 */
TEST_F(RendererStateCacheTest, InstanceTransformCacheIdentityAfterPop) {
  matrix viewOrient;
  vm_AnglesToMatrix(&viewOrient, 0x2000, 0x1000, 0x3000);
  vector viewPos{5, -7, 11};
  g3_StartFrame(&viewPos, &viewOrient, 1.0f);

  matrix objOrient;
  vm_AnglesToMatrix(&objOrient, 0x1000, 0x2000, 0x0500);
  vector objPos{3, -2, 8};
  g3_StartInstanceMatrix(&objPos, &objOrient);
  g3_DoneInstance();

  // Should return identity/zero after popping all instances.
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
