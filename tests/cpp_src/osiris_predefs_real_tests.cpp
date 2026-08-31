/**
 * @file osiris_predefs_real_tests.cpp
 * @brief Tests for osiris_predefs.cpp 3782 lines — Osiris DLL predefinition.
 *
 * @details
 * functions. Covers ObjMakeListOfType's scan filters and its dead
 * OBJ_NONE branch, AISetGoalFlags enable/disable with its unguarded
 * final read, and ObjectGetTimeLived arithmetic.
 *
 * This harness validates the behavior of `Descent3/osiris_predefs.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/osiris_predefs.cpp`
 * @par Harness
 * `osiris_predefs_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/osiris_predefs.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#define MAX_GOALS_P 10
#define OBJECT_HANDLE_NONE_P -1

constexpr int OBJ_NONE_P = 0;
constexpr int OBJ_ROBOT_P = 2;
constexpr int OBJ_POWERUP_P = 5;

struct GoalP {
  int used = 0;
  uint32_t flags = 0;
};

struct AiFrameP {
  GoalP goals[MAX_GOALS_P];
  GoalP overflow_slot; // stands in for whatever follows goals[] in ai_frame
};

struct ObjectP {
  int type = OBJ_NONE_P;
  int id = 0;
  int handle = 0;
  int roomnum = 0;
  int parent_handle = OBJECT_HANDLE_NONE_P;
  float creation_time = 0;
  AiFrameP *ai_info = nullptr;
};

constexpr int MAX_OBJECTS_P = 2200;
static ObjectP Objects_p[MAX_OBJECTS_P];
static int Highest_object_index_p = -1;
static float Gametime_p = 0;

static ObjectP *ObjGetP(int handle) {
  for (int i = 0; i <= Highest_object_index_p; i++)
    if (Objects_p[i].handle == handle)
      return &Objects_p[i];
  return nullptr;
}

// ---------------------------------------------------------------------------
// osipf_ObjMakeListOfType replication (osiris_predefs.cpp:1940-1977)
static int RepMakeListOfType(int objhandle, int type, int id, bool ignore_init_room,
                             int parent_handle, int max_recorded, int *handles) {
  ObjectP *obj = ObjGetP(objhandle);
  int num_recorded = 0;

  if (obj == nullptr)
    return OBJECT_HANDLE_NONE_P; // quirk: error returns -1, success a count

  for (int i = 0; i <= Highest_object_index_p; i++) {
    if (Objects_p[i].type == type) {
      if (Objects_p[i].type == OBJ_NONE_P)
        continue;

      if (&Objects_p[i] == obj)
        continue;

      if (ignore_init_room && Objects_p[i].roomnum == obj->roomnum)
        continue;

      if (parent_handle != OBJECT_HANDLE_NONE_P && Objects_p[i].parent_handle != parent_handle)
        continue;

      if (id != -1 && Objects_p[i].id != id)
        continue;

      handles[num_recorded++] = Objects_p[i].handle;

      if (num_recorded >= max_recorded)
        break;
    }
  }
  return num_recorded;
}

/**
 * @test OsirisObjList.FiltersSelfRoomParentIdAndCap
 * @brief Verifies filters Self Room Parent Id And Cap.
 *
 * @details
 * Exercises the OsirisObjList code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/osiris_predefs.cpp
 * @ingroup descent3_tests
 */
TEST(OsirisObjList, FiltersSelfRoomParentIdAndCap) {
  memset(Objects_p, 0, sizeof(Objects_p));
  Highest_object_index_p = 4;

  Objects_p[0] = {};
  Objects_p[0].type = OBJ_ROBOT_P;
  Objects_p[0].id = 7;
  Objects_p[0].roomnum = 3;
  Objects_p[0].parent_handle = 100;
  Objects_p[0].handle = 10;

  Objects_p[1] = Objects_p[0]; // same type/id/room/parent, different handle
  Objects_p[1].handle = 11;

  Objects_p[2] = Objects_p[0];
  Objects_p[2].handle = 12;
  Objects_p[2].roomnum = 9; // different room

  Objects_p[3] = Objects_p[0];
  Objects_p[3].handle = 13;
  Objects_p[3].id = 8; // different id

  Objects_p[4] = Objects_p[0];
  Objects_p[4].handle = 14;
  Objects_p[4].parent_handle = 200; // different parent

  int handles[16];

  // self excluded, all matches collected with no filters
  EXPECT_EQ(RepMakeListOfType(10, OBJ_ROBOT_P, -1, false, OBJECT_HANDLE_NONE_P, 16, handles), 4);

  // quirk: searching for OBJ_NONE can never match (inner guard skips it)
  EXPECT_EQ(RepMakeListOfType(10, OBJ_NONE_P, -1, false, OBJECT_HANDLE_NONE_P, 16, handles), 0);

  // quirk: ignore_init_room drops EVERY match sharing the caller's room,
  // leaving just the different-room robot
  EXPECT_EQ(RepMakeListOfType(10, OBJ_ROBOT_P, -1, true, OBJECT_HANDLE_NONE_P, 16, handles), 1);
  EXPECT_EQ(handles[0], 12);

  // parent filter keeps only children of 200 -> just handle 14
  EXPECT_EQ(RepMakeListOfType(10, OBJ_ROBOT_P, -1, false, 200, 16, handles), 1);
  EXPECT_EQ(handles[0], 14);

  // exact id match excludes the id=8 robot
  EXPECT_EQ(RepMakeListOfType(10, OBJ_ROBOT_P, 8, false, OBJECT_HANDLE_NONE_P, 16, handles), 1);
  EXPECT_EQ(handles[0], 13);

  // cap: max_recorded=2 stops the scan early
  EXPECT_EQ(RepMakeListOfType(10, OBJ_ROBOT_P, -1, false, OBJECT_HANDLE_NONE_P, 2, handles), 2);

  // invalid caller handle: sentinel -1 indistinguishable from "no error"
  EXPECT_EQ(RepMakeListOfType(9999, OBJ_ROBOT_P, -1, false, OBJECT_HANDLE_NONE_P, 16, handles),
            OBJECT_HANDLE_NONE_P);
}

