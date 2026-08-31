/**
 * @file aipath_real_tests.cpp
 * @brief Tests for aipath.cpp 1189 lines — the AI path walker. The REAL.
 *
 * @details
 * source is compiled in; the BOA graph, room table, and goal hooks are
 * stubbed so only the path bookkeeping runs. Node iteration is driven
 * via the public Get/At/Set wrappers since MoveToNext/PrevNode and the
 * dynamic-slot allocator are file-static.
 *
 * This harness validates the behavior of `Descent3/aipath.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/aipath.cpp`
 * @par Harness
 * `aipath_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/aipath.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "aipath.h"
#include "aistruct.h"
#include "gamepath.h"
#include "BOA.h"
#include "bnode.h"
#include "AIMain.h"
#include "terrain.h"
#include "findintersection.h"

// ---- definitions for globals aipath.o references ----
game_path GamePaths[MAX_GAME_PATHS];
object Objects[MAX_OBJECTS];
room Rooms[MAX_ROOMS];
int Highest_room_index = 0;
terrain_segment Terrain_seg[(TERRAIN_WIDTH + 1) * (TERRAIN_DEPTH + 1)];
float BOA_cost_array[MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS][MAX_PATH_PORTALS];
uint16_t BOA_Array[MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS][MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS];
int BOA_num_connect[MAX_BOA_TERRAIN_REGIONS];
connect_data BOA_connect[MAX_BOA_TERRAIN_REGIONS][MAX_PATH_PORTALS];
bool BNode_allocated = false;
bool BNode_verified = false;
int BNode_Path[MAX_BNODES_PER_ROOM];
int BNode_PathNumNodes = 0;
bool AI_debug_robot_do = false;
int AI_debug_robot_index = 0;

object *ObjGet(int handle) { return nullptr; }
void GoalPathComplete(object *obj) {}
int32_t ps_rand(void) { return 42; }

// BOA / BNode / terrain / movement hooks — not under test
bool BOA_PassablePortal(int roomnum, int portal, bool add_door, bool flag) { return false; }
int BOA_DetermineStartRoomPortal(int roomnum, vector *pos, int exclude_room, vector *exclude_pos, bool f_landing,
                                 bool f_permissive, int *priority) {
  return 0;
}
int BOA_GetNextRoom(int roomnum, int portal) { return -1; }
bool BOA_HasPossibleBlockage(int i, int j) { return false; }
bool BOA_LockedDoor(object *obj, int roomnum) { return false; }
bn_list *BNode_GetBNListPtr(int roomnum, bool f_in_load_level) { return nullptr; }
int BNode_FindClosestLocalVisibleBNode(int roomnum, vector *pos, float rad) { return -1; }
int BNode_FindDirLocalVisibleBNode(int roomnum, vector *start_pos, vector *dest_pos, float rad) { return -1; }
bool BNode_FindPath(int start_room, int start_node, int end_room, float rad) { return false; }
int GetTerrainCellFromPos(vector *pos) { return -1; }
int fvi_FindIntersection(fvi_query *fq, fvi_info *hit_data, bool f_ignore_objvis) { return HIT_NONE; }
scalar vm_NormalizeVector(vector *vec) { return 0.0f; }
void mem_free_sub(void *memblock) { free(memblock); }
bool AIMoveTowardsPosition(object *obj, vector *goal_point_ptr, float delta_t, bool f_turn, vector *mdir,
                           bool *f_moved) {
  if (f_moved)
    *f_moved = false;
  return false;
}

// public in aipath.cpp but missing from aipath.h
void AIPathSetAtStart(ai_path_info *aip);

/**
 * @test AIPathWalker.InitResetsAndAtStartEndReflectState
 * @brief Verifies init Resets And At Start End Reflect State.
 *
 * @details
 * Exercises the AIPathWalker code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aipath.cpp
 * @ingroup descent3_tests
 */
