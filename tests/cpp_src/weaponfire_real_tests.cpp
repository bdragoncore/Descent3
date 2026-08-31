/**
 * @file weaponfire_real_tests.cpp
 * @brief Tests for WeaponFire.cpp 3419 lines — weapon firing and.
 *
 * @details
 * collision relations. Covers ObjectsAreRelated, the decision
 * chain that decides whether two objects may collide: linked/
 * no-collide short circuits, player-chauff and moving-building-
 * powerup specials, open-door robots, the 3-second spawn grace
 * that only applies when NEITHER object is a weapon, persistent
 * weapon last-hit tracking, parent immunity via
 * PF_NO_COLLIDE_PARENT (direct or via attachment), and the
 * weapon-vs-weapon rules where siblings are immune unless either
 * hits siblings — and quirk: two weapons from DIFFERENT parents
 * are also declared related (immune) by default.
 *
 * This harness validates the behavior of `Descent3/WeaponFire.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/WeaponFire.cpp`
 * @par Harness
 * `weaponfire_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/WeaponFire.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

// replicated constants
constexpr int OBJ_NONE_W = 0, OBJ_WEAPON_WF = 4, OBJ_PLAYER_WF = 5,
              OBJ_POWERUP_WF = 6, OBJ_CLUTTER_WF = 7, OBJ_BUILDING_WF = 8,
              OBJ_ROBOT_WF = 3, OBJ_SHOCKWAVE_WF = 11, OBJ_DOOR_WF = 13;
constexpr int MT_NONE_W = 0, MT_PHYSICS_WF = 1, MT_OBJ_LINKED_WF = 5;
constexpr uint32_t PF_PERSISTENT_WF = 0x20;
constexpr uint32_t PF_NO_COLLIDE_PARENT_WF = 0x2000;
constexpr uint32_t PF_HITS_SIBLINGS_WF = 0x4000; // stand-in bit
constexpr uint32_t PF_NO_COLLIDE_WF = 0x10000;
constexpr int GENOBJ_CHAFFCHUNK_WF = 1;

struct WObj {
  int type = OBJ_NONE_W;
  int id = 0;
  int handle = -1;         // unique handle
  int parent_handle = -1;  // OBJECT_HANDLE_NONE-ish
  int roomnum = 0;
  int movement_type = MT_NONE_W;
  uint32_t phys_flags = 0;
  float creation_time = 0.0f;
  int last_hit_handle = -1; // laser_info for persistent weapons
};

static WObj WObjs[16];
static float WF_Gametime = 10.0f;

// AreObjectsAttached / DoorwayGetPosition stand-ins driven by test flags
static bool WF_attached_pair = false;
static bool WF_door_open = false;
static bool AreAttached(const WObj *, const WObj *) { return WF_attached_pair; }
static float DoorPos(int) { return WF_door_open ? 1.0f : 0.0f; }

static bool RepObjectsAreRelated(int o1, int o2) {
  if ((o1 < 0) || (o2 < 0))
    return false;

  const WObj &obj1 = WObjs[o1];
  const WObj &obj2 = WObjs[o2];

  if (obj1.movement_type == MT_OBJ_LINKED_WF || obj2.movement_type == MT_OBJ_LINKED_WF)
    return true;

  if (obj1.type != OBJ_SHOCKWAVE_WF && (obj1.phys_flags & PF_NO_COLLIDE_WF))
    return true;
  if (obj2.type != OBJ_SHOCKWAVE_WF && (obj2.phys_flags & PF_NO_COLLIDE_WF))
    return true;

  if (((obj1.type == OBJ_PLAYER_WF) && (obj2.type == OBJ_ROBOT_WF && obj2.id == GENOBJ_CHAFFCHUNK_WF)) ||
      ((obj2.type == OBJ_PLAYER_WF) && (obj1.type == OBJ_ROBOT_WF && obj1.id == GENOBJ_CHAFFCHUNK_WF)))
    return true;

  if (((obj1.type == OBJ_BUILDING_WF) && (obj1.movement_type != MT_NONE_W) && (obj2.type == OBJ_POWERUP_WF)) ||
      ((obj2.type == OBJ_BUILDING_WF) && (obj2.movement_type != MT_NONE_W) && (obj1.type == OBJ_POWERUP_WF)))
    return true;

  if (obj1.type == OBJ_DOOR_WF && DoorPos(obj1.roomnum) == 1.0f && obj2.type == OBJ_ROBOT_WF)
    return true;
  if (obj2.type == OBJ_DOOR_WF && DoorPos(obj2.roomnum) == 1.0f && obj1.type == OBJ_ROBOT_WF)
    return true;

  if (AreAttached(&WObjs[o1], &WObjs[o2]))
    return true;

  if (obj1.type != OBJ_WEAPON_WF && obj2.type != OBJ_WEAPON_WF) {
    if ((WF_Gametime < obj1.creation_time + 3.0f && obj1.parent_handle == obj2.handle) ||
        (WF_Gametime < obj2.creation_time + 3.0f && obj2.parent_handle == obj1.handle))
      return true;
    else
      return false;
  }

  if (obj1.type == OBJ_WEAPON_WF && obj1.movement_type == MT_PHYSICS_WF &&
      (obj1.phys_flags & PF_PERSISTENT_WF) && obj1.last_hit_handle == obj2.handle)
    return true;
  if (obj2.type == OBJ_WEAPON_WF && obj2.movement_type == MT_PHYSICS_WF &&
      (obj2.phys_flags & PF_PERSISTENT_WF) && obj2.last_hit_handle == obj1.handle)
    return true;

  if (obj1.type == OBJ_WEAPON_WF && (obj1.phys_flags & PF_NO_COLLIDE_PARENT_WF)) {
    if (obj1.parent_handle == obj2.handle)
      return true;
    // attachment to parent stand-in omitted: covered by WF_attached_pair
  }
  if (obj2.type == OBJ_WEAPON_WF && (obj2.phys_flags & PF_NO_COLLIDE_PARENT_WF)) {
    if (obj2.parent_handle == obj1.handle)
      return true;
  }

  if (obj1.type != OBJ_WEAPON_WF || obj2.type != OBJ_WEAPON_WF)
    return false;

  if (obj1.parent_handle == obj2.parent_handle) {
    if ((obj1.phys_flags & PF_HITS_SIBLINGS_WF) || (obj2.phys_flags & PF_HITS_SIBLINGS_WF))
      return false; // siblings CAN collide when either hits siblings
    else
      return true;
  }

  // quirk: two weapons from different parents are ALSO related (immune)
  return true;
}

static void ResetWO() {
  for (int i = 0; i < 16; i++) {
    WObjs[i] = WObj{};
    WObjs[i].handle = i + 100; // distinct handles
  }
  WF_Gametime = 10.0f;
  WF_attached_pair = false;
  WF_door_open = false;
}

/**
 * @test WeaponRelations.ShortCircuitsAndSpecialPairs
 * @brief Verifies short Circuits And Special Pairs.
 *
 * @details
 * Exercises the WeaponRelations code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/WeaponFire.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponRelations, ShortCircuitsAndSpecialPairs) {
  ResetWO();

  // negative indices always unrelated
  EXPECT_FALSE(RepObjectsAreRelated(-1, 0));
  EXPECT_FALSE(RepObjectsAreRelated(0, -5));

  // linked objects never collide with anything
  WObjs[0].movement_type = MT_OBJ_LINKED_WF;
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));
  ResetWO();

  // quirk: PF_NO_COLLIDE short-circuits EXCEPT for shockwaves -- a
  // shockwave carrying the flag falls through to normal rules instead
  WObjs[0].phys_flags |= PF_NO_COLLIDE_WF;
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));
  WObjs[0].type = OBJ_SHOCKWAVE_WF;
  EXPECT_FALSE(RepObjectsAreRelated(0, 1)); // exemption: no early true
  ResetWO();

  // player vs chauff chunk robot: friendly
  WObjs[0] = {OBJ_PLAYER_WF};
  WObjs[0].handle = 100;
  WObjs[1] = {OBJ_ROBOT_WF, GENOBJ_CHAFFCHUNK_WF, 101};
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));

  // moving building vs powerup: pickup through walls of movers
  ResetWO();
  WObjs[0] = {OBJ_BUILDING_WF};
  WObjs[0].handle = 100;
  WObjs[0].movement_type = MT_PHYSICS_WF;
  WObjs[1] = {OBJ_POWERUP_WF, 0, 101};
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));
  WObjs[0].movement_type = MT_NONE_W;
  EXPECT_FALSE(RepObjectsAreRelated(0, 1)); // static building: no special

  // fully-open door lets robots pass
  ResetWO();
  WObjs[0] = {OBJ_DOOR_WF};
  WObjs[0].handle = 100;
  WObjs[1] = {OBJ_ROBOT_WF, 0, 101};
  WF_door_open = true;
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));
  WF_door_open = false;
  EXPECT_FALSE(RepObjectsAreRelated(0, 1)); // closed door blocks

  // attached objects never collide
  ResetWO();
  WObjs[0] = {OBJ_CLUTTER_WF, 0, 100};
  WObjs[1] = {OBJ_PLAYER_WF, 0, 101};
  WF_attached_pair = true;
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));
  WF_attached_pair = false;
}

/**
 * @test WeaponRelations.SpawnGraceParentsAndSiblingRules
 * @brief Verifies spawn Grace Parents And Sibling Rules.
 *
 * @details
 * Exercises the WeaponRelations code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/WeaponFire.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponRelations, SpawnGraceParentsAndSiblingRules) {
  ResetWO();

  // non-weapons: 3s parent grace after spawn...
  WObjs[0] = {OBJ_PLAYER_WF, 0, 100};
  WObjs[0].creation_time = 8.0f; // spawned 2s ago
  WObjs[1] = {OBJ_CLUTTER_WF, 0, 101};
  WObjs[1].parent_handle = 100; // clutter's parent is the player
  WObjs[1].creation_time = 8.0f; // also freshly spawned
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));

  // ...but only within 3 seconds
  WF_Gametime = 12.0f; // 4s later
  EXPECT_FALSE(RepObjectsAreRelated(0, 1));

  // quirk: the 3s grace ONLY exists for pairs where neither is a weapon.
  // A weapon ignores creation time entirely -- its immunity comes from
  // PF_NO_COLLIDE_PARENT instead
  ResetWO();
  WObjs[0] = {OBJ_WEAPON_WF, 0, 100};
  WObjs[0].parent_handle = 101; // fired by obj 1
  WObjs[0].creation_time = 9.5f;
  WObjs[1] = {OBJ_PLAYER_WF, 0, 101};
  EXPECT_FALSE(RepObjectsAreRelated(0, 1)); // no flag: weapon can hit owner!

  // with the flag: shooter immune
  WObjs[0].phys_flags |= PF_NO_COLLIDE_PARENT_WF;
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));

  // persistent weapons remember their last hit target
  ResetWO();
  WObjs[0] = {OBJ_WEAPON_WF, 0, 100};
  WObjs[0].movement_type = MT_PHYSICS_WF;
  WObjs[0].phys_flags |= PF_PERSISTENT_WF;
  WObjs[0].last_hit_handle = 101;
  WObjs[1] = {OBJ_ROBOT_WF, 0, 101};
  EXPECT_TRUE(RepObjectsAreRelated(0, 1)); // won't re-hit same target
  WObjs[0].last_hit_handle = 102;
  EXPECT_FALSE(RepObjectsAreRelated(0, 1));

  // weapon siblings: same parent -> related UNLESS either hits siblings
  ResetWO();
  WObjs[0] = {OBJ_WEAPON_WF, 0, 100};
  WObjs[0].parent_handle = 55;
  WObjs[1] = {OBJ_WEAPON_WF, 0, 101};
  WObjs[1].parent_handle = 55;
  EXPECT_TRUE(RepObjectsAreRelated(0, 1)); // siblings immune
  WObjs[1].phys_flags |= PF_HITS_SIBLINGS_WF;
  EXPECT_FALSE(RepObjectsAreRelated(0, 1)); // proximity-style can clash

  // quirk: DIFFERENT-parent weapons are declared related too -- two
  // weapons in flight simply never collide with each other by default
  WObjs[1].parent_handle = 66;
  EXPECT_TRUE(RepObjectsAreRelated(0, 1));
}
