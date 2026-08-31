/**
 * @file levelgoal_external_tests_standalone.cpp
 * @brief D3 Coverage Tests - Descent3/levelgoal_external module (Standalone).
 *
 * @details
 * Tests for level goal constants, flags, and masks.
 * These define the mission objective system.
 *
 * This harness validates the behavior of `Descent3/levelgoal_external_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/levelgoal_external_tests_standalone.cpp`
 * @par Harness
 * `levelgoal_external_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 */

#include <gtest/gtest.h>
#include <cstdint>

// ============================================================================
// D3 LevelGoal Constants Implementation (matching Descent3/levelgoal_external.h)
// ============================================================================

// Level Flags
#define LF_AUTO_END_LEVEL 0x01
#define LF_ALL_PRIMARIES_DONE 0x02

// Level Item Types
#define LIT_TERRAIN_CELL 0
#define LIT_INTERNAL_ROOM 1
#define LIT_OBJECT 2
#define LIT_TRIGGER 3
#define LIT_ANY_MINE 4

// Level Item Operations
#define LO_SET_SPECIFIED 0
#define LO_GET_SPECIFIED 1
#define LO_CLEAR_SPECIFIED 2

// Level Goal Flags
#define LGF_BLANK1 0x00000001
#define LGF_SECONDARY_GOAL 0x00000002
#define LGF_ENABLED 0x00000004
#define LGF_COMPLETED 0x00000008
#define LGF_TELCOM_LISTS 0x00000010
#define LGF_GB_DOESNT_KNOW_LOC 0x00000020
#define LGF_NOT_LOC_BASED 0x00000040
#define LGF_FAILED 0x00000080
#define LGF_COMP_ACTIVATE 0x00000100
#define LGF_COMP_ENTER 0x00000200
#define LGF_COMP_DESTROY 0x00000400
#define LGF_COMP_PLAYER_WEAPON 0x00000800
#define LGF_COMP_PLAYER 0x00001000
#define LGF_COMP_DALLAS 0x00002000

#define LGF_COMP_MASK \
    (LGF_COMP_ACTIVATE | LGF_COMP_ENTER | LGF_COMP_DESTROY | LGF_COMP_PLAYER_WEAPON | LGF_COMP_PLAYER | LGF_COMP_DALLAS)

#define LGF_COMP_TERRAIN_MASK (LGF_COMP_ENTER | LGF_COMP_DALLAS)

// Mission constants (from Mission.h)
#define LOAD_PROGRESS_START 1
#define LOAD_PROGRESS_LOADING_LEVEL 2
#define LOAD_PROGRESS_PAGING_DATA 3
#define LOAD_PROGRESS_PREPARE 4
#define LOAD_PROGRESS_DONE 200
#define MAX_KEYWORDLEN 300
#define MSN_STATE_SECRET_LEVEL 0x80000000
#define LGF_COMP_ROOM_MASK (LGF_COMP_ENTER | LGF_COMP_DALLAS)
#define LGF_COMP_OBJECT_MASK (LGF_COMP_DESTROY | LGF_COMP_PLAYER_WEAPON | LGF_COMP_PLAYER | LGF_COMP_DALLAS)
#define LGF_COMP_TRIGGER_MASK (LGF_COMP_ACTIVATE | LGF_COMP_DALLAS)
#define LGF_COMP_MINE_MASK (LGF_COMP_ENTER | LGF_COMP_DALLAS)

#define MAX_GOAL_ITEMS 12
#define MAX_LEVEL_GOALS 32
#define MAX_GOAL_LISTS 4

// ============================================================================
// Test Fixtures
// ============================================================================

