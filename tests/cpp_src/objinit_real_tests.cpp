/**
 * @file objinit_real_tests.cpp
 * @brief Tests for ObjInit.cpp 1209 lines — object construction defaults.
 *
 * @details
 * The REAL source is compiled in; the polymodel page-in system, control
 * type plumbing, weapon battery clearing, and the ship/weapon/door
 * tables are stubbed. Exercises ObjInit's zeroing contract, the
 * per-type dispatch, debris/shard/weapon/player/generic initializers,
 * and the ObjReInitAll walk.
 *
 * This harness validates the behavior of `Descent3/ObjInit.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/ObjInit.cpp`
 * @par Harness
 * `objinit_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/ObjInit.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "object.h"
#include "objinit.h"
#include "objinfo.h"
#include "ship.h"
#include "player.h"
#include "weapon.h"
#include "fireball.h"
#include "door.h"
#include "marker.h"
#include "vclip.h"
#include "robotfire.h"
#include "AIMain.h"
#include "polymodel.h"
#include "sounds.h"
#include "physics.h"

// public in ObjInit.cpp but missing from objinit.h
int ObjInitTypeSpecific(object *objp, bool reinitializing);
int ObjInitPlayer(object *objp);
int ObjInitGeneric(object *objp, bool reinit);
int ObjInitDebris(object *objp);
int ObjInitShard(object *objp);
int ObjInitWeapon(object *objp);
void ObjCreateEffectInfo(object *objp);

// pulled in via player.h; not exercised here
Inventory::Inventory() {}
Inventory::~Inventory() {}

// ---- world tables (definitions normally living elsewhere) ----
object Objects[MAX_OBJECTS];
int Highest_object_index = -1;
ship Ships[MAX_SHIPS];
player Players[MAX_PLAYERS];
int Player_num = 0;
object *Player_object = nullptr;
weapon Weapons[MAX_WEAPONS];
fireball Fireballs[NUM_FIREBALLS];
int Marker_polynum = 11;
door Doors[MAX_DOORS];
object_info Object_info[MAX_OBJECT_IDS];
poly_model Poly_models[MAX_POLY_MODELS];

// ---- stubs ----
static int g_last_control_type = -1;
static int g_last_paged_model = -1;
static int g_last_paged_type = -1;
static bool g_vclip_paged = false;
static bool g_ai_inited = false;

void SetObjectControlType(object *obj, int control_type) {
  obj->control_type = control_type;
  g_last_control_type = control_type;
}
void PageInPolymodel(int polynum, int type, float *size_ptr) {
  g_last_paged_model = polynum;
  g_last_paged_type = type;
}
void PageInVClip(int vcnum) { g_vclip_paged = true; }
void WBClearInfo(object *obj) {}
bool AIInit(object *obj, uint8_t ai_class, uint8_t ai_type, uint8_t ai_movement) {
  g_ai_inited = true;
  return true;
}
float ComputeDefaultSize(int type, int handle, float *size_ptr) {
  *size_ptr = 42.0f;
  return 42.0f;
}
int GetDoorImage(int handle) { return handle + 500; }
int GetNextShip(int n) { return 2; } // caller marks it used
poly_model *GetPolymodelPointer(int polynum) { return &Poly_models[polynum]; }

// public in ObjInit.cpp but missing from objinit.h
int ObjInitTypeSpecific(object *objp, bool reinitializing);
int ObjInitPlayer(object *objp);
int ObjInitGeneric(object *objp, bool reinit);
int ObjInitDebris(object *objp);
int ObjInitShard(object *objp);
int ObjInitWeapon(object *objp);

static void ResetCounters() {
  g_last_control_type = -1;
  g_last_paged_model = -1;
  g_last_paged_type = -1;
  g_vclip_paged = false;
  g_ai_inited = false;
}

/**
 * @test ObjInit.ZeroesStructThenWiresPassedInValues
 * @brief Verifies zeroes Struct Then Wires Passed In Values.
 *
 * @details
 * Exercises the ObjInit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjInit.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInit, ZeroesStructThenWiresPassedInValues) {
  object obj;
  memset(&obj, 0xAA, sizeof(obj)); // poison every byte

  vector pos = {3.0f, 4.0f, 5.0f};
  ResetCounters();
  ASSERT_EQ(ObjInit(&obj, OBJ_VIEWER, 6, 777, &pos, 12.5f, 42), 1);

  // everything passed in lands where it should
  EXPECT_EQ(obj.type, OBJ_VIEWER);
  EXPECT_EQ(obj.id, 6);
  EXPECT_EQ(obj.handle, 777);
  EXPECT_FLOAT_EQ(pos.x(), obj.pos.x());
  EXPECT_FLOAT_EQ(pos.z(), obj.last_pos.z());
  EXPECT_EQ(obj.parent_handle, 42);
  EXPECT_FLOAT_EQ(obj.creation_time, 12.5f);

  // defaults applied on top of the wipe
  EXPECT_EQ(obj.roomnum, -1);
  EXPECT_EQ(obj.next, -1);
  EXPECT_EQ(obj.prev, -1);
  EXPECT_EQ(obj.dummy_type, OBJ_NONE);
  EXPECT_EQ(obj.flags, 0);
  EXPECT_EQ(obj.contains_type, -1);
  EXPECT_EQ(obj.name, nullptr);
  EXPECT_EQ(obj.osiris_script, nullptr);
  EXPECT_EQ(obj.custom_default_script_name, nullptr);
  EXPECT_EQ(obj.effect_info, nullptr);
  EXPECT_EQ(obj.ai_info, nullptr);
  EXPECT_EQ(obj.dynamic_wb, nullptr);
  EXPECT_FLOAT_EQ(obj.orient.rvec.x(), 1.0f); // Identity_matrix
  EXPECT_FLOAT_EQ(obj.orient.uvec.y(), 1.0f);
  EXPECT_FLOAT_EQ(obj.orient.fvec.z(), 1.0f);

  // viewer-specific block
  EXPECT_FLOAT_EQ(obj.size, 5.0f);
  EXPECT_EQ(obj.shields, 0);
  EXPECT_EQ(g_last_control_type, CT_NONE);
  EXPECT_EQ(obj.movement_type, MT_NONE);
  EXPECT_EQ(obj.render_type, RT_NONE);
}

/**
 * @test ObjInit.TypeSpecificDispatchCoversLeafTypes
 * @brief Verifies type Specific Dispatch Covers Leaf Types.
 *
 * @details
 * Exercises the ObjInit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjInit.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInit, TypeSpecificDispatchCoversLeafTypes) {
  ResetCounters();

  // camera / waypoint / sound source share the 0.5-size template
  object cam;
  memset(&cam, 0, sizeof(cam));
  cam.type = OBJ_CAMERA;
  ASSERT_EQ(ObjInitTypeSpecific(&cam, false), 1);
  EXPECT_FLOAT_EQ(cam.size, 0.5f);
  EXPECT_EQ(cam.shields, 0);
  EXPECT_EQ(cam.render_type, RT_NONE);

  object wp;
  memset(&wp, 0, sizeof(wp));
  wp.type = OBJ_WAYPOINT;
  ASSERT_EQ(ObjInitTypeSpecific(&wp, false), 1);
  EXPECT_FLOAT_EQ(wp.size, 0.5f);

  object snd;
  memset(&snd, 0, sizeof(snd));
  snd.type = OBJ_SOUNDSOURCE;
  ASSERT_EQ(ObjInitTypeSpecific(&snd, false), 1);
  EXPECT_EQ(g_last_control_type, CT_SOUNDSOURCE);
  EXPECT_FLOAT_EQ(snd.size, 0.5f);

  // marker uses the shared marker polymodel and beefy stats
  object mk;
  memset(&mk, 0, sizeof(mk));
  mk.type = OBJ_MARKER;
  Marker_polynum = 13;
  ASSERT_EQ(ObjInitTypeSpecific(&mk, false), 1);
  EXPECT_EQ(mk.shields, 100);
  EXPECT_FLOAT_EQ(mk.size, 2.0f);
  EXPECT_EQ(mk.render_type, RT_POLYOBJ);
  EXPECT_EQ(mk.rtype.pobj_info.model_num, 13);
  EXPECT_EQ(g_last_paged_model, 13);

  // room renders as a special case but still flags as polygon object
  object rm;
  memset(&rm, 0, sizeof(rm));
  rm.type = OBJ_ROOM;
  ASSERT_EQ(ObjInitTypeSpecific(&rm, false), 1);
  EXPECT_EQ(rm.render_type, RT_ROOM);
  EXPECT_TRUE(rm.flags & OF_POLYGON_OBJECT);

  // shockwave and splinter are accepted no-ops: fields untouched
  object sw;
  memset(&sw, 0, sizeof(sw));
  sw.type = OBJ_SHOCKWAVE;
  sw.size = 9.0f;
  sw.shields = 9.0f;
  ASSERT_EQ(ObjInitTypeSpecific(&sw, false), 1);
  EXPECT_FLOAT_EQ(sw.size, 9.0f);
  EXPECT_FLOAT_EQ(sw.shields, 9.0f);

  object spl;
  memset(&spl, 0, sizeof(spl));
  spl.type = OBJ_SPLINTER;
  ASSERT_EQ(ObjInitTypeSpecific(&spl, true), 1);

  // anything else fails
  object bad;
  bad.type = 200;
  EXPECT_EQ(ObjInitTypeSpecific(&bad, false), 0);
}

/**
 * @test ObjInit.DebrisAndShardGetPhysicsLifeleftAndEffectMemory
 * @brief Verifies debris And Shard Get Physics Lifeleft And Effect Memory.
 *
 * @details
 * Exercises the ObjInit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjInit.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInit, DebrisAndShardGetPhysicsLifeleftAndEffectMemory) {
  ResetCounters();

  object deb;
  memset(&deb, 0, sizeof(deb));
  deb.type = OBJ_DEBRIS;
  ASSERT_EQ(ObjInitDebris(&deb), 1);
  EXPECT_EQ(deb.movement_type, MT_PHYSICS);
  EXPECT_EQ(g_last_control_type, CT_DEBRIS);
  EXPECT_FLOAT_EQ(deb.lifeleft, DEBRIS_LIFE);
  EXPECT_TRUE(deb.flags & OF_USES_LIFELEFT);
  EXPECT_EQ(deb.ctype.debris_info.death_flags, 0);
  EXPECT_EQ(deb.lighting_render_type, LRT_GOURAUD);
  // render shell points at no model yet
  EXPECT_EQ(deb.render_type, RT_POLYOBJ);
  EXPECT_EQ(deb.rtype.pobj_info.model_num, -1);
  // effect memory allocated fresh with sentinel sound + volume lit
  ASSERT_NE(deb.effect_info, nullptr);
  EXPECT_EQ(deb.effect_info->sound_handle, SOUND_NONE_INDEX);
  EXPECT_EQ(deb.effect_info->type_flags, EF_VOLUME_LIT);

  object shard;
  memset(&shard, 0, sizeof(shard));
  shard.type = OBJ_SHARD;
  ASSERT_EQ(ObjInitShard(&shard), 1);
  EXPECT_FLOAT_EQ(shard.lifeleft, DEBRIS_LIFE * 5);
  EXPECT_EQ(shard.mtype.phys_info.flags, PF_GRAVITY | PF_BOUNCE);
  EXPECT_FLOAT_EQ(shard.mtype.phys_info.mass, 1.0f);
  EXPECT_FLOAT_EQ(shard.mtype.phys_info.drag, 0.0001f);
  EXPECT_FLOAT_EQ(shard.mtype.phys_info.coeff_restitution, 0.3f);
  EXPECT_EQ(shard.mtype.phys_info.num_bounces, 0);
  EXPECT_EQ(shard.render_type, RT_SHARD);

  // reallocating effect info reseeds the sentinel (the allocator may
  // hand back the same block, so watch contents not the pointer)
  effect_info_s *old = deb.effect_info;
  old->sound_handle = 999;
  ObjCreateEffectInfo(&deb);
  EXPECT_EQ(deb.effect_info->sound_handle, SOUND_NONE_INDEX);
}

/**
 * @test ObjInit.WeaponInitValidatesIdCopiesPhysioAndRenderMode
 * @brief Verifies weapon Init Validates Id Copies Physio And Render Mode.
 *
 * @details
 * Exercises the ObjInit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjInit.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInit, WeaponInitValidatesIdCopiesPhysioAndRenderMode) {
  ResetCounters();
  weapon *w = &Weapons[3];
  memset(w, 0, sizeof(*w));
  w->used = 1;
  w->phys_info.mass = 7.0f;
  w->phys_info.drag = 0.2f;
  w->phys_info.num_bounces = 4;
  w->life_time = 9.5f;
  w->thrust_time = 2;
  w->impact_size = 1.5f;
  w->impact_player_damage = 30;
  w->impact_generic_damage = 20;
  w->impact_force = 8.0f;
  w->size = 3.25f;
  w->fire_image_handle = 17;

  object obj;
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_WEAPON;
  obj.id = 3;

  // polymodel-fired weapon
  ASSERT_EQ(ObjInitWeapon(&obj), 1);
  EXPECT_FLOAT_EQ(obj.mtype.phys_info.mass, 7.0f);
  EXPECT_EQ(obj.mtype.phys_info.num_bounces, 4);
  EXPECT_FLOAT_EQ(obj.lifeleft, 9.5f);
  EXPECT_FLOAT_EQ(obj.lifetime, 9.5f);
  EXPECT_EQ(obj.ctype.laser_info.track_handle, OBJECT_HANDLE_NONE);
  EXPECT_EQ(obj.ctype.laser_info.thrust_left, 2);
  EXPECT_FLOAT_EQ(obj.impact_size, 1.5f);
  EXPECT_EQ(obj.impact_player_damage, 30);
  EXPECT_EQ(obj.impact_generic_damage, 20);
  EXPECT_TRUE(obj.mtype.phys_info.flags & PF_NO_COLLIDE_PARENT);
  EXPECT_TRUE(obj.flags & OF_USES_LIFELEFT);
  EXPECT_EQ(obj.render_type, RT_WEAPON);
  EXPECT_EQ(obj.rtype.pobj_info.model_num, 17);
  EXPECT_EQ(obj.rtype.pobj_info.dying_model_num, -1);
  EXPECT_TRUE(obj.flags & OF_POLYGON_OBJECT);
  EXPECT_FALSE(g_vclip_paged);
  EXPECT_FLOAT_EQ(obj.size, 3.25f);

  // custom size wins over weapon size
  w->flags |= WF_CUSTOM_SIZE;
  w->custom_size = 6.75f;
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_WEAPON;
  obj.id = 3;
  ASSERT_EQ(ObjInitWeapon(&obj), 1);
  EXPECT_FLOAT_EQ(obj.size, 6.75f);
  w->flags &= ~WF_CUSTOM_SIZE;

  // vclip weapons page a vclip instead of touching polyobj info
  w->flags |= WF_IMAGE_VCLIP;
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_WEAPON;
  obj.id = 3;
  ASSERT_EQ(ObjInitWeapon(&obj), 1);
  EXPECT_TRUE(g_vclip_paged);
  EXPECT_EQ(obj.rtype.pobj_info.model_num, 0); // untouched by this branch
  EXPECT_FALSE(obj.flags & OF_POLYGON_OBJECT);
  w->flags &= ~WF_IMAGE_VCLIP;

  // invalid ids rejected without side effects
  memset(&obj, 0xFF, sizeof(obj));
  obj.type = OBJ_WEAPON;
  obj.id = -1;
  EXPECT_EQ(ObjInitWeapon(&obj), 0);
  obj.id = MAX_WEAPONS;
  EXPECT_EQ(ObjInitWeapon(&obj), 0);
}

/**
 * @test ObjInit.GenericAppliesTableEntryAndClampsRotdrag
 * @brief Verifies generic Applies Table Entry And Clamps Rotdrag.
 *
 * @details
 * Exercises the ObjInit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjInit.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInit, GenericAppliesTableEntryAndClampsRotdrag) {
  ResetCounters();
  object_info *info = &Object_info[5];
  memset(info, 0, sizeof(*info));
  info->type = OBJ_ROBOT;
  info->hit_points = 80.0f;
  info->impact_size = 2.0f;
  info->damage = 10.0f;
  info->render_handle = 21;
  info->size = 4.0f;
  info->flags = OIF_DESTROYABLE | OIF_AI_SCRIPTED_DEATH | OIF_DO_CEILING_CHECK | OIF_USES_PHYSICS | OIF_CONTROL_AI;
  info->phys_info.velocity = {0.0f, 1.5f, 0.0f};
  info->phys_info.rotdrag = 10.0f;
  static t_ai_info ai_storage = {};
  info->ai_info = &ai_storage;

  Poly_models[21].num_wbs = 2;

  object obj;
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_ROBOT;
  obj.id = 5;

  ASSERT_EQ(ObjInitGeneric(&obj, false), 1);
  EXPECT_FLOAT_EQ(obj.shields, 80.0f);
  EXPECT_FLOAT_EQ(obj.impact_size, 2.0f);
  EXPECT_FLOAT_EQ(obj.impact_force, 500.0f); // damage * 50
  EXPECT_TRUE(obj.flags & OF_DESTROYABLE);
  EXPECT_TRUE(obj.flags & OF_AI_DO_DEATH);
  EXPECT_TRUE(obj.flags & OF_FORCE_CEILING_CHECK);
  EXPECT_EQ(obj.movement_type, MT_PHYSICS);
  EXPECT_FLOAT_EQ(obj.mtype.phys_info.velocity.y(), 1.5f);
  EXPECT_FLOAT_EQ(obj.mtype.phys_info.rotdrag, 60.0f); // clamped up
  EXPECT_EQ(obj.render_type, RT_POLYOBJ);
  EXPECT_EQ(obj.rtype.pobj_info.model_num, 21);
  EXPECT_FLOAT_EQ(obj.rtype.pobj_info.anim_frame, 0.0f); // no anim table
  EXPECT_FLOAT_EQ(obj.size, 4.0f);
  EXPECT_TRUE(g_ai_inited);
  EXPECT_EQ(g_last_control_type, CT_AI);
  ASSERT_NE(obj.dynamic_wb, nullptr); // num_wbs > 0 allocates battery slots
  EXPECT_NE(obj.effect_info, nullptr);
  EXPECT_EQ(obj.effect_info->type_flags, EF_VOLUME_LIT); // robots get lit

  // physics-less entry falls back to MT_NONE + CT_NONE
  info->flags &= ~(OIF_USES_PHYSICS | OIF_CONTROL_AI);
  info->ai_info = nullptr;
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_ROBOT;
  obj.id = 5;
  ASSERT_EQ(ObjInitGeneric(&obj, false), 1);
  EXPECT_EQ(obj.movement_type, MT_NONE);
  EXPECT_EQ(g_last_control_type, CT_NONE);
  EXPECT_EQ(obj.dynamic_wb, nullptr); // not AI -> never allocated

  // powerups keep their own control type and pull ammo count
  info->type = OBJ_POWERUP;
  info->ammo_count = 4;
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_POWERUP;
  obj.id = 5;
  ASSERT_EQ(ObjInitGeneric(&obj, false), 1);
  EXPECT_EQ(g_last_control_type, CT_POWERUP);
  EXPECT_EQ(obj.ctype.powerup_info.count, 4);
  EXPECT_EQ(obj.effect_info->type_flags, 0); // powerups stay unlit

  // table/type mismatch corrects the live object's type
  info->type = OBJ_CLUTTER;
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_ROBOT;
  obj.id = 5;
  ASSERT_EQ(ObjInitGeneric(&obj, false), 1);
  EXPECT_EQ(obj.type, OBJ_CLUTTER);

  // out-of-range ids bail immediately
  memset(&obj, 0, sizeof(obj));
  obj.type = OBJ_ROBOT;
  obj.id = MAX_OBJECT_IDS;
  EXPECT_EQ(ObjInitGeneric(&obj, false), 0);
  obj.id = -1;
  EXPECT_EQ(ObjInitGeneric(&obj, false), 0);
}

/**
 * @test ObjInit.PlayerInitBranchesOnLocalRemoteAndRecoversFromDeadShip
 * @brief Verifies player Init Branches On Local Remote And Recovers From Dead Ship.
 *
 * @details
 * Exercises the ObjInit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjInit.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInit, PlayerInitBranchesOnLocalRemoteAndRecoversFromDeadShip) {
  ResetCounters();
  Player_num = 4;

  // healthy ship in slot 1
  ship *s = &Ships[1];
  memset(s, 0, sizeof(*s));
  s->used = true;
  s->model_handle = 7;
  s->dying_model_handle = -1;
  s->lo_render_handle = -1;
  s->med_render_handle = -1;
  s->size = 12.5f;
  s->phys_info.mass = 3.0f;
  Players[4].ship_index = 1;
  Players[5].ship_index = 1;

  object local_p;
  memset(&local_p, 0, sizeof(local_p));
  local_p.type = OBJ_PLAYER;
  local_p.id = 4;
  ASSERT_EQ(ObjInitPlayer(&local_p), 1);
  EXPECT_EQ(Player_object, &local_p); // locals claim the global
  EXPECT_EQ(g_last_control_type, CT_FLYING);
  EXPECT_EQ(local_p.movement_type, MT_PHYSICS);
  EXPECT_FLOAT_EQ(local_p.mtype.phys_info.mass, 3.0f);
  EXPECT_EQ(local_p.mtype.phys_info.num_bounces, PHYSICS_UNLIMITED_BOUNCE);
  EXPECT_FLOAT_EQ(local_p.size, 12.5f);
  EXPECT_EQ(local_p.render_type, RT_POLYOBJ);
  EXPECT_EQ(local_p.rtype.pobj_info.model_num, 7);
  ASSERT_NE(local_p.dynamic_wb, nullptr);
  EXPECT_EQ(Players[4].team, 0);
  EXPECT_EQ(local_p.effect_info->type_flags, EF_VOLUME_LIT);

  // remotes fly nothing and drift on rails
  object remote_p;
  memset(&remote_p, 0, sizeof(remote_p));
  remote_p.type = OBJ_PLAYER;
  remote_p.id = 5;
  ASSERT_EQ(ObjInitPlayer(&remote_p), 1);
  EXPECT_NE(Player_object, &remote_p);
  EXPECT_EQ(remote_p.control_type, CT_NONE);
  EXPECT_EQ(remote_p.movement_type, MT_NONE);
  EXPECT_TRUE(remote_p.mtype.phys_info.flags & PF_FIXED_VELOCITY);
  EXPECT_EQ(Players[5].team, 1);

  // dead ship forces a fallback slot and reports failure
  Ships[1].used = false;
  Players[6].ship_index = 1;
  object rescue;
  memset(&rescue, 0, sizeof(rescue));
  rescue.type = OBJ_PLAYER;
  rescue.id = 6;
  EXPECT_EQ(ObjInitPlayer(&rescue), 0);
  EXPECT_EQ(Players[6].ship_index, 2); // GetNextShip's pick
}

/**
 * @test ObjInit.ReInitAllWalksOnlyLiveObjectsAndPreservesLightmapsOnDoors
 * @brief Verifies re Init All Walks Only Live Objects And Preserves Lightmaps On Doors.
 *
 * @details
 * Exercises the ObjInit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjInit.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInit, ReInitAllWalksOnlyLiveObjectsAndPreservesLightmapsOnDoors) {
  ResetCounters();

  Doors[0].flags = DF_BLASTABLE;
  Doors[0].hit_points = 55.0f;

  object *door_obj = &Objects[1];
  memset(door_obj, 0, sizeof(*door_obj));
  door_obj->type = OBJ_DOOR;
  door_obj->id = 0;
  door_obj->lm_object.used = 7; // must survive a REINIT pass

  Highest_object_index = 1;
  memset(&Objects[0], 0, sizeof(Objects[0]));
  Objects[0].type = OBJ_NONE; // skipped

  ObjReInitAll();

  EXPECT_EQ(door_obj->movement_type, MT_NONE);
  EXPECT_EQ(door_obj->control_type, CT_NONE);
  EXPECT_EQ(door_obj->lighting_render_type, LRT_LIGHTMAPS);
  EXPECT_EQ(door_obj->rtype.pobj_info.model_num, 500); // GetDoorImage(0)+500
  EXPECT_TRUE(door_obj->flags & OF_DESTROYABLE);       // blastable doors die
  EXPECT_FLOAT_EQ(door_obj->shields, 55.0f);
  EXPECT_EQ(door_obj->lm_object.used, 7); // reinit keeps lightmap slot

  // dead object untouched: its bytes still hold the NONE sentinel pattern
  EXPECT_EQ(Objects[0].type, OBJ_NONE);
}
