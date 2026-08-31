/**
 * @file renderobject_real_tests.cpp
 * @brief Tests for Descent3/renderobject.cpp -- object render gating, lighting.
 *
 * @details
 * mode routing, polymodel effect assembly, LOD selection, and the
 * motion-blur interpolation helper.
 *
 * Renderer primitives (g3_*, rend_*, DrawPolygonModel) and the per-type
 * draw helpers are recorded via stubs; game globals are fabricated.
 *
 * This harness validates the behavior of `Descent3/renderobject.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/renderobject.cpp`
 * @par Harness
 * `renderobject_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/renderobject.cpp
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cstring>

#include "object.h"
#include "object_lighting.h"
#include "3d.h"
#include "polymodel.h"
#include "renderer.h"
#include "weapon.h"
#include "fireball.h"
#include "descent.h"
#include "renderobject.h"
#include "AIMain.h"
#include "objinfo.h"
#include "splinter.h"
#include "render.h"
#include "gametexture.h"
#include "game.h"
#include "player.h"
#include "gameloop.h"
#include "findintersection.h"
#include "grtext.h"
#include "gamefont.h"
#include "config.h"
#include "viseffect.h"
#include "game2dll.h"
#include "ship.h"
#include "terrain.h"
#include "multi.h"

// defined in renderobject.cpp but missing from renderobject.h
bool GetLinearPosition(vector *points, float *times, int num_points, float t, vector *pos);

// ---- recorders ----
struct PolyDraw {
  int kind; // 0 static 1 gouraud 2 lightmaps
  int model_num;
  float r, g, b;
  vector lightdir;
  uint32_t subflags;
  int use_effect;
  int overlay;
  bool has_nt;   // normalized_times pointer non-null
};
static std::vector<PolyDraw> g_polydraws;
static std::vector<polymodel_effect> g_effects;
static int g_fireballs = 0, g_weapons = 0, g_splinters = 0;
static float g_alpha_factor = -1;

void DrawPolygonModel(vector *pos, matrix *orient, int model_num, float *normalized_time, int flags, float r, float g,
                      float b, uint32_t subflags, uint8_t use_effect, uint8_t overlay) {
  (void)pos;
  (void)orient;
  (void)flags;
  g_polydraws.push_back({0, model_num, r, g, b, {}, subflags, use_effect, overlay, normalized_time != nullptr});
}
void DrawPolygonModel(vector *pos, matrix *orient, int model_num, float *normalized_time, int flags, vector *lightdir,
                      float r, float g, float b, uint32_t subflags, uint8_t use_effect, uint8_t overlay) {
  (void)pos;
  (void)orient;
  (void)flags;
  g_polydraws.push_back({1, model_num, r, g, b, *lightdir, subflags, use_effect, overlay,
                         normalized_time != nullptr});
}
void DrawPolygonModel(vector *pos, matrix *orient, int model_num, float *normalized_time, int flags,
                      lightmap_object *lmo, uint32_t subflags, uint8_t use_effect, uint8_t overlay) {
  (void)pos;
  (void)orient;
  (void)flags;
  (void)lmo;
  g_polydraws.push_back({2, model_num, 0, 0, 0, {}, subflags, use_effect, overlay, normalized_time != nullptr});
}
void SetPolymodelEffect(polymodel_effect *pe) { g_effects.push_back(*pe); }
void DrawFireballObject(object *) { g_fireballs++; }
void DrawWeaponObject(object *) { g_weapons++; }
void DrawSplinterObject(object *) { g_splinters++; }
// NOTE: DrawShardObject is REAL in renderobject.cpp -- observed via
// g3_DrawPoly records below.
struct PolyCall {
  int nv;
};
static std::vector<PolyCall> g_polys;

// controllable g3 rotate: records position, exposes chosen depth
static float g_rotate_z = 0.0f;
static int g_rotates = 0;
uint8_t g3_RotatePoint(g3Point *pnt, vector *pos) {
  pnt->p3_vec = *pos;
  pnt->p3_z = g_rotate_z;
  g_rotates++;
  return 0;
}
void g3_StartInstanceMatrix(vector *, matrix *) {}
void g3_DoneInstance() {}
uint8_t g3_CodePoint(g3Point *) { return 0; }
void g3_ProjectPoint(g3Point *) {}
bool g3_CheckNormalFacing(vector *, vector *) { return false; }
int g3_DrawPoly(int nv, g3Point **, int, int, g3Codes *) {
  g_polys.push_back({nv});
  return 1;
}
void g3_DrawBitmap(vector *, float, float, int, int) {}
void g3_DrawRotatedBitmap(vector *, uint16_t, float, float, int, int) {}
void g3_DrawPlanarRotatedBitmap(vector *, vector *, uint16_t, float, float, int) {}
void g3_DrawSpecialLine(g3Point *, g3Point *) {}
void g3_GetViewPosition(vector *v) { memset(v, 0, sizeof(*v)); }
void g3_GetUnscaledMatrix(matrix *m) { memset(m, 0, sizeof(*m)); }

void rend_SetAlphaType(signed char) {}
void rend_SetAlphaValue(unsigned char) {}
void rend_SetAlphaFactor(float f) { g_alpha_factor = f; }
float rend_GetAlphaFactor() { return g_alpha_factor; }
void rend_SetColorModel(color_model) {}
void rend_SetLighting(light_state) {}
void rend_SetTextureType(texture_type) {}
void rend_SetWrapType(wrap_type) {}
void rend_SetOverlayType(unsigned char) {}
void rend_SetZBias(float) {}
void rend_SetZBufferWriteMask(int) {}

extern "C" {
void grtext_CenteredPrintf(int, int, const char *, ...) {}
void grtext_Flush() {}
void grtext_SetColor(ddgr_color) {}
void grtext_SetFont(int) {}
}

void SetNormalizedTimeObj(object *, float *) {}
int IsNonRenderableSubmodel(poly_model *, int) { return 0; }
int VisEffectCreate(uint8_t, uint8_t, int, vector *) { return -1; }
light_info *ObjGetLightInfo(object *) { return nullptr; }
void PlayerGetBallPosition(vector *dest, int, int) { memset(dest, 0, sizeof(*dest)); }
void CallGameDLL(int, dllinfo *) {}
int FindTextureName(const char *) { return -1; }
float GetRoomDynamicScalarReturn = 0.5f;
void GetRoomDynamicScalar(vector *, room *, float *r, float *g, float *b) {
  *r = *g = *b = GetRoomDynamicScalarReturn;
}
float GetTerrainDynamicScalar(vector *, int) { return 0.5f; }
int fvi_FindIntersection(fvi_query *, fvi_info *, bool) { return 0; }

static std::vector<int> g_psrand;
int ps_rand_return = 7;
int32_t ps_rand(void) { return ps_rand_return; }

// controllable function mode for the AI list gate
static function_mode g_function_mode = GAME_MODE;
function_mode GetFunctionMode() { return g_function_mode; }

// ObjGet resolves handles as plain indices
object *ObjGet(int handle) {
  int idx = handle & 0xFFFF;
  if (idx < 0 || idx >= MAX_OBJECTS || Objects[idx].type == OBJ_NONE)
    return nullptr;
  return &Objects[idx];
}

// ---- world state ----
object Objects[MAX_OBJECTS];
player Players[MAX_PLAYERS];
int Player_num = 0;
object *Player_object = nullptr;
object *Viewer_object = nullptr;
ship Ships[MAX_SHIPS];
object_info Object_info[MAX_OBJECT_IDS];
poly_model Poly_models[MAX_POLY_MODELS];
weapon Weapons[MAX_WEAPONS];
fireball Fireballs[64];
vis_effect *VisEffects = nullptr; // array never touched by these tests
room Rooms[MAX_ROOMS];
texture GameTextures[MAX_TEXTURES];
netgame_info Netgame;
netplayer NetPlayers[MAX_NET_PLAYERS];
tDetailSettings Detail_settings;
terrain_sky Terrain_sky;
int Game_fonts[8] = {};
int Game_mode = 0;
float Gametime = 0.0f;
int FrameCount = 0;
bool UseHardware = true;
bool Use_motion_blur = false;
bool Katmai = false;
bool NoLightmaps = false;
bool Render_mirror_for_room = false;
int Marker_polynum = 9;
int Num_teams = 0;
bool Multi_logo_state = false;
float HudNameTan = 0.0f;
uint32_t Players_typing = 0;
float Far_clip_z = 500.0f;
float Room_fog_distance = 0.0f, Room_fog_eye_distance = 0.0f;
int Room_fog_plane_check = -1;
vector Room_fog_plane;
vector Room_fog_portal_vert;
bool Player_has_camera = false;
int Player_camera_objnum = -1;
tPosHistory Object_position_samples[MAX_OBJECT_POS_HISTORY];
uint8_t Object_position_head = 0;
int16_t Object_map_position_history[MAX_OBJECTS];
float Last_position_history_update[MAX_POSITION_HISTORY];
int AI_NumRendered = 0;
int AI_RenderedList[MAX_OBJECTS];

// assorted globals referenced but not defined by the module under test
int Game_window_w = 640;
bool Game_paused = false;
float Render_FOV = 1.0f;
int bm_w(int, int) { return 32; }
int bm_h(int, int) { return 32; }
int GetTextureBitmap(int, int, bool) { return -1; }
dllinfo DLLInfo;
void DrawColoredDisk(vector *, float, float, float, float, float, float, unsigned char, unsigned char) {}

// pulled in via player.h; not exercised here
Inventory::Inventory() {}
Inventory::~Inventory() {}

// ---- helpers ----
static void ResetWorld() {
  memset(Objects, 0, sizeof(Objects));
  memset(Players, 0, sizeof(Players));
  memset(Ships, 0, sizeof(Ships));
  memset(Object_info, 0, sizeof(Object_info));
  memset(Poly_models, 0, sizeof(Poly_models));
  memset(Rooms, 0, sizeof(Rooms));
  memset(Object_map_position_history, 0xFF, sizeof(Object_map_position_history));
  g_polydraws.clear();
  g_effects.clear();
  g_polys.clear();
  g_fireballs = g_weapons = g_splinters = 0;
  g_alpha_factor = -1;
  g_rotate_z = 0.0f;
  g_rotates = 0;
  AI_NumRendered = 0;
  UseHardware = true;
  Use_motion_blur = false;
  Katmai = false;
  NoLightmaps = false;
  Render_mirror_for_room = false;
  Num_teams = 0;
  Multi_logo_state = 0;
  Room_fog_plane_check = -1;
  Detail_settings = {};
  Detail_settings.Object_complexity = 1; // medium: detail_scalar 1.0
  Poly_models[7].new_style = 1;
  FrameCount = 100;
  g_function_mode = GAME_MODE;
  Viewer_object = &Objects[1];
  Objects[1].type = OBJ_PLAYER; // keep viewer deref safe
}

// build a minimal indoor poly object ready to render. Objects must
// live in the global table because OBJNUM() is pointer arithmetic
// against Objects[].
static void MakePolyObj(object *obj, int objnum) {
  memset(obj, 0, sizeof(*obj));
  obj->type = OBJ_ROBOT;
  obj->id = 3;
  obj->render_type = RT_POLYOBJ;
  obj->control_type = CT_NONE;
  obj->movement_type = MT_NONE;
  obj->roomnum = 5; // indoor
  obj->flags = OF_SAFE_TO_RENDER;
  obj->rtype.pobj_info.model_num = 7;
  obj->rtype.pobj_info.subobj_flags = 0xFFFFFFFF;
  obj->size = 10.0f;
}

/**
 * @test RenderObject.GetLinearPositionInterpolatesAndClampsSegmentSearch
 * @brief Verifies get Linear Position Interpolates And Clamps Segment Search.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, GetLinearPositionInterpolatesAndClampsSegmentSearch) {
  vector pts[3] = {{0, 0, 0}, {10, 0, 0}, {10, 0, 20}};
  float times[3] = {0, 2, 4};

  // halfway through segment one
  vector pos = {-1, -1, -1};
  ASSERT_TRUE(GetLinearPosition(pts, times, 3, 1.0f, &pos));
  EXPECT_FLOAT_EQ(pos.x(), 5.0f);
  EXPECT_FLOAT_EQ(pos.y(), 0.0f);

  // second segment
  ASSERT_TRUE(GetLinearPosition(pts, times, 3, 3.0f, &pos));
  EXPECT_FLOAT_EQ(pos.x(), 10.0f);
  EXPECT_FLOAT_EQ(pos.z(), 10.0f);

  // before first / after last sample -> no segment found
  EXPECT_FALSE(GetLinearPosition(pts, times, 3, -0.5f, &pos));
  EXPECT_FALSE(GetLinearPosition(pts, times, 3, 4.5f, &pos));

  // a lone point can never be interpolated
  EXPECT_FALSE(GetLinearPosition(pts, times, 1, 0.0f, &pos));
}

/**
 * @test RenderObject.GatesDummyAttachedAndUnsafeObjects
 * @brief Verifies gates Dummy Attached And Unsafe Objects.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, GatesDummyAttachedAndUnsafeObjects) {
  ResetWorld();

  // dummy objects are silently skipped
  MakePolyObj(&Objects[4], 4);
  Objects[4].type = OBJ_DUMMY;
  RenderObject(&Objects[4]);
  EXPECT_TRUE(g_polydraws.empty());
  EXPECT_FALSE(Objects[4].flags & OF_RENDERED);

  // an invisible MOVING parent hides its attached child...
  ResetWorld();
  MakePolyObj(&Objects[5], 5);
  Objects[9].type = OBJ_ROBOT;
  Objects[9].render_type = RT_NONE;
  Objects[9].movement_type = MT_PHYSICS;
  Objects[5].flags |= OF_ATTACHED;
  Objects[5].attach_ultimate_handle = 9;
  RenderObject(&Objects[5]);
  EXPECT_TRUE(g_polydraws.empty());

  // QUIRK: an invisible but STATIONARY robot parent does not hide its
  // children -- only players or moving parents do
  ResetWorld();
  MakePolyObj(&Objects[5], 5);
  Objects[9].type = OBJ_ROBOT;
  Objects[9].render_type = RT_NONE;
  Objects[9].movement_type = MT_NONE;
  Objects[5].flags |= OF_ATTACHED;
  Objects[5].attach_ultimate_handle = 9;
  RenderObject(&Objects[5]);
  EXPECT_FALSE(g_polydraws.empty());

  // ...but powerup parents never hide their children, even when
  // invisible and motionless
  ResetWorld();
  MakePolyObj(&Objects[5], 5);
  Objects[9].type = OBJ_POWERUP;
  Objects[9].render_type = RT_NONE;
  Objects[9].movement_type = MT_NONE;
  Objects[5].flags |= OF_ATTACHED;
  Objects[5].attach_ultimate_handle = 9;
  RenderObject(&Objects[5]);
  EXPECT_FALSE(g_polydraws.empty());
}

/**
 * @test RenderObject.MarksRenderedFrameAndClearsSafeFlagOutsideMirrors
 * @brief Verifies marks Rendered Frame And Clears Safe Flag Outside Mirrors.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, MarksRenderedFrameAndClearsSafeFlagOutsideMirrors) {
  ResetWorld();
  MakePolyObj(&Objects[6], 6);

  RenderObject(&Objects[6]);
  EXPECT_TRUE(Objects[6].flags & OF_RENDERED);
  EXPECT_EQ(Objects[6].renderframe, FrameCount % 65536);
  // non-mirror pass consumes the safe-to-render token
  EXPECT_FALSE(Objects[6].flags & OF_SAFE_TO_RENDER);

  // mirror passes leave it set so both eyes see the object
  ResetWorld();
  Render_mirror_for_room = true;
  MakePolyObj(&Objects[6], 6);
  RenderObject(&Objects[6]);
  EXPECT_TRUE(Objects[6].flags & OF_SAFE_TO_RENDER);
}

/**
 * @test RenderObject.AiRobotsAreListedForTheFrameListOnlyInGame
 * @brief Verifies ai Robots Are Listed For The Frame List Only In Game.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, AiRobotsAreListedForTheFrameListOnlyInGame) {
  ResetWorld();
  MakePolyObj(&Objects[42], 42);
  Objects[42].control_type = CT_AI;

  RenderObject(&Objects[42]);
  ASSERT_EQ(AI_NumRendered, 1);
  EXPECT_EQ(AI_RenderedList[0], 42);

  // menus don't populate the AI list
  ResetWorld();
  g_function_mode = MENU_MODE;
  MakePolyObj(&Objects[42], 42);
  Objects[42].control_type = CT_AI;
  RenderObject(&Objects[42]);
  EXPECT_EQ(AI_NumRendered, 0);
  g_function_mode = GAME_MODE;
}

/**
 * @test RenderObject.DispatchesEachRenderTypeToItsDrawer
 * @brief Verifies dispatches Each Render Type To Its Drawer.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, DispatchesEachRenderTypeToItsDrawer) {
  ResetWorld();

  // each drawer consumes the safe-to-render flag outside mirrors, so
  // every case runs on a freshly built object
  auto run_case = [&](int slot, int rt) {
    MakePolyObj(&Objects[slot], slot);
    Objects[slot].lighting_render_type = LRT_STATIC;
    Objects[slot].render_type = rt;
    RenderObject(&Objects[slot]);
  };

  run_case(10, RT_FIREBALL);
  EXPECT_EQ(g_fireballs, 1);

  run_case(11, RT_WEAPON);
  EXPECT_EQ(g_weapons, 1);

  run_case(12, RT_SPLINTER);
  EXPECT_EQ(g_splinters, 1);

  run_case(13, RT_SHARD);
  // real DrawShardObject rotates 3 verts and issues one 3-vert poly
  ASSERT_EQ(g_polys.size(), 1u);
  EXPECT_EQ(g_polys[0].nv, 3);

  // RT_NONE draws nothing but still marks the frame
  run_case(14, RT_NONE);
  EXPECT_EQ(g_polydraws.size(), 0u);
  EXPECT_TRUE(Objects[14].flags & OF_RENDERED);
}

/**
 * @test RenderObject.LightingModeRoutesToMatchingPolygonDrawOverload
 * @brief Verifies lighting Mode Routes To Matching Polygon Draw Overload.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, LightingModeRoutesToMatchingPolygonDrawOverload) {
  ResetWorld();

  // LRT_STATIC path: SetupMineObject sets static(1,1,1)
  MakePolyObj(&Objects[20], 20);
  Objects[20].lighting_render_type = LRT_STATIC;
  RenderObject(&Objects[20]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].kind, 0);
  EXPECT_FLOAT_EQ(g_polydraws[0].r, 1.0f);
  EXPECT_EQ(g_polydraws[0].subflags, 0xFFFFFFFFu);
  EXPECT_EQ(g_polydraws[0].overlay, 0); // full subobj mask -> no overlay

  // robots with partial subobj masks get the overlay flag
  ResetWorld();
  MakePolyObj(&Objects[21], 21);
  Objects[21].lighting_render_type = LRT_STATIC;
  Objects[21].rtype.pobj_info.subobj_flags = 0xF;
  RenderObject(&Objects[21]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].overlay, 1);

  // LRT_GOURAUD (new-style model, lightmaps on) uses the gouraud
  // overload with the straight-down light vector
  ResetWorld();
  MakePolyObj(&Objects[22], 22);
  Objects[22].lighting_render_type = LRT_GOURAUD;
  RenderObject(&Objects[22]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].kind, 1);
  EXPECT_FLOAT_EQ(g_polydraws[0].lightdir.y(), -1.0f);

  // old-style models are forced down the static path no matter what
  ResetWorld();
  Poly_models[7].new_style = 0;
  MakePolyObj(&Objects[23], 23);
  Objects[23].lighting_render_type = LRT_GOURAUD;
  RenderObject(&Objects[23]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].kind, 0);

  // LRT_LIGHTMAPS falls back to static when no lightmap is attached
  ResetWorld();
  MakePolyObj(&Objects[24], 24);
  Objects[24].lighting_render_type = LRT_LIGHTMAPS;
  RenderObject(&Objects[24]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].kind, 0);

  // ...and routes the lightmap object when present
  ResetWorld();
  MakePolyObj(&Objects[25], 25);
  Objects[25].lighting_render_type = LRT_LIGHTMAPS;
  lightmap_object lm = {};
  lm.used = 1;
  Objects[25].lm_object = lm;
  RenderObject(&Objects[25]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].kind, 2);
}

/**
 * @test RenderObject.PolymodelEffectsAssembleFromObjectState
 * @brief Verifies polymodel Effects Assemble From Object State.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, PolymodelEffectsAssembleFromObjectState) {
  ResetWorld();
  MakePolyObj(&Objects[30], 30);
  Objects[30].effect_info = new effect_info_s;
  memset(Objects[30].effect_info, 0, sizeof(effect_info_s));

  // fading out: alpha interpolates from .08 toward 1.0 as time elapses
  Objects[30].effect_info->type_flags = EF_FADING_OUT;
  Objects[30].effect_info->fade_time = 0.25f;
  Objects[30].effect_info->fade_max_time = 0.5f;
  RenderObject(&Objects[30]);
  ASSERT_EQ(g_effects.size(), 1u);
  EXPECT_EQ(g_effects[0].type, PEF_ALPHA);
  EXPECT_NEAR(g_effects[0].alpha, .08 + (.92 * .5), .0001f);
  ASSERT_EQ(g_polydraws.back().use_effect, 1);

  // cloaked: fixed alpha plus deform
  ResetWorld();
  MakePolyObj(&Objects[31], 31);
  Objects[31].effect_info = new effect_info_s;
  memset(Objects[31].effect_info, 0, sizeof(effect_info_s));
  Objects[31].effect_info->type_flags = EF_CLOAKED;
  RenderObject(&Objects[31]);
  ASSERT_EQ(g_effects.size(), 1u);
  EXPECT_EQ(g_effects[0].type, PEF_ALPHA | PEF_DEFORM);
  EXPECT_FLOAT_EQ(g_effects[0].alpha, .13f);
  EXPECT_FLOAT_EQ(g_effects[0].deform_range, .1f);

  // deformed: white core fading green/blue by deform_time
  ResetWorld();
  MakePolyObj(&Objects[32], 32);
  Objects[32].effect_info = new effect_info_s;
  memset(Objects[32].effect_info, 0, sizeof(effect_info_s));
  Objects[32].effect_info->type_flags = EF_DEFORM;
  Objects[32].effect_info->deform_range = 2.0f;
  Objects[32].effect_info->deform_time = 0.8f;
  RenderObject(&Objects[32]);
  ASSERT_EQ(g_effects.size(), 1u);
  EXPECT_TRUE(g_effects[0].type & PEF_DEFORM);
  EXPECT_TRUE(g_effects[0].type & PEF_COLOR);
  EXPECT_FLOAT_EQ(g_effects[0].deform_range, 2.0f * .8f);
  EXPECT_FLOAT_EQ(g_effects[0].r, 1.0f);
  EXPECT_FLOAT_EQ(g_effects[0].g, 1 - .4f);
  EXPECT_FLOAT_EQ(g_effects[0].b, 1 - .4f);

  delete Objects[30].effect_info;
}

/**
 * @test RenderObject.IndoorFogAndSpecularFeedPolymodelEffect
 * @brief Verifies indoor Fog And Specular Feed Polymodel Effect.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, IndoorFogAndSpecularFeedPolymodelEffect) {
  ResetWorld();
  MakePolyObj(&Objects[33], 33);
  Rooms[5].flags |= RF_FOG;
  Rooms[5].fog_depth = 33.0f;
  Rooms[5].fog_r = .1f;
  Rooms[5].fog_g = .2f;
  Rooms[5].fog_b = .3f;
  Room_fog_distance = 40.0f;
  Room_fog_eye_distance = 12.0f;
  Room_fog_plane_check = 3;

  RenderObject(&Objects[33]);
  ASSERT_EQ(g_effects.size(), 1u);
  EXPECT_TRUE(g_effects[0].type & PEF_FOGGED_MODEL);
  EXPECT_FLOAT_EQ(g_effects[0].fog_depth, 33.0f);
  EXPECT_FLOAT_EQ(g_effects[0].fog_r, .1f);
  EXPECT_EQ(g_effects[0].fog_plane_check, 3);

  // specular adds the face-level variant with the effect's light data
  ResetWorld();
  MakePolyObj(&Objects[34], 34);
  Objects[34].effect_info = new effect_info_s;
  memset(Objects[34].effect_info, 0, sizeof(effect_info_s));
  Objects[34].effect_info->type_flags = EF_SPECULAR;
  Objects[34].effect_info->spec_r = .4f;
  RenderObject(&Objects[34]);
  ASSERT_EQ(g_effects.size(), 1u);
  EXPECT_TRUE(g_effects[0].type & PEF_SPECULAR_FACES); // robot, not powerup
  EXPECT_FLOAT_EQ(g_effects[0].spec_r, .4f);
  EXPECT_FLOAT_EQ(g_effects[0].spec_scalar, 1.0f);
  delete Objects[34].effect_info;
}

// configure LOD distances for a freshly built robot in the table
static void SetupLodObject(int slot) {
  ResetWorld();
  MakePolyObj(&Objects[slot], slot);
  Object_info[Objects[slot].id].med_lod_distance = 100.0f;
  Object_info[Objects[slot].id].lo_lod_distance = 200.0f;
  Object_info[Objects[slot].id].med_render_handle = 77;
  Object_info[Objects[slot].id].lo_render_handle = 88;
}

/**
 * @test RenderObject.LodSelectionUsesDepthThresholdsWithDetailScalar
 * @brief Verifies lod Selection Uses Depth Thresholds With Detail Scalar.
 *
 * @details
 * Exercises the RenderObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/renderobject.cpp
 * @ingroup descent3_tests
 */
TEST(RenderObject, LodSelectionUsesDepthThresholdsWithDetailScalar) {
  // close: base model
  SetupLodObject(40);
  g_rotate_z = 50.0f;
  RenderObject(&Objects[40]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].model_num, 7);

  // mid range: med LOD swap
  SetupLodObject(41);
  g_rotate_z = 150.0f;
  RenderObject(&Objects[41]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].model_num, 77);

  // far: lo LOD swap
  SetupLodObject(42);
  g_rotate_z = 250.0f;
  RenderObject(&Objects[42]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].model_num, 88);

  // lowest detail setting shrinks thresholds by .6 -> 150 now counts far
  SetupLodObject(43);
  Detail_settings.Object_complexity = 0;
  g_rotate_z = 150.0f;
  RenderObject(&Objects[43]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].model_num, 88);

  // highest detail setting grows thresholds by 1.2 -> 250 still med
  SetupLodObject(44);
  Detail_settings.Object_complexity = 2;
  g_rotate_z = 230.0f;
  RenderObject(&Objects[44]);
  ASSERT_EQ(g_polydraws.size(), 1u);
  EXPECT_EQ(g_polydraws[0].model_num, 77);
}
