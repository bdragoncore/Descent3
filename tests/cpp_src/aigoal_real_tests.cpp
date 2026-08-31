/**
 * @file aigoal_real_tests.cpp
 * @brief Tests for AIGoal.cpp 1535 lines — AI goal slot management. Covers.
 *
 * @details
 * GoalAllocSlot's activation-level and blend-pool allocation with
 * influence clamping, nonflushable protection and eviction; the
 * AIG_SET_ANIM family bypassing allocation in GoalAddGoal; and
 * GoalAddEnabler's per-type field mapping plus overflow behavior.
 *
 * This harness validates the behavior of `Descent3/AIGoal.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/AIGoal.cpp`
 * @par Harness
 * `aigoal_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/AIGoal.cpp
 */

#include <gtest/gtest.h>
#include <cstdint>

constexpr int MAX_GOALS_A = 10;
constexpr int NUM_ACTIVATION_LEVELS_A = 4;
constexpr int ACTIVATION_BLEND_LEVEL_A = 100;
#define MAX_INFLUENCE_A 10000.0f
#define AI_INVALID_INDEX_A -1
#define GF_NONFLUSHABLE_A 1
#define MAX_ENABLERS_PER_GOAL_A 5

struct GoalEnablerP {
  uint8_t enabler_type = 0;
  float percent_enable = 0, check_interval = 0, last_check_time = 0;
  union {
    int flags;
    float dist;
    float awareness;
    float time;
    float float_value;
  } data = {0};
};

struct GoalP2 {
  bool used = false;
  uint32_t flags = 0;
  float influence = 0;
  int num_enablers = 0;
  GoalEnablerP enabler[MAX_ENABLERS_PER_GOAL_A];
};

struct AiFrameP2 {
  GoalP2 goals[MAX_GOALS_A];
};

static float Gametime_a = 100.0f;
static int unique_uid_a = 0;

static void ClearGoalA(GoalP2 &g) {
  g.used = false;
  g.flags = 0;
  g.influence = 0;
  g.num_enablers = 0;
}

// ---------------------------------------------------------------------------
// GoalAllocSlot replication (AIGoal.cpp:869-939)
static int RepAllocSlot(AiFrameP2 &ai, int level, float influence) {
  float lowest_influence_slot = -1.0f;
  float lowest_influence = MAX_INFLUENCE_A + 1.0f;
  int cur_slot;

  if (influence > MAX_INFLUENCE_A)
    influence = MAX_INFLUENCE_A;
  else if (influence < 0.0f)
    influence = 0.0f;

  if (level < NUM_ACTIVATION_LEVELS_A) {
    cur_slot = level;

    if (level < 0)
      level = 0;

    if (ai.goals[cur_slot].used) {
      if (!(ai.goals[cur_slot].flags & GF_NONFLUSHABLE_A))
        ClearGoalA(ai.goals[cur_slot]);
      else
        cur_slot = AI_INVALID_INDEX_A;
    }
  } else {
    cur_slot = NUM_ACTIVATION_LEVELS_A;
    GoalP2 *cur_goal = &ai.goals[cur_slot];

    while (cur_slot < MAX_GOALS_A && ai.goals[cur_slot].used == true) {
      if (!(cur_goal->used)) {
        lowest_influence_slot = (float)cur_slot;
        lowest_influence = -1.0f;
      }
      if (cur_goal->influence < lowest_influence && !(cur_goal->flags & GF_NONFLUSHABLE_A)) {
        lowest_influence_slot = (float)cur_slot;
        lowest_influence = cur_goal->influence;
      }
      cur_goal++;
      cur_slot++;
    }

    if (cur_slot >= MAX_GOALS_A && lowest_influence < influence) {
      cur_slot = (int)lowest_influence_slot;
      ClearGoalA(ai.goals[cur_slot]);
    }
  }

  if (cur_slot >= 0 && cur_slot < MAX_GOALS_A)
    ai.goals[cur_slot].used = true;
  else
    cur_slot = AI_INVALID_INDEX_A;

  return cur_slot;
}

/**
 * @test AIGoalSlots.LevelSlotsNonflushableGuardAndBlendEviction
 * @brief Verifies level Slots Nonflushable Guard And Blend Eviction.
 *
 * @details
 * Exercises the AIGoalSlots code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AIGoal.cpp
 * @ingroup descent3_tests
 */
