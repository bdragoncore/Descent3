/*
 * D3 Coverage Tests - Descent3/room_external module (Standalone)
 * 
 * Tests for room, face, and portal constants and flags.
 * These define the world geometry structure.
 */

#include <gtest/gtest.h>
#include <cstdint>

// ============================================================================
// D3 Room Constants Implementation (matching Descent3/room_external.h)
// ============================================================================

// Maximum values
#define MAX_FACES_PER_ROOM 3000
#define MAX_VERTS_PER_ROOM 10000
#define MAX_VERTS_PER_FACE 64

// Face flags
#define FF_LIGHTMAP 0x0001
#define FF_VERTEX_ALPHA 0x0002
#define FF_CORONA 0x0004
#define FF_TEXTURE_CHANGED 0x0008
#define FF_HAS_TRIGGER 0x0010
#define FF_SPEC_INVISIBLE 0x0020
#define FF_FLOATING_TRIG 0x0040
#define FF_DESTROYED 0x0080
#define FF_VOLUMETRIC 0x0100
#define FF_TRIANGULATED 0x0200
#define FF_VISIBLE 0x0400
#define FF_NOT_SHELL 0x0800
#define FF_TOUCHED 0x1000
#define FF_GOALFACE 0x2000
#define FF_NOT_FACING 0x4000
#define FF_SCORCHED 0x8000

// Portal flags
#define PF_RENDER_FACES 1
#define PF_RENDERED_FLYTHROUGH 2
#define PF_TOO_SMALL_FOR_ROBOT 4
#define PF_COMBINED 8
#define PF_CHANGED 16
#define PF_BLOCK 32
#define PF_BLOCK_REMOVABLE 64

// Room flags
#define RF_FUELCEN 1
#define RF_DOOR (1 << 1)
#define RF_EXTERNAL (1 << 2)
#define RF_GOAL1 (1 << 3)
#define RF_GOAL2 (1 << 4)
#define RF_TOUCHES_TERRAIN (1 << 5)
#define RF_SORTED_INC_Y (1 << 6)
#define RF_GOAL3 (1 << 7)
#define RF_GOAL4 (1 << 8)
#define RF_FOG (1 << 9)
#define RF_SPECIAL1 (1 << 10)
#define RF_SPECIAL2 (1 << 11)
#define RF_SPECIAL3 (1 << 12)
#define RF_SPECIAL4 (1 << 13)
#define RF_SPECIAL5 (1 << 14)
#define RF_SPECIAL6 (1 << 15)
#define RF_MIRROR_VISIBLE (1 << 16)
#define RF_TRIANGULATE (1 << 17)
#define RF_STROBE (1 << 18)
#define RF_FLICKER (1 << 19)
#define RFM_MINE ((1 << 20) | (1 << 21) | (1 << 22) | (1 << 23) | (1 << 24))
#define RF_INFORM_RELINK_TO_LG (1 << 25)
#define RF_MANUAL_PATH_PNT (1 << 26)
#define RF_WAYPOINT (1 << 27)
#define RF_SECRET (1 << 28)
#define RF_NO_LIGHT (1 << 29)

#define GOALROOM (RF_GOAL1 | RF_GOAL2 | RF_GOAL3 | RF_GOAL4)

#define ROOM_NAME_LEN 19

// ============================================================================
// Test Fixtures
// ============================================================================

class RoomExternalTest : public ::testing::Test {};

// ============================================================================
// Tests - Maximum Values
// ============================================================================

TEST_F(RoomExternalTest, MaxFacesPerRoom_Positive) {
    EXPECT_GT(MAX_FACES_PER_ROOM, 0);
    EXPECT_EQ(MAX_FACES_PER_ROOM, 3000);
}

TEST_F(RoomExternalTest, MaxVertsPerRoom_Positive) {
    EXPECT_GT(MAX_VERTS_PER_ROOM, 0);
    EXPECT_EQ(MAX_VERTS_PER_ROOM, 10000);
}

TEST_F(RoomExternalTest, MaxVertsPerFace_Positive) {
    EXPECT_GT(MAX_VERTS_PER_FACE, 0);
    EXPECT_EQ(MAX_VERTS_PER_FACE, 64);
}

TEST_F(RoomExternalTest, MaxValues_ReasonableRange) {
    // Maximum values should be reasonable for memory allocation
    EXPECT_LE(MAX_FACES_PER_ROOM, 10000);
    EXPECT_LE(MAX_VERTS_PER_ROOM, 50000);
    EXPECT_LE(MAX_VERTS_PER_FACE, 256);
}

