/**
 * @file polymodel_batch_real_tests.cpp
 * @brief Unit tests for polymodel face batching in RenderSubmodelFacesUnsorted.
 *
 * @details
 * Compiles the real model/newstyle.cpp with stubbed engine subsystems and
 * verifies that gpu_SetBatchMode / gpu_FlushBatch are called correctly when
 * StateLimited is active and lighting conditions allow batching.
 *
 * @par Source
 * `model/newstyle.cpp`
 * @par Harness
 * `polymodel_batch_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — TEST / TEST_F macros
 * @par Linkage
 * Real newstyle.cpp compiled directly into the test executable with stubbed
 * renderer and game subsystems.
 *
 * @ingroup descent3_tests
 * @see model/newstyle.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <algorithm>

#include "pstypes.h"
#include "vecmat.h"
#include "3d.h"
#include "renderer.h"
#include "polymodel_external.h"
#include "polymodel.h"
#include "gametexture.h"
#include "lightmap_info.h"
#include "render.h"
#include "lighting.h"
#include "fireball.h"
#include "fireball_external.h"
#include "grdefs.h"

// ---------------------------------------------------------------------------
// Stubs: g3 functions
// ---------------------------------------------------------------------------

static int g_rotate_count = 0;
uint8_t g3_RotatePoint(g3Point *dest, vector *src) {
  dest->p3_vec = *src;
  dest->p3_sx = 100.0f;
  dest->p3_sy = 100.0f;
  dest->p3_flags = PF_ORIGPOINT;
  dest->p3_codes = 0;
  dest->p3_l = 1.0f;
  dest->p3_r = dest->p3_g = dest->p3_b = 1.0f;
  g_rotate_count++;
  return 1;
}

void g3_ProjectPoint(g3Point *p) { (void)p; }
uint8_t g3_CodePoint(g3Point *point) { (void)point; return 0; }

// Draw-call counter — the central observable for batching tests.
static int g_poly_calls = 0;
static int g_poly_verts = 0;
int g3_DrawPoly(int nv, g3Point **pointlist, int bm, int map_type, g3Codes *clip_codes) {
  (void)pointlist; (void)bm; (void)map_type; (void)clip_codes;
  g_poly_calls++;
  g_poly_verts += nv;
  return 0;
}

int g3_DrawPolyList(int ntri, g3Point **pointlist, int bm, int map_type) {
  (void)pointlist; (void)bm; (void)map_type;
  g_poly_calls++;
  g_poly_verts += ntri * 3;
  return 0;
}

bool g3_CheckNormalFacing(vector *v, vector *norm) { (void)v; (void)norm; return true; }
void g3_GetViewPosition(vector *v) { *v = vector{0, 0, -500}; }
void g3_GetUnscaledMatrix(matrix *m) { vm_MakeIdentity(m); }
void g3_StartInstanceAngles(vector *pos, angvec *angles) { (void)pos; (void)angles; }
void g3_DoneInstance() {}
void g3_SetTriangulationTest(int state) { (void)state; }
void g3_DrawBitmap(vector *pos, float width, float height, int bm, int color) {
  (void)pos; (void)width; (void)height; (void)bm; (void)color;
}
void g3_DrawLine(ddgr_color c, g3Point *a, g3Point *b) { (void)c; (void)a; (void)b; }

// ---------------------------------------------------------------------------
// Stubs: rend_* (no-ops)
// ---------------------------------------------------------------------------
void rend_SetOverlayMap(int m) { (void)m; }
void rend_SetOverlayType(uint8_t t) { (void)t; }
void rend_SetBumpmapReadyState(int a, int b) { (void)a; (void)b; }
void rend_SetTextureType(texture_type t) { (void)t; }
void rend_SetLighting(light_state s) { (void)s; }
void rend_SetFlatColor(ddgr_color c) { (void)c; }
void rend_SetAlphaValue(uint8_t v) { (void)v; }
void rend_SetAlphaType(signed char t) { (void)t; }
void rend_SetColorModel(color_model m) { (void)m; }
void rend_SetWrapType(wrap_type w) { (void)w; }
void rend_SetZBias(float b) { (void)b; }
void rend_SetZBufferWriteMask(int m) { (void)m; }
void rend_SetCoplanarPolygonOffset(float f) { (void)f; }
void rend_SetFogState(signed char s) { (void)s; }
void rend_SetFogBorders(float a, float b) { (void)a; (void)b; }
void rend_FillRect(ddgr_color c, int l, int t, int w, int h) { (void)c; (void)l; (void)t; (void)w; (void)h; }
void rend_DrawSpecialLine(g3Point *a, g3Point *b) { (void)a; (void)b; }

// ---------------------------------------------------------------------------
// Stubs: gpu batch API — track calls to verify orchestration
// ---------------------------------------------------------------------------
static int g_batch_enable_count = 0;
static int g_batch_flush_count = 0;
static bool g_batch_last_state = false;

void gpu_SetBatchMode(bool active) {
  g_batch_enable_count++;
  g_batch_last_state = active;
}
void gpu_FlushBatch() {
  g_batch_flush_count++;
}

uint8_t gpu_Overlay_type = OT_NONE;

// ---------------------------------------------------------------------------
// Stubs: renderer globals
// ---------------------------------------------------------------------------
bool UseHardware = true;
bool StateLimited = true;
vector View_position{0, 0, -500};
matrix View_matrix{};
matrix Unscaled_matrix{};
float Z_bias = 0.0f;

// ---------------------------------------------------------------------------
// Stubs: bitmap
// ---------------------------------------------------------------------------
int bm_w(int handle, int miplevel) { (void)handle; (void)miplevel; return 64; }
int bm_h(int handle, int miplevel) { (void)handle; (void)miplevel; return 64; }

// ---------------------------------------------------------------------------
// Stubs: game globals (from Descent3/)
// ---------------------------------------------------------------------------
texture GameTextures[MAX_TEXTURES] = {};

int GetTextureBitmap(int handle, int framenum, bool force) {
  (void)framenum; (void)force;
  return handle + 100; // distinct bitmap per texture index
}

int FindTextureName(const char *name) { (void)name; return -1; }

lightmap_info Lightmap_stub_storage[4] = {};
lightmap_info *LightmapInfo = Lightmap_stub_storage;

state_limited_element State_elements[MAX_STATE_ELEMENTS] = {};

void SortStates(state_limited_element *arr, int n) {
  std::sort(arr, arr + n,
            [](const state_limited_element &a, const state_limited_element &b) {
              return a.sort_key < b.sort_key;
            });
}

float Gametime = 0.0f;
fireball Fireballs[200] = {};
float Specular_tables[3][MAX_SPECULAR_INCREMENTS] = {};
float Ubyte_to_float[256] = {};

// ---------------------------------------------------------------------------
// Stubs: model globals (from model/polymodel.cpp)
// ---------------------------------------------------------------------------
g3Point Robot_points[MAX_POLYGON_VECS] = {};
poly_model Poly_models[MAX_POLY_MODELS] = {};
int Num_poly_models = 0;
int Polymodel_use_effect = 0;
polymodel_effect Polymodel_effect{};
polymodel_light_type Polymodel_light_type = POLYMODEL_LIGHTING_GOURAUD;
float Polylighting_static_red = 1.0f;
float Polylighting_static_green = 1.0f;
float Polylighting_static_blue = 1.0f;
vector *Polymodel_light_direction = nullptr;
vector Polymodel_fog_portal_vert{};
vector Polymodel_fog_plane{};
vector Polymodel_specular_pos{};
vector Polymodel_bump_pos{};
lightmap_object *Polylighting_lightmap_object = nullptr;
bool Polymodel_outline_mode = false;
vector Interp_pos_instance_vec{};

void StartPolyModelPosInstance(vector *posvec) { (void)posvec; }
void DonePolyModelPosInstance() {}
void StartLightInstance(vector *pos, matrix *orient) { (void)pos; (void)orient; }
void DoneLightInstance() {}
int IsNonRenderableSubmodel(poly_model *pm, int submodelnum) { (void)pm; (void)submodelnum; return 0; }
poly_model *GetPolymodelPointer(int polynum) { (void)polynum; return &Poly_models[0]; }
void SetNormalizedTimeAnim(float norm_anim_frame, float *normalized_time, poly_model *pm) {
  (void)norm_anim_frame; (void)normalized_time; (void)pm;
}
void SetModelAnglesAndPos(poly_model *po, float *normalized_time, uint32_t subobj_flags) {
  (void)po; (void)normalized_time; (void)subobj_flags;
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class PolymodelBatchTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_poly_calls = 0;
    g_poly_verts = 0;
    g_rotate_count = 0;
    g_batch_enable_count = 0;
    g_batch_flush_count = 0;
    g_batch_last_state = false;

    memset(GameTextures, 0, sizeof(GameTextures));
    memset(State_elements, 0, sizeof(State_elements));
    memset(Robot_points, 0, sizeof(Robot_points));
    memset(&Polymodel_effect, 0, sizeof(Polymodel_effect));

    StateLimited = true;
    UseHardware = true;
    Polymodel_use_effect = 0;
    Polymodel_light_type = POLYMODEL_LIGHTING_GOURAUD;
    Polylighting_static_red = 1.0f;
    Polylighting_static_green = 1.0f;
    Polylighting_static_blue = 1.0f;
    static vector light_dir{0, 1, 0};
    Polymodel_light_direction = &light_dir;
    Polymodel_outline_mode = false;
    gpu_Overlay_type = OT_NONE;
    Gametime = 0.0f;

    vm_MakeIdentity(&View_matrix);
    vm_MakeIdentity(&Unscaled_matrix);

    // Set up minimal polymodel: 1 submodel
    memset(&pm_, 0, sizeof(pm_));
    pm_.n_models = 1;
    pm_.textures[0] = 0;
    pm_.name[0] = '\0';
    pm_.submodel = submodels_;

    bsp_info *sm = &pm_.submodel[0];
    sm->nverts = 0;
    sm->num_faces = 0;
    sm->flags = 0;
    sm->parent = -1;
    sm->num_children = 0;
    sm->glow_info = nullptr;
    vm_MakeZero(&sm->mod_pos);
    vm_MakeZero(&sm->offset);
    sm->angs = angvec{0, 0, 0};

    // bsp_info fields are pointers — allocate backing storage
    sm->verts = verts_;
    sm->vertnorms = vertnorms_;
    sm->alpha = alpha_;
    sm->faces = faces_;

    // Initialize lightmap_object backing storage for lightmap lighting path
    memset(&lm_obj_, 0, sizeof(lm_obj_));
    memset(lm_faces_, 0, sizeof(lm_faces_));
    lm_obj_.num_models = 1;
    lm_obj_.num_faces[0] = 0;
    lm_obj_.lightmap_faces[0] = lm_faces_[0];
    Polylighting_lightmap_object = &lm_obj_;
  }

  void AddVertex(float x, float y, float z) {
    bsp_info *sm = &pm_.submodel[0];
    int v = sm->nverts;
    sm->verts[v] = vector{x, y, z};
    sm->vertnorms[v] = vector{0, 0, 1};
    sm->nverts++;
  }

  void AddFace(int texnum, int nv, int *vertnums) {
    bsp_info *sm = &pm_.submodel[0];
    int f = sm->num_faces;
    sm->faces[f].texnum = texnum;
    sm->faces[f].nverts = nv;
    // polyface.vertnums/u/v are pointers — use per-face backing storage
    sm->faces[f].vertnums = face_vertnums_[f];
    sm->faces[f].u = face_u_[f];
    sm->faces[f].v = face_v_[f];
    for (int i = 0; i < nv; i++)
      sm->faces[f].vertnums[i] = vertnums[i];
    for (int i = 0; i < nv; i++) {
      sm->faces[f].u[i] = (float)i / nv;
      sm->faces[f].v[i] = 0.0f;
    }
    sm->faces[f].normal = vector{0, 0, -1};
    sm->faces[f].color = GR_RGB(255, 255, 255);
    sm->num_faces++;
  }

  void MakeFacesVisible() {
    bsp_info *sm = &pm_.submodel[0];
    for (int i = 0; i < sm->num_faces; i++)
      sm->faces[i].normal = vector{0, 0, -1};
  }

  poly_model pm_;
  // Backing storage for bsp_info pointer fields
  bsp_info submodels_[MAX_SUBOBJECTS];
  vector verts_[MAX_POLYGON_VECS];
  vector vertnorms_[MAX_POLYGON_VECS];
  float alpha_[MAX_POLYGON_VECS];
  polyface faces_[MAX_FACES_PER_ROOM];
  // Backing storage for polyface pointer fields (per face)
  int16_t face_vertnums_[MAX_FACES_PER_ROOM][MAX_POLYGON_VECS];
  float face_u_[MAX_FACES_PER_ROOM][MAX_POLYGON_VECS];
  float face_v_[MAX_FACES_PER_ROOM][MAX_POLYGON_VECS];
  // Backing storage for lightmap_object (needed for lightmap lighting path)
  lightmap_object_face lm_faces_[MAX_SUBOBJECTS][MAX_FACES_PER_ROOM];
  lightmap_object lm_obj_;
};

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

/**
 * @test PolymodelBatchTest.BatchingEnabledForGouraud
 * @brief Verifies gpu_SetBatchMode(true) is called when batching conditions are met.
 */
