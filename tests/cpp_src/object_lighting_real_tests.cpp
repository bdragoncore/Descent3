/**
 * @file object_lighting_real_tests.cpp
 * @brief Tests for object_lighting.cpp (~653 lines).
 *
 * @details
 * Covers cloak/fade state via MakeObjectInvisible/Visible (incl. static
 * sound-name caching and HUD message quirks), ObjSetLocalLighting /
 * ObjGetLightInfo fallback chain, thruster glow color lookup, and the
 * object lightmap memory setup/teardown cycle.
 *
 * This harness validates the behavior of `Descent3/object_lighting.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/object_lighting.cpp`
 * @par Harness
 * `object_lighting_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/object_lighting.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "object_lighting.h"
#include "object.h"
#include "objinfo.h"
#include "polymodel.h"
#include "weapon.h"
#include "player.h"
#include "lighting.h"
#include "renderobject.h"
#include "soundload.h"
#include "hud.h"
#include "hlsoundlib.h"
#include "lightmap_info.h"
#include "physics/findintersection.h"
#include "fireball.h"
#include "config.h"
#include "Inventory.h"
#include "bitmap.h"
#include "room.h"
#include "log.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
object Objects[MAX_OBJECTS];
poly_model Poly_models[4];
object_info Object_info[64];
weapon Weapons[MAX_WEAPONS];
player Players[MAX_PLAYERS];
int Player_num = 0;
int Highest_object_index = 0;
tDetailSettings Detail_settings;
room Rooms[MAX_ROOMS];
texture GameTextures[MAX_TEXTURES];
float Gametime = 0.0f;
bool Katmai = false;

// player's embedded Inventory pulls these in
Inventory::Inventory() {}
Inventory::~Inventory() {}
fireball Fireballs[64]; // size not fixed by header

// ---------------------------------------------------------------------------
// Scripted object lookup
// ---------------------------------------------------------------------------
static object *g_lookup[MAX_OBJECTS];

object *ObjGet(int handle) {
  if (handle < 0 || handle >= MAX_OBJECTS)
    return nullptr;
  return g_lookup[handle];
}

// ---------------------------------------------------------------------------
// Engine stubs
// ---------------------------------------------------------------------------
static bool g_point_visible = true;
int IsPointVisible(vector *, float, float *) { return g_point_visible; }

void ApplyLightingToRooms(vector *, int, float, float, float, float, vector *, float) {}
void ApplyLightingToTerrain(vector *, int, float, float, float, float, vector *, float) {}
void ReflectRay(vector *a, vector *, vector *) { (void)a; }
int FindWeaponName(const char *) { return 0; }
vector *PlayerGetBallPosition_res;
void PlayerGetBallPosition(vector *dest, int, int) { *dest = {0.f, 0.f, 0.f}; }
int ps_rand() { return 0; }
void ps_srand(unsigned int) {}

static int fvi_calls;
int fvi_FindIntersection(fvi_query *, fvi_info *, bool) {
  fvi_calls++;
  return 0;
}

int IsNonRenderableSubmodel(poly_model *pm, int submodelnum) {
  if ((pm->submodel[submodelnum].flags & SOF_FRONTFACE) || (pm->submodel[submodelnum].flags & SOF_SHELL))
    return 1;
  if (pm->submodel[submodelnum].num_faces == 0)
    return 1;
  return 0;
}

static std::vector<int> g_freed_lmi;
void FreeLightmapInfo(int handle) { g_freed_lmi.push_back(handle); }

static int g_sound_lookups;
int FindSoundName(const char *name) {
  (void)name;
  g_sound_lookups++;
  return 42; // valid sound id
}

static std::vector<std::string> g_hud_msgs;
bool AddHUDMessage(const char *format, ...) {
  g_hud_msgs.push_back(format);
  return true;
}
const char *GetStringFromTable(int index) {
  static char buf[32];
  snprintf(buf, sizeof(buf), "<txt%d>", index);
  return buf;
}

// ---------------------------------------------------------------------------
// Sound system stubs
// ---------------------------------------------------------------------------
static int g_next_sound_handle = 1;
static std::vector<int> g_played_sounds;
hlsSystem Sound_system;
hlsSystem::hlsSystem() {}
int hlsSystem::Play2dSound(int, float, float, uint16_t) { return -1; }
int hlsSystem::Play3dSound(int sound_index, int priority, object *, float volume, int flags, float offset) {
  (void)priority;
  (void)volume;
  (void)flags;
  (void)offset;
  g_played_sounds.push_back(sound_index);
  return g_next_sound_handle++;
}
void hlsSystem::KillSoundLib(bool) {}

// Defined in object_lighting.cpp but not exported in any header
void FindPlayerThrusterColor(int slot, float *r, float *g, float *b);

// ---------------------------------------------------------------------------
// Fixture helpers
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for ObjLightingTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class ObjLightingTest : public ::testing::Test {
protected:
  void SetUp() override {
    memset(g_lookup, 0, sizeof(g_lookup));
    memset(Objects, 0, sizeof(object) * 8);
    memset(Object_info, 0, sizeof(object_info) * 64);
    memset(&Poly_models, 0, sizeof(poly_model) * 4);
    memset(Players, 0, sizeof(player) * 2);
    g_hud_msgs.clear();
    g_played_sounds.clear();
    g_freed_lmi.clear();
    g_sound_lookups = 0;
    Player_num = 0;
    Highest_object_index = 0;

    // model 0: submodel 0 non-renderable (no faces), submodel 1 with 2 faces
    Poly_models[0] = poly_model{};
    Poly_models[0].n_models = 2;
    Poly_models[0].new_style = 1;
    Poly_models[0].submodel = s_subs;
    for (auto &b : s_subs)
      b = bsp_info{};
    s_subs[1].num_faces = 2;
    s_subs[1].faces = s_faces;
    s_faces[0].nverts = 4;
    s_faces[1].nverts = 3;
  }

  static bsp_info s_subs[2];
  static polyface s_faces[2];

  object &MakeObj(int idx, uint8_t type = OBJ_ROBOT, int id = 1) {
    object &o = Objects[idx];
    o = object{};
    o.type = type;
    o.id = id;
    o.handle = 700 + idx;
    o.roomnum = 42; // inside
    g_lookup[o.handle] = &o;
    Highest_object_index = idx > Highest_object_index ? idx : Highest_object_index;
    return o;
  }
};
bsp_info ObjLightingTest::s_subs[2];
polyface ObjLightingTest::s_faces[2];

/**
 * @test ObjLightingTest.InvisibleSetsFadeOutState
 * @brief Verifies invisible Sets Fade Out State.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, InvisibleSetsFadeOutState) {
  object &o = MakeObj(1);
  effect_info_s eff{};
  eff.fade_max_time = 9.9f;
  o.effect_info = &eff;

  MakeObjectInvisible(&o, 5.0f, 2.0f, true);

  EXPECT_TRUE(eff.type_flags & EF_FADING_OUT);
  EXPECT_FLOAT_EQ(eff.cloak_time, 5.0f);
  EXPECT_FLOAT_EQ(eff.fade_time, 2.0f);
  EXPECT_FLOAT_EQ(eff.fade_max_time, 2.0f); // overwritten by fade_time arg
}

/**
 * @test ObjLightingTest.InvisibleNonEffectObjectSafe
 * @brief Verifies invisible Non Effect Object Safe.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, InvisibleNonEffectObjectSafe) {
  object &o = MakeObj(1);
  MakeObjectInvisible(&o, 1.0f, 1.0f, false); // no effect_info: no crash
  SUCCEED();
}

/**
 * @test ObjLightingTest.InvisiblePlayerPlaysCloakOnAndHudMsg
 * @brief Verifies invisible Player Plays Cloak On And Hud Msg.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, InvisiblePlayerPlaysCloakOnAndHudMsg) {
  object &o = MakeObj(1, OBJ_PLAYER, 0); // id == Player_num
  effect_info_s eff{};
  o.effect_info = &eff;

  MakeObjectInvisible(&o, 4.0f, 1.0f, false);

  ASSERT_EQ(g_played_sounds.size(), 1u);
  EXPECT_EQ(g_played_sounds[0], 42); // resolved "Cloak on" id
  ASSERT_EQ(g_hud_msgs.size(), 1u);
  EXPECT_STREQ(g_hud_msgs[0].c_str(), "<txt642>"); // TXT_CLOAKON
  EXPECT_TRUE(eff.type_flags & EF_CLOAK_WITH_MSG);

  // other player's cloak: sound attempted but no local HUD message
  g_hud_msgs.clear();
  g_played_sounds.clear();
  object &o2 = MakeObj(2, OBJ_PLAYER, 1);
  o2.effect_info = &eff;
  MakeObjectInvisible(&o2, 4.0f, 1.0f, false);
  EXPECT_EQ(g_played_sounds.size(), 1u);
  EXPECT_TRUE(g_hud_msgs.empty());
}

/**
 * @test ObjLightingTest.CloakSoundNameLookupCachedStatically
 * @brief Verifies cloak Sound Name Lookup Cached Statically.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, CloakSoundNameLookupCachedStatically) {
  object &a = MakeObj(1, OBJ_PLAYER, 0);
  object &b = MakeObj(2, OBJ_PLAYER, 1);
  effect_info_s eff{};
  a.effect_info = &eff;
  b.effect_info = &eff;

  // Quirk: the sound name is resolved once per PROCESS via a function-local
  // static (-2 sentinel); subsequent cloaks skip the lookup entirely.
  object &warm = MakeObj(0, OBJ_PLAYER, 0);
  warm.effect_info = &eff;
  MakeObjectInvisible(&warm, 1, 1, false);
  int after_warm = g_sound_lookups;

  MakeObjectInvisible(&a, 1, 1, false);
  MakeObjectInvisible(&b, 1, 1, false);
  // cached from the first-ever cloak in this process: no further lookups
  EXPECT_EQ(g_sound_lookups, after_warm);
}

/**
 * @test ObjLightingTest.VisibleRestoresFadeInState
 * @brief Verifies visible Restores Fade In State.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, VisibleRestoresFadeInState) {
  object &o = MakeObj(1, OBJ_PLAYER, 0);
  effect_info_s eff{};
  eff.type_flags = EF_CLOAKED | EF_CLOAK_WITH_MSG;
  eff.fade_max_time = 7.0f;
  eff.fade_time = 0.0f;
  eff.cloak_time = 99.0f;
  o.effect_info = &eff;

  MakeObjectVisible(&o);

  EXPECT_FALSE(eff.type_flags & EF_CLOAKED);
  EXPECT_TRUE(eff.type_flags & EF_FADING_IN);
  EXPECT_FLOAT_EQ(eff.fade_time, 7.0f);
  EXPECT_FLOAT_EQ(eff.cloak_time, 0.0f);

  ASSERT_EQ(g_played_sounds.size(), 1u); // cloak-off sound
  ASSERT_EQ(g_hud_msgs.size(), 1u);
  EXPECT_STREQ(g_hud_msgs[0].c_str(), "<txt634>"); // TXT_MSG_CLOAKOFF

  // Quirk: EF_CLOAK_WITH_MSG is never cleared -> a second Visible call
  // replays the sound/message
  g_played_sounds.clear();
  g_hud_msgs.clear();
  MakeObjectVisible(&o);
  EXPECT_EQ(g_played_sounds.size(), 1u);
  EXPECT_EQ(g_hud_msgs.size(), 1u);
}

/**
 * @test ObjLightingTest.SetLocalLightingCopiesTypeInfoOnce
 * @brief Verifies set Local Lighting Copies Type Info Once.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, SetLocalLightingCopiesTypeInfoOnce) {
  object &o = MakeObj(1, OBJ_POWERUP, 3);
  Object_info[3].lighting_info.light_distance = 33.0f;

  ObjSetLocalLighting(&o);
  ASSERT_NE(o.lighting_info, nullptr);
  EXPECT_FLOAT_EQ(o.lighting_info->light_distance, 33.0f);

  light_info *first = o.lighting_info;
  Object_info[3].lighting_info.light_distance = 99.0f; // type info changes
  ObjSetLocalLighting(&o);                              // second call is a no-op
  EXPECT_EQ(o.lighting_info, first);                    // still the original copy
  EXPECT_FLOAT_EQ(o.lighting_info->light_distance, 33.0f);
}

/**
 * @test ObjLightingTest.GetLightInfoFallbackChain
 * @brief Verifies get Light Info Fallback Chain.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, GetLightInfoFallbackChain) {
  // own pointer wins
  object &own = MakeObj(1, OBJ_ROBOT, 2);
  light_info own_li{};
  own.lighting_info = &own_li;
  EXPECT_EQ(ObjGetLightInfo(&own), &own_li);

  // generic types fall back to their Object_info entry
  object &gen = MakeObj(2, OBJ_CLUTTER, 4);
  EXPECT_EQ(ObjGetLightInfo(&gen), &Object_info[4].lighting_info);

  // weapons fall back to the Weapons table
  object &wpn = MakeObj(3, OBJ_WEAPON, 5);
  EXPECT_EQ(ObjGetLightInfo(&wpn), &Weapons[5].lighting_info);

  // anything else has no lighting source
  object &odd = MakeObj(4, OBJ_PLAYER, 0);
  EXPECT_EQ(ObjGetLightInfo(&odd), nullptr);
}

/**
 * @test ObjLightingTest.ThrusterColorNeedsPolyobjAndGlowSubmodel
 * @brief Verifies thruster Color Needs Polyobj And Glow Submodel.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, ThrusterColorNeedsPolyobjAndGlowSubmodel) {
  float r = 1, g = 1, b = 1;
  Players[0].objnum = 1;

  object &o = MakeObj(1);
  o.render_type = RT_NONE; // not a polyobj: early-out zeros
  FindPlayerThrusterColor(0, &r, &g, &b);
  EXPECT_FLOAT_EQ(r, 0.f);
  EXPECT_FLOAT_EQ(g, 0.f);
  EXPECT_FLOAT_EQ(b, 0.f);

  glowinfo glow{};

  glow.glow_r = 0.25f;
  glow.glow_g = 0.5f;
  glow.glow_b = 0.75f;
  s_subs[1].flags |= SOF_GLOW;
  s_subs[1].glow_info = &glow;
  o.render_type = RT_POLYOBJ;
  o.rtype.pobj_info.model_num = 0;

  FindPlayerThrusterColor(0, &r, &g, &b);
  EXPECT_FLOAT_EQ(r, 0.25f);
  EXPECT_FLOAT_EQ(g, 0.5f);
  EXPECT_FLOAT_EQ(b, 0.75f);
}

/**
 * @test ObjLightingTest.LightmapMemoryRoundTrip
 * @brief Verifies lightmap Memory Round Trip.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, LightmapMemoryRoundTrip) {
  object &o = MakeObj(1);
  o.rtype.pobj_info.model_num = 0;

  SetupObjectLightmapMemory(&o);
  EXPECT_EQ(o.lm_object.used, 1);
  EXPECT_EQ(o.lm_object.num_models, 2);

  // submodel 0 skipped (no faces), submodel 1 gets face storage
  EXPECT_EQ(o.lm_object.num_faces[0], 0);
  EXPECT_EQ(o.lm_object.lightmap_faces[0], nullptr);
  ASSERT_EQ(o.lm_object.num_faces[1], 2);
  ASSERT_NE(o.lm_object.lightmap_faces[1], nullptr);

  auto *lf = o.lm_object.lightmap_faces[1];
  EXPECT_EQ(lf[0].num_verts, 4);
  EXPECT_EQ(lf[1].num_verts, 3);
  EXPECT_EQ(lf[0].lmi_handle, BAD_LMI_INDEX);

  // u2/v2 pointers walk one shared uv block in face order:
  // face0 [u2 x4][v2 x4], face1 [u2 x3][v2 x3]
  EXPECT_EQ(lf[0].v2, lf[0].u2 + lf[0].num_verts);
  EXPECT_EQ(lf[1].u2, lf[0].v2 + lf[0].num_verts);
  EXPECT_EQ(lf[1].v2, lf[1].u2 + lf[1].num_verts);

  ClearObjectLightmaps(&o);
  EXPECT_EQ(o.lm_object.used, 0);
  EXPECT_EQ(o.lm_object.lightmap_faces[1], nullptr);
  EXPECT_EQ(o.lm_object.num_faces[1], 0);
  EXPECT_TRUE(g_freed_lmi.empty()); // no lmi handles were assigned
}

/**
 * @test ObjLightingTest.ClearLightmapsFreesAssignedHandles
 * @brief Verifies clear Lightmaps Frees Assigned Handles.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, ClearLightmapsFreesAssignedHandles) {
  object &o = MakeObj(1);
  o.rtype.pobj_info.model_num = 0;
  SetupObjectLightmapMemory(&o);

  auto *lf = o.lm_object.lightmap_faces[1];
  lf[0].lmi_handle = 77;
  lf[1].lmi_handle = BAD_LMI_INDEX; // unassigned faces are skipped

  ClearObjectLightmaps(&o);

  ASSERT_EQ(g_freed_lmi.size(), 1u);
  EXPECT_EQ(g_freed_lmi[0], 77);
}

/**
 * @test ObjLightingTest.ClearUnusedLightmapsIsNoop
 * @brief Verifies clear Unused Lightmaps Is Noop.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, ClearUnusedLightmapsIsNoop) {
  object &o = MakeObj(1);
  ClearObjectLightmaps(&o); // lm_object.used == 0
  SUCCEED();
}

/**
 * @test ObjLightingTest.ClearAllFiltersByTerrainSide
 * @brief Verifies clear All Filters By Terrain Side.
 *
 * @details
 * Exercises the ObjLightingTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_lighting.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjLightingTest, ClearAllFiltersByTerrainSide) {
  object &inside = MakeObj(1); // roomnum 42 -> inside
  inside.rtype.pobj_info.model_num = 0;
  SetupObjectLightmapMemory(&inside);

  object &outside = MakeObj(2);
  outside.roomnum = MAKE_ROOMNUM(7); // outside terrain cell
  outside.rtype.pobj_info.model_num = 0;
  SetupObjectLightmapMemory(&outside);

  ClearAllObjectLightmaps(1); // terrain pass: clears only outside objects
  EXPECT_EQ(outside.lm_object.used, 0);
  EXPECT_EQ(inside.lm_object.used, 1);

  ClearAllObjectLightmaps(0); // mine pass: clears the rest
  EXPECT_EQ(inside.lm_object.used, 0);
}