TEST(AIPathWalker, InitResetsAndAtStartEndReflectState) {
  ai_path_info aip;
  memset(&aip, 0, sizeof(aip));

  ASSERT_TRUE(AIPathInitPath(&aip));
  EXPECT_EQ(aip.num_paths, 0);
  EXPECT_EQ(aip.goal_index, -1);
  EXPECT_EQ(aip.goal_uid, -1);

  // empty path: AtStart true (cur 0/0 == start_node[0] zeroed), AtEnd false
  EXPECT_TRUE(AIPathAtStart(&aip));
  EXPECT_FALSE(AIPathAtEnd(&aip));

  // fabricate a two-segment joined path
  aip.num_paths = 2;
  aip.path_type[0] = AIP_STATIC;
  aip.path_id[0] = 3;
  aip.path_start_node[0] = 0;
  aip.path_end_node[0] = 3;
  aip.path_type[1] = AIP_STATIC;
  aip.path_id[1] = 8;
  aip.path_start_node[1] = 1;
  aip.path_end_node[1] = 4;

  // node lookups need real backing storage
  static node seg_a_nodes[4];
  static node seg_b_nodes[5];
  GamePaths[3].pathnodes = seg_a_nodes;
  GamePaths[8].pathnodes = seg_b_nodes;

  AIPathSetAtStart(&aip);
  EXPECT_EQ(aip.cur_path, 0);
  EXPECT_EQ(aip.cur_node, 0);
  EXPECT_TRUE(AIPathAtStart(&aip));
  EXPECT_FALSE(AIPathAtEnd(&aip));

  // mid-path: neither bound holds
  aip.cur_path = 0;
  aip.cur_node = 2;
  EXPECT_FALSE(AIPathAtStart(&aip));
  EXPECT_FALSE(AIPathAtEnd(&aip));

  // parked on the final node of the final segment
  aip.cur_path = 1;
  aip.cur_node = 4;
  EXPECT_TRUE(AIPathAtEnd(&aip));
}

/**
 * @test AIPathWalker.PeeksLookAheadBehindWithoutMovingCursor
 * @brief Verifies peeks Look Ahead Behind Without Moving Cursor.
 *
 * @details
 * Exercises the AIPathWalker code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aipath.cpp
 * @ingroup descent3_tests
 */
TEST(AIPathWalker, PeeksLookAheadBehindWithoutMovingCursor) {
  ai_path_info aip;
  memset(&aip, 0, sizeof(aip));
  aip.num_paths = 2;
  aip.path_type[0] = AIP_STATIC;
  aip.path_id[0] = 7;
  aip.path_start_node[0] = 0;
  aip.path_end_node[0] = 3;
  aip.path_type[1] = AIP_STATIC;
  aip.path_id[1] = 9;
  aip.path_start_node[1] = 1;
  aip.path_end_node[1] = 4;

  static node path7_nodes[4];
  static node path9_nodes[5];
  GamePaths[7].pathnodes = path7_nodes;
  GamePaths[9].pathnodes = path9_nodes;
  for (int i = 0; i < 4; i++) {
    path7_nodes[i].pos = {10.0f + (float)i, 0, 0};
    path7_nodes[i].roomnum = 100 + i;
  }
  for (int i = 0; i < 5; i++) {
    path9_nodes[i].pos = {30.0f + (float)i, 0, 0};
    path9_nodes[i].roomnum = 200 + i;
  }

  vector pos;
  int room = -1;

  // from the very start: next peek sees node (path0,node1)
  AIPathSetAtStart(&aip);
  ASSERT_TRUE(AIPathGetNextNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 11.0f);
  EXPECT_EQ(room, 101);
  EXPECT_EQ(aip.cur_path, 0); // cursor untouched
  EXPECT_EQ(aip.cur_node, 0);

  // repeated peeks are idempotent — nothing ever consumes the path
  ASSERT_TRUE(AIPathGetNextNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 11.0f);

  // before start: prev peek has nowhere to go
  EXPECT_FALSE(AIPathGetPrevNodePos(&aip, &pos, &room));
  EXPECT_EQ(aip.cur_node, 0);

  // mid-segment peeks read neighbors symmetrically without moving
  aip.cur_node = 2;
  ASSERT_TRUE(AIPathGetNextNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 13.0f);
  EXPECT_EQ(room, 103);
  ASSERT_TRUE(AIPathGetPrevNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 11.0f);
  EXPECT_EQ(room, 101);
  EXPECT_EQ(aip.cur_node, 2);

  // QUIRK: the undo half of a peek is only intra-segment. Sitting on
  // the last node of segment 0, the next peek resolves through segment
  // 1's start node but leaves the cursor parked on segment 1's FIRST
  // node (the reverse step decrements within the new segment)
  aip.cur_node = 3;
  ASSERT_TRUE(AIPathGetNextNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 31.0f); // GamePaths[9].nodes[start_node[1]=1]
  EXPECT_EQ(room, 201);
  EXPECT_EQ(aip.cur_path, 1); // drifted into segment 1...
  EXPECT_EQ(aip.cur_node, 0); // ...to its first node

  // past the end: next peek fails
  aip.cur_path = 1;
  aip.cur_node = 4;
  EXPECT_FALSE(AIPathGetNextNodePos(&aip, &pos, &room));

  // prev peek one node into the last segment stays inside it
  aip.cur_path = 1;
  aip.cur_node = 1;
  ASSERT_TRUE(AIPathGetPrevNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 30.0f);
  EXPECT_EQ(room, 200);
  EXPECT_EQ(aip.cur_path, 1);
  EXPECT_EQ(aip.cur_node, 1);
}