TEST_F(PolymodelBatchTest, BatchingEnabledForGouraud) {
  AddVertex(-1, -1, 0); AddVertex(1, -1, 0); AddVertex(1, 1, 0); AddVertex(-1, 1, 0);
  int v0[] = {0, 1, 2, 3};
  for (int i = 0; i < 4; i++)
    AddFace(0, 4, v0);
  MakeFacesVisible();

  Polymodel_light_type = POLYMODEL_LIGHTING_GOURAUD;
  gpu_Overlay_type = OT_NONE;
  StateLimited = true;

  RenderSubmodel(&pm_, &pm_.submodel[0], 1);

  EXPECT_EQ(g_poly_calls, 4);
  // Batch mode should have been enabled (true) then disabled (false)
  EXPECT_GE(g_batch_enable_count, 2); // at least enable + disable
  EXPECT_EQ(g_batch_last_state, false); // final state is disabled
  EXPECT_GE(g_batch_flush_count, 1); // at least one flush at end
}

/**
 * @test PolymodelBatchTest.NoBatchingForLightmap
 * @brief Verifies batch mode is NOT enabled when Polymodel_light_type is POLYMODEL_LIGHTING_LIGHTMAP.
 */
TEST_F(PolymodelBatchTest, NoBatchingForLightmap) {
  AddVertex(-1, -1, 0); AddVertex(1, -1, 0); AddVertex(1, 1, 0); AddVertex(-1, 1, 0);
  int v0[] = {0, 1, 2, 3};
  for (int i = 0; i < 3; i++)
    AddFace(0, 4, v0);
  MakeFacesVisible();

  // Set up lightmap faces so RenderSubmodelFace's lightmap path doesn't crash
  static float lm_u2[4] = {0, 1, 1, 0};
  static float lm_v2[4] = {0, 0, 1, 1};
  lm_obj_.num_faces[0] = 3;
  for (int f = 0; f < 3; f++) {
    lm_faces_[0][f].lmi_handle = 0;
    lm_faces_[0][f].num_verts = 4;
    lm_faces_[0][f].u2 = lm_u2;
    lm_faces_[0][f].v2 = lm_v2;
  }

  Polymodel_light_type = POLYMODEL_LIGHTING_LIGHTMAP;
  gpu_Overlay_type = OT_NONE;
  StateLimited = true;

  RenderSubmodel(&pm_, &pm_.submodel[0], 1);

  EXPECT_EQ(g_poly_calls, 3);
  EXPECT_EQ(g_batch_enable_count, 0); // no batching at all
  EXPECT_EQ(g_batch_flush_count, 0);
}

