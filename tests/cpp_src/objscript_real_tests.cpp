/**
 * @file objscript_real_tests.cpp
 * @brief Tests for ObjScript.cpp (241 lines).
 *
 * @details
 * Covers level script assignment/free sweeps and per-object Osiris event
 * sequencing (bind, EVT_AI_INIT/EVT_CREATED ordering, EVT_DESTROY payload),
 * using recording stubs for the Osiris DLL layer.
 *
 * This harness validates the behavior of `Descent3/ObjScript.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/ObjScript.cpp`
 * @par Harness
 * `objscript_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/ObjScript.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <vector>

#include "object.h"
#include "trigger.h"
#include "game.h"
#include "osiris_dll.h"
#include "Mission.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
object Objects[MAX_OBJECTS];
int Highest_object_index = 0;
// Triggers array is defined by trigger.cpp (not linked); declare + define here.
#define MAX_TRIGGERS 100
trigger Triggers[MAX_TRIGGERS];
int Num_triggers = 0;

// ---------------------------------------------------------------------------
// Recording Osiris layer (real osiris_dll.cpp not linked)
// ---------------------------------------------------------------------------
struct EventRec {
  object *obj;
  int event;
  int is_dying;
};
static std::vector<object *> g_binds;
static std::vector<object *> g_detaches;
static std::vector<EventRec> g_events;
static int g_free_level_calls;

bool Osiris_BindScriptsToObject(object *obj) {
  g_binds.push_back(obj);
  return true;
}
void Osiris_DetachScriptsFromObject(object *obj) { g_detaches.push_back(obj); }
bool Osiris_CallEvent(object *obj, int event, tOSIRISEventInfo *data) {
  g_events.push_back({obj, event, data ? data->evt_destroy.is_dying : -1});
  return true;
}
void FreeLevelScript() { g_free_level_calls++; }

/**
 * @brief GTest fixture for ObjScriptTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class ObjScriptTest : public ::testing::Test {
protected:
  void SetUp() override {
    memset(Objects, 0, sizeof(Objects));
    // Free slots are marked by type==OBJ_NONE (255), not by zeroed memory
    // (zero happens to be OBJ_WALL).
    for (auto &o : Objects)
      o.type = OBJ_NONE;
    Highest_object_index = 0;
    Num_triggers = 0;
    g_binds.clear();
    g_detaches.clear();
    g_events.clear();
    g_free_level_calls = 0;
  }

  static object *Slot(int i, uint8_t type, bool bound = false) {
    Objects[i].type = type;
    Objects[i].control_type = CT_NONE;
    Objects[i].osiris_script = bound ? (tOSIRISScript *)0x1 : nullptr;
    Highest_object_index = i > Highest_object_index ? i : Highest_object_index;
    return &Objects[i];
  }
};

// ---------------------------------------------------------------------------
// AssignScriptsForLevel
// ---------------------------------------------------------------------------

/**
 * @test ObjScriptTest.AssignBindsUnboundRealObjectsAndFiresCreated
 * @brief Verifies assign Binds Unbound Real Objects And Fires Created.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, AssignBindsUnboundRealObjectsAndFiresCreated) {
  Slot(0, OBJ_NONE);            // free slot: skipped
  object *b = Slot(1, OBJ_ROBOT);
  Slot(2, OBJ_POWERUP, /*bound*/ true); // already has a script: skipped
  object *d = Slot(3, OBJ_DOOR);

  AssignScriptsForLevel();

  // Assignment calls InitObjectScripts with the default do_evt_created=true,
  // so every freshly bound object also receives EVT_CREATED.
  ASSERT_EQ(g_binds.size(), 2u);
  EXPECT_EQ(g_binds[0], b);
  EXPECT_EQ(g_binds[1], d);
  ASSERT_EQ(g_events.size(), 2u);
  EXPECT_EQ(g_events[0].obj, b);
  EXPECT_EQ(g_events[0].event, EVT_CREATED);
  EXPECT_EQ(g_events[1].obj, d);
}