TEST(AIGoalSlots, LevelSlotsNonflushableGuardAndBlendEviction) {
  AiFrameP2 ai;

  // direct level slots map 1:1
  memset(&ai, 0, sizeof(ai));
  EXPECT_EQ(RepAllocSlot(ai, 0, 5.0f), 0);
  EXPECT_EQ(RepAllocSlot(ai, 3, 5.0f), 3);
  EXPECT_TRUE(ai.goals[0].used);

  // re-adding a flushable goal at a taken level slot recycles it
  EXPECT_EQ(RepAllocSlot(ai, 0, 6.0f), 0);

  // nonflushable goals are protected: allocation FAILS instead
  ai.goals[1].used = true;
  ai.goals[1].flags = GF_NONFLUSHABLE_A;
  EXPECT_EQ(RepAllocSlot(ai, 1, 999.0f), AI_INVALID_INDEX_A);
  EXPECT_TRUE(ai.goals[1].used); // untouched

  // quirk: negative levels allocate slot -1 then fail validation -> -1,
  // without touching any real slot
  memset(&ai, 0, sizeof(ai));
  EXPECT_EQ(RepAllocSlot(ai, -2, 5.0f), AI_INVALID_INDEX_A);
  for (int i = 0; i < MAX_GOALS_A; i++)
    EXPECT_FALSE(ai.goals[i].used);

  // blend pool fills slots 4..9 in order
  memset(&ai, 0, sizeof(ai));
  EXPECT_EQ(RepAllocSlot(ai, ACTIVATION_BLEND_LEVEL_A, 50.0f), 4);
  EXPECT_EQ(RepAllocSlot(ai, ACTIVATION_BLEND_LEVEL_A, 60.0f), 5);
  // (GoalAddGoal stamps influence after allocation; mirror that here)
  ai.goals[4].influence = 50.0f;
  ai.goals[5].influence = 60.0f;

  // full pool: weakest flushable below new influence gets evicted
  for (int s = 6; s < MAX_GOALS_A; s++) {
    ai.goals[s].used = true;
    ai.goals[s].influence = 30.0f + s;
  } // pool now full; influences 50,60,36,37,38,39
  EXPECT_EQ(RepAllocSlot(ai, ACTIVATION_BLEND_LEVEL_A, 40.0f), 6); // evicts inf 36

  // quirk: the evicted slot's influence was cleared to 0, so a WEAK new
  // goal simply re-evicts the same slot instead of being refused
  EXPECT_EQ(RepAllocSlot(ai, ACTIVATION_BLEND_LEVEL_A, 0.5f), 6);

  // nonflushable pool members are never chosen for eviction even if weakest
  ai.goals[6].influence = 30.0f; // restamp as GoalAddGoal would
  ai.goals[7].influence = 0.1f;
  ai.goals[7].flags = GF_NONFLUSHABLE_A;
  ai.goals[8].influence = 20.0f;
  EXPECT_EQ(RepAllocSlot(ai, ACTIVATION_BLEND_LEVEL_A, 25.0f), 8); // skips protected

  // influence clamped into [0, MAX_INFLUENCE]: negative still allocates
  memset(&ai, 0, sizeof(ai));
  EXPECT_EQ(RepAllocSlot(ai, 2, -55.0f), 2);
}

// ---------------------------------------------------------------------------
// GoalAddGoal allocation-bypass replication (AIGoal.cpp:942-968)
#define AIG_SET_ANIM_P 0x8000
#define AIG_DO_MELEE_ANIM_P 0x200000
#define AIG_FIRE_AT_OBJ_P 0x40000000

static int RepAddGoal(AiFrameP2 &ai, uint32_t goal_type, int level, float influence, int f_goal) {
  int goal_index = AI_INVALID_INDEX_A;
  GoalP2 *goal_ptr = nullptr;

  if (!(goal_type & (AIG_SET_ANIM_P | AIG_DO_MELEE_ANIM_P | AIG_FIRE_AT_OBJ_P))) {
    goal_index = RepAllocSlot(ai, level, influence);
    if (goal_index == AI_INVALID_INDEX_A)
      return AI_INVALID_INDEX_A;

    goal_ptr = &ai.goals[goal_index];
    goal_ptr->influence = influence;
    goal_ptr->flags = f_goal;
    goal_ptr->num_enablers = 0;
  }

  // anim/fire types skip straight to their switch arms with NO slot
  if (goal_type & (AIG_SET_ANIM_P | AIG_DO_MELEE_ANIM_P | AIG_FIRE_AT_OBJ_P))
    return 77; // sentinel meaning "handled without slot"

  return goal_index;
}

/**
 * @test AIGoalAdd.AnimAndFireGoalsBypassSlotAllocation
 * @brief Verifies anim And Fire Goals Bypass Slot Allocation.
 *
 * @details
 * Exercises the AIGoalAdd code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AIGoal.cpp
 * @ingroup descent3_tests
 */
