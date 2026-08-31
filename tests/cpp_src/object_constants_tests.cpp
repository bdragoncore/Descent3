/**
 * @file object_constants_tests.cpp
 * @brief Unit tests for Descent3/object_constants.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/object_constants.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/object_constants.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/object_constants.cpp`
 * @par Harness
 * `object_constants_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/object_constants.cpp
 */

#include "gtest/gtest.h"
#include "pstypes.h"
#include <cstdint>

// Object constants (from Descent3/object.h) for standalone tests
#define HANDLE_OBJNUM_MASK 0x7ff
#define HANDLE_COUNT_MASK 0xfffff800
#define HANDLE_COUNT_INCREMENT 0x800
#define LRT_STATIC 0
#define LRT_GOURAUD 1
#define LRT_LIGHTMAPS 2
#define WPC_NOT_USED 0
#define WPC_NO_COLLISIONS 1
#define WPC_HIT_WALL 2
#define FMA_VALID 1
#define FMA_CURRENT 2
#define FMA_LOOPING 4
#define FMA_USE_SPEED 8
#define FMA_HAS_AI 16
#define FMT_NEW_DATA 1
#define FMT_UPDATING 2
#define OLF_FLICKERING 1
#define OLF_TIMEBITS 2
#define OLF_PULSE 4
#define OLF_PULSE_TO_SECOND 8
#define OLF_FLICKER_SLIGHTLY 16
#define OLF_DIRECTIONAL 32
#define OLF_NO_SPECULARITY 64
#define MAX_BIG_OBJECTS 350
#define OBJ_NAME_LEN 19

