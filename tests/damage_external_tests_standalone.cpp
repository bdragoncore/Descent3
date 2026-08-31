/*
 * D3 Coverage Tests - Descent3/damage_external module (Standalone)
 * 
 * Tests for damage type constants.
 * These are used to categorize different types of damage in the game.
 */

#include <gtest/gtest.h>
#include <cstdint>

// ============================================================================
// D3 Damage Constants Implementation (matching Descent3/damage_external.h)
// ============================================================================

// Player Damage types.  Used only to make a sound.
#define PD_NONE 0             // Make no sound
#define PD_ENERGY_WEAPON 1    // Hit by laser, etc.
#define PD_MATTER_WEAPON 2    // Hit by missile, etc.
#define PD_MELEE_ATTACK 3     // Whacked by robot
#define PD_CONCUSSIVE_FORCE 4 // Hit by shockwave
#define PD_WALL_HIT 5         // Crashed into a wall
#define PD_VOLATILE_HISS 6    // Touched a volatile substance (such as acid)

// Generic damage types
#define GD_SCRIPTED 0      // Script is saying to do the damage
#define GD_ELECTRIC 1      // Electrical weapons
#define GD_CONCUSSIVE 2    // Concussive damage
#define GD_FIRE 3          // Fire and napalm like stuff
#define GD_MATTER 4        // Matter weapons
#define GD_ENERGY 5        // Energy weapons and fields
#define GD_PHYSICS 6       // Bumping into a wall or player too hard
#define GD_MELEE_ATTACK 7  // From a melee robot attack
#define GD_VOLATILE_HISS 8 // Touched a volatile substance (such as acid)

// Count of damage types
#define PD_COUNT 7
#define GD_COUNT 9

// Damage magnitude constants (from damage.h)
#define MAX_DAMAGE_MAG 20.0f
#define MAX_EDRAIN_MAG 18.0f

// ============================================================================
// Test Fixtures
// ============================================================================

class DamageExternalTest : public ::testing::Test {
protected:
    static constexpr int PD_START = PD_NONE;
    static constexpr int PD_END = PD_VOLATILE_HISS;
    static constexpr int GD_START = GD_SCRIPTED;
    static constexpr int GD_END = GD_VOLATILE_HISS;
};

// ============================================================================
// Tests - Player Damage Type Constants
// ============================================================================

TEST_F(DamageExternalTest, PlayerDamageTypes_Defined) {
    // Verify all player damage type constants are defined with expected values
    EXPECT_EQ(PD_NONE, 0);
    EXPECT_EQ(PD_ENERGY_WEAPON, 1);
    EXPECT_EQ(PD_MATTER_WEAPON, 2);
    EXPECT_EQ(PD_MELEE_ATTACK, 3);
    EXPECT_EQ(PD_CONCUSSIVE_FORCE, 4);
    EXPECT_EQ(PD_WALL_HIT, 5);
    EXPECT_EQ(PD_VOLATILE_HISS, 6);
}

TEST_F(DamageExternalTest, PlayerDamageTypes_Contiguous) {
    // Player damage types should be contiguous from 0-6
    int expected = 0;
    EXPECT_EQ(PD_NONE, expected++);
    EXPECT_EQ(PD_ENERGY_WEAPON, expected++);
    EXPECT_EQ(PD_MATTER_WEAPON, expected++);
    EXPECT_EQ(PD_MELEE_ATTACK, expected++);
    EXPECT_EQ(PD_CONCUSSIVE_FORCE, expected++);
    EXPECT_EQ(PD_WALL_HIT, expected++);
    EXPECT_EQ(PD_VOLATILE_HISS, expected++);
}

TEST_F(DamageExternalTest, PlayerDamageTypes_Count) {
    // Verify count matches expected
    int count = PD_END - PD_START + 1;
    EXPECT_EQ(count, PD_COUNT);
}

// ============================================================================
// Tests - Generic Damage Type Constants
// ============================================================================

TEST_F(DamageExternalTest, GenericDamageTypes_Defined) {
    // Verify all generic damage type constants are defined with expected values
    EXPECT_EQ(GD_SCRIPTED, 0);
    EXPECT_EQ(GD_ELECTRIC, 1);
    EXPECT_EQ(GD_CONCUSSIVE, 2);
    EXPECT_EQ(GD_FIRE, 3);
    EXPECT_EQ(GD_MATTER, 4);
    EXPECT_EQ(GD_ENERGY, 5);
    EXPECT_EQ(GD_PHYSICS, 6);
    EXPECT_EQ(GD_MELEE_ATTACK, 7);
    EXPECT_EQ(GD_VOLATILE_HISS, 8);
}

