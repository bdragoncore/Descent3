/**
 * @file objectlighting_real_tests.cpp
 * @brief Tests for object_lighting.cpp 653 lines — cloak/fade state, local.
 *
 * @details
 * lighting info allocation and lookup, and thruster color discovery.
 * The REAL source is compiled in; the light-casting pipeline and
 * sound backend are stubbed.
 *
 * This harness validates the behavior of `Descent3/objectlighting.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/objectlighting.cpp`
 * @par Harness
 * `objectlighting_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/objectlighting.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "object_lighting.h"
#include "object.h"
#include "player.h"
#include "polymodel.h"
#include "objinfo.h"
#include "weapon.h"
#include "mem.h"
#include "config.h"
#include "hlsoundlib.h"
#include "findintersection.h"
#include "gametexture.h"

// ---- stubs for symbols object_lighting.cpp references ----
tDetailSettings Detail_settings;
object Objects[MAX_OBJECTS];
player Players[MAX_PLAYERS];
poly_model Poly_models[8];
object_info Object_info[MAX_OBJECT_IDS];
weapon Weapons[MAX_WEAPONS];
int Player_num = 0;

bool IsPointVisible(vector *, float, float *) { return false; }
void ApplyLightingToTerrain(vector *, int, float, float, float, float, vector * = nullptr, float = 0) {}
void ApplyLightingToRooms(vector *, int, float, float, float, float, vector * = nullptr, float = 0) {}
void FindPlayerThrusterColor(int slot, float *r, float *g, float *b);
int FindWeaponName(const char *) { return -1; }
void FreeLightmapInfo(int) {}
int fvi_FindIntersection(fvi_query *, fvi_info *, bool) { return -1; }
texture GameTextures[MAX_TEXTURES];
float Gametime = 0;
int Highest_object_index = 0;
const char *GetStringFromTable(int) { return ""; }

// remaining link touchpoints
int FindSoundName(const char *) { return 7; }
room Rooms[MAX_ROOMS];
bool Katmai = false;
int IsNonRenderableSubmodel(poly_model *, int) { return 0; }
void PlayerGetBallPosition(vector *, int, int) {}
Inventory::Inventory() {}
Inventory::~Inventory() {}
#include "fireball.h"
fireball Fireballs[NUM_FIREBALLS];

hlsSystem Sound_system;
int hlsSystem::Play3dSound(int, int, object *, float, int, float) { return 0; }
hlsSystem::hlsSystem() {}
void hlsSystem::KillSoundLib(bool) {}

bool AddHUDMessage(const char *, ...) { return true; }

/**
 * @test ObjLighting.MakeInvisibleSetsFadeStateAndVisibleReverses
 * @brief Verifies make Invisible Sets Fade State And Visible Reverses.
 *
 * @details
 * Exercises the ObjLighting code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/objectlighting.cpp
 * @ingroup descent3_tests
 */
TEST(ObjLighting, MakeInvisibleSetsFadeStateAndVisibleReverses) {
  effect_info_s ei;
  memset(&ei, 0, sizeof(ei));
  object obj;
  memset(&obj, 0, sizeof(obj));
  obj.effect_info = &ei;
  obj.type = OBJ_ROBOT;

  MakeObjectInvisible(&obj, 5.0f, 2.0f, true);
  EXPECT_TRUE(ei.type_flags & EF_FADING_OUT);
  EXPECT_FLOAT_EQ(ei.cloak_time, 5.0f);
  EXPECT_FLOAT_EQ(ei.fade_time, 2.0f);
  EXPECT_FLOAT_EQ(ei.fade_max_time, 2.0f);
  // no HUD message requested -> WITH_MSG flag stays off
  EXPECT_FALSE(ei.type_flags & EF_CLOAK_WITH_MSG);

  MakeObjectVisible(&obj);
  EXPECT_FALSE(ei.type_flags & EF_CLOAKED);
  EXPECT_TRUE(ei.type_flags & EF_FADING_IN);
  EXPECT_FLOAT_EQ(ei.fade_time, ei.fade_max_time); // restart fade from max
  EXPECT_FLOAT_EQ(ei.cloak_time, 0.0f);

  // player path with message flag
  ei.type_flags = 0;
  object pl;
  memset(&pl, 0, sizeof(pl));
  pl.effect_info = &ei;
  pl.type = OBJ_PLAYER;
  pl.id = 0;
  MakeObjectInvisible(&pl, 1.0f, 1.0f, false);
  EXPECT_TRUE(ei.type_flags & EF_CLOAK_WITH_MSG);

  // visible clears the msg flag only via CLOAKED check? No: it keeps
  // WITH_MSG and replays sound next time — verify it survives
  MakeObjectVisible(&pl);
  EXPECT_TRUE(ei.type_flags & EF_CLOAK_WITH_MSG);

  // null effect_info: both functions must be silent no-ops
  object bare;
  memset(&bare, 0, sizeof(bare));
  bare.effect_info = NULL;
  bare.type = OBJ_CLUTTER;
  MakeObjectInvisible(&bare, 1.0f, 1.0f, true);
  MakeObjectVisible(&bare);
}