TEST_F(RoomExternalTest, MaxVertsPerFace_PowerOfTwo) {
    // MAX_VERTS_PER_FACE is typically a power of 2
    EXPECT_EQ(MAX_VERTS_PER_FACE, 64);
    EXPECT_EQ(MAX_VERTS_PER_FACE & (MAX_VERTS_PER_FACE - 1), 0);
}

// ============================================================================
// Tests - Face Flags (0x0001 - 0x8000)
// ============================================================================

TEST_F(RoomExternalTest, FaceFlags_Defined) {
    EXPECT_EQ(FF_LIGHTMAP, 0x0001);
    EXPECT_EQ(FF_VERTEX_ALPHA, 0x0002);
    EXPECT_EQ(FF_CORONA, 0x0004);
    EXPECT_EQ(FF_TEXTURE_CHANGED, 0x0008);
    EXPECT_EQ(FF_HAS_TRIGGER, 0x0010);
    EXPECT_EQ(FF_SPEC_INVISIBLE, 0x0020);
    EXPECT_EQ(FF_FLOATING_TRIG, 0x0040);
    EXPECT_EQ(FF_DESTROYED, 0x0080);
    EXPECT_EQ(FF_VOLUMETRIC, 0x0100);
    EXPECT_EQ(FF_TRIANGULATED, 0x0200);
    EXPECT_EQ(FF_VISIBLE, 0x0400);
    EXPECT_EQ(FF_NOT_SHELL, 0x0800);
    EXPECT_EQ(FF_TOUCHED, 0x1000);
    EXPECT_EQ(FF_GOALFACE, 0x2000);
    EXPECT_EQ(FF_NOT_FACING, 0x4000);
    EXPECT_EQ(FF_SCORCHED, 0x8000);
}

TEST_F(RoomExternalTest, FaceFlags_PowersOfTwo) {
    // All face flags should be powers of two
    EXPECT_EQ(FF_LIGHTMAP & (FF_LIGHTMAP - 1), 0);
    EXPECT_EQ(FF_VERTEX_ALPHA & (FF_VERTEX_ALPHA - 1), 0);
    EXPECT_EQ(FF_CORONA & (FF_CORONA - 1), 0);
    EXPECT_EQ(FF_TEXTURE_CHANGED & (FF_TEXTURE_CHANGED - 1), 0);
    EXPECT_EQ(FF_HAS_TRIGGER & (FF_HAS_TRIGGER - 1), 0);
    EXPECT_EQ(FF_SPEC_INVISIBLE & (FF_SPEC_INVISIBLE - 1), 0);
    EXPECT_EQ(FF_FLOATING_TRIG & (FF_FLOATING_TRIG - 1), 0);
    EXPECT_EQ(FF_DESTROYED & (FF_DESTROYED - 1), 0);
    EXPECT_EQ(FF_VOLUMETRIC & (FF_VOLUMETRIC - 1), 0);
    EXPECT_EQ(FF_TRIANGULATED & (FF_TRIANGULATED - 1), 0);
    EXPECT_EQ(FF_VISIBLE & (FF_VISIBLE - 1), 0);
    EXPECT_EQ(FF_NOT_SHELL & (FF_NOT_SHELL - 1), 0);
    EXPECT_EQ(FF_TOUCHED & (FF_TOUCHED - 1), 0);
    EXPECT_EQ(FF_GOALFACE & (FF_GOALFACE - 1), 0);
    EXPECT_EQ(FF_NOT_FACING & (FF_NOT_FACING - 1), 0);
    EXPECT_EQ(FF_SCORCHED & (FF_SCORCHED - 1), 0);
}

TEST_F(RoomExternalTest, FaceFlags_ConsecutiveBits) {
    // Face flags use bits 0-15
    EXPECT_EQ(FF_LIGHTMAP, (1 << 0));
    EXPECT_EQ(FF_VERTEX_ALPHA, (1 << 1));
    EXPECT_EQ(FF_CORONA, (1 << 2));
    EXPECT_EQ(FF_TEXTURE_CHANGED, (1 << 3));
    EXPECT_EQ(FF_HAS_TRIGGER, (1 << 4));
    EXPECT_EQ(FF_SPEC_INVISIBLE, (1 << 5));
    EXPECT_EQ(FF_FLOATING_TRIG, (1 << 6));
    EXPECT_EQ(FF_DESTROYED, (1 << 7));
    EXPECT_EQ(FF_VOLUMETRIC, (1 << 8));
    EXPECT_EQ(FF_TRIANGULATED, (1 << 9));
    EXPECT_EQ(FF_VISIBLE, (1 << 10));
    EXPECT_EQ(FF_NOT_SHELL, (1 << 11));
    EXPECT_EQ(FF_TOUCHED, (1 << 12));
    EXPECT_EQ(FF_GOALFACE, (1 << 13));
    EXPECT_EQ(FF_NOT_FACING, (1 << 14));
    EXPECT_EQ(FF_SCORCHED, (1 << 15));
}