/**
 * @test ObjScriptTest.AssignSweepsUpToHighestIndexOnly
 * @brief Verifies assign Sweeps Up To Highest Index Only.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, AssignSweepsUpToHighestIndexOnly) {
  Slot(5, OBJ_CLUTTER); // highest is 5; slots 0-4 are OBJ_NONE

  AssignScriptsForLevel();
  EXPECT_EQ(g_binds.size(), 1u);
}

/**
 * @test ObjScriptTest.TriggerLoopsAreNoopsButObjectSweepStillRuns
 * @brief Verifies trigger Loops Are Noops But Object Sweep Still Runs.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, TriggerLoopsAreNoopsButObjectSweepStillRuns) {
  // InitTriggerScript/FreeTriggerScript are empty stubs in ObjScript.cpp,
  // so trigger presence never reaches the Osiris layer.
  Num_triggers = 3;

  AssignScriptsForLevel();
  EXPECT_TRUE(g_binds.empty());

  FreeScriptsForLevel();
  // Quirk: FreeScriptsForLevel sweeps objects 0..Highest WITHOUT skipping
  // OBJ_NONE slots - the single free slot still gets detached.
  ASSERT_EQ(g_detaches.size(), 1u);
  EXPECT_TRUE(g_events.empty());
  EXPECT_EQ(g_free_level_calls, 1);
}

// ---------------------------------------------------------------------------
// InitObjectScripts
// ---------------------------------------------------------------------------

/**
 * @test ObjScriptTest.InitFiresCreatedForPlainObject
 * @brief Verifies init Fires Created For Plain Object.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, InitFiresCreatedForPlainObject) {
  object *o = Slot(4, OBJ_POWERUP);

  InitObjectScripts(o);

  ASSERT_EQ(g_binds.size(), 1u);
  ASSERT_EQ(g_events.size(), 1u);
  EXPECT_EQ(g_events[0].event, EVT_CREATED);
  EXPECT_EQ(g_events[0].obj, o);
}

/**
 * @test ObjScriptTest.InitAIFiresAIInitBeforeCreated
 * @brief Verifies init AIFires AIInit Before Created.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, InitAIFiresAIInitBeforeCreated) {
  object *o = Slot(2, OBJ_ROBOT);
  o->control_type = CT_AI;

  InitObjectScripts(o);

  ASSERT_EQ(g_events.size(), 2u);
  EXPECT_EQ(g_events[0].event, EVT_AI_INIT);
  EXPECT_EQ(g_events[1].event, EVT_CREATED);
}

/**
 * @test ObjScriptTest.InitWithoutCreatedEventSkipsEvents
 * @brief Verifies init Without Created Event Skips Events.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, InitWithoutCreatedEventSkipsEvents) {
  object *o = Slot(1, OBJ_ROBOT);

  InitObjectScripts(o, /*do_evt_created*/ false);

  EXPECT_EQ(g_binds.size(), 1u);
  EXPECT_TRUE(g_events.empty());
}

// ---------------------------------------------------------------------------
// FreeObjectScripts / FreeScriptsForLevel
// ---------------------------------------------------------------------------

/**
 * @test ObjScriptTest.MidLevelFreeSendsDestroyWithInvertedDyingFlag
 * @brief Verifies mid Level Free Sends Destroy With Inverted Dying Flag.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, MidLevelFreeSendsDestroyWithInvertedDyingFlag) {
  object *o = Slot(3, OBJ_ROBOT);

  FreeObjectScripts(o, /*level_end*/ false);

  ASSERT_EQ(g_events.size(), 1u);
  EXPECT_EQ(g_events[0].event, EVT_DESTROY);
  // Quirk: tOSIRISEVTDESTROY documents is_dying!=0 as "really being
  // destroyed", but the code passes level_end?1:0 - a genuine mid-level
  // destroy arrives flagged as if caused by a level end.
  EXPECT_EQ(g_events[0].is_dying, 0);
  ASSERT_EQ(g_detaches.size(), 1u);
  EXPECT_EQ(g_detaches[0], o);
}

/**
 * @test ObjScriptTest.LevelEndFreeDetachesWithoutDestroyEvent
 * @brief Verifies level End Free Detaches Without Destroy Event.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, LevelEndFreeDetachesWithoutDestroyEvent) {
  object *o = Slot(3, OBJ_ROBOT);

  FreeObjectScripts(o, /*level_end*/ true);

  EXPECT_TRUE(g_events.empty()); // no EVT_DESTROY on level teardown
  EXPECT_EQ(g_detaches.size(), 1u);
}

/**
 * @test ObjScriptTest.LevelFreeSweepsEverySlotAndClearsLevelScript
 * @brief Verifies level Free Sweeps Every Slot And Clears Level Script.
 *
 * @details
 * Exercises the ObjScriptTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ObjScript.cpp
 * @ingroup descent3_tests
 */
TEST_F(ObjScriptTest, LevelFreeSweepsEverySlotAndClearsLevelScript) {
  Slot(0, OBJ_WALL);   // note: sweep does NOT skip free/none slots
  Slot(1, OBJ_ROBOT);
  Highest_object_index = 4; // slots 2..4 never touched -> still swept

  FreeScriptsForLevel();

  EXPECT_EQ(g_detaches.size(), 5u); // Highest_object_index + 1
  EXPECT_EQ(g_free_level_calls, 1);
}
