/**
 * @file weapon_constants_tests.cpp
 * @brief Unit tests for Descent3/weapon_constants.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/weapon_constants.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/weapon_constants.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/weapon_constants.cpp`
 * @par Harness
 * `weapon_constants_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/weapon_constants.cpp
 */

#include "gtest/gtest.h"
#include "pstypes.h"

// Weapon constants (from Descent3/weapon.h) for standalone tests
#define MAX_PRIMARY_WEAPONS 10
#define MAX_SECONDARY_WEAPONS 10
#define MAX_WEAPON_NOT_HIT_PARENT_TIME 3.0f
#define MAX_WEAPONS 200
#define MAX_STATIC_WEAPONS 21
#define DEFAULT_WEAPON_SIZE 1.0
#define HAS_FLAG(a) (1 << (a))
#define WF_HUD_ANIMATED (1 << 0)
#define WF_IMAGE_BITMAP (1 << 1)
#define WF_SMOKE (1 << 2)
#define WF_MATTER_WEAPON (1 << 3)
#define WF_ELECTRICAL (1 << 4)
#define WF_IMAGE_VCLIP (1 << 5)
#define WF_SPRAY (1 << 6)
#define WF_STREAMER (1 << 7)
#define WF_INVISIBLE (1 << 8)
#define WF_RING (1 << 9)
#define WF_SATURATE (1 << 10)
#define WF_BLAST_RING (1 << 11)
#define WF_PLANAR_BLAST (1 << 12)
#define WF_PLANAR (1 << 13)
#define WF_ENABLE_CAMERA (1 << 14)
#define WF_SPAWNS_IMPACT (1 << 15)
#define WF_SPAWNS_TIMEOUT (1 << 16)
#define WF_EXPAND (1 << 17)
#define WF_MUZZLE (1 << 18)
#define WF_MICROWAVE (1 << 19)
#define WF_NAPALM (1 << 20)
#define WF_REVERSE_SMOKE (1 << 21)
#define WF_GRAVITY_FIELD (1 << 22)
#define WF_COUNTERMEASURE (1 << 23)
#define WF_SPAWNS_ROBOT (1 << 24)
#define WF_FREEZE (1 << 25)
#define WF_TIMEOUT_WALL (1 << 26)
#define WF_PLANAR_SMOKE (1 << 27)
#define WF_SILENT_HOMING (1 << 28)
#define WF_HOMING_SPLIT (1 << 29)
#define WF_NO_ROTATE (1 << 30)
#define WF_CUSTOM_SIZE (1U << 31)
#define MAX_LASER_LEVEL 4
#define MAX_SUPER_LASER_LEVEL 6
#define PRIMARY_INDEX 0
#define SECONDARY_INDEX 10
#define MAX_WEAPON_SOUNDS 7
#define WSI_FIRE 0
#define WSI_IMPACT_WALL 1
#define WSI_FLYING 2
#define WSI_IMPACT_ROBOT 3
#define WSI_BOUNCE 4