/**
 * @test PolymodelBatchTest.BatchFlushesAtTextureBoundary
 * @brief Verifies gpu_FlushBatch is called when the texture changes between faces.
 *
 * @details
 * Creates 2 faces with texture 0 and 2 faces with texture 1. After sorting by
 * sort_key (descending), the batch should flush when transitioning from one
 * texture group to the next.
 */
TEST_F(PolymodelBatchTest, BatchFlushesAtTextureBoundary) {
  AddVertex(-1, -1, 0); AddVertex(1, -1, 0); AddVertex(1, 1, 0); AddVertex(-1, 1, 0);
  int v0[] = {0, 1, 2, 3};
  AddFace(0, 4, v0); // face 0: tex 0
  AddFace(0, 4, v0); // face 1: tex 0
  AddFace(1, 4, v0); // face 2: tex 1
  AddFace(1, 4, v0); // face 3: tex 1
  pm_.textures[0] = 0;
  pm_.textures[1] = 1;
  GameTextures[0].flags = 0;
  GameTextures[1].flags = 0;
  MakeFacesVisible();

  Polymodel_light_type = POLYMODEL_LIGHTING_GOURAUD;
  gpu_Overlay_type = OT_NONE;
  StateLimited = true;

  RenderSubmodel(&pm_, &pm_.submodel[0], 1);

  EXPECT_EQ(g_poly_calls, 4);
  EXPECT_EQ(g_poly_verts, 16);
  // With 2 texture groups, there should be at least 2 flushes (one at boundary, one at end)
  EXPECT_GE(g_batch_flush_count, 2);
}

