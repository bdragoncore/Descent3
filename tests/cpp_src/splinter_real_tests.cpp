/**
 * @file splinter_real_tests.cpp
 * @brief Tests for splinter.cpp — splinter object helpers (89 lines).
 *
 * @details
 * Covers DoSplinterFrame logic.
 *
 * This harness validates the behavior of `Descent3/splinter.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/splinter.cpp`
 * @par Harness
 * `splinter_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/splinter.cpp
 */

#include <gtest/gtest.h>
#include "object.h"
#include "splinter.h"

// Provide globals/stubs
object Objects[10];
int Highest_object_index = 0;

// Stub ps_rand to deterministic
#include "psrand.h"
static int ps_rand_return = 0;
int ps_rand() { return ps_rand_return; }
void ps_srand(unsigned int seed) { (void)seed; }

// Stubs for splinter dependencies
void SetObjectDeadFlag(object *obj, bool a, bool b) { (void)a;(void)b; obj->flags |= OF_DEAD; }
int GetRandomSmallExplosion() { return 0; }
#define BLACK_SMOKE_INDEX 1
int CreateFireball(vector *pos, int type, int roomnum, int flags) { (void)pos;(void)type;(void)roomnum;(void)flags; return 0; }

// Renderer and model stubs for DrawSplinterObject (not exercised but linked)
#include "polymodel.h"
#include "renderer.h"
#include "3d.h"
#include "gametexture.h"
poly_model Poly_models[1];
void rend_SetLighting(light_state ls) { (void)ls; }
void rend_SetAlphaType(int8_t at) { (void)at; }
void rend_SetAlphaValue(uint8_t v) { (void)v; }
void rend_SetTextureType(texture_type tt) { (void)tt; }
int GetTextureBitmap(int handle, int framenum, bool force) { (void)handle;(void)framenum;(void)force; return 0; }
uint8_t g3_RotatePoint(g3Point *dest, vector *src) { (void)dest;(void)src; return 0; }
int g3_DrawPoly(int nv, g3Point **pointlist, int bm, int map_type, g3Codes *clip_codes) { (void)nv;(void)pointlist;(void)bm;(void)map_type;(void)clip_codes; return 0; }

/**
 * @test Splinter.DoFrameDiesWhenLifeExpiredAndRandEven
 * @brief Verifies do Frame Dies When Life Expired And Rand Even.
 *
 * @details
 * Exercises the Splinter code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/splinter.cpp
 * @ingroup descent3_tests
 */
TEST(Splinter, DoFrameDiesWhenLifeExpiredAndRandEven) {
  object obj{};
  obj.type = OBJ_SPLINTER;
  obj.control_type = CT_SPLINTER;
  obj.lifeleft = 0.0f;
  obj.lifetime = 10.0f;
  obj.flags = 0;
  // Ensure ps_rand %2 ==0
  ps_rand_return = 0;
  DoSplinterFrame(&obj);
  EXPECT_TRUE(obj.flags & OF_DEAD);
}

/**
 * @test Splinter.DoFrameNotDieWhenRandOdd
 * @brief Verifies do Frame Not Die When Rand Odd.
 *
 * @details
 * Exercises the Splinter code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/splinter.cpp
 * @ingroup descent3_tests
 */
TEST(Splinter, DoFrameNotDieWhenRandOdd) {
  object obj{};
  obj.type = OBJ_SPLINTER;
  obj.control_type = CT_SPLINTER;
  obj.lifeleft = 0.0f;
  obj.flags = 0;
  ps_rand_return = 1; // odd
  DoSplinterFrame(&obj);
  EXPECT_FALSE(obj.flags & OF_DEAD);
}

/**
 * @test Splinter.DoFrameNotDieWhenLifeLeftPositive
 * @brief Verifies do Frame Not Die When Life Left Positive.
 *
 * @details
 * Exercises the Splinter code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/splinter.cpp
 * @ingroup descent3_tests
 */
TEST(Splinter, DoFrameNotDieWhenLifeLeftPositive) {
  object obj{};
  obj.type = OBJ_SPLINTER;
  obj.control_type = CT_SPLINTER;
  obj.lifeleft = 5.0f;
  obj.flags = 0;
  // Place obj at index 1 so (obj-Objects)%8 !=0 to avoid smoke path randomness
  object *obj_ptr = &Objects[1];
  *obj_ptr = obj;
  obj_ptr->lifeleft = 5.0f;
  obj_ptr->control_type = CT_SPLINTER;
  ps_rand_return = 0;
  DoSplinterFrame(obj_ptr);
  EXPECT_FALSE(obj_ptr->flags & OF_DEAD);
}