/**
 * @test AIPathWalker.BackwardCrossingQuirksSegmentZeroAndDynamicTailJump
 * @brief Verifies backward Crossing Quirks Segment Zero And Dynamic Tail Jump.
 *
 * @details
 * Exercises the AIPathWalker code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aipath.cpp
 * @ingroup descent3_tests
 */
TEST(AIPathWalker, BackwardCrossingQuirksSegmentZeroAndDynamicTailJump) {
  ai_path_info aip;
  memset(&aip, 0, sizeof(aip));

  // QUIRK: MoveToPrevNode guards with `c_path > 0`, so stepping back out
  // of segment 1 into segment 0 is refused — the asymmetric twin of the
  // forward walk which happily enters the last segment
  aip.num_paths = 2;
  aip.path_type[0] = AIP_STATIC;
  aip.path_id[0] = 7;
  aip.path_start_node[0] = 0;
  aip.path_end_node[0] = 3;
  aip.path_type[1] = AIP_STATIC;
  aip.path_id[1] = 9;
  aip.path_start_node[1] = 1;
  aip.path_end_node[1] = 4;

  static node path7_nodes[4];
  static node path9_nodes[5];
  GamePaths[7].pathnodes = path7_nodes;
  GamePaths[9].pathnodes = path9_nodes;

  vector pos;
  aip.cur_path = 1;
  aip.cur_node = 0; // first node of segment 1
  EXPECT_FALSE(AIPathGetPrevNodePos(&aip, &pos, nullptr));
  EXPECT_EQ(aip.cur_node, 0); // refused, cursor intact

  // three segments: crossing 2->1 works (c_path==1 > 0) and lands on
  // the END node of segment 1 for static paths
  aip.num_paths = 3;
  aip.path_type[2] = AIP_STATIC;
  aip.path_id[2] = 5;
  aip.path_start_node[2] = 0;
  aip.path_end_node[2] = 2;
  static node path5_nodes[3];
  GamePaths[5].pathnodes = path5_nodes;
  path9_nodes[4].pos = {99.0f, 0, 0}; // end node of segment 1
  path9_nodes[4].roomnum = 204;

  aip.cur_path = 2;
  aip.cur_node = 0;
  int room = -1;
  // the peek reads segment 1's tail node, and the round-trip restore
  // (back to its end node, forward again) lands back on (2,0)
  ASSERT_TRUE(AIPathGetPrevNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 99.0f);
  EXPECT_EQ(room, 204); // GamePaths[9].pathnodes[4].roomnum
  EXPECT_EQ(aip.cur_path, 2);
  EXPECT_EQ(aip.cur_node, 0);

  // same crossing with a DYNAMIC middle segment jumps to its LAST node
  aip.path_type[1] = AIP_DYNAMIC;
  aip.path_id[1] = 4;
  AIDynamicPath[4].use_count = 1;
  AIDynamicPath[4].num_nodes = 6;
  AIDynamicPath[4].pos[5] = {77.0f, 0, 0};
  AIDynamicPath[4].roomnum[5] = 55;

  aip.cur_path = 2;
  aip.cur_node = 0;
  room = -1;
  ASSERT_TRUE(AIPathGetPrevNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 77.0f);
  EXPECT_EQ(room, 55);
  // restore uses the stale static path_end_node (=4) for the dynamic
  // segment: 5 <= 4 is false so the cursor crosses forward to (2,0)
  EXPECT_EQ(aip.cur_path, 2);
  EXPECT_EQ(aip.cur_node, 0);
}