/**
 * @test PolymodelBatchTest.NoBatchingWhenOverlayActive
 * @brief Verifies batch mode is NOT enabled when gpu_Overlay_type != OT_NONE.
 */
TEST_F(PolymodelBatchTest, NoBatchingWhenOverlayActive) {
  AddVertex(-1, -1, 0); AddVertex(1, -1, 0); AddVertex(1, 1, 0); AddVertex(-1, 1, 0);
  int v0[] = {0, 1, 2, 3};
  for (int i = 0; i < 3; i++)
    AddFace(0, 4, v0);
  MakeFacesVisible();

  Polymodel_light_type = POLYMODEL_LIGHTING_GOURAUD;
  gpu_Overlay_type = OT_BLEND;
  StateLimited = true;

  RenderSubmodel(&pm_, &pm_.submodel[0], 1);

  EXPECT_EQ(g_poly_calls, 3);
  EXPECT_EQ(g_batch_enable_count, 0); // no batching
  EXPECT_EQ(g_batch_flush_count, 0);
}

/**
 * @test PolymodelBatchTest.StateLimitedFalseNoBatching
 * @brief Verifies that when StateLimited is false, faces are rendered directly.
 */
TEST_F(PolymodelBatchTest, StateLimitedFalseNoBatching) {
  AddVertex(-1, -1, 0); AddVertex(1, -1, 0); AddVertex(1, 1, 0); AddVertex(-1, 1, 0);
  int v0[] = {0, 1, 2, 3};
  for (int i = 0; i < 4; i++)
    AddFace(0, 4, v0);
  MakeFacesVisible();

  StateLimited = false;
  Polymodel_light_type = POLYMODEL_LIGHTING_GOURAUD;
  gpu_Overlay_type = OT_NONE;

  RenderSubmodel(&pm_, &pm_.submodel[0], 1);

  EXPECT_EQ(g_poly_calls, 4);
  EXPECT_EQ(g_poly_verts, 16);
  EXPECT_EQ(g_batch_enable_count, 0);
}