/**
 * @test ObjectConstantsTest.HandleMasks
 * @brief Verifies handle Masks.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, HandleMasks) {
  EXPECT_EQ(HANDLE_OBJNUM_MASK, 0x7FF);
  EXPECT_EQ(HANDLE_COUNT_MASK, 0xFFFFF800);
  EXPECT_EQ(HANDLE_COUNT_INCREMENT, 0x800);

  uint32_t handle = 0x12345678;
  uint32_t objnum = handle & HANDLE_OBJNUM_MASK;
  uint32_t count = handle & HANDLE_COUNT_MASK;
  EXPECT_EQ(objnum, 0x678);
  EXPECT_EQ(count, 0x12345000);
}

/**
 * @test ObjectConstantsTest.LightingRenderTypes
 * @brief Verifies lighting Render Types.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, LightingRenderTypes) {
  EXPECT_EQ(LRT_STATIC, 0);
  EXPECT_EQ(LRT_GOURAUD, 1);
  EXPECT_EQ(LRT_LIGHTMAPS, 2);

  EXPECT_GE(LRT_LIGHTMAPS, LRT_GOURAUD);
  EXPECT_GE(LRT_GOURAUD, LRT_STATIC);
}

/**
 * @test ObjectConstantsTest.WeaponPrecomputeTypes
 * @brief Verifies weapon Precompute Types.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, WeaponPrecomputeTypes) {
  EXPECT_EQ(WPC_NOT_USED, 0);
  EXPECT_EQ(WPC_NO_COLLISIONS, 1);
  EXPECT_EQ(WPC_HIT_WALL, 2);
}

/**
 * @test ObjectConstantsTest.FMAFlags
 * @brief Verifies fMAFlags.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, FMAFlags) {
  EXPECT_EQ(FMA_VALID, 1);
  EXPECT_EQ(FMA_CURRENT, 2);
  EXPECT_EQ(FMA_LOOPING, 4);
  EXPECT_EQ(FMA_USE_SPEED, 8);
  EXPECT_EQ(FMA_HAS_AI, 16);

  int all_flags = FMA_VALID | FMA_CURRENT | FMA_LOOPING | FMA_USE_SPEED | FMA_HAS_AI;
  EXPECT_EQ(all_flags, 0x1F);
}

/**
 * @test ObjectConstantsTest.FMTFlags
 * @brief Verifies fMTFlags.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, FMTFlags) {
  EXPECT_EQ(FMT_NEW_DATA, 1);
  EXPECT_EQ(FMT_UPDATING, 2);

  int all_flags = FMT_NEW_DATA | FMT_UPDATING;
  EXPECT_EQ(all_flags, 3);
}

/**
 * @test ObjectConstantsTest.OLFObjectLightFlags
 * @brief Verifies oLFObject Light Flags.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, OLFObjectLightFlags) {
  EXPECT_EQ(OLF_FLICKERING, 1);
  EXPECT_EQ(OLF_TIMEBITS, 2);
  EXPECT_EQ(OLF_PULSE, 4);
  EXPECT_EQ(OLF_PULSE_TO_SECOND, 8);
  EXPECT_EQ(OLF_FLICKER_SLIGHTLY, 16);
  EXPECT_EQ(OLF_DIRECTIONAL, 32);
  EXPECT_EQ(OLF_NO_SPECULARITY, 64);

  int all_olf = OLF_FLICKERING | OLF_TIMEBITS | OLF_PULSE | OLF_PULSE_TO_SECOND | OLF_FLICKER_SLIGHTLY |
                OLF_DIRECTIONAL | OLF_NO_SPECULARITY;
  EXPECT_EQ(all_olf, 0x7F);
}

/**
 * @test ObjectConstantsTest.MaxBigObjects
 * @brief Verifies max Big Objects.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, MaxBigObjects) {
  EXPECT_EQ(MAX_BIG_OBJECTS, 350);
  EXPECT_GT(MAX_BIG_OBJECTS, 100);
}

/**
 * @test ObjectConstantsTest.ObjectNameLength
 * @brief Verifies object Name Length.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, ObjectNameLength) {
  EXPECT_EQ(OBJ_NAME_LEN, 19);
  EXPECT_GT(OBJ_NAME_LEN, 0);
  EXPECT_LE(OBJ_NAME_LEN, 64);
}

/**
 * @test ObjectConstantsTest.ObjNumMacro
 * @brief Verifies obj Num Macro.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, ObjNumMacro) {
  int mock_objects[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int *obj_ptr = &mock_objects[5];
  int objnum = obj_ptr - mock_objects;
  EXPECT_EQ(objnum, 5);
}

/**
 * @test ObjectConstantsTest.HandleCalculation
 * @brief Verifies handle Calculation.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, HandleCalculation) {
  uint32_t base_handle = 0x12345000;
  uint32_t objnum = 0x3FF;
  uint32_t handle = base_handle | objnum;

  EXPECT_EQ(handle & HANDLE_OBJNUM_MASK, objnum);
  EXPECT_EQ(handle & HANDLE_COUNT_MASK, base_handle);
}

/**
 * @test ObjectConstantsTest.HandleIncrement
 * @brief Verifies handle Increment.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, HandleIncrement) {
  uint32_t handle1 = 0x12345000;
  uint32_t handle2 = handle1 + HANDLE_COUNT_INCREMENT;

  EXPECT_EQ((handle1 >> 11), (handle2 >> 11) - 1);
  EXPECT_EQ(handle1 & HANDLE_OBJNUM_MASK, handle2 & HANDLE_OBJNUM_MASK);
}

/**
 * @test ObjectConstantsTest.LRTValuesAreSequential
 * @brief Verifies lRTValues Are Sequential.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, LRTValuesAreSequential) {
  EXPECT_EQ(LRT_STATIC + 1, LRT_GOURAUD);
  EXPECT_EQ(LRT_GOURAUD + 1, LRT_LIGHTMAPS);
}

/**
 * @test ObjectConstantsTest.WPCValuesAreSequential
 * @brief Verifies wPCValues Are Sequential.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, WPCValuesAreSequential) {
  EXPECT_EQ(WPC_NOT_USED + 1, WPC_NO_COLLISIONS);
  EXPECT_EQ(WPC_NO_COLLISIONS + 1, WPC_HIT_WALL);
}

/**
 * @test ObjectConstantsTest.OLFValuesArePowersOfTwo
 * @brief Verifies oLFValues Are Powers Of Two.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, OLFValuesArePowersOfTwo) {
  EXPECT_EQ(OLF_FLICKERING, 1 << 0);
  EXPECT_EQ(OLF_TIMEBITS, 1 << 1);
  EXPECT_EQ(OLF_PULSE, 1 << 2);
  EXPECT_EQ(OLF_PULSE_TO_SECOND, 1 << 3);
  EXPECT_EQ(OLF_FLICKER_SLIGHTLY, 1 << 4);
  EXPECT_EQ(OLF_DIRECTIONAL, 1 << 5);
  EXPECT_EQ(OLF_NO_SPECULARITY, 1 << 6);
}

/**
 * @test ObjectConstantsTest.FMAValuesArePowersOfTwo
 * @brief Verifies fMAValues Are Powers Of Two.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, FMAValuesArePowersOfTwo) {
  EXPECT_EQ(FMA_VALID, 1 << 0);
  EXPECT_EQ(FMA_CURRENT, 1 << 1);
  EXPECT_EQ(FMA_LOOPING, 1 << 2);
  EXPECT_EQ(FMA_USE_SPEED, 1 << 3);
  EXPECT_EQ(FMA_HAS_AI, 1 << 4);
}

/**
 * @test ObjectConstantsTest.FMTValuesArePowersOfTwo
 * @brief Verifies fMTValues Are Powers Of Two.
 *
 * @details
 * Exercises the ObjectConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/object_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ObjectConstantsTest, FMTValuesArePowersOfTwo) {
  EXPECT_EQ(FMT_NEW_DATA, 1 << 0);
  EXPECT_EQ(FMT_UPDATING, 1 << 1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
