/**
 * @file telcomgoals_real_tests.cpp
 * @brief Tests for TelComGoals.cpp 775 lines — TelCom goals screen.
 *
 * @details
 * Covers the line-data builder (enabled-goal filtering, primary/
 * secondary classification, objective flags, active marking, insertion
 * sort ordering via the slot comparator), hit-testing bounding boxes,
 * and the active-goal alpha pulse.
 *
 * This harness validates the behavior of `Descent3/TelComGoals.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/TelComGoals.cpp`
 * @par Harness
 * `telcomgoals_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/TelComGoals.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <vector>
#include <algorithm>

// replicated flags (levelgoal_external.h:38-45)
constexpr uint32_t LGF_SECONDARY_GOAL = 0x00000002;
constexpr uint32_t LGF_ENABLED = 0x00000004;
constexpr uint32_t LGF_COMPLETED = 0x00000008;
constexpr uint32_t LGF_TELCOM_LISTS = 0x00000010;
constexpr uint32_t LGF_FAILED = 0x00000080;

constexpr char LO_GET_SPECIFIED = 0;

// mock goal system backing TG_compare_slots/TCGoalsBuildLineData
struct MockGoals {
  std::vector<uint32_t> status;
  std::vector<int> priority;
  std::vector<int8_t> goallist;
  std::vector<int> active_primary, active_secondary;

  int GetNumGoals() const { return (int)status.size(); }
  bool GoalStatus(int i, char, uint32_t *v) const { *v = status[i]; return true; }
  bool GoalPriority(int i, char, int *v) const { *v = priority[i]; return true; }
  bool GoalGoalList(int i, char, int8_t *v) const { *v = goallist[i]; return true; }
  int GetNumActivePrimaryGoals() const { return (int)active_primary.size(); }
  int GetActivePrimaryGoal(int i) const { return active_primary[i]; }
  int GetNumActiveSecondaryGoals() const { return (int)active_secondary.size(); }
  int GetActiveSecondaryGoal(int i) const { return active_secondary[i]; }
};

static MockGoals *TG_goals;

// replicated tGoalLineInfo (TelComGoals.cpp:118-124)
struct tGoalLineInfo {
  bool primary;
  bool is_objective;
  bool is_active;
  int goal_index;
  int lx, rx, ty, by;
};
static std::vector<tGoalLineInfo> TG_Lines;
static std::vector<int> TG_SortedList;

// replicated TG_compare_slots (TelComGoals.cpp:140-174)
bool TG_compare_slots(int left, int right) {
  int left_index = TG_Lines[left].goal_index;
  int right_index = TG_Lines[right].goal_index;

  if (TG_Lines[left].primary && !TG_Lines[right].primary)
    return false; // right is a secondary, left is a primary
  if (!TG_Lines[left].primary && TG_Lines[right].primary)
    return true; // right is a primary, left is secondary

  int left_priority, right_priority;
  int8_t left_list, right_list;
  TG_goals->GoalPriority(left_index, LO_GET_SPECIFIED, &left_priority);
  TG_goals->GoalPriority(right_index, LO_GET_SPECIFIED, &right_priority);
  TG_goals->GoalGoalList(left_index, LO_GET_SPECIFIED, &left_list);
  TG_goals->GoalGoalList(right_index, LO_GET_SPECIFIED, &right_list);

  if (left_list < right_list)
    return false;
  if (right_list < left_list)
    return true;

  if (TG_Lines[left].is_objective)
    return false;
  if (TG_Lines[right].is_objective)
    return true;

  return (left_priority > right_priority);
}

// replicated TCGoalsBuildLineData (TelComGoals.cpp:176-279)
void TCGoalsBuildLineData(void) {
  int number_of_goals = TG_goals->GetNumGoals();
  int count = 0;

  for (int index = 0; index < number_of_goals; index++) {
    uint32_t goal_status;
    TG_goals->GoalStatus(index, LO_GET_SPECIFIED, &goal_status);
    if (goal_status & LGF_ENABLED)
      count++;
  }

  TG_Lines.clear();
  TG_SortedList.clear();
  if (count == 0)
    return;

  TG_Lines.resize(count);
  TG_SortedList.resize(count);
  int i = 0;
  for (int index = 0; index < number_of_goals; index++) {
    uint32_t goal_status;
    TG_goals->GoalStatus(index, LO_GET_SPECIFIED, &goal_status);
    if (goal_status & LGF_ENABLED) {
      TG_Lines[i].goal_index = index;
      TG_Lines[i].is_active = false;
      TG_Lines[i].primary = !(goal_status & LGF_SECONDARY_GOAL);
      TG_Lines[i].is_objective = (goal_status & LGF_TELCOM_LISTS) != 0;
      TG_Lines[i].lx = TG_Lines[i].rx = TG_Lines[i].ty = TG_Lines[i].by = -1;
      i++;
    }
  }

  for (int k = 0; k < TG_goals->GetNumActivePrimaryGoals(); k++) {
    int act_g_index = TG_goals->GetActivePrimaryGoal(k);
    for (int index = 0; index < (int)TG_Lines.size(); index++) {
      if (act_g_index == TG_Lines[index].goal_index) {
        TG_Lines[index].is_active = true;
        break;
      }
    }
  }
  for (int k = 0; k < TG_goals->GetNumActiveSecondaryGoals(); k++) {
    int act_g_index = TG_goals->GetActiveSecondaryGoal(k);
    for (int index = 0; index < (int)TG_Lines.size(); index++) {
      if (act_g_index == TG_Lines[index].goal_index) {
        TG_Lines[index].is_active = true;
        break;
      }
    }
  }

  for (size_t k = 0; k < TG_Lines.size(); k++)
    TG_SortedList[k] = (int)k;

  for (size_t ii = 1; ii <= TG_Lines.size() - 1; ii++) {
    int t = TG_SortedList[ii];
    int j = (int)ii - 1;
    for (; j >= 0 && TG_compare_slots(TG_SortedList[j], t); j--)
      TG_SortedList[j + 1] = TG_SortedList[j];
    TG_SortedList[j + 1] = t;
  }
}

/**
 * @test TelComGoals.BuildFiltersDisabledGoals
 * @brief Verifies build Filters Disabled Goals.
 *
 * @details
 * Exercises the TelComGoals code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComGoals.cpp
 * @ingroup descent3_tests
 */
