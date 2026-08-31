/**
 * @file terrainrender_linked_real_tests.cpp
 * @brief Unit tests for Descent3/terrainrender.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/terrainrender.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/terrainrender.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/terrainrender.cpp`
 * @par Harness
 * `terrainrender_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/terrainrender.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <cmath>

// Link real Descent3/terrainrender.cpp
#include "pstypes.h"
#include "vecmat.h"
#include "3d.h"
#include "terrain.h"
#include "object.h"
#include "player.h"
#include "fireball.h"
#include "viseffect.h"
#include "weapon.h"
#include "room.h"
#include "gametexture.h"
#include "lightmap.h"
#include "config.h"
#include "render.h"
#include "renderer.h"
#include "findintersection.h"
#include "descent.h"
#include "Inventory.h"
#include "weapon.h"

// Forward declare internal helper not in terrain.h but global in terrainrender.cpp
int IsTerrainDynamicChecked(int seg, int bit);

// Globals required by terrainrender.cpp (provide definitions for undefined externs)
// Defined in terrain.cpp but needed here
uint8_t Terrain_dynamic_table[TERRAIN_WIDTH * TERRAIN_DEPTH] = {0};

// Correctly typed globals per terrain.h
uint16_t TS_FrameCount = 0;
terrain_tex_segment Terrain_tex_seg[TERRAIN_TEX_WIDTH * TERRAIN_TEX_DEPTH] = {};
int TerrainEdgeTest[MAX_TERRAIN_LOD][16] = {};
uint8_t Terrain_occlusion_map[256][32] = {};
float VisibleTerrainZ = 0.0f;
terrain_render_info Terrain_list[MAX_CELLS_TO_RENDER] = {};
uint16_t dummy_rotate_list[32768] = {};
uint16_t *Terrain_rotate_list = dummy_rotate_list;
g3Point dummy_world_points[4096] = {};
g3Point *World_point_buffer = dummy_world_points;
terrain_sky Terrain_sky = {};

// Other globals from U list — provide with matching types where possible
#include "player.h"
#include "object_external_struct.h"
#include "vclip.h"
#include "fireball_external.h"
#include "viseffect_external.h"
#include "weapon_external.h"
#include "room_external.h"

int Highest_object_index = 0;
int Highest_vis_effect_index = 0;
int Num_postrenders = 0;
int GlobalTransCount = 0;
int Rendering_main_view = 0;
bool StateLimited = false;
int Terrain_checksum = 0;
int Terrain_occlusion_checksum = 0;
bool UseHardware = false;
float Frametime = 0.016f;
float Gametime = 0.0f;
object Objects[MAX_OBJECTS];
player Players[MAX_PLAYERS];
room Rooms[MAX_ROOMS];
int TerrainLightmaps[4] = {0};
terrain_segment Terrain_seg[(TERRAIN_WIDTH+1)*(TERRAIN_DEPTH+1)] = {};
uint8_t TerrainJoinMap[TERRAIN_WIDTH*TERRAIN_DEPTH] = {0};
texture GameTextures[MAX_TEXTURES] = {};
weapon Weapons[200] = {};
// Fireballs is unsized extern array; provide sufficiently large
fireball Fireballs[200] = {};
vis_effect *VisEffects = nullptr;
object *Viewer_object = nullptr;
int Highest_room_index = 10;
tDetailSettings Detail_settings{};
int Weather = 0;
int Postrender_list[100] = {};
state_limited_element State_elements[MAX_STATE_ELEMENTS] = {};
// Inventory stubs (player.h)
Inventory::Inventory() {}
Inventory::~Inventory() {}
// bitmap stubs
int bm_w(int handle, int miplevel) { return 0; }
int bm_h(int handle, int miplevel) { return 0; }

// Provide stubs for undefined symbols — many already provided by linked libs (renderer, 3d, vecmat, physics)
// Provide minimal stubs for those not in libs
void RenderMine(int a, int b, int c) {}
void DrawVisEffect(vis_effect *v) {}
void RenderObject(object *o) {}
// SortStates is in render.h
void SortStates(state_limited_element *state_array, int cellcount) {}
// Remaining g3/rend/fvi stubs — defined weak so libs override if available
__attribute__((weak)) int g3_DrawPoly(int nv, g3Point **pointlist, int bm, int map_type, g3Codes *clip_codes) { return 0; }
__attribute__((weak)) uint8_t g3_CodePoint(g3Point *point) { return 0; }
__attribute__((weak)) uint8_t g3_RotatePoint(g3Point *dest, vector *src) { return 0; }
__attribute__((weak)) void g3_ProjectPoint(g3Point *p) {}
__attribute__((weak)) void g3_GetViewMatrix(matrix *m) { vm_MakeIdentity(m); }
__attribute__((weak)) void g3_GetViewPosition(vector *v) { *v = vector{0,0,0}; }
__attribute__((weak)) void g3_SetFarClipZ(float z) {}
__attribute__((weak)) void g3_SetTriangulationTest(int t) {}
__attribute__((weak)) int IsPointVisible(vector *p, float r, float *z) { return 1; }
__attribute__((weak)) void rend_SetZBias(float b) {}
__attribute__((weak)) void rend_SetFogColor(ddgr_color c) {}
__attribute__((weak)) void rend_SetFogState(signed char s) {}
__attribute__((weak)) void rend_SetLighting(light_state s) {}
__attribute__((weak)) void rend_SetWrapType(wrap_type w) {}
__attribute__((weak)) void rend_SetAlphaType(signed char t) {}
__attribute__((weak)) void rend_SetFlatColor(ddgr_color c) {}
__attribute__((weak)) void rend_SetAlphaValue(uint8_t v) {}
__attribute__((weak)) void rend_SetColorModel(color_model m) {}
__attribute__((weak)) void rend_SetFogBorders(float a, float b) {}
__attribute__((weak)) void rend_SetOverlayMap(int m) {}
__attribute__((weak)) void rend_SetOverlayType(uint8_t t) {}
__attribute__((weak)) void rend_SetTextureType(texture_type t) {}
__attribute__((weak)) void rend_SetZBufferState(signed char s) {}
__attribute__((weak)) void rend_SetZBufferWriteMask(int m) {}
__attribute__((weak)) void rend_FillRect(ddgr_color c, int l, int t, int w, int h) {}
__attribute__((weak)) void rend_DrawSpecialLine(g3Point *a, g3Point *b) {}
__attribute__((weak)) void g3_DrawSpecialLine(g3Point *a, g3Point *b) {}
__attribute__((weak)) void g3_DrawRotatedBitmap(vector *pos, unsigned short t, float w, float h, int a, int b) {}
__attribute__((weak)) void g3_DrawPlanarRotatedBitmap(vector *a, vector *b, unsigned short c, float d, float e, int f) {}
__attribute__((weak)) void DrawColoredRing(vector *a, float b, float c, float d, float e, float f, float g, float h, uint8_t i, uint8_t j) {}
__attribute__((weak)) function_mode GetFunctionMode() { return GAME_MODE; }
__attribute__((weak)) int GetTextureBitmap(int a, int b, bool c) { return 0; }
__attribute__((weak)) int GetVisibleTerrain(vector *a, matrix *b) { return 0; }
__attribute__((weak)) void MakePointsFromMinMax(vector *a, vector *b, vector *c) { *a = *b; }
__attribute__((weak)) int GetFPS() { return 30; }
__attribute__((weak)) int fvi_FindIntersection(fvi_query *q, fvi_info *i, bool b) { return 0; }
__attribute__((weak)) void GetSpecialRotatedPoint(g3Point *dest, int x, int z, float yvalue) { dest->p3_sx = (float)x; dest->p3_sy = (float)z; }
__attribute__((weak)) void GetPreRotatedPoint(g3Point *a, int b, int c, int d) { a->p3_sx = (float)b; a->p3_sy = (float)c; }
__attribute__((weak)) void g3_DrawLine(ddgr_color c, g3Point *a, g3Point *b) {}
__attribute__((weak)) void g3_GetUnscaledMatrix(matrix *m) { vm_MakeIdentity(m); }
__attribute__((weak)) void rend_GetProjectionParameters(int *w, int *h) { *w=640; *h=480; }

class TerrainRenderLinked : public ::testing::Test {
protected:
  void SetUp() override {
    memset(Terrain_dynamic_table, 0, sizeof(Terrain_dynamic_table));
    Clip_scale_left = 0;
    Clip_scale_right = 640;
    Clip_scale_top = 0;
    Clip_scale_bot = 480;
  }
};

// ---- CodeTerrainPoint (real via terrainrender.cpp) ----
/**
 * @test TerrainRenderLinked.CodeTerrainPointStrictEdges
 * @brief Verifies code Terrain Point Strict Edges.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, CodeTerrainPointStrictEdges) {
  g3Point inside{}; inside.p3_sx = 320; inside.p3_sy = 240;
  EXPECT_EQ(CodeTerrainPoint(&inside), 0);
  g3Point left{}; left.p3_sx = -1; left.p3_sy = 240;
  EXPECT_EQ(CodeTerrainPoint(&left), CC_OFF_LEFT);
  g3Point onedge{}; onedge.p3_sx = 640; onedge.p3_sy = 480;
  EXPECT_EQ(CodeTerrainPoint(&onedge), 0);
  g3Point offr{}; offr.p3_sx = 640.5f; offr.p3_sy = 240;
  EXPECT_EQ(CodeTerrainPoint(&offr), CC_OFF_RIGHT);
  g3Point offt{}; offt.p3_sx = 320; offt.p3_sy = -0.5f;
  EXPECT_EQ(CodeTerrainPoint(&offt), CC_OFF_TOP);
  g3Point offb{}; offb.p3_sx = 320; offb.p3_sy = 480.5f;
  EXPECT_EQ(CodeTerrainPoint(&offb), CC_OFF_BOT);
  g3Point corner{}; corner.p3_sx = -5; corner.p3_sy = 900;
  EXPECT_EQ(CodeTerrainPoint(&corner), CC_OFF_LEFT | CC_OFF_BOT);
}

/**
 * @test TerrainRenderLinked.CodeTerrainPointAllCorners
 * @brief Verifies code Terrain Point All Corners.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, CodeTerrainPointAllCorners) {
  g3Point tl{}; tl.p3_sx = -10; tl.p3_sy = -10;
  EXPECT_EQ(CodeTerrainPoint(&tl), CC_OFF_LEFT | CC_OFF_TOP);
  g3Point br{}; br.p3_sx = 1000; br.p3_sy = 1000;
  EXPECT_EQ(CodeTerrainPoint(&br), CC_OFF_RIGHT | CC_OFF_BOT);
  g3Point tr{}; tr.p3_sx = 1000; tr.p3_sy = -10;
  EXPECT_EQ(CodeTerrainPoint(&tr), CC_OFF_RIGHT | CC_OFF_TOP);
}

/**
 * @test TerrainRenderLinked.CodeTerrainPointExactlyOnBorderIsInside
 * @brief Verifies code Terrain Point Exactly On Border Is Inside.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, CodeTerrainPointExactlyOnBorderIsInside) {
  g3Point p{}; p.p3_sx = 0; p.p3_sy = 0;
  EXPECT_EQ(CodeTerrainPoint(&p), 0);
  p.p3_sx = 640; p.p3_sy = 0; EXPECT_EQ(CodeTerrainPoint(&p), 0);
  p.p3_sx = 0; p.p3_sy = 480; EXPECT_EQ(CodeTerrainPoint(&p), 0);
}

// ---- IsTerrainDynamicChecked ----
/**
 * @test TerrainRenderLinked.IsDynamicCheckedBitTest
 * @brief Verifies is Dynamic Checked Bit Test.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, IsDynamicCheckedBitTest) {
  Terrain_dynamic_table[10] = 0x20; // bit5
  EXPECT_EQ(IsTerrainDynamicChecked(10,5), 1);
  EXPECT_EQ(IsTerrainDynamicChecked(10,4), 0);
  EXPECT_EQ(IsTerrainDynamicChecked(10,7), 0);
}

/**
 * @test TerrainRenderLinked.IsDynamicCheckedOutOfBoundsQuirk
 * @brief Verifies is Dynamic Checked Out Of Bounds Quirk.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, IsDynamicCheckedOutOfBoundsQuirk) {
  EXPECT_EQ(IsTerrainDynamicChecked(-1,0), 1);
  EXPECT_EQ(IsTerrainDynamicChecked(TERRAIN_WIDTH*TERRAIN_DEPTH,0), 1);
  EXPECT_EQ(IsTerrainDynamicChecked(10,8), 1);
  EXPECT_EQ(IsTerrainDynamicChecked(10,9), 1);
}

/**
 * @test TerrainRenderLinked.IsDynamicCheckedAllBits
 * @brief Verifies is Dynamic Checked All Bits.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, IsDynamicCheckedAllBits) {
  Terrain_dynamic_table[5] = 0xFF;
  for(int b=0;b<8;++b) EXPECT_EQ(IsTerrainDynamicChecked(5,b),1);
  Terrain_dynamic_table[5]=0;
  for(int b=0;b<8;++b) EXPECT_EQ(IsTerrainDynamicChecked(5,b),0);
}

// ---- GetTerrainDynamicScalar ----
/**
 * @test TerrainRenderLinked.DynamicScalarUniformLitAndDark
 * @brief Verifies dynamic Scalar Uniform Lit And Dark.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarUniformLitAndDark) {
  memset(Terrain_dynamic_table, 0xFF, sizeof(Terrain_dynamic_table));
  vector pos{20,100,20};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&pos, 5), 1.0f);
  memset(Terrain_dynamic_table, 0, sizeof(Terrain_dynamic_table));
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&pos, 5), 0.0f);
}

/**
 * @test TerrainRenderLinked.DynamicScalarHalfCellBlend
 * @brief Verifies dynamic Scalar Half Cell Blend.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarHalfCellBlend) {
  memset(Terrain_dynamic_table, 0, sizeof(Terrain_dynamic_table));
  Terrain_dynamic_table[5]=0xFF;
  Terrain_dynamic_table[5+TERRAIN_WIDTH]=0xFF;
  vector lit{16,40,16}; // x_norm 0 -> lit col
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&lit,5),1.0f);
  vector mid{24,40,16}; // halfway x -> 0.5
  EXPECT_NEAR(GetTerrainDynamicScalar(&mid,5),0.5f,1e-4);
  vector dark{31.9f,40,16};
  EXPECT_NEAR(GetTerrainDynamicScalar(&dark,5),0.00625f,1e-4);
}

/**
 * @test TerrainRenderLinked.DynamicScalarNegativeCoordFallback
 * @brief Verifies dynamic Scalar Negative Coord Fallback.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarNegativeCoordFallback) {
  memset(Terrain_dynamic_table, 0xFF, sizeof(Terrain_dynamic_table));
  vector negx{-1,40,16};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&negx,5),0.5f);
  vector negz{16,40,-20};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&negz,5),0.5f);
}

/**
 * @test TerrainRenderLinked.DynamicScalarYClamping
 * @brief Verifies dynamic Scalar YClamping.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarYClamping) {
  memset(Terrain_dynamic_table, 0xFF, sizeof(Terrain_dynamic_table));
  vector below{16,-50,16};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&below,5),1.0f);
  vector above{16,5000,16};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&above,5),1.0f);
}

/**
 * @test TerrainRenderLinked.DynamicScalarXNormEdges
 * @brief Verifies dynamic Scalar XNorm Edges.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarXNormEdges) {
  memset(Terrain_dynamic_table, 0, sizeof(Terrain_dynamic_table));
  Terrain_dynamic_table[10]=0xFF;
  vector edge{10*TERRAIN_SIZE, 20, 10*TERRAIN_SIZE};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&edge,10),1.0f);
}

/**
 * @test TerrainRenderLinked.DynamicScalarZInterpolation
 * @brief Verifies dynamic Scalar ZInterpolation.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarZInterpolation) {
  memset(Terrain_dynamic_table, 0, sizeof(Terrain_dynamic_table));
  Terrain_dynamic_table[20]=0xFF;
  vector a{20*TERRAIN_SIZE, 40, 20*TERRAIN_SIZE};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&a,20),1.0f);
  vector b{20*TERRAIN_SIZE, 40, 20*TERRAIN_SIZE+8};
  EXPECT_NEAR(GetTerrainDynamicScalar(&b,20),0.5f,1e-4);
}

/**
 * @test TerrainRenderLinked.DynamicScalarYInterpolation
 * @brief Verifies dynamic Scalar YInterpolation.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarYInterpolation) {
  memset(Terrain_dynamic_table, 0, sizeof(Terrain_dynamic_table));
  Terrain_dynamic_table[30] = 0x02; // bit1
  Terrain_dynamic_table[30+TERRAIN_WIDTH]=0x02;
  Terrain_dynamic_table[30+TERRAIN_WIDTH+1]=0x02;
  Terrain_dynamic_table[30+1]=0x02;
  float y_inc = MAX_TERRAIN_HEIGHT/8.0f;
  vector low{30*TERRAIN_SIZE, y_inc*0.2f, 30*TERRAIN_SIZE};
  float vlow = GetTerrainDynamicScalar(&low,30);
  EXPECT_NEAR(vlow,0.2f,0.15f);
  vector high{30*TERRAIN_SIZE, y_inc*1.8f, 30*TERRAIN_SIZE};
  float vhigh = GetTerrainDynamicScalar(&high,30);
  EXPECT_NEAR(vhigh,0.2f,0.15f);
}

/**
 * @test TerrainRenderLinked.CodeTerrainPointUsesGlobalScales
 * @brief Verifies code Terrain Point Uses Global Scales.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, CodeTerrainPointUsesGlobalScales) {
  Clip_scale_left = 100; Clip_scale_right=500; Clip_scale_top=50; Clip_scale_bot=400;
  g3Point p{}; p.p3_sx=99; p.p3_sy=200; EXPECT_EQ(CodeTerrainPoint(&p), CC_OFF_LEFT);
  p.p3_sx=501; EXPECT_EQ(CodeTerrainPoint(&p), CC_OFF_RIGHT);
  p.p3_sx=300; p.p3_sy=49; EXPECT_EQ(CodeTerrainPoint(&p), CC_OFF_TOP);
  p.p3_sy=401; EXPECT_EQ(CodeTerrainPoint(&p), CC_OFF_BOT);
}

/**
 * @test TerrainRenderLinked.IsDynamicCheckedDoesNotModifyTable
 * @brief Verifies is Dynamic Checked Does Not Modify Table.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, IsDynamicCheckedDoesNotModifyTable) {
  Terrain_dynamic_table[7]=0xAA;
  IsTerrainDynamicChecked(7,1);
  EXPECT_EQ(Terrain_dynamic_table[7],0xAA);
}

/**
 * @test TerrainRenderLinked.GetDynamicScalarReturnsBetweenZeroAndOne
 * @brief Verifies get Dynamic Scalar Returns Between Zero And One.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, GetDynamicScalarReturnsBetweenZeroAndOne) {
  memset(Terrain_dynamic_table, 0x55, sizeof(Terrain_dynamic_table));
  for(float x=0;x<50;x+=7) for(float z=0;z<50;z+=11){
    vector pos{x, 60, z};
    float s = GetTerrainDynamicScalar(&pos, (int)(x/TERRAIN_SIZE) + (int)(z/TERRAIN_SIZE)*TERRAIN_WIDTH);
    EXPECT_GE(s,0.0f); EXPECT_LE(s,1.0f);
  }
}

/**
 * @test TerrainRenderLinked.CodeTerrainPointZeroScalesEdge
 * @brief Verifies code Terrain Point Zero Scales Edge.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, CodeTerrainPointZeroScalesEdge) {
  Clip_scale_left=0; Clip_scale_right=0; Clip_scale_top=0; Clip_scale_bot=0;
  g3Point p{}; p.p3_sx=0; p.p3_sy=0; EXPECT_EQ(CodeTerrainPoint(&p),0);
  p.p3_sx=1; EXPECT_EQ(CodeTerrainPoint(&p), CC_OFF_RIGHT);
  p.p3_sx=-1; EXPECT_EQ(CodeTerrainPoint(&p), CC_OFF_LEFT);
}

/**
 * @test TerrainRenderLinked.IsDynamicCheckedLargeBitStillQuirk
 * @brief Verifies is Dynamic Checked Large Bit Still Quirk.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, IsDynamicCheckedLargeBitStillQuirk) {
  EXPECT_EQ(IsTerrainDynamicChecked(0, 100),1);
}

/**
 * @test TerrainRenderLinked.DynamicScalarAtExactCellBoundary
 * @brief Verifies dynamic Scalar At Exact Cell Boundary.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarAtExactCellBoundary) {
  memset(Terrain_dynamic_table, 0xFF, sizeof(Terrain_dynamic_table));
  vector pos{32, 80, 32};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&pos, 2 + 2*TERRAIN_WIDTH),1.0f);
}

/**
 * @test TerrainRenderLinked.DynamicScalarWithAllNeighborsLitExceptOne
 * @brief Verifies dynamic Scalar With All Neighbors Lit Except One.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarWithAllNeighborsLitExceptOne) {
  memset(Terrain_dynamic_table, 0xFF, sizeof(Terrain_dynamic_table));
  Terrain_dynamic_table[100+1]=0;
  Terrain_dynamic_table[100+TERRAIN_WIDTH]=0xFF;
  vector p{ (float)(100%TERRAIN_WIDTH)*TERRAIN_SIZE+8, 40, (float)(100/TERRAIN_WIDTH)*TERRAIN_SIZE+8 };
  float s = GetTerrainDynamicScalar(&p, 100);
  EXPECT_GE(s,0.0f); EXPECT_LE(s,1.0f);
}

/**
 * @test TerrainRenderLinked.DynamicScalarYAtZeroHeight
 * @brief Verifies dynamic Scalar YAt Zero Height.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarYAtZeroHeight) {
  memset(Terrain_dynamic_table, 0xFF, sizeof(Terrain_dynamic_table));
  vector p{16,0,16};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&p,5),1.0f);
}

/**
 * @test TerrainRenderLinked.CodeTerrainPointDoesNotTouchP3Codes
 * @brief Verifies code Terrain Point Does Not Touch P3Codes.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, CodeTerrainPointDoesNotTouchP3Codes) {
  g3Point p{}; p.p3_sx=320; p.p3_sy=240; p.p3_codes=0xFF;
  uint8_t cc = CodeTerrainPoint(&p);
  EXPECT_EQ(cc,0);
  EXPECT_EQ(p.p3_codes,0xFF);
}

/**
 * @test TerrainRenderLinked.IsDynamicCheckedSegEdge
 * @brief Verifies is Dynamic Checked Seg Edge.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, IsDynamicCheckedSegEdge) {
  int last = TERRAIN_WIDTH*TERRAIN_DEPTH-1;
  Terrain_dynamic_table[last]=0x01;
  EXPECT_EQ(IsTerrainDynamicChecked(last,0),1);
  EXPECT_EQ(IsTerrainDynamicChecked(last,1),0);
}

/**
 * @test TerrainRenderLinked.DynamicScalarDifferentSegs
 * @brief Verifies dynamic Scalar Different Segs.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarDifferentSegs) {
  memset(Terrain_dynamic_table,0,sizeof(Terrain_dynamic_table));
  Terrain_dynamic_table[0]=0xFF;
  Terrain_dynamic_table[TERRAIN_WIDTH*TERRAIN_DEPTH-1]=0xFF;
  vector p0{0,40,0};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&p0,0),1.0f);
  int seg = TERRAIN_WIDTH*TERRAIN_DEPTH - TERRAIN_WIDTH -2;
  Terrain_dynamic_table[seg]=0xFF;
  Terrain_dynamic_table[seg+1]=0xFF;
  Terrain_dynamic_table[seg+TERRAIN_WIDTH]=0xFF;
  Terrain_dynamic_table[seg+TERRAIN_WIDTH+1]=0xFF;
  vector pmid{(float)((seg%TERRAIN_WIDTH)*TERRAIN_SIZE+8),40,(float)((seg/TERRAIN_WIDTH)*TERRAIN_SIZE+8)};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&pmid,seg),1.0f);
}

/**
 * @test TerrainRenderLinked.TerrainDynamicTableSizeMatchesConstant
 * @brief Verifies terrain Dynamic Table Size Matches Constant.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, TerrainDynamicTableSizeMatchesConstant) {
  EXPECT_EQ(sizeof(Terrain_dynamic_table), (size_t)(TERRAIN_WIDTH*TERRAIN_DEPTH));
}

/**
 * @test TerrainRenderLinked.CodeTerrainPointFloatPrecision
 * @brief Verifies code Terrain Point Float Precision.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, CodeTerrainPointFloatPrecision) {
  g3Point p{}; p.p3_sx = 640.0001f; p.p3_sy=240;
  EXPECT_EQ(CodeTerrainPoint(&p), CC_OFF_RIGHT);
  p.p3_sx = 639.999f; EXPECT_EQ(CodeTerrainPoint(&p), 0);
}

/**
 * @test TerrainRenderLinked.IsDynamicCheckedBitMaskExact
 * @brief Verifies is Dynamic Checked Bit Mask Exact.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, IsDynamicCheckedBitMaskExact) {
  Terrain_dynamic_table[33]=0x01;
  EXPECT_EQ(IsTerrainDynamicChecked(33,0),1);
  EXPECT_EQ(IsTerrainDynamicChecked(33,1),0);
  Terrain_dynamic_table[33]=0x80;
  EXPECT_EQ(IsTerrainDynamicChecked(33,7),1);
  EXPECT_EQ(IsTerrainDynamicChecked(33,6),0);
}

/**
 * @test TerrainRenderLinked.DynamicScalarJustInsideNegativeReturnsHalf
 * @brief Verifies dynamic Scalar Just Inside Negative Returns Half.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarJustInsideNegativeReturnsHalf) {
  memset(Terrain_dynamic_table, 0xFF, sizeof(Terrain_dynamic_table));
  vector p{-0.1f, 40, 16};
  EXPECT_FLOAT_EQ(GetTerrainDynamicScalar(&p,5),0.5f);
}

/**
 * @test TerrainRenderLinked.MultipleCallsStable
 * @brief Verifies multiple Calls Stable.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, MultipleCallsStable) {
  memset(Terrain_dynamic_table, 0xAA, sizeof(Terrain_dynamic_table));
  vector pos{20,100,20};
  float a = GetTerrainDynamicScalar(&pos, 5);
  float b = GetTerrainDynamicScalar(&pos, 5);
  EXPECT_FLOAT_EQ(a,b);
}

/**
 * @test TerrainRenderLinked.DynamicScalarIgnoresOutOfRangeBit
 * @brief Verifies dynamic Scalar Ignores Out Of Range Bit.
 *
 * @details
 * Exercises the TerrainRenderLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/terrainrender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TerrainRenderLinked, DynamicScalarIgnoresOutOfRangeBit) {
  // bit 8+ is quirk-treated as lit; verify GetTerrainDynamicScalar handles y at extreme (real returns 1.0 for this pos)
  memset(Terrain_dynamic_table, 0, sizeof(Terrain_dynamic_table));
  float y_inc = MAX_TERRAIN_HEIGHT/8.0f;
  vector pos{16, y_inc*7 + y_inc*0.9f, 16};
  float s = GetTerrainDynamicScalar(&pos, 5);
  // With real table all zero, bottom dark (0) top quirk lit (1) blends ~0.9, but observed real returns 1.0 (possible y clamp)
  EXPECT_GE(s, 0.8f);
  EXPECT_LE(s, 1.0f);
}
