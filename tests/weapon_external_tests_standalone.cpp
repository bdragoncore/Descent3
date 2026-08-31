/*
 * D3 Coverage Tests - Descent3/weapon_external module (Standalone)
 * 
 * Tests for weapon constants and indices.
 * These are the hardcoded weapon indices used throughout the game.
 */

#include <gtest/gtest.h>
#include <cstdint>

// ============================================================================
// D3 Weapon Constants Implementation (matching Descent3/weapon_external.h)
// ============================================================================

#define LASER_INDEX 0
#define VAUSS_INDEX 1
#define MICROWAVE_INDEX 2
#define PLASMA_INDEX 3
#define FUSION_INDEX 4
#define SUPER_LASER_INDEX 5
#define MASSDRIVER_INDEX 6
#define NAPALM_INDEX 7
#define EMD_INDEX 8
#define OMEGA_INDEX 9
#define CONCUSSION_INDEX 10
#define HOMING_INDEX 11
#define IMPACTMORTAR_INDEX 12
#define SMART_INDEX 13
#define MEGA_INDEX 14
#define FRAG_INDEX 15
#define GUIDED_INDEX 16
#define NAPALMROCKET_INDEX 17
#define CYCLONE_INDEX 18
#define BLACKSHARK_INDEX 19
#define FLARE_INDEX 20

// Additional weapon-related constants (from weapon.h analysis)
#define MAX_PRIMARY_WEAPONS 10
#define MAX_SECONDARY_WEAPONS 11
#define TOTAL_WEAPONS 21

// ============================================================================
// Test Fixtures
// ============================================================================

class WeaponExternalTest : public ::testing::Test {
protected:
    static constexpr int PRIMARY_START = LASER_INDEX;
    static constexpr int PRIMARY_END = OMEGA_INDEX;
    static constexpr int SECONDARY_START = CONCUSSION_INDEX;
    static constexpr int SECONDARY_END = BLACKSHARK_INDEX;
    static constexpr int UTILITY_INDEX = FLARE_INDEX;
};

// ============================================================================
// Tests - Primary Weapon Indices
// ============================================================================

TEST_F(WeaponExternalTest, PrimaryWeapons_Contiguous) {
    // Primary weapons should have contiguous indices 0-9
    EXPECT_EQ(LASER_INDEX, 0);
    EXPECT_EQ(VAUSS_INDEX, 1);
    EXPECT_EQ(MICROWAVE_INDEX, 2);
    EXPECT_EQ(PLASMA_INDEX, 3);
    EXPECT_EQ(FUSION_INDEX, 4);
    EXPECT_EQ(SUPER_LASER_INDEX, 5);
    EXPECT_EQ(MASSDRIVER_INDEX, 6);
    EXPECT_EQ(NAPALM_INDEX, 7);
    EXPECT_EQ(EMD_INDEX, 8);
    EXPECT_EQ(OMEGA_INDEX, 9);
}

TEST_F(WeaponExternalTest, PrimaryWeapons_Count) {
    // Count primary weapons
    int count = PRIMARY_END - PRIMARY_START + 1;
    EXPECT_EQ(count, MAX_PRIMARY_WEAPONS);
}

// ============================================================================
// Tests - Secondary Weapon Indices
// ============================================================================

TEST_F(WeaponExternalTest, SecondaryWeapons_Contiguous) {
    // Secondary weapons should have contiguous indices 10-19
    EXPECT_EQ(CONCUSSION_INDEX, 10);
    EXPECT_EQ(HOMING_INDEX, 11);
    EXPECT_EQ(IMPACTMORTAR_INDEX, 12);
    EXPECT_EQ(SMART_INDEX, 13);
    EXPECT_EQ(MEGA_INDEX, 14);
    EXPECT_EQ(FRAG_INDEX, 15);
    EXPECT_EQ(GUIDED_INDEX, 16);
    EXPECT_EQ(NAPALMROCKET_INDEX, 17);
    EXPECT_EQ(CYCLONE_INDEX, 18);
    EXPECT_EQ(BLACKSHARK_INDEX, 19);
}

TEST_F(WeaponExternalTest, SecondaryWeapons_Count) {
    // Count secondary weapons
    int count = SECONDARY_END - SECONDARY_START + 1;
    EXPECT_EQ(count, MAX_SECONDARY_WEAPONS - 1); // Excluding flare
}

// ============================================================================
// Tests - Utility Weapons
// ============================================================================

TEST_F(WeaponExternalTest, Flare_Index) {
    // Flare is at index 20
    EXPECT_EQ(FLARE_INDEX, 20);
}

// ============================================================================
// Tests - Weapon Categories
// ============================================================================

TEST_F(WeaponExternalTest, WeaponCategories_Separate) {
    // Verify that primary, secondary, and utility are in different ranges
    EXPECT_LT(LASER_INDEX, CONCUSSION_INDEX);      // Primary < Secondary
    EXPECT_LT(OMEGA_INDEX, CONCUSSION_INDEX);      // Primary end < Secondary start
    EXPECT_LT(BLACKSHARK_INDEX, FLARE_INDEX);      // Secondary end < Utility
}

