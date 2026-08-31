/**
 * @file trigger_real_tests.cpp
 * @brief Tests for trigger.cpp — trigger array (MAX_TRIGGERS 100, ~150 lines non-editor).
 *
 * @details
 * Covers InitTriggers, FindTrigger, TriggerSetState/GetState with stubbed Rooms.
 *
 * This harness validates the behavior of `Descent3/trigger.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/trigger.cpp`
 * @par Harness
 * `trigger_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/trigger.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include "room.h"
#include "trigger.h"
#include "levelgoal.h"

extern trigger *FindTrigger(int roomnum, int facenum);

// Provide definitions for externs referenced by trigger.cpp
room Rooms[2];
levelgoals Level_goals;
void levelgoals::Inform(char a, int b, int c) { (void)a; (void)b; (void)c; }

// Stub FreeTriggerScript(trigger*,bool) — used by FreeTriggers/DisableTriggers
void FreeTriggerScript(trigger *tp, bool level_end) { (void)tp; (void)level_end; }
// Stub Osiris_CallTriggerEvent — used by CheckTrigger (not exercised)
struct tOSIRISEventInfo;
void Osiris_CallTriggerEvent(int a, int b, tOSIRISEventInfo *c) { (void)a; (void)b; (void)c; }
object *ObjGetUltimateParent(object *obj) { return obj; }

/**
 * @test Trigger.InitClears
 * @brief Verifies init Clears.
 *
 * @details
 * Exercises the Trigger code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/trigger.cpp
 * @ingroup descent3_tests
 */
TEST(Trigger, InitClears) {
  InitTriggers();
  EXPECT_EQ(Num_triggers, 0);
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(Triggers[i].roomnum, -1);
    EXPECT_EQ(Triggers[i].facenum, -1);
  }
  // Find on empty should return nullptr (Int3 disabled via RELEASE)
  EXPECT_EQ(FindTrigger(0, 0), nullptr);
}

/**
 * @test Trigger.SetStateToggles
 * @brief Verifies set State Toggles.
 *
 * @details
 * Exercises the Trigger code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/trigger.cpp
 * @ingroup descent3_tests
 */
TEST(Trigger, SetStateToggles) {
  InitTriggers();
  // Manually insert a trigger for testing (AddTrigger is EDITOR only)
  Triggers[0].roomnum = 0;
  Triggers[0].facenum = 0;
  Triggers[0].flags = 0;
  Num_triggers = 1;
  EXPECT_TRUE(TriggerGetState(0));
  TriggerSetState(0, false);
  EXPECT_FALSE(TriggerGetState(0));
  EXPECT_TRUE(Triggers[0].flags & TF_DISABLED);
  TriggerSetState(0, true);
  EXPECT_TRUE(TriggerGetState(0));
  EXPECT_FALSE(Triggers[0].flags & TF_DISABLED);
}

/**
 * @test Trigger.FindExisting
 * @brief Verifies find Existing.
 *
 * @details
 * Exercises the Trigger code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/trigger.cpp
 * @ingroup descent3_tests
 */
TEST(Trigger, FindExisting) {
  InitTriggers();
  Triggers[0].roomnum = 1;
  Triggers[0].facenum = 2;
  Triggers[0].flags = 0;
  Triggers[1].roomnum = 3;
  Triggers[1].facenum = 4;
  Num_triggers = 2;
  trigger *tp = FindTrigger(1, 2);
  ASSERT_NE(tp, nullptr);
  EXPECT_EQ(tp - Triggers, 0);
  tp = FindTrigger(3, 4);
  ASSERT_NE(tp, nullptr);
  EXPECT_EQ(tp - Triggers, 1);
  EXPECT_EQ(FindTrigger(9, 9), nullptr);
}

/**
 * @test Trigger.FreeClears
 * @brief Verifies free Clears.
 *
 * @details
 * Exercises the Trigger code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/trigger.cpp
 * @ingroup descent3_tests
 */
TEST(Trigger, FreeClears) {
  InitTriggers();
  Triggers[0].roomnum = 0;
  Triggers[0].facenum = 0;
  Triggers[0].flags = TF_DISABLED;
  Num_triggers = 1;
  FreeTriggers();
  EXPECT_EQ(Num_triggers, 0);
  // FreeTriggers only resets Num_triggers and frees scripts; slots remain
  EXPECT_EQ(Triggers[0].roomnum, 0);
}