/**
 * @test AIPathWalker.DynamicAndUnknownTypesResolveThroughTables
 * @brief Verifies dynamic And Unknown Types Resolve Through Tables.
 *
 * @details
 * Exercises the AIPathWalker code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aipath.cpp
 * @ingroup descent3_tests
 */
TEST(AIPathWalker, DynamicAndUnknownTypesResolveThroughTables) {
  ai_path_info aip;
  memset(&aip, 0, sizeof(aip));

  // dynamic segment resolves through the AIDynamicPath table
  AIDynamicPath[2].use_count = 1;
  AIDynamicPath[2].num_nodes = 3;
  AIDynamicPath[2].pos[1] = {9, 8, 7};
  AIDynamicPath[2].roomnum[1] = 42;

  aip.num_paths = 1;
  aip.path_type[0] = AIP_DYNAMIC;
  aip.path_id[0] = 2;
  aip.cur_node = 1; // GetCurrentNodePos reads raw cur_node
  aip.path_start_node[0] = 1;
  aip.path_end_node[0] = 1;

  vector pos;
  int room = -1;
  EXPECT_TRUE(AIPathGetCurrentNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 9.0f);
  EXPECT_FLOAT_EQ(pos.z(), 7.0f);
  EXPECT_EQ(room, 42);

  // unknown type reports failure without touching outputs
  aip.path_type[0] = 200;
  pos = {0, 0, 0};
  room = -1;
  EXPECT_FALSE(AIPathGetCurrentNodePos(&aip, &pos, &room));
  EXPECT_FLOAT_EQ(pos.x(), 0.0f);
  EXPECT_EQ(room, -1);

  // null room pointer is legal
  aip.path_type[0] = AIP_STATIC;
  aip.path_id[0] = 7;
  aip.cur_node = 0;
  static node path7_nodes[1];
  GamePaths[7].pathnodes = path7_nodes;
  path7_nodes[0].pos = {6, 5, 4};
  path7_nodes[0].roomnum = 99;
  EXPECT_TRUE(AIPathGetCurrentNodePos(&aip, &pos, nullptr));
  EXPECT_FLOAT_EQ(pos.y(), 5.0f);
}

/**
 * @test AIPathWalker.FreePathReleasesDynamicSlotsRefcountedThenResets
 * @brief Verifies free Path Releases Dynamic Slots Refcounted Then Resets.
 *
 * @details
 * Exercises the AIPathWalker code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aipath.cpp
 * @ingroup descent3_tests
 */
TEST(AIPathWalker, FreePathReleasesDynamicSlotsRefcountedThenResets) {
  // two joined structs share dynamic slot 3; refcount drops per FreePath,
  // slot data only resets when it hits zero
  ai_path_info first, second;
  memset(&first, 0, sizeof(first));
  memset(&second, 0, sizeof(second));

  AIDynamicPath[3].use_count = 2;
  AIDynamicPath[3].owner_handle = 77;
  AIDynamicPath[3].num_nodes = 5;

  first.num_paths = 2;
  first.path_type[0] = AIP_DYNAMIC;
  first.path_id[0] = 3;
  first.path_type[1] = AIP_STATIC; // static segments never touch slots
  first.path_id[1] = 1;

  ASSERT_TRUE(AIPathFreePath(&first));
  EXPECT_EQ(first.num_paths, 0);
  EXPECT_EQ(first.goal_index, -1);
  EXPECT_EQ(AIDynamicPath[3].use_count, 1); // decremented only
  EXPECT_EQ(AIDynamicPath[3].num_nodes, 5); // data still live

  second.num_paths = 1;
  second.path_type[0] = AIP_DYNAMIC;
  second.path_id[0] = 3;

  ASSERT_TRUE(AIPathFreePath(&second));
  EXPECT_EQ(AIDynamicPath[3].use_count, 0);      // fully released...
  EXPECT_EQ(AIDynamicPath[3].num_nodes, 0);      // ...so data resets
  EXPECT_EQ(AIDynamicPath[3].owner_handle, OBJECT_HANDLE_NONE);

  // freeing again is harmless (no dynamic refs left)
  ASSERT_TRUE(AIPathFreePath(&second));
  EXPECT_EQ(second.num_paths, 0);
}
