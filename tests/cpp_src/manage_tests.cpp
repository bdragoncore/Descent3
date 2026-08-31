/**
 * @file manage_tests.cpp
 * @brief Unit tests for Descent3/manage.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/manage.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/manage.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/manage.cpp`
 * @par Harness
 * `manage_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/manage.cpp
 */

#include "gtest/gtest.h"
#include "manage.h"

// Test tracklock initialization
/**
 * @test ManageTrackLockTest.InitTrackLocks
 * @brief Verifies init Track Locks.
 *
 * @details
 * Exercises the ManageTrackLockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/manage.cpp
 * @ingroup descent3_tests
 */
TEST(ManageTrackLockTest, InitTrackLocks) {
    mng_InitTrackLocks();
    
    // After init, all tracklocks should be unused
    for (int i = 0; i < MAX_TRACKLOCKS; i++) {
        EXPECT_EQ(GlobalTrackLocks[i].used, 0);
        EXPECT_EQ(GlobalTrackLocks[i].pagetype, PAGETYPE_UNKNOWN);
        EXPECT_EQ(GlobalTrackLocks[i].name[0], '\0');
    }
}

// Test allocating a tracklock
/**
 * @test ManageTrackLockTest.AllocTrackLock
 * @brief Verifies alloc Track Lock.
 *
 * @details
 * Exercises the ManageTrackLockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/manage.cpp
 * @ingroup descent3_tests
 */
TEST(ManageTrackLockTest, AllocTrackLock) {
    mng_InitTrackLocks();
    
    int index = mng_AllocTrackLock("TestPage", PAGETYPE_TEXTURE);
    EXPECT_GE(index, 0);
    EXPECT_LT(index, MAX_TRACKLOCKS);
    
    EXPECT_EQ(GlobalTrackLocks[index].used, 1);
    EXPECT_EQ(GlobalTrackLocks[index].pagetype, PAGETYPE_TEXTURE);
    EXPECT_STREQ(GlobalTrackLocks[index].name, "TestPage");
    
    mng_FreeTrackLock(index);
}

// Test finding a tracklock
/**
 * @test ManageTrackLockTest.FindTrackLock
 * @brief Verifies find Track Lock.
 *
 * @details
 * Exercises the ManageTrackLockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/manage.cpp
 * @ingroup descent3_tests
 */
TEST(ManageTrackLockTest, FindTrackLock) {
    mng_InitTrackLocks();
    
    int index = mng_AllocTrackLock("FindMe", PAGETYPE_WEAPON);
    EXPECT_GE(index, 0);
    
    // Should find it
    int found = mng_FindTrackLock("FindMe", PAGETYPE_WEAPON);
    EXPECT_EQ(found, index);
    
    // Wrong type should not find it
    int notfound = mng_FindTrackLock("FindMe", PAGETYPE_TEXTURE);
    EXPECT_EQ(notfound, -1);
    
    // Wrong name should not find it
    int notfound2 = mng_FindTrackLock("NotThere", PAGETYPE_WEAPON);
    EXPECT_EQ(notfound2, -1);
    
    mng_FreeTrackLock(index);
}

// Test freeing a tracklock
/**
 * @test ManageTrackLockTest.FreeTrackLock
 * @brief Verifies free Track Lock.
 *
 * @details
 * Exercises the ManageTrackLockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/manage.cpp
 * @ingroup descent3_tests
 */
TEST(ManageTrackLockTest, FreeTrackLock) {
    mng_InitTrackLocks();
    
    int index = mng_AllocTrackLock("ToBeFreed", PAGETYPE_ROBOT);
    EXPECT_GE(index, 0);
    
    mng_FreeTrackLock(index);
    
    EXPECT_EQ(GlobalTrackLocks[index].used, 0);
    EXPECT_EQ(GlobalTrackLocks[index].pagetype, PAGETYPE_UNKNOWN);
    EXPECT_EQ(GlobalTrackLocks[index].name[0], '\0');
}

// Test allocating multiple tracklocks
/**
 * @test ManageTrackLockTest.AllocMultipleTrackLocks
 * @brief Verifies alloc Multiple Track Locks.
 *
 * @details
 * Exercises the ManageTrackLockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/manage.cpp
 * @ingroup descent3_tests
 */