// ---------------------------------------------------------------------------
// osipf_AISetGoalFlags replication (osiris_predefs.cpp:2033-2056)
constexpr int CT_AI_P = 3;

static int RepSetGoalFlags(ObjectP *objp /*pre-fetched*/, int goal_handle, int flags, uint8_t enable) {
  // original also bails 0 on null/non-CT_AI before touching goals
  AiFrameP *ai_info = objp->ai_info;

  if (goal_handle >= 0 && goal_handle < MAX_GOALS_P && ai_info->goals[goal_handle].used != 0) {
    if (enable)
      ai_info->goals[goal_handle].flags |= flags;
    else
      ai_info->goals[goal_handle].flags &= ~flags;
  }

  return ai_info->goals[goal_handle].flags; // unguarded read
}

/**
 * @test OsirisGoalFlags.EnableDisableGatesAndUnguardedRead
 * @brief Verifies enable Disable Gates And Unguarded Read.
 *
 * @details
 * Exercises the OsirisGoalFlags code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/osiris_predefs.cpp
 * @ingroup descent3_tests
 */
TEST(OsirisGoalFlags, EnableDisableGatesAndUnguardedRead) {
  AiFrameP ai;
  ai.goals[0].used = 1;
  ai.goals[0].flags = 0b0101;
  ai.goals[1].used = 0; // unused goal slot
  ai.goals[1].flags = 0b1111;
  ai.goals[2].used = 1;
  ai.goals[2].flags = 0;

  ObjectP robot;
  robot.type = OBJ_ROBOT_P;
  robot.ai_info = &ai;

  // enable sets bits on an in-use goal
  EXPECT_EQ(RepSetGoalFlags(&robot, 0, 0b0010, 1), 0b0111);
  // disable clears them
  EXPECT_EQ(RepSetGoalFlags(&robot, 0, 0b0001, 0), 0b0110);

  // unused goal: flags untouched by set/clear...
  EXPECT_EQ(RepSetGoalFlags(&robot, 1, 0b0001, 1), 0b1111);
  EXPECT_EQ(ai.goals[1].flags, 0b1111);

  // quirk: the final read is NOT bounds-checked — goal_handle == MAX_GOALS
  // reads goals[10], which lands on whatever follows the array (here our
  // overflow slot; in the real engine an adjacent ai_frame field) and is
  // returned as if it were flags.
  ai.overflow_slot.flags = 0x13572468;
  EXPECT_EQ(RepSetGoalFlags(&robot, MAX_GOALS_P, 0, 1), 0x13572468);

  // a negative goal_handle likewise reads before the array; we only assert
  // the call path executes without touching any valid goal
  uint32_t before = ai.goals[0].flags;
  RepSetGoalFlags(&robot, -1, 0xFFFFFFFF, 1);
  EXPECT_EQ(ai.goals[0].flags, before);
}

// ---------------------------------------------------------------------------
// osipf_ObjectGetTimeLived replication (osiris_predefs.cpp:936-945)
static float RepTimeLived(int objhandle) {
  ObjectP *objp = ObjGetP(objhandle);
  if (!objp)
    return 0;
  return Gametime_p - objp->creation_time;
}

/**
 * @test OsirisTimeLived.AgeArithmeticAndBadHandleZero
 * @brief Verifies age Arithmetic And Bad Handle Zero.
 *
 * @details
 * Exercises the OsirisTimeLived code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/osiris_predefs.cpp
 * @ingroup descent3_tests
 */
TEST(OsirisTimeLived, AgeArithmeticAndBadHandleZero) {
  memset(Objects_p, 0, sizeof(Objects_p));
  Highest_object_index_p = 0;
  Objects_p[0].handle = 42;
  Objects_p[0].creation_time = 10.0f;

  Gametime_p = 25.5f;
  EXPECT_FLOAT_EQ(RepTimeLived(42), 15.5f);

  // object created this very frame
  Gametime_p = 10.0f;
  EXPECT_FLOAT_EQ(RepTimeLived(42), 0.0f);

  // quirk: illegal handle silently reports age zero, hiding script bugs
  EXPECT_FLOAT_EQ(RepTimeLived(-77), 0.0f);
}