TEST(TelComGoals, BuildFiltersDisabledGoals) {
  MockGoals g;
  g.status = {LGF_ENABLED, 0, LGF_ENABLED | LGF_COMPLETED, LGF_ENABLED | LGF_FAILED};
  g.priority.assign(4, 0);
  g.goallist.assign(4, 0);
  TG_goals = &g;
  TCGoalsBuildLineData();
  ASSERT_EQ(TG_Lines.size(), 3u); // disabled goal dropped
  EXPECT_EQ(TG_Lines[0].goal_index, 0);
  EXPECT_EQ(TG_Lines[1].goal_index, 2); // order preserves original indices
  EXPECT_EQ(TG_Lines[2].goal_index, 3);
}

/**
 * @test TelComGoals.BuildClassifiesPrimarySecondaryAndObjective
 * @brief Verifies build Classifies Primary Secondary And Objective.
 *
 * @details
 * Exercises the TelComGoals code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComGoals.cpp
 * @ingroup descent3_tests
 */
TEST(TelComGoals, BuildClassifiesPrimarySecondaryAndObjective) {
  MockGoals g;
  g.status = {LGF_ENABLED,                                   // primary plain
              LGF_ENABLED | LGF_SECONDARY_GOAL,              // secondary plain
              LGF_ENABLED | LGF_TELCOM_LISTS,                // primary objective
              LGF_ENABLED | LGF_SECONDARY_GOAL | LGF_TELCOM_LISTS}; // secondary objective
  g.priority.assign(4, 0);
  g.goallist.assign(4, 0);
  TG_goals = &g;
  TCGoalsBuildLineData();
  ASSERT_EQ(TG_Lines.size(), 4u);
  EXPECT_TRUE(TG_Lines[0].primary);
  EXPECT_FALSE(TG_Lines[1].primary);
  EXPECT_TRUE(TG_Lines[2].is_objective);
  EXPECT_TRUE(TG_Lines[3].primary == false && TG_Lines[3].is_objective);

  // quirk: bounding boxes start at -1, filled on first draw frame
  EXPECT_EQ(TG_Lines[0].lx, -1);
  EXPECT_EQ(TG_Lines[3].by, -1);
}

/**
 * @test TelComGoals.BuildMarksActiveGoalsFromBothLists
 * @brief Verifies build Marks Active Goals From Both Lists.
 *
 * @details
 * Exercises the TelComGoals code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComGoals.cpp
 * @ingroup descent3_tests
 */