TEST(ManageTrackLockTest, AllocMultipleTrackLocks) {
    mng_InitTrackLocks();
    
    int indices[10];
    for (int i = 0; i < 10; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Page%d", i);
        indices[i] = mng_AllocTrackLock(name, PAGETYPE_GENERIC);
        EXPECT_GE(indices[i], 0);
    }
    
    // Verify all are allocated
    for (int i = 0; i < 10; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Page%d", i);
        int found = mng_FindTrackLock(name, PAGETYPE_GENERIC);
        EXPECT_EQ(found, indices[i]);
    }
    
    // Free them all
    for (int i = 0; i < 10; i++) {
        mng_FreeTrackLock(indices[i]);
    }
}

// Test that find is case-insensitive
/**
 * @test ManageTrackLockTest.FindTrackLockCaseInsensitive
 * @brief Verifies find Track Lock Case Insensitive.
 *
 * @details
 * Exercises the ManageTrackLockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/manage.cpp
 * @ingroup descent3_tests
 */
TEST(ManageTrackLockTest, FindTrackLockCaseInsensitive) {
    mng_InitTrackLocks();
    
    int index = mng_AllocTrackLock("MixedCase", PAGETYPE_DOOR);
    EXPECT_GE(index, 0);
    
    int found1 = mng_FindTrackLock("mixedcase", PAGETYPE_DOOR);
    EXPECT_EQ(found1, index);
    
    int found2 = mng_FindTrackLock("MIXEDCASE", PAGETYPE_DOOR);
    EXPECT_EQ(found2, index);
    
    mng_FreeTrackLock(index);
}

// Test different page types
/**
 * @test ManageTrackLockTest.DifferentPageTypes
 * @brief Verifies different Page Types.
 *
 * @details
 * Exercises the ManageTrackLockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/manage.cpp
 * @ingroup descent3_tests
 */
TEST(ManageTrackLockTest, DifferentPageTypes) {
    mng_InitTrackLocks();
    
    int tex = mng_AllocTrackLock("Texture", PAGETYPE_TEXTURE);
    int weapon = mng_AllocTrackLock("Weapon", PAGETYPE_WEAPON);
    int robot = mng_AllocTrackLock("Robot", PAGETYPE_ROBOT);
    int powerup = mng_AllocTrackLock("Powerup", PAGETYPE_POWERUP);
    int door = mng_AllocTrackLock("Door", PAGETYPE_DOOR);
    int ship = mng_AllocTrackLock("Ship", PAGETYPE_SHIP);
    int sound = mng_AllocTrackLock("Sound", PAGETYPE_SOUND);
    int generic = mng_AllocTrackLock("Generic", PAGETYPE_GENERIC);
    
    EXPECT_GE(tex, 0);
    EXPECT_GE(weapon, 0);
    EXPECT_GE(robot, 0);
    EXPECT_GE(powerup, 0);
    EXPECT_GE(door, 0);
    EXPECT_GE(ship, 0);
    EXPECT_GE(sound, 0);
    EXPECT_GE(generic, 0);
    
    // Each should have its correct type
    EXPECT_EQ(GlobalTrackLocks[tex].pagetype, PAGETYPE_TEXTURE);
    EXPECT_EQ(GlobalTrackLocks[weapon].pagetype, PAGETYPE_WEAPON);
    EXPECT_EQ(GlobalTrackLocks[robot].pagetype, PAGETYPE_ROBOT);
    EXPECT_EQ(GlobalTrackLocks[powerup].pagetype, PAGETYPE_POWERUP);
    EXPECT_EQ(GlobalTrackLocks[door].pagetype, PAGETYPE_DOOR);
    EXPECT_EQ(GlobalTrackLocks[ship].pagetype, PAGETYPE_SHIP);
    EXPECT_EQ(GlobalTrackLocks[sound].pagetype, PAGETYPE_SOUND);
    EXPECT_EQ(GlobalTrackLocks[generic].pagetype, PAGETYPE_GENERIC);
    
    mng_FreeTrackLock(tex);
    mng_FreeTrackLock(weapon);
    mng_FreeTrackLock(robot);
    mng_FreeTrackLock(powerup);
    mng_FreeTrackLock(door);
    mng_FreeTrackLock(ship);
    mng_FreeTrackLock(sound);
    mng_FreeTrackLock(generic);
}