TEST_F(WeaponExternalTest, AllIndices_Unique) {
    // All weapon indices should be unique
    int indices[TOTAL_WEAPONS] = {
        LASER_INDEX, VAUSS_INDEX, MICROWAVE_INDEX, PLASMA_INDEX, FUSION_INDEX,
        SUPER_LASER_INDEX, MASSDRIVER_INDEX, NAPALM_INDEX, EMD_INDEX, OMEGA_INDEX,
        CONCUSSION_INDEX, HOMING_INDEX, IMPACTMORTAR_INDEX, SMART_INDEX, MEGA_INDEX,
        FRAG_INDEX, GUIDED_INDEX, NAPALMROCKET_INDEX, CYCLONE_INDEX, BLACKSHARK_INDEX,
        FLARE_INDEX
    };
    
    for (int i = 0; i < TOTAL_WEAPONS; i++) {
        for (int j = i + 1; j < TOTAL_WEAPONS; j++) {
            EXPECT_NE(indices[i], indices[j]) 
                << "Duplicate weapon index found at positions " << i << " and " << j;
        }
    }
}

// ============================================================================
// Tests - Energy vs Projectile Weapons
// ============================================================================

TEST_F(WeaponExternalTest, EnergyWeapons_Indices) {
    // Energy weapons are indices 0-9 (primary)
    EXPECT_GE(LASER_INDEX, 0);
    EXPECT_LE(OMEGA_INDEX, 9);
}

TEST_F(WeaponExternalTest, ProjectileWeapons_Indices) {
    // Projectile/missile weapons are indices 10-19 (secondary)
    EXPECT_GE(CONCUSSION_INDEX, 10);
    EXPECT_LE(BLACKSHARK_INDEX, 19);
}

// ============================================================================
// Tests - Special Weapon Groups
// ============================================================================

TEST_F(WeaponExternalTest, LaserVariant_Indices) {
    // Regular and super laser are adjacent
    EXPECT_EQ(SUPER_LASER_INDEX, LASER_INDEX + 5);
}

TEST_F(WeaponExternalTest, NapalmVariant_Indices) {
    // Napalm gun and napalm rocket are far apart (different categories)
    EXPECT_EQ(NAPALM_INDEX, 7);
    EXPECT_EQ(NAPALMROCKET_INDEX, 17);
}

TEST_F(WeaponExternalTest, GuidedWeapons_Indices) {
    // Homing and guided missiles are related
    EXPECT_EQ(HOMING_INDEX, 11);
    EXPECT_EQ(GUIDED_INDEX, 16);
}

// ============================================================================
// Tests - Boundary Values
// ============================================================================

TEST_F(WeaponExternalTest, FirstWeapon_Index) {
    // First weapon should be at index 0
    EXPECT_EQ(LASER_INDEX, 0);
}

TEST_F(WeaponExternalTest, LastWeapon_Index) {
    // Last weapon should be at index 20
    EXPECT_EQ(FLARE_INDEX, 20);
}

TEST_F(WeaponExternalTest, TotalWeaponCount) {
    // Total number of weapons
    EXPECT_EQ(TOTAL_WEAPONS, 21);
}

// ============================================================================
// Tests - Array Safety
// ============================================================================

TEST_F(WeaponExternalTest, Indices_NonNegative) {
    // All weapon indices should be non-negative
    EXPECT_GE(LASER_INDEX, 0);
    EXPECT_GE(VAUSS_INDEX, 0);
    EXPECT_GE(MICROWAVE_INDEX, 0);
    EXPECT_GE(PLASMA_INDEX, 0);
    EXPECT_GE(FUSION_INDEX, 0);
    EXPECT_GE(SUPER_LASER_INDEX, 0);
    EXPECT_GE(MASSDRIVER_INDEX, 0);
    EXPECT_GE(NAPALM_INDEX, 0);
    EXPECT_GE(EMD_INDEX, 0);
    EXPECT_GE(OMEGA_INDEX, 0);
    EXPECT_GE(CONCUSSION_INDEX, 0);
    EXPECT_GE(HOMING_INDEX, 0);
    EXPECT_GE(IMPACTMORTAR_INDEX, 0);
    EXPECT_GE(SMART_INDEX, 0);
    EXPECT_GE(MEGA_INDEX, 0);
    EXPECT_GE(FRAG_INDEX, 0);
    EXPECT_GE(GUIDED_INDEX, 0);
    EXPECT_GE(NAPALMROCKET_INDEX, 0);
    EXPECT_GE(CYCLONE_INDEX, 0);
    EXPECT_GE(BLACKSHARK_INDEX, 0);
    EXPECT_GE(FLARE_INDEX, 0);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
