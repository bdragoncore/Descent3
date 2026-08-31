/**
 * @file gameevent_real_tests.cpp
 * @brief Tests for gameevent.cpp — event allocation/handling (247 lines, array pool).
 *
 * @details
 * Covers InitEvents, AllocEvent/FreeEvent pool, CreateNewEvent, FindEventID, ClearAllEvents.
 * Stubs: Gametime, FrameCount, Render_zoom, SpewInit/Emit, ObjGet, Players, etc.
 *
 * This harness validates the behavior of `Descent3/gameevent.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gameevent.cpp`
 * @par Harness
 * `gameevent_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gameevent.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include "gameevent.h"
#include "object.h"
#include "player.h"

// Provided by Descent3/gameevent.cpp: already defined, but we need to stub globals it uses
float Gametime = 0.0f;
int FrameCount = 0;
float Render_zoom = 1.0f;

// Spew stubs (Descent3/spew.h)
void SpewInit() {}
void SpewEmitAll() {}

// ObjGet stub — return nullptr for OBJECT_HANDLE_NONE, otherwise dummy
object *ObjGet(int handle) {
  (void)handle;
  return nullptr;
}

// Players stub for detonator check — not needed for non-detonator events, but satisfy linker
player Players[MAX_PLAYERS];
// Objects array needed by some headers but not used directly here; define minimal
object Objects[MAX_OBJECTS];
int Highest_object_index = -1;

// Inventory stubs needed for player constructor
Inventory::Inventory() {}
Inventory::~Inventory() {}

// Provide dummy for log? Already via logger.
// Provide pserror Int3 stub via pserror.h when RELEASE defined (we compile with RELEASE).

// Need to declare HandleEvent behavior: test that subfunction is called
static int g_callback_calls = 0;
static int g_last_eventnum = -1;
static void TestSubfunction(int eventnum, void *data) {
  g_callback_calls++;
  g_last_eventnum = eventnum;
  (void)data;
}

/**
 * @brief GTest fixture for GameEventTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class GameEventTest : public ::testing::Test {
protected:
  void SetUp() override {
    ClearAllEvents();
    g_callback_calls = 0;
    g_last_eventnum = -1;
    Gametime = 0.0f;
    FrameCount = 0;
  }
};

/**
 * @test GameEventTest.InitAndAllocSingle
 * @brief Verifies init And Alloc Single.
 *
 * @details
 * Exercises the GameEventTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gameevent.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameEventTest, InitAndAllocSingle) {
  // Clear ensures all free
  EXPECT_EQ(Num_events, 0);
  int idx = CreateNewEvent(OBJECT_EVENT, 5, 10.0f, nullptr, 0, TestSubfunction);
  ASSERT_NE(idx, -1);
  EXPECT_EQ(Num_events, 1);
  EXPECT_EQ(GameEvent[idx].used, 1);
  EXPECT_EQ(GameEvent[idx].type, OBJECT_EVENT);
  EXPECT_EQ(GameEvent[idx].id, 5);
  EXPECT_FLOAT_EQ(GameEvent[idx].end_time, Gametime + 10.0f);
  FreeEvent(idx);
  EXPECT_EQ(Num_events, 0);
  EXPECT_EQ(GameEvent[idx].used, 0);
}

/**
 * @test GameEventTest.AllocExhaustionReturnsMinusOne
 * @brief Verifies alloc Exhaustion Returns Minus One.
 *
 * @details
 * Exercises the GameEventTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gameevent.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameEventTest, AllocExhaustionReturnsMinusOne) {
  // Fill up
  int ids[MAX_EVENTS];
  for (int i=0;i<MAX_EVENTS;i++) {
    ids[i]= CreateNewEvent(RENDER_EVENT, i, 1.0f, nullptr, 0, TestSubfunction);
    ASSERT_NE(ids[i], -1);
  }
  EXPECT_EQ(Num_events, MAX_EVENTS);
  EXPECT_EQ(CreateNewEvent(OBJECT_EVENT, 99, 1.0f, nullptr, 0, TestSubfunction), -1);
  // Free one and alloc again should reuse
  FreeEvent(ids[0]);
  EXPECT_EQ(Num_events, MAX_EVENTS-1);
  int nid = CreateNewEvent(OBJECT_EVENT, 999, 1.0f, nullptr, 0, TestSubfunction);
  EXPECT_NE(nid, -1);
  EXPECT_EQ(Num_events, MAX_EVENTS);
  ClearAllEvents();
  EXPECT_EQ(Num_events, 0);
}

/**
 * @test GameEventTest.CreateNewEventCopiesData
 * @brief Verifies create New Event Copies Data.
 *
 * @details
 * Exercises the GameEventTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gameevent.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameEventTest, CreateNewEventCopiesData) {
  int payload = 42;
  int idx = CreateNewEvent(OBJECT_EVENT, 7, 5.0f, &payload, sizeof(payload), TestSubfunction);
  ASSERT_NE(idx, -1);
  ASSERT_NE(GameEvent[idx].data, nullptr);
  EXPECT_EQ(*(int*)GameEvent[idx].data, 42);
  FreeEvent(idx);
}

/**
 * @test GameEventTest.FindEventID
 * @brief Verifies find Event ID.
 *
 * @details
 * Exercises the GameEventTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gameevent.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameEventTest, FindEventID) {
  int a = CreateNewEvent(OBJECT_EVENT, 11, 1.0f, nullptr, 0, TestSubfunction);
  int b = CreateNewEvent(OBJECT_EVENT, 22, 1.0f, nullptr, 0, TestSubfunction);
  EXPECT_EQ(FindEventID(11), a);
  EXPECT_EQ(FindEventID(22), b);
  EXPECT_EQ(FindEventID(999), -1);
  FreeEvent(a);
  EXPECT_EQ(FindEventID(11), -1);
  ClearAllEvents();
}

/**
 * @test GameEventTest.ProcessNormalEventsFiresAtEndTime
 * @brief Verifies process Normal Events Fires At End Time.
 *
 * @details
 * Exercises the GameEventTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gameevent.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameEventTest, ProcessNormalEventsFiresAtEndTime) {
  Gametime = 0.0f;
  int idx = CreateNewEvent(OBJECT_EVENT, 33, 5.0f, nullptr, 0, TestSubfunction);
  ASSERT_NE(idx, -1);
  // Before end_time, not fired
  Gametime = 4.0f;
  ProcessNormalEvents();
  EXPECT_EQ(g_callback_calls, 0);
  EXPECT_EQ(Num_events, 1);
  // At end_time, should fire and free
  Gametime = 5.0f;
  ProcessNormalEvents();
  EXPECT_EQ(g_callback_calls, 1);
  EXPECT_EQ(g_last_eventnum, idx);
  EXPECT_EQ(Num_events, 0);
}

/**
 * @test GameEventTest.ProcessRenderEventsOnlyFiresRenderType
 * @brief Verifies process Render Events Only Fires Render Type.
 *
 * @details
 * Exercises the GameEventTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gameevent.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameEventTest, ProcessRenderEventsOnlyFiresRenderType) {
  Gametime = 0.0f;
  int o = CreateNewEvent(OBJECT_EVENT, 1, 0.0f, nullptr, 0, TestSubfunction);
  int r = CreateNewEvent(RENDER_EVENT, 2, 0.0f, nullptr, 0, TestSubfunction);
  Gametime = 0.0f;
  ProcessNormalEvents(); // should fire object, not render
  EXPECT_EQ(g_callback_calls, 1);
  EXPECT_EQ(Num_events, 1); // render remains
  g_callback_calls = 0;
  ProcessRenderEvents();
  EXPECT_EQ(g_callback_calls, 1);
  EXPECT_EQ(Num_events, 0);
  (void)o; (void)r;
}

/**
 * @test GameEventTest.DetonatorCancelsBeforeFire
 * @brief Verifies detonator Cancels Before Fire.
 *
 * @details
 * Exercises the GameEventTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gameevent.cpp
 * @ingroup descent3_tests
 */
TEST_F(GameEventTest, DetonatorCancelsBeforeFire) {
  // Create event with detonator handle 123; ObjGet returns nullptr -> should cancel
  Gametime = 0.0f;
  int idx = CreateNewEvent(OBJECT_EVENT, 50, 10.0f, nullptr, 0, TestSubfunction, 123);
  ASSERT_NE(idx, -1);
  // ProcessNormalEvents should detect detonator dead and free without calling subfunction
  ProcessNormalEvents();
  EXPECT_EQ(g_callback_calls, 0);
  EXPECT_EQ(Num_events, 0);
  EXPECT_EQ(GameEvent[idx].used, 0);
}
