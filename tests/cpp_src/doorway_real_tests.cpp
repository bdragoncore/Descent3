/**
 * @file doorway_real_tests.cpp
 * @brief Tests for doorway.cpp (~661 lines).
 *
 * @details
 * Covers the door state machine: DoorwayActivate/SetPosition/Stop/Destroy,
 * DoorwayDoFrame open/close/auto-wait cycles incl. blocked-door handling,
 * active doorway list bookkeeping, lock/key/openable queries, and
 * animation frame updates.
 *
 * This harness validates the behavior of `Descent3/doorway.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/doorway.cpp`
 * @par Harness
 * `doorway_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/doorway.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "doorway.h"
#include "door.h"
#include "room.h"
#include "object.h"
#include "polymodel.h"
#include "player.h"
#include "multi.h"
#include "game.h"
#include "terrain.h"
#include "hlsoundlib.h"
#include "osiris_dll.h"
#include "Inventory.h"
#include "log.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
object Objects[MAX_OBJECTS];
room Rooms[MAX_ROOMS];
door Doors[MAX_DOORS];
poly_model Poly_models[4];
int Highest_room_index = 0;
float Frametime = 0.0f;
player Players[MAX_PLAYERS];
int Player_num = 0;

// ---------------------------------------------------------------------------
// Scripted object lookup
// ---------------------------------------------------------------------------
static object *g_lookup[MAX_OBJECTS];

object *ObjGet(int handle) {
  if (handle < 0 || handle >= MAX_OBJECTS)
    return nullptr;
  return g_lookup[handle];
}

object *ObjGetUltimateParent(object *obj) {
  while (obj && obj->attach_parent_handle != OBJECT_HANDLE_NONE)
    obj = ObjGet(obj->attach_parent_handle);
  return obj;
}

// ---------------------------------------------------------------------------
// Event + sound recorders
// ---------------------------------------------------------------------------
static int g_last_event_obj_handle;
static int g_last_event_id;
static int g_event_count;

bool Osiris_CallEvent(object *obj, int event, tOSIRISEventInfo *data) {
  (void)data;
  g_last_event_obj_handle = obj ? obj->handle : -1;
  g_last_event_id = event;
  g_event_count++;
  return true;
}

// player's embedded Inventory pulls these in
Inventory::Inventory() {}
Inventory::~Inventory() {}

hlsSystem Sound_system;
hlsSystem::hlsSystem() {}
void hlsSystem::KillSoundLib(bool) {}
static int g_next_sound_handle = 1;
static int g_last_played_sound;
static float g_last_sound_offset;
static std::vector<int> g_stopped_sounds;
int hlsSystem::Play3dSound(int sound_index, int priority, object *cur_obj, float volume, int flags, float offset) {
  (void)priority;
  (void)cur_obj;
  (void)volume;
  (void)flags;
  g_last_played_sound = sound_index;
  g_last_sound_offset = offset;
  return g_next_sound_handle++;
}
void hlsSystem::StopSoundImmediate(int hlsound_uid) { g_stopped_sounds.push_back(hlsound_uid); }

// ---------------------------------------------------------------------------
// door.cpp stubs
// ---------------------------------------------------------------------------
int GetDoorImage(int handle) {
  (void)handle;
  return 0; // all doors use polymodel 0
}

// ---------------------------------------------------------------------------
// Fixture helpers
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for DoorwayTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class DoorwayTest : public ::testing::Test {
protected:
  object door_obj{};

  void SetUp() override {
    memset(g_lookup, 0, sizeof(g_lookup));
    memset(Objects, 0, sizeof(object) * 16);
    memset(&Rooms, 0, sizeof(room) * 4);
    memset(&Doors, 0, sizeof(door) * 8);
    memset(&Poly_models, 0, sizeof(poly_model) * 4);
    memset(Players, 0, sizeof(player) * 2);
    Num_active_doorways = 0;
    Global_keys = 0;
    g_event_count = 0;
    g_last_played_sound = -1;
    g_stopped_sounds.clear();
    g_next_sound_handle = 1;

    // model 0: keyframe-animated with 10 keys
    Poly_models[0].max_keys = 10;
    Poly_models[0].frame_max = 20;
    Poly_models[0].flags = 0;

    Doors[0] = door{};
    Doors[0].used = 1;
    Doors[0].total_open_time = 2.0f;  // seconds
    Doors[0].total_close_time = 1.0f; // seconds
    Doors[0].total_time_open = 3.0f;  // auto-wait seconds
    Doors[0].open_sound = 55;
    Doors[0].close_sound = 66;
  }

  // Builds a used door room + its door object; returns the object handle
  int BuildDoor(int roomnum, uint8_t dflags = 0, int objidx = 7) {
    room &rp = Rooms[roomnum];
    rp.used = 1;
    rp.flags |= RF_DOOR;
    rp.objects = objidx;
    rp.doorway_data = new doorway{};
    rp.doorway_data->doornum = 0;
    rp.doorway_data->activenum = -1;
    rp.doorway_data->sound_handle = -1;
    rp.doorway_data->flags = dflags;

    door_obj = object{};
    door_obj.type = OBJ_DOOR;
    door_obj.id = 0; // doornum
    door_obj.roomnum = roomnum;
    door_obj.next = -1;
    door_obj.prev = -1;
    door_obj.handle = 500 + roomnum;
    Objects[objidx] = door_obj;
    g_lookup[door_obj.handle] = &Objects[objidx];
    Highest_room_index = roomnum > Highest_room_index ? roomnum : Highest_room_index;
    return door_obj.handle;
  }

  void AdvanceFrame(float dt) {
    Frametime = dt;
    DoorwayDoFrame();
  }
};

/**
 * @test DoorwayTest.ActivateOpensNonAutoDoorAndPlaysOpenSound
 * @brief Verifies activate Opens Non Auto Door And Plays Open Sound.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, ActivateOpensNonAutoDoorAndPlaysOpenSound) {
  int h = BuildDoor(1);

  DoorwayActivate(h);

  doorway *dp = Rooms[1].doorway_data;
  EXPECT_EQ(dp->state, DOORWAY_OPENING);
  EXPECT_FLOAT_EQ(dp->dest_pos, 1.0f);
  EXPECT_EQ(Num_active_doorways, 1);
  EXPECT_EQ(Active_doorways[0], 1);
  EXPECT_EQ(dp->activenum, 0);
  EXPECT_EQ(g_last_played_sound, 55); // open sound
}

/**
 * @test DoorwayTest.ActivateAutoDoorUsesOpeningAutoState
 * @brief Verifies activate Auto Door Uses Opening Auto State.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, ActivateAutoDoorUsesOpeningAutoState) {
  int h = BuildDoor(1, DF_AUTO);

  DoorwayActivate(h);
  EXPECT_EQ(Rooms[1].doorway_data->state, DOORWAY_OPENING_AUTO);
}

/**
 * @test DoorwayTest.ActivateBlastedOrMovingDoorIsNoop
 * @brief Verifies activate Blasted Or Moving Door Is Noop.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, ActivateBlastedOrMovingDoorIsNoop) {
  int h = BuildDoor(1, DF_BLASTED);
  DoorwayActivate(h);
  EXPECT_EQ(Num_active_doorways, 0);

  Rooms[1].doorway_data->flags = 0;
  DoorwayActivate(h);
  EXPECT_EQ(Rooms[1].doorway_data->state, DOORWAY_OPENING);

  DoorwayActivate(h); // already opening: no second entry, no state change
  EXPECT_EQ(Num_active_doorways, 1);
}

/**
 * @test DoorwayTest.DoFrameOpeningCompletesAndDeactivates
 * @brief Verifies do Frame Opening Completes And Deactivates.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, DoFrameOpeningCompletesAndDeactivates) {
  BuildDoor(1);
  DoorwayActivate(500 + 1);

  AdvanceFrame(1.0f); // half of total_open_time=2 -> pos .5
  EXPECT_FLOAT_EQ(Rooms[1].doorway_data->position, 0.5f);
  EXPECT_EQ(Num_active_doorways, 1);

  AdvanceFrame(1.0f); // reaches dest 1.0 -> STOPPED + removed
  doorway *dp = Rooms[1].doorway_data;
  EXPECT_EQ(dp->state, DOORWAY_STOPPED);
  EXPECT_FLOAT_EQ(dp->position, 1.0f);
  EXPECT_EQ(Num_active_doorways, 0);
  EXPECT_EQ(dp->activenum, -1);
}

/**
 * @test DoorwayTest.AutoDoorWaitsThenClosesOnExpiry
 * @brief Verifies auto Door Waits Then Closes On Expiry.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, AutoDoorWaitsThenClosesOnExpiry) {
  BuildDoor(1, DF_AUTO);
  DoorwayActivate(500 + 1);

  AdvanceFrame(2.0f); // fully open
  doorway *dp = Rooms[1].doorway_data;
  EXPECT_EQ(dp->state, DOORWAY_WAITING);
  EXPECT_FLOAT_EQ(dp->dest_pos, 3.0f); // wait countdown seeded from door->total_time_open

  AdvanceFrame(2.0f); // not expired yet, nothing blocking
  EXPECT_EQ(dp->state, DOORWAY_WAITING);

  AdvanceFrame(1.5f); // countdown hits <= 0 -> CLOSING starts
  EXPECT_EQ(dp->state, DOORWAY_CLOSING);
  EXPECT_EQ(g_last_played_sound, 66); // close sound

  AdvanceFrame(1.0f); // close completes -> EVT_DOOR_CLOSE fired
  EXPECT_EQ(dp->state, DOORWAY_STOPPED);
  EXPECT_FLOAT_EQ(dp->position, 0.0f);
  EXPECT_EQ(g_last_event_id, EVT_DOOR_CLOSE);
}

/**
 * @test DoorwayTest.BlockedWaitingDoorStaysOpen
 * @brief Verifies blocked Waiting Door Stays Open.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, BlockedWaitingDoorStaysOpen) {
  BuildDoor(1, DF_AUTO);
  DoorwayActivate(500 + 1);
  AdvanceFrame(2.0f); // -> WAITING

  // an intersecting object keeps the door open past its wait timer
  Objects[7].prev = 3;
  AdvanceFrame(5.0f); // countdown expires...
  EXPECT_EQ(Rooms[1].doorway_data->state, DOORWAY_WAITING);

  Objects[7].prev = -1; // path clear
  AdvanceFrame(0.1f);
  EXPECT_EQ(Rooms[1].doorway_data->state, DOORWAY_CLOSING);
}

/**
 * @test DoorwayTest.SetPositionDrivesDirectionAndSounds
 * @brief Verifies set Position Drives Direction And Sounds.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, SetPositionDrivesDirectionAndSounds) {
  int h = BuildDoor(1);

  DoorwaySetPosition(h, 0.75f); // opening upward
  doorway *dp = Rooms[1].doorway_data;
  EXPECT_EQ(dp->state, DOORWAY_OPENING);
  EXPECT_FLOAT_EQ(g_last_sound_offset, 0.0f); // offset = position*open_time

  AdvanceFrame(1.5f); // pos = 0.75 == dest; door stops

  DoorwaySetPosition(h, 0.25f); // lower target than current pos -> closing
  EXPECT_EQ(dp->state, DOORWAY_CLOSING);
  // offset = (1-position)*close_time
  EXPECT_NEAR(g_last_sound_offset, 0.25f * 1.0f, 0.01f);
}

/**
 * @test DoorwayTest.StopFreezesDoorAndRemovesFromActiveList
 * @brief Verifies stop Freezes Door And Removes From Active List.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, StopFreezesDoorAndRemovesFromActiveList) {
  int h = BuildDoor(1);
  DoorwayActivate(h);
  AdvanceFrame(0.5f); // pos 0.25

  DoorwayStop(h);
  doorway *dp = Rooms[1].doorway_data;
  EXPECT_EQ(dp->state, DOORWAY_STOPPED);
  EXPECT_FLOAT_EQ(dp->dest_pos, dp->position);
  EXPECT_EQ(Num_active_doorways, 0);
}

/**
 * @test DoorwayTest.DestroyMarksBlastedFullyOpen
 * @brief Verifies destroy Marks Blasted Fully Open.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, DestroyMarksBlastedFullyOpen) {
  BuildDoor(1);
  object *objp = &Objects[7];

  DoorwayDestroy(objp);

  doorway *dp = Rooms[1].doorway_data;
  EXPECT_TRUE(dp->flags & DF_BLASTED);
  EXPECT_FLOAT_EQ(dp->position, 1.0f);
  EXPECT_EQ(dp->state, DOORWAY_STOPPED);
}

/**
 * @test DoorwayTest.DeactivateAllClearsActiveListButLeavesDoorsMoving
 * @brief Verifies deactivate All Clears Active List But Leaves Doors Moving.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, DeactivateAllClearsActiveListButLeavesDoorsMoving) {
  BuildDoor(1, DF_AUTO, 6);
  BuildDoor(2, DF_AUTO, 7);

  DoorwayActivate(500 + 1);
  DoorwayActivate(500 + 2);
  ASSERT_EQ(Num_active_doorways, 2);

  // KNOWN ENGINE BUG: the scan loop increments r but never rp, so only
  // room 0 is ever inspected and no door is snapped. The doors simply
  // drop off the active list and freeze mid-motion.
  DoorwayDeactivateAll();

  EXPECT_EQ(Num_active_doorways, 0);
  for (int r = 1; r <= 2; r++) {
    doorway *dp = Rooms[r].doorway_data;
    EXPECT_EQ(dp->state, DOORWAY_OPENING_AUTO); // untouched
    EXPECT_FLOAT_EQ(dp->position, 0.0f);        // not snapped to dest
  }
}

/**
 * @test DoorwayTest.LockUnlockRoundTrip
 * @brief Verifies lock Unlock Round Trip.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, LockUnlockRoundTrip) {
  int h = BuildDoor(1);

  EXPECT_FALSE(DoorwayLocked(h));
  DoorwayLockUnlock(h, true);
  EXPECT_TRUE(DoorwayLocked(h));
  EXPECT_TRUE(DoorwayLocked(&Rooms[1])); // by-room overload
  DoorwayLockUnlock(h, false);
  EXPECT_FALSE(DoorwayLocked(h));
}

/**
 * @test DoorwayTest.OpenableKeyLogic
 * @brief Verifies openable Key Logic.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, OpenableKeyLogic) {
  int h = BuildDoor(1);
  doorway *dp = Rooms[1].doorway_data;

  // no keys needed: anyone can open
  EXPECT_TRUE(DoorwayOpenable(h, OBJECT_HANDLE_NONE));

  // locked blocks everything
  dp->keys_needed = KEY_FLAG(1);
  dp->flags |= DF_LOCKED;
  EXPECT_FALSE(DoorwayOpenable(h, OBJECT_HANDLE_NONE));
  dp->flags &= ~DF_LOCKED;

  // player needs ALL keys unless DF_KEY_ONLY_ONE
  Players[0].keys = KEY_FLAG(1);
  object opener{};
  opener.type = OBJ_PLAYER;
  opener.id = 0;
  opener.handle = 900;
  g_lookup[900] = &opener;

  dp->keys_needed = KEY_FLAG(1) | KEY_FLAG(2);
  EXPECT_FALSE(DoorwayOpenable(h, 900)); // has 1 of 2
  dp->flags |= DF_KEY_ONLY_ONE;
  EXPECT_TRUE(DoorwayOpenable(h, 900)); // one suffices
  dp->flags &= ~DF_KEY_ONLY_ONE;

  Players[0].keys = KEY_FLAG(1) | KEY_FLAG(2);
  EXPECT_TRUE(DoorwayOpenable(h, 900));
}

/**
 * @test DoorwayTest.WeaponOpenerResolvesToUltimateParent
 * @brief Verifies weapon Opener Resolves To Ultimate Parent.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, WeaponOpenerResolvesToUltimateParent) {
  int h = BuildDoor(1);
  Rooms[1].doorway_data->keys_needed = KEY_FLAG(1);

  object weapon{};
  weapon.type = OBJ_WEAPON;
  weapon.attach_parent_handle = 901;
  weapon.handle = 950;
  g_lookup[950] = &weapon;

  object owner{};
  owner.attach_parent_handle = OBJECT_HANDLE_NONE;
  owner.type = OBJ_PLAYER;
  owner.id = 0;
  owner.handle = 901;
  g_lookup[901] = &owner;
  Players[0].keys = KEY_FLAG(1);

  EXPECT_TRUE(DoorwayOpenable(h, 950));
}

/**
 * @test DoorwayTest.RobotOpenerUsesGlobalKeys
 * @brief Verifies robot Opener Uses Global Keys.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, RobotOpenerUsesGlobalKeys) {
  int h = BuildDoor(1);
  Rooms[1].doorway_data->keys_needed = KEY_FLAG(2);

  object bot{};
  bot.type = OBJ_ROBOT;
  bot.handle = 960;
  g_lookup[960] = &bot;

  EXPECT_FALSE(DoorwayOpenable(h, 960));
  Global_keys = KEY_FLAG(2);
  EXPECT_TRUE(DoorwayOpenable(h, 960));
}

/**
 * @test DoorwayTest.StateAndPositionQueriesHandleBadHandles
 * @brief Verifies state And Position Queries Handle Bad Handles.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, StateAndPositionQueriesHandleBadHandles) {
  BuildDoor(1);

  EXPECT_EQ(DoorwayState(12345), DOORWAY_STOPPED);
  EXPECT_FLOAT_EQ(DoorwayPosition(12345), 0.0f);

  Rooms[1].doorway_data->position = 0.4f;
  Rooms[1].doorway_data->state = DOORWAY_CLOSING;
  EXPECT_EQ(DoorwayState(500 + 1), DOORWAY_CLOSING);
  EXPECT_FLOAT_EQ(DoorwayPosition(500 + 1), 0.4f);
  EXPECT_FLOAT_EQ(DoorwayPosition(&Rooms[1]), 0.4f);
}

/**
 * @test DoorwayTest.UpdateAnimationSetsKeyframeOrTimedFrame
 * @brief Verifies update Animation Sets Keyframe Or Timed Frame.
 *
 * @details
 * Exercises the DoorwayTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/doorway.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorwayTest, UpdateAnimationSetsKeyframeOrTimedFrame) {
  BuildDoor(1);
  Rooms[1].doorway_data->position = 0.5f;
  object *objp = &Objects[7];

  DoorwayUpdateAnimation(&Rooms[1]);
  EXPECT_FLOAT_EQ(objp->rtype.pobj_info.anim_frame, 5.0f); // max_keys(10)*0.5

  Poly_models[0].flags |= PMF_TIMED;
  DoorwayUpdateAnimation(&Rooms[1]);
  EXPECT_FLOAT_EQ(objp->rtype.pobj_info.anim_frame, 10.0f); // frame_max(20)*0.5
}