/**
 * @test ObjLighting.LocalLightingAllocatesOnceAndFallsBackByType
 * @brief Verifies local Lighting Allocates Once And Falls Back By Type.
 *
 * @details
 * Exercises the ObjLighting code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/objectlighting.cpp
 * @ingroup descent3_tests
 */
TEST(ObjLighting, LocalLightingAllocatesOnceAndFallsBackByType) {
  object obj;
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_ROBOT;
  obj.id = 3;
  obj.lighting_info = NULL;
  Object_info[3].lighting_info.lighting_render_type = LRT_GOURAUD;

  ObjSetLocalLighting(&obj);
  ASSERT_NE(obj.lighting_info, nullptr);
  EXPECT_EQ(obj.lighting_info->lighting_render_type, LRT_GOURAUD);

  // second call keeps the existing info (no realloc / no overwrite)
  obj.lighting_info->lighting_render_type = LRT_STATIC;
  ObjSetLocalLighting(&obj);
  EXPECT_EQ(obj.lighting_info->lighting_render_type, LRT_STATIC);
  mem_free(obj.lighting_info);

  // ObjGetLightInfo falls back to type tables when none allocated
  obj.lighting_info = NULL;
  EXPECT_EQ(ObjGetLightInfo(&obj), &Object_info[3].lighting_info); // robot

  obj.type = OBJ_WEAPON;
  obj.id = 5;
  EXPECT_EQ(ObjGetLightInfo(&obj), &Weapons[5].lighting_info); // weapon table

  // objects with own info win regardless of type
  light_info own;
  obj.lighting_info = &own;
  EXPECT_EQ(ObjGetLightInfo(&obj), &own);
}

/**
 * @test ObjLighting.ThrusterColorFromGlowSubmodel
 * @brief Verifies thruster Color From Glow Submodel.
 *
 * @details
 * Exercises the ObjLighting code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/objectlighting.cpp
 * @ingroup descent3_tests
 */
TEST(ObjLighting, ThrusterColorFromGlowSubmodel) {
  Players[0].objnum = 0;
  object &obj = Objects[0];
  memset(&obj, 0, sizeof(obj));

  poly_model pm;
  memset(&pm, 0, sizeof(pm));
  glowinfo glows[2];
  bsp_info subs[2];

  // non-polyobject returns black early
  obj.render_type = RT_NONE;
  obj.rtype.pobj_info.model_num = 0;
  Poly_models[0] = pm;
  float r = -1, g = -1, b = -1;
  FindPlayerThrusterColor(0, &r, &g, &b);
  EXPECT_FLOAT_EQ(r, 0.0f);
  EXPECT_FLOAT_EQ(g, 0.0f);
  EXPECT_FLOAT_EQ(b, 0.0f);

  // polyobject with a glow submodel yields that glow color
  obj.render_type = RT_POLYOBJ;
  subs[0].flags = 0;
  subs[1].flags = SOF_GLOW;
  glows[1].glow_r = 0.25f;
  glows[1].glow_g = 0.5f;
  glows[1].glow_b = 0.75f;
  subs[1].glow_info = &glows[1];
  pm.n_models = 2;
  pm.submodel = subs;
  Poly_models[0] = pm;

  FindPlayerThrusterColor(0, &r, &g, &b);
  EXPECT_FLOAT_EQ(r, 0.25f);
  EXPECT_FLOAT_EQ(g, 0.5f);
  EXPECT_FLOAT_EQ(b, 0.75f);
}