/**
 * @test PolymodelBatchTest.SingleFaceRendersCorrectly
 * @brief Verifies a single face is rendered correctly with batching enabled.
 */
TEST_F(PolymodelBatchTest, SingleFaceRendersCorrectly) {
  AddVertex(-1, -1, 0); AddVertex(1, -1, 0); AddVertex(1, 1, 0);
  int v0[] = {0, 1, 2};
  AddFace(0, 3, v0);
  MakeFacesVisible();

  Polymodel_light_type = POLYMODEL_LIGHTING_GOURAUD;
  gpu_Overlay_type = OT_NONE;
  StateLimited = true;

  RenderSubmodel(&pm_, &pm_.submodel[0], 1);

  EXPECT_EQ(g_poly_calls, 1);
  EXPECT_EQ(g_poly_verts, 3);
  // Single face: enable + flush + disable
  EXPECT_GE(g_batch_enable_count, 2);
  EXPECT_GE(g_batch_flush_count, 1);
}

/**
 * @test PolymodelBatchTest.MultipleTexturesAllFacesRendered
 * @brief Verifies all faces are rendered when there are multiple textures.
 */
TEST_F(PolymodelBatchTest, MultipleTexturesAllFacesRendered) {
  AddVertex(-1, -1, 0); AddVertex(1, -1, 0); AddVertex(1, 1, 0); AddVertex(-1, 1, 0);
  pm_.textures[0] = 0;
  pm_.textures[1] = 1;
  pm_.textures[2] = 2;
  int v0[] = {0, 1, 2, 3};
  AddFace(0, 4, v0); // tex 0
  AddFace(1, 4, v0); // tex 1
  AddFace(2, 4, v0); // tex 2
  AddFace(0, 4, v0); // tex 0
  AddFace(1, 4, v0); // tex 1
  AddFace(2, 4, v0); // tex 2
  for (int i = 0; i < 3; i++)
    GameTextures[pm_.textures[i]].flags = 0;
  MakeFacesVisible();

  Polymodel_light_type = POLYMODEL_LIGHTING_GOURAUD;
  gpu_Overlay_type = OT_NONE;
  StateLimited = true;

  RenderSubmodel(&pm_, &pm_.submodel[0], 1);

  EXPECT_EQ(g_poly_calls, 6);
  EXPECT_EQ(g_poly_verts, 24);
  // With 3 texture groups, expect multiple flushes
  EXPECT_GE(g_batch_flush_count, 3);
}