TEST_F(DamageExternalTest, GenericDamageTypes_Contiguous) {
    // Generic damage types should be contiguous from 0-8
    int expected = 0;
    EXPECT_EQ(GD_SCRIPTED, expected++);
    EXPECT_EQ(GD_ELECTRIC, expected++);
    EXPECT_EQ(GD_CONCUSSIVE, expected++);
    EXPECT_EQ(GD_FIRE, expected++);
    EXPECT_EQ(GD_MATTER, expected++);
    EXPECT_EQ(GD_ENERGY, expected++);
    EXPECT_EQ(GD_PHYSICS, expected++);
    EXPECT_EQ(GD_MELEE_ATTACK, expected++);
    EXPECT_EQ(GD_VOLATILE_HISS, expected++);
}

TEST_F(DamageExternalTest, GenericDamageTypes_Count) {
    // Verify count matches expected
    int count = GD_END - GD_START + 1;
    EXPECT_EQ(count, GD_COUNT);
}

// ============================================================================
// Tests - Damage Type Overlap
// ============================================================================

TEST_F(DamageExternalTest, SharedDamageTypes) {
    // Some damage types exist in both player and generic categories
    EXPECT_EQ(PD_MELEE_ATTACK, 3);
    EXPECT_EQ(GD_MELEE_ATTACK, 7);
    // They have different values (purpose is different)
    EXPECT_NE(PD_MELEE_ATTACK, GD_MELEE_ATTACK);
}

TEST_F(DamageExternalTest, VolatileHiss_BothCategories) {
    // VOLATILE_HISS exists in both
    EXPECT_EQ(PD_VOLATILE_HISS, 6);
    EXPECT_EQ(GD_VOLATILE_HISS, 8);
    EXPECT_NE(PD_VOLATILE_HISS, GD_VOLATILE_HISS);
}

// ============================================================================
// Tests - Damage Type Categories
// ============================================================================

TEST_F(DamageExternalTest, EnergyDamage_Types) {
    // Energy-related damage types
    EXPECT_EQ(PD_ENERGY_WEAPON, 1);
    EXPECT_EQ(GD_ELECTRIC, 1);
    EXPECT_EQ(GD_ENERGY, 5);
}

TEST_F(DamageExternalTest, MatterDamage_Types) {
    // Matter/physical damage types
    EXPECT_EQ(PD_MATTER_WEAPON, 2);
    EXPECT_EQ(GD_MATTER, 4);
    EXPECT_EQ(GD_PHYSICS, 6);
}

TEST_F(DamageExternalTest, ExplosiveDamage_Types) {
    // Explosive/concussive damage types
    EXPECT_EQ(PD_CONCUSSIVE_FORCE, 4);
    EXPECT_EQ(GD_CONCUSSIVE, 2);
}

TEST_F(DamageExternalTest, FireDamage_Types) {
    // Fire damage
    EXPECT_EQ(GD_FIRE, 3);
}

TEST_F(DamageExternalTest, EnvironmentalDamage_Types) {
    // Environmental damage types
    EXPECT_EQ(PD_WALL_HIT, 5);
    EXPECT_EQ(PD_VOLATILE_HISS, 6);
    EXPECT_EQ(GD_VOLATILE_HISS, 8);
}

// ============================================================================
// Tests - Special Damage Types
// ============================================================================

TEST_F(DamageExternalTest, NoneDamage_IsZero) {
    // PD_NONE should be 0 (represents no damage)
    EXPECT_EQ(PD_NONE, 0);
}

TEST_F(DamageExternalTest, ScriptedDamage_IsZero) {
    // GD_SCRIPTED is also 0 (script-controlled damage)
    EXPECT_EQ(GD_SCRIPTED, 0);
}

// ============================================================================
// Tests - Array Safety
// ============================================================================

TEST_F(DamageExternalTest, PlayerDamageIndices_InRange) {
    // All player damage indices should be non-negative
    EXPECT_GE(PD_NONE, 0);
    EXPECT_GE(PD_ENERGY_WEAPON, 0);
    EXPECT_GE(PD_MATTER_WEAPON, 0);
    EXPECT_GE(PD_MELEE_ATTACK, 0);
    EXPECT_GE(PD_CONCUSSIVE_FORCE, 0);
    EXPECT_GE(PD_WALL_HIT, 0);
    EXPECT_GE(PD_VOLATILE_HISS, 0);
}

