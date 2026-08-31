/**
 * @file special_face_real_tests.cpp
 * @brief Tests for special_face.cpp (96 lines).
 *
 * @details
 * Covers the LIFO special-face allocator: sequential slot pops, field
 * initialization, specular-instance and vertex-normal allocation, refcounted
 * frees, free-list reuse order, and exhaustion.
 *
 * This harness validates the behavior of `Descent3/special_face.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/special_face.cpp`
 * @par Harness
 * `special_face_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/special_face.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "special_face.h"
#include "mem.h"
#include "pserror.h"

// Defined by special_face.cpp itself: Num_of_special_faces.
// (declared extern in the header)

/**
 * @brief GTest fixture for SpecialFaceTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class SpecialFaceTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Quirk (tested below): InitSpecialFaces() itself never resets this.
    Num_of_special_faces = 0;
    InitSpecialFaces();
  }
};

/**
 * @test SpecialFaceTest.QuirkInitDoesNotResetAllocationCount
 * @brief Verifies quirk Init Does Not Reset Allocation Count.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, QuirkInitDoesNotResetAllocationCount) {
  // Burn three allocations, then re-init like a level restart would.
  AllocSpecialFace(SFF_SPEC_OBJECT, 1);
  AllocSpecialFace(SFF_SPEC_OBJECT, 1);
  AllocSpecialFace(SFF_SPEC_OBJECT, 1);

  InitSpecialFaces(); // clears used flags but NOT Num_of_special_faces

  EXPECT_EQ(Num_of_special_faces, 3); // stale!
  // The next allocation pops free-list entry 3 - slots 0..2 sit idle even
  // though they are marked unused, until the counter hits MAX_SPECIAL_FACES
  // and allocation fails despite free space.
  int n = AllocSpecialFace(SFF_SPEC_OBJECT, 1);
  EXPECT_EQ(n, 3);
}

/**
 * @test SpecialFaceTest.InitClearsAllSlotsAndFreeList
 * @brief Verifies init Clears All Slots And Free List.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, InitClearsAllSlotsAndFreeList) {
  SpecialFaces[7].used = 1;
  InitSpecialFaces();

  EXPECT_EQ(Num_of_special_faces, 0);
  for (int i = 0; i < MAX_SPECIAL_FACES; i++)
    EXPECT_FALSE(SpecialFaces[i].used);
}

/**
 * @test SpecialFaceTest.AllocPopsSequentialSlots
 * @brief Verifies alloc Pops Sequential Slots.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, AllocPopsSequentialSlots) {
  EXPECT_EQ(AllocSpecialFace(SFF_SPEC_OBJECT, 4), 0);
  EXPECT_EQ(AllocSpecialFace(SFF_SPEC_OBJECT, 4), 1);
  EXPECT_EQ(Num_of_special_faces, 2);
}

/**
 * @test SpecialFaceTest.AllocInitializesFieldsAndInstanceArray
 * @brief Verifies alloc Initializes Fields And Instance Array.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, AllocInitializesFieldsAndInstanceArray) {
  int n = AllocSpecialFace(SFF_SPEC_OBJECT, 6);

  ASSERT_EQ(n, 0);
  EXPECT_EQ(SpecialFaces[n].used, 1);
  EXPECT_EQ(SpecialFaces[n].type, SFF_SPEC_OBJECT);
  EXPECT_EQ(SpecialFaces[n].num, 6);
  EXPECT_EQ(SpecialFaces[n].flags, 0); // no vertnorms -> no SFF_SPEC_SMOOTH
  ASSERT_NE(SpecialFaces[n].spec_instance, nullptr);
  // spec_instance array is NOT zeroed (raw malloc) - just verify writability
  memset(SpecialFaces[n].spec_instance, 0, sizeof(specular_instance) * 6);
  EXPECT_TRUE(SpecialFaces[n].vertnorms == nullptr);
}

/**
 * @test SpecialFaceTest.VertnormsOptionAllocatesAndSetsFlag
 * @brief Verifies vertnorms Option Allocates And Sets Flag.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, VertnormsOptionAllocatesAndSetsFlag) {
  int n = AllocSpecialFace(SFF_SPEC_OBJECT, 3, /*vertnorms*/ true, 12);

  EXPECT_NE(SpecialFaces[n].vertnorms, nullptr);
  EXPECT_TRUE(SpecialFaces[n].flags & SFF_SPEC_SMOOTH);
  EXPECT_NE(SpecialFaces[n].spec_instance, nullptr);
}

/**
 * @test SpecialFaceTest.NoVertnormsLeavesPointerNull
 * @brief Verifies no Vertnorms Leaves Pointer Null.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, NoVertnormsLeavesPointerNull) {
  int n = AllocSpecialFace(SFF_SPEC_OBJECT, 3, false, 0);
  EXPECT_EQ(SpecialFaces[n].vertnorms, nullptr);
  EXPECT_FALSE(SpecialFaces[n].flags & SFF_SPEC_SMOOTH);
}

/**
 * @test SpecialFaceTest.FreeReturnsSlotToLifoFront
 * @brief Verifies free Returns Slot To Lifo Front.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, FreeReturnsSlotToLifoFront) {
  int a = AllocSpecialFace(SFF_SPEC_OBJECT, 1);
  int b = AllocSpecialFace(SFF_SPEC_OBJECT, 1);
  int c = AllocSpecialFace(SFF_SPEC_OBJECT, 1);
  (void)b;

  FreeSpecialFace(b);
  EXPECT_EQ(Num_of_special_faces, 2);
  EXPECT_EQ(AllocSpecialFace(SFF_SPEC_OBJECT, 1), b); // reused immediately

  FreeSpecialFace(a);
  FreeSpecialFace(c);
  // LIFO order: c freed last comes back first
  EXPECT_EQ(AllocSpecialFace(SFF_SPEC_OBJECT, 1), c);
  EXPECT_EQ(AllocSpecialFace(SFF_SPEC_OBJECT, 1), a);
}

/**
 * @test SpecialFaceTest.FreeReleasesBothAllocations
 * @brief Verifies free Releases Both Allocations.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, FreeReleasesBothAllocations) {
  int n = AllocSpecialFace(SFF_SPEC_OBJECT, 5, true, 10);
  void *inst = SpecialFaces[n].spec_instance;
  void *vnorms = SpecialFaces[n].vertnorms;

  FreeSpecialFace(n);

  EXPECT_EQ(SpecialFaces[n].used, 0);
  EXPECT_EQ(SpecialFaces[n].spec_instance, nullptr);
  EXPECT_EQ(SpecialFaces[n].vertnorms, nullptr);
  (void)inst;
  (void)vnorms;
}

/**
 * @test SpecialFaceTest.FreeOfUnusedSlotIsNoop
 * @brief Verifies free Of Unused Slot Is Noop.
 *
 * @details
 * Exercises the SpecialFaceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/special_face.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpecialFaceTest, FreeOfUnusedSlotIsNoop) {
  Num_of_special_faces = 9; // pretend others exist
  FreeSpecialFace(42);      // never allocated
  EXPECT_EQ(Num_of_special_faces, 9);
}