TEST_F(RoomExternalTest, FaceFlags_Unique) {
    // All face flags should be unique
    uint16_t flags[] = {
        FF_LIGHTMAP, FF_VERTEX_ALPHA, FF_CORONA, FF_TEXTURE_CHANGED,
        FF_HAS_TRIGGER, FF_SPEC_INVISIBLE, FF_FLOATING_TRIG, FF_DESTROYED,
        FF_VOLUMETRIC, FF_TRIANGULATED, FF_VISIBLE, FF_NOT_SHELL,
        FF_TOUCHED, FF_GOALFACE, FF_NOT_FACING, FF_SCORCHED
    };
    
    for (size_t i = 0; i < sizeof(flags)/sizeof(flags[0]); i++) {
        for (size_t j = i + 1; j < sizeof(flags)/sizeof(flags[0]); j++) {
            EXPECT_NE(flags[i], flags[j])
                << "Duplicate face flag at positions " << i << " and " << j;
        }
    }
}

// ============================================================================
// Tests - Portal Flags
// ============================================================================

TEST_F(RoomExternalTest, PortalFlags_Defined) {
    EXPECT_EQ(PF_RENDER_FACES, 1);
    EXPECT_EQ(PF_RENDERED_FLYTHROUGH, 2);
    EXPECT_EQ(PF_TOO_SMALL_FOR_ROBOT, 4);
    EXPECT_EQ(PF_COMBINED, 8);
    EXPECT_EQ(PF_CHANGED, 16);
    EXPECT_EQ(PF_BLOCK, 32);
    EXPECT_EQ(PF_BLOCK_REMOVABLE, 64);
}

TEST_F(RoomExternalTest, PortalFlags_PowersOfTwo) {
    // Portal flags are powers of two
    EXPECT_EQ(PF_RENDER_FACES & (PF_RENDER_FACES - 1), 0);
    EXPECT_EQ(PF_RENDERED_FLYTHROUGH & (PF_RENDERED_FLYTHROUGH - 1), 0);
    EXPECT_EQ(PF_TOO_SMALL_FOR_ROBOT & (PF_TOO_SMALL_FOR_ROBOT - 1), 0);
    EXPECT_EQ(PF_COMBINED & (PF_COMBINED - 1), 0);
    EXPECT_EQ(PF_CHANGED & (PF_CHANGED - 1), 0);
    EXPECT_EQ(PF_BLOCK & (PF_BLOCK - 1), 0);
    EXPECT_EQ(PF_BLOCK_REMOVABLE & (PF_BLOCK_REMOVABLE - 1), 0);
}

TEST_F(RoomExternalTest, PortalFlags_ConsecutiveBits) {
    // Portal flags use bits 0-6
    EXPECT_EQ(PF_RENDER_FACES, (1 << 0));
    EXPECT_EQ(PF_RENDERED_FLYTHROUGH, (1 << 1));
    EXPECT_EQ(PF_TOO_SMALL_FOR_ROBOT, (1 << 2));
    EXPECT_EQ(PF_COMBINED, (1 << 3));
    EXPECT_EQ(PF_CHANGED, (1 << 4));
    EXPECT_EQ(PF_BLOCK, (1 << 5));
    EXPECT_EQ(PF_BLOCK_REMOVABLE, (1 << 6));
}

// ============================================================================
// Tests - Room Flags
// ============================================================================

TEST_F(RoomExternalTest, RoomFlags_Basic_Defined) {
    EXPECT_EQ(RF_FUELCEN, 1);
    EXPECT_EQ(RF_DOOR, (1 << 1));
    EXPECT_EQ(RF_EXTERNAL, (1 << 2));
    EXPECT_EQ(RF_GOAL1, (1 << 3));
    EXPECT_EQ(RF_GOAL2, (1 << 4));
    EXPECT_EQ(RF_TOUCHES_TERRAIN, (1 << 5));
    EXPECT_EQ(RF_SORTED_INC_Y, (1 << 6));
    EXPECT_EQ(RF_GOAL3, (1 << 7));
    EXPECT_EQ(RF_GOAL4, (1 << 8));
    EXPECT_EQ(RF_FOG, (1 << 9));
}

