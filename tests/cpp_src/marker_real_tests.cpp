/**
 * @file marker_real_tests.cpp
 * @brief Tests for marker.cpp — marker array (MAX_PLAYERS*2 messages, trivial init).
 *
 * @details
 * Covers InitMarkers, ResetMarkers.
 *
 * This harness validates the behavior of `Descent3/marker.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/marker.cpp`
 * @par Harness
 * `marker_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/marker.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>

// Pull headers that define vector/matrix/object/player so ObjCreate signature matches
#include "object.h"
#include "player.h"
#include "Inventory.h"
Inventory::Inventory() {}
Inventory::~Inventory() {}

// Provide definitions for externs referenced by marker.cpp
object Objects[10];
player Players[32];
object *Player_object = &Objects[0];
int Highest_object_index = 0;
int Game_mode = 0;
int Player_num = 0;

// Stub LoadPolyModel used by InitMarkers
int LoadPolyModel(const std::filesystem::path &filename, int flags) { (void)filename; (void)flags; return 1; }

// Stub Multi
void MultiSendRequestForMarker(char *msg) { (void)msg; }

// Stub ObjCreate/ObjDelete not called in Init/Reset but linked
int ObjCreate(uint8_t type, uint16_t id, int roomnum, vector *pos, const matrix *orient, int parent_handle) {
  (void)type; (void)id; (void)roomnum; (void)pos; (void)orient; (void)parent_handle; return -1; }
void ObjDelete(int objnum) { (void)objnum; }

// Forward declare marker API
extern int Marker_polynum;
extern int Marker_message;
extern char MarkerMessages[][40];
void ResetMarkers();
void InitMarkers();

/**
 * @test Marker.ResetClearsMessage
 * @brief Verifies reset Clears Message.
 *
 * @details
 * Exercises the Marker code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/marker.cpp
 * @ingroup descent3_tests
 */
TEST(Marker, ResetClearsMessage) {
  Marker_message = 7;
  ResetMarkers();
  EXPECT_EQ(Marker_message, 0);
}

/**
 * @test Marker.InitLoadsModel
 * @brief Verifies init Loads Model.
 *
 * @details
 * Exercises the Marker code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/marker.cpp
 * @ingroup descent3_tests
 */
TEST(Marker, InitLoadsModel) {
  Marker_polynum = -1;
  InitMarkers();
  EXPECT_GE(Marker_polynum, 0);
  // MarkerMessages should be accessible (64x40)
  strcpy(MarkerMessages[0], "test");
  EXPECT_STREQ(MarkerMessages[0], "test");
  ResetMarkers();
  EXPECT_EQ(Marker_message, 0);
}