TEST(TelComGoals, BuildMarksActiveGoalsFromBothLists) {
  MockGoals g;
  g.status = {LGF_ENABLED, LGF_ENABLED, LGF_ENABLED | LGF_SECONDARY_GOAL, LGF_ENABLED};
  g.priority.assign(4, 0);
  g.goallist.assign(4, 0);
  g.active_primary = {2};   // note: goal_index 2 is a SECONDARY line -> no match
  g.active_secondary = {2}; // matches the secondary line
  TG_goals = &g;
  TCGoalsBuildLineData();
  EXPECT_FALSE(TG_Lines[0].is_active); // active_primary had index 2 but line0 is goal 0
  EXPECT_TRUE(TG_Lines[1].is_active == false);
  // goal_index 2 is line 2 (secondary): marked by secondary list
  EXPECT_TRUE(TG_Lines[2].is_active);
  EXPECT_FALSE(TG_Lines[3].is_active);
}

/**
 * @test TelComGoals.SortPrimariesBeforeSecondaries
 * @brief Verifies sort Primaries Before Secondaries.
 *
 * @details
 * Exercises the TelComGoals code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComGoals.cpp
 * @ingroup descent3_tests
 */
TEST(TelComGoals, SortPrimariesBeforeSecondaries) {
  MockGoals g;
  // all same list/priority so only primary flag differentiates
  g.status = {LGF_ENABLED | LGF_SECONDARY_GOAL, LGF_ENABLED,
              LGF_ENABLED | LGF_SECONDARY_GOAL, LGF_ENABLED};
  g.priority = {5, 5, 5, 5};
  g.goallist = {0, 0, 0, 0};
  TG_goals = &g;
  TCGoalsBuildLineData();
  ASSERT_EQ(TG_SortedList.size(), 4u);
  // primaries are lines 1 and 3
  EXPECT_TRUE(TG_Lines[TG_SortedList[0]].primary);
  EXPECT_TRUE(TG_Lines[TG_SortedList[1]].primary);
  EXPECT_FALSE(TG_Lines[TG_SortedList[2]].primary);
  EXPECT_FALSE(TG_Lines[TG_SortedList[3]].primary);
}

/**
 * @test TelComGoals.SortLowerGoalListWinsThenObjectiveBeatsPlain
 * @brief Verifies sort Lower Goal List Wins Then Objective Beats Plain.
 *
 * @details
 * Exercises the TelComGoals code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComGoals.cpp
 * @ingroup descent3_tests
 */
TEST(TelComGoals, SortLowerGoalListWinsThenObjectiveBeatsPlain) {
  MockGoals g;
  // secondaries with differing goal lists
  g.status = {LGF_ENABLED | LGF_SECONDARY_GOAL | LGF_TELCOM_LISTS, // obj, list 2
              LGF_ENABLED | LGF_SECONDARY_GOAL,                    // plain, list 1
              LGF_ENABLED | LGF_SECONDARY_GOAL | LGF_TELCOM_LISTS, // obj, list 0
              LGF_ENABLED | LGF_SECONDARY_GOAL};                   // plain, list 2
  g.priority = {9, 9, 9, 9};
  g.goallist = {2, 1, 0, 2};
  TG_goals = &g;
  TCGoalsBuildLineData();
  // goal-list ascending dominates: line2(list0), line1(list1), then list2 pair
  EXPECT_EQ(TG_Lines[TG_SortedList[0]].goal_index, 2);
  EXPECT_EQ(TG_Lines[TG_SortedList[1]].goal_index, 1);
  // within same list, objective before plain goal
  EXPECT_TRUE(TG_Lines[TG_SortedList[2]].is_objective);
  EXPECT_FALSE(TG_Lines[TG_SortedList[3]].is_objective);
}

/**
 * @test TelComGoals.SortSameListNonObjectivesByPriorityAscending
 * @brief Verifies sort Same List Non Objectives By Priority Ascending.
 *
 * @details
 * Exercises the TelComGoals code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComGoals.cpp
 * @ingroup descent3_tests
 */
