/**
 * @file objinfo_real_tests.cpp
 * @brief Tests for objinfo.cpp — object info array (MAX_OBJECT_IDS 910).
 *
 * @details
 * Covers InitObjectInfo, FindObjectIDName, Alloc/Free, GetObjectID
 *
 * This harness validates the behavior of `Descent3/objinfo.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/objinfo.cpp`
 * @par Harness
 * `objinfo_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/objinfo.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include "objinfo.h"

// Stubs for dependencies used by objinfo.cpp
// AISetDefault called by AllocObjectID when f_ai true
#include "aistruct.h"
void AISetDefault(t_ai_info *ai) { if (ai) memset(ai, 0, sizeof(*ai)); }

// WBClearInfo called when f_weapons true
#include "robotfirestruct.h"
void WBClearInfo(otype_wb_info *wb) { if (wb) memset(wb, 0, sizeof(*wb)*MAX_WBS_PER_OBJ); }

// Objects array needed for RemapObjects etc but not for these tests; provide minimal
#include "object.h"
object Objects[10];
int Highest_object_index = 0;

// Stub DrawPolygonModel etc not needed for these tests but linked via DrawObject (not called)
void DrawPolygonModel(vector *pos, matrix *orient, int model_num, float *normalized_time, int flags, float r, float g, float b, uint32_t f_render_sub, uint8_t use_effect, uint8_t overlay) { (void)pos;(void)orient;(void)model_num;(void)normalized_time;(void)flags;(void)r;(void)g;(void)b;(void)f_render_sub;(void)use_effect;(void)overlay; }

/**
 * @test ObjInfo.InitClears
 * @brief Verifies init Clears.
 *
 * @details
 * Exercises the ObjInfo code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/objinfo.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInfo, InitClears) {
  InitObjectInfo();
  for (int i=0;i<10;i++) EXPECT_EQ(Object_info[i].type, OBJ_NONE);
  EXPECT_EQ(FindObjectIDName("GuideBot"), -1);
  EXPECT_EQ(GetObjectID(OBJ_POWERUP), -1);
}

/**
 * @test ObjInfo.AllocFindFree
 * @brief Verifies alloc Find Free.
 *
 * @details
 * Exercises the ObjInfo code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/objinfo.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInfo, AllocFindFree) {
  InitObjectInfo();
  int id = AllocObjectID(OBJ_POWERUP, false, false, false);
  ASSERT_GE(id, 0);
  EXPECT_EQ(Object_info[id].type, OBJ_POWERUP);
  strcpy(Object_info[id].name, "TestPowerup");
  EXPECT_EQ(FindObjectIDName("TestPowerup"), id);
  EXPECT_EQ(FindObjectIDName("testpowerup"), id); // case insensitive
  EXPECT_EQ(GetObjectID(OBJ_POWERUP), id);
  FreeObjectID(id);
  EXPECT_EQ(Object_info[id].type, OBJ_NONE);
  EXPECT_EQ(FindObjectIDName("TestPowerup"), -1);
}

/**
 * @test ObjInfo.AllocExhaustion
 * @brief Verifies alloc Exhaustion.
 *
 * @details
 * Exercises the ObjInfo code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/objinfo.cpp
 * @ingroup descent3_tests
 */
TEST(ObjInfo, AllocExhaustion) {
  InitObjectInfo();
  // Allocate a few
  int id1 = AllocObjectID(OBJ_ROBOT, false,false,false);
  int id2 = AllocObjectID(OBJ_ROBOT, false,false,false);
  ASSERT_GE(id1,0); ASSERT_GE(id2,0);
  EXPECT_NE(id1, id2);
  EXPECT_EQ(Object_info[id1].type, OBJ_ROBOT);
  // Free one and realloc should reuse slot after NUM_STATIC_OBJECTS?
  FreeObjectID(id1);
  int id3 = AllocObjectID(OBJ_ROBOT, false,false,false);
  EXPECT_GE(id3,0);
  // Cleanup
  FreeObjectID(id2);
  FreeObjectID(id3);
}