TEST_F(DamageExternalTest, GenericDamageIndices_InRange) {
    // All generic damage indices should be non-negative
    EXPECT_GE(GD_SCRIPTED, 0);
    EXPECT_GE(GD_ELECTRIC, 0);
    EXPECT_GE(GD_CONCUSSIVE, 0);
    EXPECT_GE(GD_FIRE, 0);
    EXPECT_GE(GD_MATTER, 0);
    EXPECT_GE(GD_ENERGY, 0);
    EXPECT_GE(GD_PHYSICS, 0);
    EXPECT_GE(GD_MELEE_ATTACK, 0);
    EXPECT_GE(GD_VOLATILE_HISS, 0);
}

TEST_F(DamageExternalTest, PlayerDamageIndices_Bounded) {
    // All player damage indices should be < 10 for safe array usage
    EXPECT_LT(PD_NONE, 10);
    EXPECT_LT(PD_ENERGY_WEAPON, 10);
    EXPECT_LT(PD_MATTER_WEAPON, 10);
    EXPECT_LT(PD_MELEE_ATTACK, 10);
    EXPECT_LT(PD_CONCUSSIVE_FORCE, 10);
    EXPECT_LT(PD_WALL_HIT, 10);
    EXPECT_LT(PD_VOLATILE_HISS, 10);
}

TEST_F(DamageExternalTest, GenericDamageIndices_Bounded) {
    // All generic damage indices should be < 10 for safe array usage
    EXPECT_LT(GD_SCRIPTED, 10);
    EXPECT_LT(GD_ELECTRIC, 10);
    EXPECT_LT(GD_CONCUSSIVE, 10);
    EXPECT_LT(GD_FIRE, 10);
    EXPECT_LT(GD_MATTER, 10);
    EXPECT_LT(GD_ENERGY, 10);
    EXPECT_LT(GD_PHYSICS, 10);
    EXPECT_LT(GD_MELEE_ATTACK, 10);
    EXPECT_LT(GD_VOLATILE_HISS, 10);
}

// ============================================================================
// Tests - Damage Type Uniqueness
// ============================================================================

TEST_F(DamageExternalTest, PlayerDamageTypes_Unique) {
    // All player damage types should be unique
    int types[PD_COUNT] = {PD_NONE, PD_ENERGY_WEAPON, PD_MATTER_WEAPON, 
                           PD_MELEE_ATTACK, PD_CONCUSSIVE_FORCE, PD_WALL_HIT, PD_VOLATILE_HISS};
    
    for (int i = 0; i < PD_COUNT; i++) {
        for (int j = i + 1; j < PD_COUNT; j++) {
            EXPECT_NE(types[i], types[j])
                << "Duplicate player damage type at positions " << i << " and " << j;
        }
    }
}

TEST_F(DamageExternalTest, GenericDamageTypes_Unique) {
    // All generic damage types should be unique
    int types[GD_COUNT] = {GD_SCRIPTED, GD_ELECTRIC, GD_CONCUSSIVE, GD_FIRE, GD_MATTER,
                           GD_ENERGY, GD_PHYSICS, GD_MELEE_ATTACK, GD_VOLATILE_HISS};
    
    for (int i = 0; i < GD_COUNT; i++) {
        for (int j = i + 1; j < GD_COUNT; j++) {
            EXPECT_NE(types[i], types[j])
                << "Duplicate generic damage type at positions " << i << " and " << j;
        }
    }
}

// ============================================================================
// Tests - Damage Magnitude Constants (from damage.h)
// ============================================================================

TEST(DamageMagnitudeTest, MaxDamageMagnitude) {
    EXPECT_EQ(MAX_DAMAGE_MAG, 20.0f);
    EXPECT_GT(MAX_DAMAGE_MAG, 0.0f);
}

TEST(DamageMagnitudeTest, MaxEnergyDrainMagnitude) {
    EXPECT_EQ(MAX_EDRAIN_MAG, 18.0f);
    EXPECT_GT(MAX_EDRAIN_MAG, 0.0f);
}

TEST(DamageMagnitudeTest, EnergyDrainLessThanDamage) {
    EXPECT_LT(MAX_EDRAIN_MAG, MAX_DAMAGE_MAG);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