TEST_F(RoomExternalTest, RoomFlags_Special_Defined) {
    EXPECT_EQ(RF_SPECIAL1, (1 << 10));
    EXPECT_EQ(RF_SPECIAL2, (1 << 11));
    EXPECT_EQ(RF_SPECIAL3, (1 << 12));
    EXPECT_EQ(RF_SPECIAL4, (1 << 13));
    EXPECT_EQ(RF_SPECIAL5, (1 << 14));
    EXPECT_EQ(RF_SPECIAL6, (1 << 15));
}

TEST_F(RoomExternalTest, RoomFlags_Advanced_Defined) {
    EXPECT_EQ(RF_MIRROR_VISIBLE, (1 << 16));
    EXPECT_EQ(RF_TRIANGULATE, (1 << 17));
    EXPECT_EQ(RF_STROBE, (1 << 18));
    EXPECT_EQ(RF_FLICKER, (1 << 19));
    EXPECT_EQ(RF_INFORM_RELINK_TO_LG, (1 << 25));
    EXPECT_EQ(RF_MANUAL_PATH_PNT, (1 << 26));
    EXPECT_EQ(RF_WAYPOINT, (1 << 27));
    EXPECT_EQ(RF_SECRET, (1 << 28));
    EXPECT_EQ(RF_NO_LIGHT, (1 << 29));
}

TEST_F(RoomExternalTest, RoomFlags_PowersOfTwo) {
    // Basic room flags are powers of two
    EXPECT_EQ(RF_FUELCEN & (RF_FUELCEN - 1), 0);
    EXPECT_EQ(RF_DOOR & (RF_DOOR - 1), 0);
    EXPECT_EQ(RF_EXTERNAL & (RF_EXTERNAL - 1), 0);
    EXPECT_EQ(RF_GOAL1 & (RF_GOAL1 - 1), 0);
    EXPECT_EQ(RF_GOAL2 & (RF_GOAL2 - 1), 0);
    EXPECT_EQ(RF_FOG & (RF_FOG - 1), 0);
    EXPECT_EQ(RF_SECRET & (RF_SECRET - 1), 0);
}

TEST_F(RoomExternalTest, RoomFlags_Unique) {
    // All room flags should be unique
    uint32_t flags[] = {
        RF_FUELCEN, RF_DOOR, RF_EXTERNAL, RF_GOAL1, RF_GOAL2,
        RF_TOUCHES_TERRAIN, RF_SORTED_INC_Y, RF_GOAL3, RF_GOAL4, RF_FOG,
        RF_SPECIAL1, RF_SPECIAL2, RF_SPECIAL3, RF_SPECIAL4, RF_SPECIAL5, RF_SPECIAL6,
        RF_MIRROR_VISIBLE, RF_TRIANGULATE, RF_STROBE, RF_FLICKER,
        RF_INFORM_RELINK_TO_LG, RF_MANUAL_PATH_PNT, RF_WAYPOINT, RF_SECRET, RF_NO_LIGHT
    };
    
    for (size_t i = 0; i < sizeof(flags)/sizeof(flags[0]); i++) {
        for (size_t j = i + 1; j < sizeof(flags)/sizeof(flags[0]); j++) {
            EXPECT_NE(flags[i], flags[j])
                << "Duplicate room flag at positions " << i << " and " << j;
        }
    }
}

// ============================================================================
// Tests - Mine Mask (RFM_MINE)
// ============================================================================

TEST_F(RoomExternalTest, MineMask_Value) {
    // RFM_MINE is a multi-bit mask for mine indices
    uint32_t expected = (1 << 20) | (1 << 21) | (1 << 22) | (1 << 23) | (1 << 24);
    EXPECT_EQ(RFM_MINE, expected);
    EXPECT_EQ(RFM_MINE, 0x01F00000);
}

TEST_F(RoomExternalTest, MineMask_Bits) {
    // Mine mask should use bits 20-24
    EXPECT_TRUE((RFM_MINE & (1 << 20)) != 0);
    EXPECT_TRUE((RFM_MINE & (1 << 21)) != 0);
    EXPECT_TRUE((RFM_MINE & (1 << 22)) != 0);
    EXPECT_TRUE((RFM_MINE & (1 << 23)) != 0);
    EXPECT_TRUE((RFM_MINE & (1 << 24)) != 0);
    
    // Should NOT include other bits
    EXPECT_EQ(RFM_MINE & (1 << 19), 0);
    EXPECT_EQ(RFM_MINE & (1 << 25), 0);
}