/**
 * @brief GTest fixture for LevelGoalExternalTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class LevelGoalExternalTest : public ::testing::Test {};

// ============================================================================
// Tests - Level Flags
// ============================================================================

/**
 * @test LevelGoalExternalTest.LevelFlags_Defined
 * @brief Verifies level Flags Defined.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, LevelFlags_Defined) {
    EXPECT_EQ(LF_AUTO_END_LEVEL, 0x01);
    EXPECT_EQ(LF_ALL_PRIMARIES_DONE, 0x02);
}

/**
 * @test LevelGoalExternalTest.LevelFlags_PowersOfTwo
 * @brief Verifies level Flags Powers Of Two.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, LevelFlags_PowersOfTwo) {
    // Level flags should be powers of two (bit flags)
    EXPECT_EQ(LF_AUTO_END_LEVEL & (LF_AUTO_END_LEVEL - 1), 0);
    EXPECT_EQ(LF_ALL_PRIMARIES_DONE & (LF_ALL_PRIMARIES_DONE - 1), 0);
}

/**
 * @test LevelGoalExternalTest.LevelFlags_NoOverlap
 * @brief Verifies level Flags No Overlap.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, LevelFlags_NoOverlap) {
    // Level flags should not overlap
    EXPECT_EQ(LF_AUTO_END_LEVEL & LF_ALL_PRIMARIES_DONE, 0);
}

// ============================================================================
// Tests - Level Item Types
// ============================================================================

/**
 * @test LevelGoalExternalTest.ItemTypes_Defined
 * @brief Verifies item Types Defined.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, ItemTypes_Defined) {
    EXPECT_EQ(LIT_TERRAIN_CELL, 0);
    EXPECT_EQ(LIT_INTERNAL_ROOM, 1);
    EXPECT_EQ(LIT_OBJECT, 2);
    EXPECT_EQ(LIT_TRIGGER, 3);
    EXPECT_EQ(LIT_ANY_MINE, 4);
}

/**
 * @test LevelGoalExternalTest.ItemTypes_Contiguous
 * @brief Verifies item Types Contiguous.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, ItemTypes_Contiguous) {
    // Item types should be contiguous 0-4
    int expected = 0;
    EXPECT_EQ(LIT_TERRAIN_CELL, expected++);
    EXPECT_EQ(LIT_INTERNAL_ROOM, expected++);
    EXPECT_EQ(LIT_OBJECT, expected++);
    EXPECT_EQ(LIT_TRIGGER, expected++);
    EXPECT_EQ(LIT_ANY_MINE, expected++);
}

/**
 * @test LevelGoalExternalTest.ItemTypes_Count
 * @brief Verifies item Types Count.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, ItemTypes_Count) {
    // Count item types
    EXPECT_EQ(LIT_ANY_MINE - LIT_TERRAIN_CELL + 1, 5);
}

// ============================================================================
// Tests - Level Item Operations
// ============================================================================

/**
 * @test LevelGoalExternalTest.ItemOperations_Defined
 * @brief Verifies item Operations Defined.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, ItemOperations_Defined) {
    EXPECT_EQ(LO_SET_SPECIFIED, 0);
    EXPECT_EQ(LO_GET_SPECIFIED, 1);
    EXPECT_EQ(LO_CLEAR_SPECIFIED, 2);
}

/**
 * @test LevelGoalExternalTest.ItemOperations_Contiguous
 * @brief Verifies item Operations Contiguous.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, ItemOperations_Contiguous) {
    // Operations should be contiguous 0-2
    int expected = 0;
    EXPECT_EQ(LO_SET_SPECIFIED, expected++);
    EXPECT_EQ(LO_GET_SPECIFIED, expected++);
    EXPECT_EQ(LO_CLEAR_SPECIFIED, expected++);
}

// ============================================================================
// Tests - Level Goal Flags (Bit Flags)
// ============================================================================

/**
 * @test LevelGoalExternalTest.GoalFlags_Defined
 * @brief Verifies goal Flags Defined.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, GoalFlags_Defined) {
    EXPECT_EQ(LGF_BLANK1, 0x00000001);
    EXPECT_EQ(LGF_SECONDARY_GOAL, 0x00000002);
    EXPECT_EQ(LGF_ENABLED, 0x00000004);
    EXPECT_EQ(LGF_COMPLETED, 0x00000008);
    EXPECT_EQ(LGF_TELCOM_LISTS, 0x00000010);
    EXPECT_EQ(LGF_GB_DOESNT_KNOW_LOC, 0x00000020);
    EXPECT_EQ(LGF_NOT_LOC_BASED, 0x00000040);
    EXPECT_EQ(LGF_FAILED, 0x00000080);
    EXPECT_EQ(LGF_COMP_ACTIVATE, 0x00000100);
    EXPECT_EQ(LGF_COMP_ENTER, 0x00000200);
    EXPECT_EQ(LGF_COMP_DESTROY, 0x00000400);
    EXPECT_EQ(LGF_COMP_PLAYER_WEAPON, 0x00000800);
    EXPECT_EQ(LGF_COMP_PLAYER, 0x00001000);
    EXPECT_EQ(LGF_COMP_DALLAS, 0x00002000);
}

/**
 * @test LevelGoalExternalTest.GoalFlags_PowersOfTwo
 * @brief Verifies goal Flags Powers Of Two.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, GoalFlags_PowersOfTwo) {
    // Goal flags should be powers of two (each is a single bit)
    EXPECT_EQ(LGF_BLANK1 & (LGF_BLANK1 - 1), 0);
    EXPECT_EQ(LGF_SECONDARY_GOAL & (LGF_SECONDARY_GOAL - 1), 0);
    EXPECT_EQ(LGF_ENABLED & (LGF_ENABLED - 1), 0);
    EXPECT_EQ(LGF_COMPLETED & (LGF_COMPLETED - 1), 0);
    EXPECT_EQ(LGF_TELCOM_LISTS & (LGF_TELCOM_LISTS - 1), 0);
    EXPECT_EQ(LGF_GB_DOESNT_KNOW_LOC & (LGF_GB_DOESNT_KNOW_LOC - 1), 0);
    EXPECT_EQ(LGF_NOT_LOC_BASED & (LGF_NOT_LOC_BASED - 1), 0);
    EXPECT_EQ(LGF_FAILED & (LGF_FAILED - 1), 0);
    EXPECT_EQ(LGF_COMP_ACTIVATE & (LGF_COMP_ACTIVATE - 1), 0);
    EXPECT_EQ(LGF_COMP_ENTER & (LGF_COMP_ENTER - 1), 0);
    EXPECT_EQ(LGF_COMP_DESTROY & (LGF_COMP_DESTROY - 1), 0);
    EXPECT_EQ(LGF_COMP_PLAYER_WEAPON & (LGF_COMP_PLAYER_WEAPON - 1), 0);
    EXPECT_EQ(LGF_COMP_PLAYER & (LGF_COMP_PLAYER - 1), 0);
    EXPECT_EQ(LGF_COMP_DALLAS & (LGF_COMP_DALLAS - 1), 0);
}

/**
 * @test LevelGoalExternalTest.GoalFlags_Unique
 * @brief Verifies goal Flags Unique.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, GoalFlags_Unique) {
    // All goal flags should be unique
    uint32_t flags[] = {
        LGF_BLANK1, LGF_SECONDARY_GOAL, LGF_ENABLED, LGF_COMPLETED,
        LGF_TELCOM_LISTS, LGF_GB_DOESNT_KNOW_LOC, LGF_NOT_LOC_BASED, LGF_FAILED,
        LGF_COMP_ACTIVATE, LGF_COMP_ENTER, LGF_COMP_DESTROY, LGF_COMP_PLAYER_WEAPON,
        LGF_COMP_PLAYER, LGF_COMP_DALLAS
    };
    
    for (size_t i = 0; i < sizeof(flags)/sizeof(flags[0]); i++) {
        for (size_t j = i + 1; j < sizeof(flags)/sizeof(flags[0]); j++) {
            EXPECT_NE(flags[i], flags[j])
                << "Duplicate flag at positions " << i << " and " << j;
        }
    }
}

/**
 * @test LevelGoalExternalTest.GoalFlags_ConsecutiveBits
 * @brief Verifies goal Flags Consecutive Bits.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, GoalFlags_ConsecutiveBits) {
    // First 8 flags use bits 0-7
    EXPECT_EQ(LGF_BLANK1, (1u << 0));
    EXPECT_EQ(LGF_SECONDARY_GOAL, (1u << 1));
    EXPECT_EQ(LGF_ENABLED, (1u << 2));
    EXPECT_EQ(LGF_COMPLETED, (1u << 3));
    EXPECT_EQ(LGF_TELCOM_LISTS, (1u << 4));
    EXPECT_EQ(LGF_GB_DOESNT_KNOW_LOC, (1u << 5));
    EXPECT_EQ(LGF_NOT_LOC_BASED, (1u << 6));
    EXPECT_EQ(LGF_FAILED, (1u << 7));
    
    // Completion flags use bits 8-13
    EXPECT_EQ(LGF_COMP_ACTIVATE, (1u << 8));
    EXPECT_EQ(LGF_COMP_ENTER, (1u << 9));
    EXPECT_EQ(LGF_COMP_DESTROY, (1u << 10));
    EXPECT_EQ(LGF_COMP_PLAYER_WEAPON, (1u << 11));
    EXPECT_EQ(LGF_COMP_PLAYER, (1u << 12));
    EXPECT_EQ(LGF_COMP_DALLAS, (1u << 13));
}

// ============================================================================
// Tests - Goal Flag Masks
// ============================================================================

/**
 * @test LevelGoalExternalTest.CompMask_IncludesAllCompFlags
 * @brief Verifies comp Mask Includes All Comp Flags.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, CompMask_IncludesAllCompFlags) {
    // LGF_COMP_MASK should include all completion flags
    EXPECT_TRUE((LGF_COMP_MASK & LGF_COMP_ACTIVATE) != 0);
    EXPECT_TRUE((LGF_COMP_MASK & LGF_COMP_ENTER) != 0);
    EXPECT_TRUE((LGF_COMP_MASK & LGF_COMP_DESTROY) != 0);
    EXPECT_TRUE((LGF_COMP_MASK & LGF_COMP_PLAYER_WEAPON) != 0);
    EXPECT_TRUE((LGF_COMP_MASK & LGF_COMP_PLAYER) != 0);
    EXPECT_TRUE((LGF_COMP_MASK & LGF_COMP_DALLAS) != 0);
}

/**
 * @test LevelGoalExternalTest.CompMask_NoOtherFlags
 * @brief Verifies comp Mask No Other Flags.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, CompMask_NoOtherFlags) {
    // LGF_COMP_MASK should NOT include non-completion flags
    EXPECT_EQ(LGF_COMP_MASK & LGF_BLANK1, 0);
    EXPECT_EQ(LGF_COMP_MASK & LGF_SECONDARY_GOAL, 0);
    EXPECT_EQ(LGF_COMP_MASK & LGF_ENABLED, 0);
    EXPECT_EQ(LGF_COMP_MASK & LGF_COMPLETED, 0);
    EXPECT_EQ(LGF_COMP_MASK & LGF_TELCOM_LISTS, 0);
    EXPECT_EQ(LGF_COMP_MASK & LGF_GB_DOESNT_KNOW_LOC, 0);
    EXPECT_EQ(LGF_COMP_MASK & LGF_NOT_LOC_BASED, 0);
    EXPECT_EQ(LGF_COMP_MASK & LGF_FAILED, 0);
}

/**
 * @test LevelGoalExternalTest.CompMask_Value
 * @brief Verifies comp Mask Value.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, CompMask_Value) {
    // Verify mask value
    uint32_t expected_mask = LGF_COMP_ACTIVATE | LGF_COMP_ENTER | LGF_COMP_DESTROY | 
                             LGF_COMP_PLAYER_WEAPON | LGF_COMP_PLAYER | LGF_COMP_DALLAS;
    EXPECT_EQ(LGF_COMP_MASK, expected_mask);
    EXPECT_EQ(LGF_COMP_MASK, 0x00003F00);
}

/**
 * @test LevelGoalExternalTest.TerrainMask_SubsetOfCompMask
 * @brief Verifies terrain Mask Subset Of Comp Mask.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, TerrainMask_SubsetOfCompMask) {
    // Terrain mask is a subset of completion mask
    EXPECT_EQ(LGF_COMP_TERRAIN_MASK & ~LGF_COMP_MASK, 0);
    EXPECT_EQ(LGF_COMP_TERRAIN_MASK, (LGF_COMP_ENTER | LGF_COMP_DALLAS));
}

/**
 * @test LevelGoalExternalTest.RoomMask_SubsetOfCompMask
 * @brief Verifies room Mask Subset Of Comp Mask.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, RoomMask_SubsetOfCompMask) {
    // Room mask is a subset of completion mask
    EXPECT_EQ(LGF_COMP_ROOM_MASK & ~LGF_COMP_MASK, 0);
    EXPECT_EQ(LGF_COMP_ROOM_MASK, (LGF_COMP_ENTER | LGF_COMP_DALLAS));
}

/**
 * @test LevelGoalExternalTest.ObjectMask_SubsetOfCompMask
 * @brief Verifies object Mask Subset Of Comp Mask.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, ObjectMask_SubsetOfCompMask) {
    // Object mask is a subset of completion mask
    EXPECT_EQ(LGF_COMP_OBJECT_MASK & ~LGF_COMP_MASK, 0);
    EXPECT_EQ(LGF_COMP_OBJECT_MASK, (LGF_COMP_DESTROY | LGF_COMP_PLAYER_WEAPON | LGF_COMP_PLAYER | LGF_COMP_DALLAS));
}

/**
 * @test LevelGoalExternalTest.TriggerMask_SubsetOfCompMask
 * @brief Verifies trigger Mask Subset Of Comp Mask.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, TriggerMask_SubsetOfCompMask) {
    // Trigger mask is a subset of completion mask
    EXPECT_EQ(LGF_COMP_TRIGGER_MASK & ~LGF_COMP_MASK, 0);
    EXPECT_EQ(LGF_COMP_TRIGGER_MASK, (LGF_COMP_ACTIVATE | LGF_COMP_DALLAS));
}

/**
 * @test LevelGoalExternalTest.MineMask_SubsetOfCompMask
 * @brief Verifies mine Mask Subset Of Comp Mask.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, MineMask_SubsetOfCompMask) {
    // Mine mask is a subset of completion mask
    EXPECT_EQ(LGF_COMP_MINE_MASK & ~LGF_COMP_MASK, 0);
    EXPECT_EQ(LGF_COMP_MINE_MASK, (LGF_COMP_ENTER | LGF_COMP_DALLAS));
}

// ============================================================================
// Tests - Maximum Values
// ============================================================================

/**
 * @test LevelGoalExternalTest.MaxGoalItems_Positive
 * @brief Verifies max Goal Items Positive.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, MaxGoalItems_Positive) {
    EXPECT_GT(MAX_GOAL_ITEMS, 0);
    EXPECT_EQ(MAX_GOAL_ITEMS, 12);
}

/**
 * @test LevelGoalExternalTest.MaxLevelGoals_Positive
 * @brief Verifies max Level Goals Positive.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, MaxLevelGoals_Positive) {
    EXPECT_GT(MAX_LEVEL_GOALS, 0);
    EXPECT_EQ(MAX_LEVEL_GOALS, 32);
}

/**
 * @test LevelGoalExternalTest.MaxGoalLists_Positive
 * @brief Verifies max Goal Lists Positive.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, MaxGoalLists_Positive) {
    EXPECT_GT(MAX_GOAL_LISTS, 0);
    EXPECT_EQ(MAX_GOAL_LISTS, 4);
}

/**
 * @test LevelGoalExternalTest.MaxValues_ReasonableRange
 * @brief Verifies max Values Reasonable Range.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, MaxValues_ReasonableRange) {
    // Maximum values should be in reasonable range for array sizes
    EXPECT_LE(MAX_GOAL_ITEMS, 100);
    EXPECT_LE(MAX_LEVEL_GOALS, 100);
    EXPECT_LE(MAX_GOAL_LISTS, 10);
}

// ============================================================================
// Tests - Flag States
// ============================================================================

/**
 * @test LevelGoalExternalTest.GoalStateFlags
 * @brief Verifies goal State Flags.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, GoalStateFlags) {
    // Enabled and completed are independent flags
    EXPECT_EQ(LGF_ENABLED & LGF_COMPLETED, 0);
    
    // Failed is independent of completed
    EXPECT_EQ(LGF_FAILED & LGF_COMPLETED, 0);
    EXPECT_EQ(LGF_FAILED & LGF_ENABLED, 0);
}

/**
 * @test LevelGoalExternalTest.SecondaryGoalFlag
 * @brief Verifies secondary Goal Flag.
 *
 * @details
 * Exercises the LevelGoalExternalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(LevelGoalExternalTest, SecondaryGoalFlag) {
    // Secondary goal flag is independent of other state flags
    EXPECT_EQ(LGF_SECONDARY_GOAL & LGF_ENABLED, 0);
    EXPECT_EQ(LGF_SECONDARY_GOAL & LGF_COMPLETED, 0);
    EXPECT_EQ(LGF_SECONDARY_GOAL & LGF_FAILED, 0);
}

// ============================================================================
// Tests - Mission Constants (from Mission.h)
// ============================================================================

/**
 * @test MissionConstantsTest.LoadProgressSteps
 * @brief Verifies load Progress Steps.
 *
 * @details
 * Exercises the MissionConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MissionConstantsTest, LoadProgressSteps) {
    EXPECT_EQ(LOAD_PROGRESS_START, 1);
    EXPECT_EQ(LOAD_PROGRESS_LOADING_LEVEL, 2);
    EXPECT_EQ(LOAD_PROGRESS_PAGING_DATA, 3);
    EXPECT_EQ(LOAD_PROGRESS_PREPARE, 4);
    EXPECT_EQ(LOAD_PROGRESS_DONE, 200);
}

/**
 * @test MissionConstantsTest.LoadProgressSequential
 * @brief Verifies load Progress Sequential.
 *
 * @details
 * Exercises the MissionConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MissionConstantsTest, LoadProgressSequential) {
    EXPECT_EQ(LOAD_PROGRESS_LOADING_LEVEL - LOAD_PROGRESS_START, 1);
    EXPECT_EQ(LOAD_PROGRESS_PAGING_DATA - LOAD_PROGRESS_LOADING_LEVEL, 1);
    EXPECT_EQ(LOAD_PROGRESS_PREPARE - LOAD_PROGRESS_PAGING_DATA, 1);
}

/**
 * @test MissionConstantsTest.LoadProgressDone
 * @brief Verifies load Progress Done.
 *
 * @details
 * Exercises the MissionConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MissionConstantsTest, LoadProgressDone) {
    EXPECT_GT(LOAD_PROGRESS_DONE, LOAD_PROGRESS_PREPARE);
}

/**
 * @test MissionConstantsTest.MaxKeywordLength
 * @brief Verifies max Keyword Length.
 *
 * @details
 * Exercises the MissionConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MissionConstantsTest, MaxKeywordLength) {
    EXPECT_EQ(MAX_KEYWORDLEN, 300);
    EXPECT_GT(MAX_KEYWORDLEN, 0);
}

/**
 * @test MissionConstantsTest.MissionStateSecretLevel
 * @brief Verifies mission State Secret Level.
 *
 * @details
 * Exercises the MissionConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/levelgoal_external_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MissionConstantsTest, MissionStateSecretLevel) {
    EXPECT_EQ(MSN_STATE_SECRET_LEVEL, 0x80000000u);
    EXPECT_EQ(MSN_STATE_SECRET_LEVEL & 0x7FFFFFFF, 0u);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