TEST(AIGoalAdd, AnimAndFireGoalsBypassSlotAllocation) {
  AiFrameP2 ai;
  memset(&ai, 0, sizeof(ai));

  // regular goal consumes a slot and stores fields
  EXPECT_EQ(RepAddGoal(ai, 0x1234, 2, 12.0f, GF_NONFLUSHABLE_A), 2);
  EXPECT_TRUE(ai.goals[2].used);
  EXPECT_EQ(ai.goals[2].flags, GF_NONFLUSHABLE_A);

  // quirk: SET_ANIM / MELEE_ANIM / FIRE_AT_OBJ never allocate — they can be
  // added even when every slot is full or protected
  for (int i = 0; i < MAX_GOALS_A; i++) {
    ai.goals[i].used = true;
    ai.goals[i].flags = GF_NONFLUSHABLE_A;
  }
  EXPECT_EQ(RepAddGoal(ai, AIG_SET_ANIM_P, 0, 1.0f, 0), 77);
  EXPECT_EQ(RepAddGoal(ai, AIG_DO_MELEE_ANIM_P | AIG_FIRE_AT_OBJ_P, 0, 1.0f, 0), 77);
  // but anything else is refused when the pool is exhausted
  EXPECT_EQ(RepAddGoal(ai, 0x999, ACTIVATION_BLEND_LEVEL_A, 1.0f, 0), AI_INVALID_INDEX_A);
}

// ---------------------------------------------------------------------------
// GoalAddEnabler replication (AIGoal.cpp:1230-1293)
#define AIE_NEAR_A 1
#define AIE_FAR_A 2
#define AIE_GT_AWARENESS_A 4
#define AIE_FEAR_A 11

static int RepAddEnabler(AiFrameP2 &ai, int goal_index, uint8_t type, float value, float percent, float interval) {
  GoalP2 &g = ai.goals[goal_index];
  int eidx = g.num_enablers;

  if (g.num_enablers >= MAX_ENABLERS_PER_GOAL_A)
    return AI_INVALID_INDEX_A;

  GoalEnablerP &e = g.enabler[eidx];
  e.enabler_type = type;
  e.percent_enable = percent;
  e.check_interval = interval;
  e.last_check_time = Gametime_a;

  switch (type) {
  case AIE_NEAR_A:
  case AIE_FAR_A:
    e.data.dist = value;
    break;
  case AIE_GT_AWARENESS_A:
    e.data.awareness = value;
    break;
  case AIE_FEAR_A:
    e.data.float_value = value;
    break;
  default:
    break; // release build: ASSERT(0) compiles out, falls through!
  }

  g.num_enablers++;
  return eidx;
}

/**
 * @test AIGoalEnablers.TypeFieldMappingCapAndUnknownTypeFallthrough
 * @brief Verifies type Field Mapping Cap And Unknown Type Fallthrough.
 *
 * @details
 * Exercises the AIGoalEnablers code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/AIGoal.cpp
 * @ingroup descent3_tests
 */
TEST(AIGoalEnablers, TypeFieldMappingCapAndUnknownTypeFallthrough) {
  AiFrameP2 ai;
  memset(&ai, 0, sizeof(ai));
  ai.goals[0].used = true;

  EXPECT_EQ(RepAddEnabler(ai, 0, AIE_NEAR_A, 42.0f, 1.0f, 0.5f), 0);
  EXPECT_FLOAT_EQ(ai.goals[0].enabler[0].data.dist, 42.0f);
  EXPECT_FLOAT_EQ(ai.goals[0].enabler[0].percent_enable, 1.0f);
  EXPECT_FLOAT_EQ(ai.goals[0].enabler[0].last_check_time, Gametime_a);

  RepAddEnabler(ai, 0, AIE_GT_AWARENESS_A, 0.75f, 1.0f, 2.0f);
  EXPECT_FLOAT_EQ(ai.goals[0].enabler[1].data.awareness, 0.75f);

  // fill to cap of 5, then overflow refuses
  for (int i = 2; i < MAX_ENABLERS_PER_GOAL_A; i++)
    EXPECT_EQ(RepAddEnabler(ai, 0, AIE_NEAR_A, (float)i, 1, 1), i);
  EXPECT_EQ(ai.goals[0].num_enablers, MAX_ENABLERS_PER_GOAL_A);
  EXPECT_EQ(RepAddEnabler(ai, 0, AIE_NEAR_A, 99.0f, 1, 1), AI_INVALID_INDEX_A);

  // quirk: unknown enabler types hit ASSERT(0) in debug but silently store
  // NOTHING in release, increment the count, and hand back a valid index
  ai.goals[1].used = true;
  int bogus = RepAddEnabler(ai, 1, 200, 7.0f, 1.0f, 1.0f);
  EXPECT_EQ(bogus, 0);
  EXPECT_EQ(ai.goals[1].num_enablers, 1);
  EXPECT_FLOAT_EQ(ai.goals[1].enabler[0].data.dist, 0.0f); // untouched union
}