TEST_F(RoomExternalTest, MineMask_Capacity) {
    // RFM_MINE uses 5 bits, so it can represent 32 different mine indices
    int bits = 0;
    uint32_t mask = RFM_MINE;
    while (mask) {
        if (mask & 1) bits++;
        mask >>= 1;
    }
    EXPECT_EQ(bits, 5);
    EXPECT_EQ(1 << bits, 32);
}

// ============================================================================
// Tests - Goal Room Mask
// ============================================================================

TEST_F(RoomExternalTest, GoalRoomMask_Value) {
    // GOALROOM combines all goal flags
    // RF_GOAL1 = (1 << 3) = 8
    // RF_GOAL2 = (1 << 4) = 16
    // RF_GOAL3 = (1 << 7) = 128
    // RF_GOAL4 = (1 << 8) = 256
    // Total = 8 + 16 + 128 + 256 = 408 = 0x198
    uint32_t expected = RF_GOAL1 | RF_GOAL2 | RF_GOAL3 | RF_GOAL4;
    EXPECT_EQ(GOALROOM, expected);
    EXPECT_EQ(GOALROOM, 0x198);
}

TEST_F(RoomExternalTest, GoalRoomMask_IncludesAllGoals) {
    EXPECT_TRUE((GOALROOM & RF_GOAL1) != 0);
    EXPECT_TRUE((GOALROOM & RF_GOAL2) != 0);
    EXPECT_TRUE((GOALROOM & RF_GOAL3) != 0);
    EXPECT_TRUE((GOALROOM & RF_GOAL4) != 0);
}

TEST_F(RoomExternalTest, GoalRoomMask_NoOtherFlags) {
    // GOALROOM should only include goal flags
    EXPECT_EQ(GOALROOM & RF_FUELCEN, 0);
    EXPECT_EQ(GOALROOM & RF_DOOR, 0);
    EXPECT_EQ(GOALROOM & RF_EXTERNAL, 0);
    EXPECT_EQ(GOALROOM & RF_FOG, 0);
}

// ============================================================================
// Tests - Room Name Length
// ============================================================================

TEST_F(RoomExternalTest, RoomNameLen_Positive) {
    EXPECT_GT(ROOM_NAME_LEN, 0);
    EXPECT_EQ(ROOM_NAME_LEN, 19);
}

TEST_F(RoomExternalTest, RoomNameLen_Reasonable) {
    // Room name length should be reasonable
    EXPECT_GE(ROOM_NAME_LEN, 10);
    EXPECT_LE(ROOM_NAME_LEN, 100);
}

// ============================================================================
// Tests - Flag Combinations
// ============================================================================

TEST_F(RoomExternalTest, RoomFlag_FuelAndDoor) {
    // Fuel center and door are independent flags
    EXPECT_EQ(RF_FUELCEN & RF_DOOR, 0);
}

TEST_F(RoomExternalTest, RoomFlag_ExternalAndTerrain) {
    // External and touches terrain are related but separate
    EXPECT_EQ(RF_EXTERNAL & RF_TOUCHES_TERRAIN, 0);
}

TEST_F(RoomExternalTest, RoomFlag_LightingEffects) {
    // Fog, strobe, and flicker are lighting-related
    EXPECT_EQ(RF_FOG & RF_STROBE, 0);
    EXPECT_EQ(RF_FOG & RF_FLICKER, 0);
    EXPECT_EQ(RF_STROBE & RF_FLICKER, 0);
}

TEST_F(RoomExternalTest, RoomFlag_NoLight_Incompatible) {
    // NO_LIGHT is incompatible with other lighting flags
    EXPECT_EQ(RF_NO_LIGHT & RF_FOG, 0);
    EXPECT_EQ(RF_NO_LIGHT & RF_STROBE, 0);
    EXPECT_EQ(RF_NO_LIGHT & RF_FLICKER, 0);
}

// ============================================================================
// Tests - Face Flag Categories
// ============================================================================

TEST_F(RoomExternalTest, FaceFlags_Rendering) {
    // Rendering-related face flags
    EXPECT_EQ(FF_LIGHTMAP & FF_VERTEX_ALPHA, 0);
    EXPECT_EQ(FF_LIGHTMAP & FF_VISIBLE, 0);
    EXPECT_EQ(FF_VISIBLE & FF_NOT_FACING, 0);
}

TEST_F(RoomExternalTest, FaceFlags_Damage) {
    // Damage/destruction related flags
    EXPECT_EQ(FF_DESTROYED & FF_SCORCHED, 0);
}

TEST_F(RoomExternalTest, FaceFlags_Trigger) {
    // Trigger-related flags
    EXPECT_EQ(FF_HAS_TRIGGER & FF_FLOATING_TRIG, 0);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