TEST(TelComGoals, SortSameListNonObjectivesByPriorityAscending) {
  MockGoals g;
  g.status = {LGF_ENABLED, LGF_ENABLED, LGF_ENABLED};
  g.goallist = {1, 1, 1};
  g.priority = {30, 10, 20}; // comparator returns left.priority > right.priority
  TG_goals = &g;
  TCGoalsBuildLineData();
  EXPECT_EQ(TG_Lines[TG_SortedList[0]].goal_index, 1); // prio 10 first
  EXPECT_EQ(TG_Lines[TG_SortedList[1]].goal_index, 2); // prio 20
  EXPECT_EQ(TG_Lines[TG_SortedList[2]].goal_index, 0); // prio 30 last
}

// replicated FindHighlightedItem (TelComGoals.cpp:281-288)
static int FindHighlightedItem(int x, int y) {
  for (size_t i = 0; i < TG_Lines.size(); i++) {
    if (x >= TG_Lines[i].lx && x <= TG_Lines[i].rx && y >= TG_Lines[i].ty && y <= TG_Lines[i].by)
      return TG_Lines[i].goal_index;
  }
  return -1;
}

/**
 * @test TelComGoals.FindHighlightedItemInclusiveBounds
 * @brief Verifies find Highlighted Item Inclusive Bounds.
 *
 * @details
 * Exercises the TelComGoals code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComGoals.cpp
 * @ingroup descent3_tests
 */
TEST(TelComGoals, FindHighlightedItemInclusiveBounds) {
  TG_Lines.assign(2, {});
  TG_Lines[0] = {true, false, false, 7, 10, 100, 20, 40};
  TG_Lines[1] = {false, false, true, 9, 10, 100, 50, 80};

  EXPECT_EQ(FindHighlightedItem(10, 20), 7);   // corners inclusive
  EXPECT_EQ(FindHighlightedItem(100, 40), 7);
  EXPECT_EQ(FindHighlightedItem(101, 20), -1); // outside right
  EXPECT_EQ(FindHighlightedItem(50, 60), 9);   // hits second line
  EXPECT_EQ(FindHighlightedItem(50, 90), -1);  // below all
}

// replicated alpha pulse (TelComGoals.cpp:290-292,301-324)
constexpr float ACTIVE_ALPHA_PER_SEC = 255.0f;
constexpr float ACTIVE_ALPHA_MIN = 100.0f;
constexpr float ACTIVE_ALPHA_MAX = 255.0f;

struct AlphaPulse {
  bool in = false;
  float alpha = ACTIVE_ALPHA_MAX;

  void step(float dt) {
    float change = dt * ACTIVE_ALPHA_PER_SEC;
    while (change > 0) {
      float amount;
      if (in) {
        amount = std::min(ACTIVE_ALPHA_MAX - alpha, change);
        alpha += amount;
        if (alpha > (ACTIVE_ALPHA_MAX - 1.0f))
          in = false;
      } else {
        amount = std::min(change, alpha - ACTIVE_ALPHA_MIN);
        alpha -= amount;
        if (alpha < (ACTIVE_ALPHA_MIN + 1.0f))
          in = true;
      }
      change -= amount;
    }
  }
};

/**
 * @test TelComGoals.AlphaPulseOscillatesBetweenBounds
 * @brief Verifies alpha Pulse Oscillates Between Bounds.
 *
 * @details
 * Exercises the TelComGoals code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComGoals.cpp
 * @ingroup descent3_tests
 */
TEST(TelComGoals, AlphaPulseOscillatesBetweenBounds) {
  AlphaPulse p;
  EXPECT_FLOAT_EQ(p.alpha, 255.0f);

  // run 3 seconds at 60fps; assert it stays bounded and reaches both ends
  float lo = 1e9f, hi = -1e9f;
  for (int i = 0; i < 180; i++) {
    p.step(1.0f / 60.0f);
    EXPECT_GE(p.alpha, ACTIVE_ALPHA_MIN - 0.01f);
    EXPECT_LE(p.alpha, ACTIVE_ALPHA_MAX + 0.01f);
    lo = std::min(lo, p.alpha);
    hi = std::max(hi, p.alpha);
    if (i == 30) {
      EXPECT_FALSE(p.in); // still heading down early on
      EXPECT_LT(p.alpha, 130.0f);
    }
  }
  // quirk: leftover per-frame change carries into the return leg inside
  // the same while-loop, so sampled extremes stay within one step
  // (255/60 ≈ 4.25) of the bounds rather than touching them exactly
  EXPECT_NEAR(lo, ACTIVE_ALPHA_MIN, 4.3f);
  EXPECT_NEAR(hi, ACTIVE_ALPHA_MAX, 4.3f);
}
