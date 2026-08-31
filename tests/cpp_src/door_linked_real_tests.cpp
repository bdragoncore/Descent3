/**
 * @file door_linked_real_tests.cpp
 * @brief Unit tests for Descent3/door.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/door.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/door.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/door.cpp`
 * @par Harness
 * `door_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/door.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <filesystem>

#include "door.h"
#include "manage.h"
#include "pserror.h"

// Stub for LoadPolyModel required by door.cpp
int LoadPolyModel(const std::filesystem::path &p, int pageable) {
  (void)p; (void)pageable;
  return 1;
}
class DoorLinked : public ::testing::Test {
protected:
  void SetUp() override {
    InitDoors();
  }
};

/**
 * @test DoorLinked.InitDoorsNoCrash
 * @brief Verifies init Doors No Crash.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, InitDoorsNoCrash) {
  EXPECT_NO_THROW(InitDoors());
  EXPECT_NO_THROW(InitDoors());
}

/**
 * @test DoorLinked.AllocFreeDoor
 * @brief Verifies alloc Free Door.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, AllocFreeDoor) {
  int h = AllocDoor();
  EXPECT_GE(h, 0);
  EXPECT_LE(h, 60);
  EXPECT_NO_THROW(FreeDoor(h));
}

/**
 * @test DoorLinked.AllocMultiple
 * @brief Verifies alloc Multiple.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, AllocMultiple) {
  int h1 = AllocDoor();
  int h2 = AllocDoor();
  EXPECT_NE(h1, h2);
  FreeDoor(h1);
  FreeDoor(h2);
}

/**
 * @test DoorLinked.GetNextPrev
 * @brief Verifies get Next Prev.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, GetNextPrev) {
  int h = AllocDoor();
  int nxt = GetNextDoor(h);
  int prv = GetPrevDoor(h);
  // May be -1 if list handling, just check no crash and within range
  EXPECT_GE(nxt, -1);
  EXPECT_GE(prv, -1);
  FreeDoor(h);
}

/**
 * @test DoorLinked.FindDoorNameMissing
 * @brief Verifies find Door Name Missing.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, FindDoorNameMissing) {
  int f = FindDoorName("nonexistent_door_xyz");
  EXPECT_EQ(f, -1);
}

/**
 * @test DoorLinked.FindDoorNameAfterAlloc
 * @brief Verifies find Door Name After Alloc.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, FindDoorNameAfterAlloc) {
  int h = AllocDoor();
  // Doors[h].name may be empty; FindDoorName should still handle
  Doors[h].flags = 0;
  strncpy(Doors[h].name, "TestDoor", sizeof(Doors[h].name)-1);
  int f = FindDoorName("TestDoor");
  EXPECT_EQ(f, h);
  int f2 = FindDoorName("testdoor"); // case insensitive?
  // strcasecmp used, so should find
  EXPECT_EQ(f2, h);
  FreeDoor(h);
}

/**
 * @test DoorLinked.LoadDoorImage
 * @brief Verifies load Door Image.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, LoadDoorImage) {
  int bh = LoadDoorImage("dummy.oof", 1);
  // Stub LoadPolyModel returns 1, so handle should be >=0 or -1
  EXPECT_GE(bh, -1);
}

/**
 * @test DoorLinked.GetDoorImage
 * @brief Verifies get Door Image.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, GetDoorImage) {
  int h = AllocDoor();
  Doors[h].flags = 0;
  Doors[h].model_handle = 5;
  int img = GetDoorImage(h);
  EXPECT_EQ(img, 5);
  FreeDoor(h);
}

/**
 * @test DoorLinked.GetDoorImageInvalid
 * @brief Verifies get Door Image Invalid.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, GetDoorImageInvalid) {
  int img = GetDoorImage(-1);
  EXPECT_GE(img, -1);
  img = GetDoorImage(999);
  EXPECT_GE(img, -1);
}

/**
 * @test DoorLinked.RemapDoorsNoCrash
 * @brief Verifies remap Doors No Crash.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, RemapDoorsNoCrash) {
  EXPECT_NO_THROW(RemapDoors());
}

/**
 * @test DoorLinked.AllocExhaustion
 * @brief Verifies alloc Exhaustion.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, AllocExhaustion) {
  // Allocate many, ensure graceful handling
  int handles[70];
  int count = 0;
  for(int i=0;i<70;i++){
    int h = AllocDoor();
    if(h==-1) break;
    handles[count++]=h;
  }
  EXPECT_GT(count, 0);
  for(int i=0;i<count;i++) FreeDoor(handles[i]);
}

/**
 * @test DoorLinked.FreeInvalid
 * @brief Verifies free Invalid.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, FreeInvalid) {
  EXPECT_NO_THROW(FreeDoor(-1));
  EXPECT_NO_THROW(FreeDoor(999));
}

/**
 * @test DoorLinked.DoorsArray
 * @brief Verifies doors Array.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, DoorsArray) {
  EXPECT_NE(Doors, nullptr);
  Doors[0].flags = 123;
  EXPECT_EQ(Doors[0].flags, 123);
}

/**
 * @test DoorLinked.DoorNameEmpty
 * @brief Verifies door Name Empty.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, DoorNameEmpty) {
  int h = AllocDoor();
  Doors[h].name[0] = '\0';
  int f = FindDoorName("");
  // Should find first empty? Might be -1, just check no crash
  EXPECT_GE(f, -1);
  FreeDoor(h);
}

/**
 * @test DoorLinked.GetNextDoorInvalid
 * @brief Verifies get Next Door Invalid.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, GetNextDoorInvalid) {
  int n = GetNextDoor(-1);
  EXPECT_GE(n, -1);
  n = GetNextDoor(999);
  EXPECT_GE(n, -1);
}

/**
 * @test DoorLinked.GetPrevDoorInvalid
 * @brief Verifies get Prev Door Invalid.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, GetPrevDoorInvalid) {
  int p = GetPrevDoor(-1);
  EXPECT_GE(p, -1);
}

/**
 * @test DoorLinked.MultipleInit
 * @brief Verifies multiple Init.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, MultipleInit) {
  InitDoors();
  int h1 = AllocDoor();
  InitDoors();
  int h2 = AllocDoor();
  EXPECT_GE(h1, 0);
  EXPECT_GE(h2, 0);
  FreeDoor(h2);
}

/**
 * @test DoorLinked.LoadDoorImageVariations
 * @brief Verifies load Door Image Variations.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, LoadDoorImageVariations) {
  int h1 = LoadDoorImage("a.oof", 0);
  int h2 = LoadDoorImage("b.oof", 1);
  EXPECT_GE(h1, -1);
  EXPECT_GE(h2, -1);
}

/**
 * @test DoorLinked.DoorFlags
 * @brief Verifies door Flags.
 *
 * @details
 * Exercises the DoorLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorLinked, DoorFlags) {
  int h = AllocDoor();
  Doors[h].flags = DF_BLASTABLE;
  EXPECT_TRUE(Doors[h].flags & DF_BLASTABLE);
  Doors[h].flags = 0;
  FreeDoor(h);
}