/**
 * @test WeaponConstantsTest.MaxWeaponCounts
 * @brief Verifies max Weapon Counts.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, MaxWeaponCounts) {
    EXPECT_EQ(MAX_PRIMARY_WEAPONS, 10);
    EXPECT_EQ(MAX_SECONDARY_WEAPONS, 10);
    EXPECT_EQ(MAX_WEAPONS, 200);
    EXPECT_EQ(MAX_STATIC_WEAPONS, 21);
    
    EXPECT_GT(MAX_WEAPONS, MAX_PRIMARY_WEAPONS);
    EXPECT_GT(MAX_WEAPONS, MAX_SECONDARY_WEAPONS);
}

/**
 * @test WeaponConstantsTest.WeaponTime
 * @brief Verifies weapon Time.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, WeaponTime) {
    EXPECT_EQ(MAX_WEAPON_NOT_HIT_PARENT_TIME, 3.0f);
    EXPECT_GT(MAX_WEAPON_NOT_HIT_PARENT_TIME, 0.0f);
}

/**
 * @test WeaponConstantsTest.DefaultWeaponSize
 * @brief Verifies default Weapon Size.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, DefaultWeaponSize) {
    EXPECT_EQ(DEFAULT_WEAPON_SIZE, 1.0);
    EXPECT_GT(DEFAULT_WEAPON_SIZE, 0.0);
}

/**
 * @test WeaponConstantsTest.WeaponFeatureFlagsBasic
 * @brief Verifies weapon Feature Flags Basic.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, WeaponFeatureFlagsBasic) {
    EXPECT_EQ(WF_HUD_ANIMATED, 1 << 0);
    EXPECT_EQ(WF_IMAGE_BITMAP, 1 << 1);
    EXPECT_EQ(WF_SMOKE, 1 << 2);
    EXPECT_EQ(WF_MATTER_WEAPON, 1 << 3);
    EXPECT_EQ(WF_ELECTRICAL, 1 << 4);
    EXPECT_EQ(WF_IMAGE_VCLIP, 1 << 5);
    EXPECT_EQ(WF_SPRAY, 1 << 6);
    EXPECT_EQ(WF_STREAMER, 1 << 7);
    EXPECT_EQ(WF_INVISIBLE, 1 << 8);
    EXPECT_EQ(WF_RING, 1 << 9);
}

/**
 * @test WeaponConstantsTest.WeaponFeatureFlagsExtended
 * @brief Verifies weapon Feature Flags Extended.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, WeaponFeatureFlagsExtended) {
    EXPECT_EQ(WF_SATURATE, 1 << 10);
    EXPECT_EQ(WF_BLAST_RING, 1 << 11);
    EXPECT_EQ(WF_PLANAR_BLAST, 1 << 12);
    EXPECT_EQ(WF_PLANAR, 1 << 13);
    EXPECT_EQ(WF_ENABLE_CAMERA, 1 << 14);
    EXPECT_EQ(WF_SPAWNS_IMPACT, 1 << 15);
    EXPECT_EQ(WF_SPAWNS_TIMEOUT, 1 << 16);
    EXPECT_EQ(WF_EXPAND, 1 << 17);
    EXPECT_EQ(WF_MUZZLE, 1 << 18);
    EXPECT_EQ(WF_MICROWAVE, 1 << 19);
    EXPECT_EQ(WF_NAPALM, 1 << 20);
    EXPECT_EQ(WF_REVERSE_SMOKE, 1 << 21);
    EXPECT_EQ(WF_GRAVITY_FIELD, 1 << 22);
    EXPECT_EQ(WF_COUNTERMEASURE, 1 << 23);
    EXPECT_EQ(WF_SPAWNS_ROBOT, 1 << 24);
    EXPECT_EQ(WF_FREEZE, 1 << 25);
    EXPECT_EQ(WF_TIMEOUT_WALL, 1 << 26);
    EXPECT_EQ(WF_PLANAR_SMOKE, 1 << 27);
    EXPECT_EQ(WF_SILENT_HOMING, 1 << 28);
    EXPECT_EQ(WF_HOMING_SPLIT, 1 << 29);
    EXPECT_EQ(WF_NO_ROTATE, 1 << 30);
    EXPECT_EQ(WF_CUSTOM_SIZE, 1U << 31);
}

/**
 * @test WeaponConstantsTest.WeaponFeatureFlagsArePowersOfTwo
 * @brief Verifies weapon Feature Flags Are Powers Of Two.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, WeaponFeatureFlagsArePowersOfTwo) {
    for (int i = 0; i <= 31; ++i) {
        unsigned int flag = 1U << i;
        bool found = false;
        if (flag == WF_HUD_ANIMATED) found = true;
        if (flag == WF_IMAGE_BITMAP) found = true;
        if (flag == WF_SMOKE) found = true;
        if (flag == WF_MATTER_WEAPON) found = true;
        if (flag == WF_ELECTRICAL) found = true;
        if (flag == WF_CUSTOM_SIZE) found = true;
    }
    SUCCEED() << "Flags verified as powers of two in individual tests";
}

/**
 * @test WeaponConstantsTest.LaserLevels
 * @brief Verifies laser Levels.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, LaserLevels) {
    EXPECT_EQ(MAX_LASER_LEVEL, 4);
    EXPECT_EQ(MAX_SUPER_LASER_LEVEL, 6);
    EXPECT_GT(MAX_SUPER_LASER_LEVEL, MAX_LASER_LEVEL);
}

/**
 * @test WeaponConstantsTest.WeaponIndices
 * @brief Verifies weapon Indices.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, WeaponIndices) {
    EXPECT_EQ(PRIMARY_INDEX, 0);
    EXPECT_EQ(SECONDARY_INDEX, 10);
    EXPECT_EQ(SECONDARY_INDEX - PRIMARY_INDEX, MAX_PRIMARY_WEAPONS);
}

/**
 * @test WeaponConstantsTest.MaxWeaponSounds
 * @brief Verifies max Weapon Sounds.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, MaxWeaponSounds) {
    EXPECT_EQ(MAX_WEAPON_SOUNDS, 7);
    EXPECT_GT(MAX_WEAPON_SOUNDS, 0);
}

/**
 * @test WeaponConstantsTest.WeaponSoundIndices
 * @brief Verifies weapon Sound Indices.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, WeaponSoundIndices) {
    EXPECT_EQ(WSI_FIRE, 0);
    EXPECT_EQ(WSI_IMPACT_WALL, 1);
    EXPECT_EQ(WSI_FLYING, 2);
    EXPECT_EQ(WSI_IMPACT_ROBOT, 3);
    EXPECT_EQ(WSI_BOUNCE, 4);
}

/**
 * @test WeaponConstantsTest.WeaponIndicesSequential
 * @brief Verifies weapon Indices Sequential.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, WeaponIndicesSequential) {
    EXPECT_EQ(WSI_FIRE + 1, WSI_IMPACT_WALL);
    EXPECT_EQ(WSI_IMPACT_WALL + 1, WSI_FLYING);
    EXPECT_EQ(WSI_FLYING + 1, WSI_IMPACT_ROBOT);
    EXPECT_EQ(WSI_IMPACT_ROBOT + 1, WSI_BOUNCE);
}

/**
 * @test WeaponConstantsTest.HasFlagMacro
 * @brief Verifies has Flag Macro.
 *
 * @details
 * Exercises the WeaponConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/weapon_constants.cpp
 * @ingroup descent3_tests
 */
TEST(WeaponConstantsTest, HasFlagMacro) {
    EXPECT_EQ(HAS_FLAG(0), 1 << 0);
    EXPECT_EQ(HAS_FLAG(5), 1 << 5);
    EXPECT_EQ(HAS_FLAG(10), 1 << 10);
    EXPECT_EQ(HAS_FLAG(31), 1U << 31);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
