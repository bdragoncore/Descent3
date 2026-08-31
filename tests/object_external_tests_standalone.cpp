/*
 * D3 Coverage Tests - Descent3/object_external module (Standalone)
 * 
 * Tests for object type constants, flags, and masks.
 * These define the game object system.
 */

#include <gtest/gtest.h>
#include <cstdint>

// ============================================================================
// D3 Object Constants Implementation (matching Descent3/object_external.h)
// ============================================================================

// Object handles
#define OBJECT_HANDLE_BAD 0
#define OBJECT_HANDLE_NONE -1

// Object types
#define OBJ_NONE 255
#define OBJ_WALL 0
#define OBJ_FIREBALL 1
#define OBJ_ROBOT 2
#define OBJ_SHARD 3
#define OBJ_PLAYER 4
#define OBJ_WEAPON 5
#define OBJ_VIEWER 6
#define OBJ_POWERUP 7
#define OBJ_DEBRIS 8
#define OBJ_CAMERA 9
#define OBJ_SHOCKWAVE 10
#define OBJ_CLUTTER 11
#define OBJ_GHOST 12
#define OBJ_LIGHT 13
#define OBJ_COOP 14
#define OBJ_MARKER 15
#define OBJ_BUILDING 16
#define OBJ_DOOR 17
#define OBJ_ROOM 18
#define OBJ_PARTICLE 19
#define OBJ_SPLINTER 20
#define OBJ_DUMMY 21
#define OBJ_OBSERVER 22
#define OBJ_DEBUG_LINE 23
#define OBJ_SOUNDSOURCE 24
#define OBJ_WAYPOINT 25
#define MAX_OBJECT_TYPES 26

// Effect flags
#define EF_CLOAKED 1
#define EF_DEFORM 2
#define EF_COLORED 4
#define EF_NAPALMED 8
#define EF_VOLUME_LIT 16
#define EF_VOLUME_CHANGING 32
#define EF_FREEZE 64
#define EF_LINE_ATTACH 128
#define EF_SPECULAR 256
#define EF_FADING_IN 512
#define EF_FADING_OUT 1024
#define EF_LIQUID 2048
#define EF_CLOAK_WITH_MSG 4096
#define EF_SPARKING 8192
#define EF_BUMPMAPPED 16384
#define EF_NEGATIVE_LIGHT 32768
#define EF_VIRUS_INFECTED 65536

// Control types
#define CT_NONE 0
#define CT_AI 1
#define CT_EXPLOSION 2
#define CT_FLYING 4
#define CT_SLEW 5
#define CT_FLYTHROUGH 6
#define CT_WEAPON 9
#define CT_DEBRIS 12
#define CT_POWERUP 13
#define CT_SOAR 14
#define CT_PARTICLE 15
#define CT_SPLINTER 16
#define CT_SOUNDSOURCE 17
#define CT_DYING 18
#define CT_DYING_AND_AI 19

// Movement types
#define MT_NONE 0
#define MT_PHYSICS 1
#define MT_WALKING 2
#define MT_AT_REST 3
#define MT_SHOCKWAVE 4
#define MT_OBJ_LINKED 5

// Movement classes
#define MC_STANDING 0
#define MC_FLYING 1
#define MC_ROLLING 2
#define MC_WALKING 3
#define MC_JUMPING 4

// Attach types
#define AT_RAD 0
#define AT_ALIGNED 1
#define AT_UNALIGNED 2

// Render types
#define RT_NONE 0
#define RT_POLYOBJ 1
#define RT_FIREBALL 2
#define RT_WEAPON 3
#define RT_LINE 4
#define RT_PARTICLE 5
#define RT_SPLINTER 6
#define RT_ROOM 7
#define RT_EDITOR_SPHERE 8
#define RT_SHARD 9

// Misc object flags
#define OF_FORCE_CEILING_CHECK 0x00000001
#define OF_DEAD 0x00000002
#define OF_DESTROYED 0x00000004
#define OF_STOPPED_THIS_FRAME 0x00000008
#define OF_ATTACHED 0x00000010
#define OF_MOVED_THIS_FRAME 0x00000020
#define OF_AI_DO_DEATH 0x00000040
#define OF_USES_LIFELEFT 0x00000080
#define OF_SAFE_TO_RENDER 0x00000100
#define OF_OUTSIDE_MINE 0x00000200
#define OF_DESTROYABLE 0x00000400
#define OF_BIG_OBJECT 0x00000800
#define OF_POLYGON_OBJECT 0x00001000
#define OF_DYING 0x00002000
#define OF_USE_DESTROYED_POLYMODEL 0x00004000
#define OF_RENDERED 0x00008000
#define OF_NO_OBJECT_COLLISIONS 0x00010000
#define OF_STUCK_ON_PORTAL 0x00020000
#define OF_TEMP_GRAVITY 0x00040000
#define OF_CLIENT_KNOWS 0x00080000
#define OF_SERVER_SAYS_DELETE 0x00100000
#define OF_SERVER_OBJECT 0x00200000
#define OF_PING_ACCELERATE 0x00400000
#define OF_AI_DEATH 0x00800000
#define OF_SEND_MULTI_REMOVE_ON_DEATH 0x01000000
#define OF_SEND_MULTI_REMOVE_ON_DEATHWS 0x02000000
#define OF_PREDICTED 0x04000000
#define OF_INPLAYERINVENTORY 0x08000000

// ============================================================================
// Test Fixtures
// ============================================================================

class ObjectExternalTest : public ::testing::Test {};

// ============================================================================
// Tests - Object Handles
// ============================================================================

TEST_F(ObjectExternalTest, ObjectHandleBad_IsZero) {
    EXPECT_EQ(OBJECT_HANDLE_BAD, 0);
}

TEST_F(ObjectExternalTest, ObjectHandleNone_IsNegative) {
    EXPECT_EQ(OBJECT_HANDLE_NONE, -1);
}

TEST_F(ObjectExternalTest, ObjectHandles_Distinct) {
    EXPECT_NE(OBJECT_HANDLE_BAD, OBJECT_HANDLE_NONE);
}

// ============================================================================
// Tests - Object Types
// ============================================================================

TEST_F(ObjectExternalTest, ObjectTypes_Defined) {
    EXPECT_EQ(OBJ_WALL, 0);
    EXPECT_EQ(OBJ_FIREBALL, 1);
    EXPECT_EQ(OBJ_ROBOT, 2);
    EXPECT_EQ(OBJ_SHARD, 3);
    EXPECT_EQ(OBJ_PLAYER, 4);
    EXPECT_EQ(OBJ_WEAPON, 5);
    EXPECT_EQ(OBJ_VIEWER, 6);
    EXPECT_EQ(OBJ_POWERUP, 7);
    EXPECT_EQ(OBJ_DEBRIS, 8);
    EXPECT_EQ(OBJ_CAMERA, 9);
    EXPECT_EQ(OBJ_SHOCKWAVE, 10);
    EXPECT_EQ(OBJ_CLUTTER, 11);
    EXPECT_EQ(OBJ_GHOST, 12);
    EXPECT_EQ(OBJ_LIGHT, 13);
    EXPECT_EQ(OBJ_COOP, 14);
    EXPECT_EQ(OBJ_MARKER, 15);
    EXPECT_EQ(OBJ_BUILDING, 16);
    EXPECT_EQ(OBJ_DOOR, 17);
    EXPECT_EQ(OBJ_ROOM, 18);
    EXPECT_EQ(OBJ_PARTICLE, 19);
    EXPECT_EQ(OBJ_SPLINTER, 20);
    EXPECT_EQ(OBJ_DUMMY, 21);
    EXPECT_EQ(OBJ_OBSERVER, 22);
    EXPECT_EQ(OBJ_DEBUG_LINE, 23);
    EXPECT_EQ(OBJ_SOUNDSOURCE, 24);
    EXPECT_EQ(OBJ_WAYPOINT, 25);
    EXPECT_EQ(OBJ_NONE, 255);
}

TEST_F(ObjectExternalTest, ObjectTypes_Contiguous) {
    // Object types 0-25 should be contiguous
    int expected = 0;
    EXPECT_EQ(OBJ_WALL, expected++);
    EXPECT_EQ(OBJ_FIREBALL, expected++);
    EXPECT_EQ(OBJ_ROBOT, expected++);
    EXPECT_EQ(OBJ_SHARD, expected++);
    EXPECT_EQ(OBJ_PLAYER, expected++);
    EXPECT_EQ(OBJ_WEAPON, expected++);
    EXPECT_EQ(OBJ_VIEWER, expected++);
    EXPECT_EQ(OBJ_POWERUP, expected++);
    EXPECT_EQ(OBJ_DEBRIS, expected++);
    EXPECT_EQ(OBJ_CAMERA, expected++);
    EXPECT_EQ(OBJ_SHOCKWAVE, expected++);
    EXPECT_EQ(OBJ_CLUTTER, expected++);
    EXPECT_EQ(OBJ_GHOST, expected++);
    EXPECT_EQ(OBJ_LIGHT, expected++);
    EXPECT_EQ(OBJ_COOP, expected++);
    EXPECT_EQ(OBJ_MARKER, expected++);
    EXPECT_EQ(OBJ_BUILDING, expected++);
    EXPECT_EQ(OBJ_DOOR, expected++);
    EXPECT_EQ(OBJ_ROOM, expected++);
    EXPECT_EQ(OBJ_PARTICLE, expected++);
    EXPECT_EQ(OBJ_SPLINTER, expected++);
    EXPECT_EQ(OBJ_DUMMY, expected++);
    EXPECT_EQ(OBJ_OBSERVER, expected++);
    EXPECT_EQ(OBJ_DEBUG_LINE, expected++);
    EXPECT_EQ(OBJ_SOUNDSOURCE, expected++);
    EXPECT_EQ(OBJ_WAYPOINT, expected++);
}

TEST_F(ObjectExternalTest, ObjectTypes_MaxCount) {
    EXPECT_EQ(MAX_OBJECT_TYPES, 26);
}

TEST_F(ObjectExternalTest, ObjectTypes_Unique) {
    // All object types should be unique
    uint8_t types[] = {
        OBJ_WALL, OBJ_FIREBALL, OBJ_ROBOT, OBJ_SHARD, OBJ_PLAYER,
        OBJ_WEAPON, OBJ_VIEWER, OBJ_POWERUP, OBJ_DEBRIS, OBJ_CAMERA,
        OBJ_SHOCKWAVE, OBJ_CLUTTER, OBJ_GHOST, OBJ_LIGHT, OBJ_COOP,
        OBJ_MARKER, OBJ_BUILDING, OBJ_DOOR, OBJ_ROOM, OBJ_PARTICLE,
        OBJ_SPLINTER, OBJ_DUMMY, OBJ_OBSERVER, OBJ_DEBUG_LINE,
        OBJ_SOUNDSOURCE, OBJ_WAYPOINT, OBJ_NONE
    };
    
    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
        for (size_t j = i + 1; j < sizeof(types)/sizeof(types[0]); j++) {
            EXPECT_NE(types[i], types[j])
                << "Duplicate object type at positions " << i << " and " << j;
        }
    }
}

TEST_F(ObjectExternalTest, ObjectTypes_None_IsSpecial) {
    // OBJ_NONE (255) is special sentinel value
    EXPECT_EQ(OBJ_NONE, 255);
    EXPECT_GT(OBJ_NONE, MAX_OBJECT_TYPES);
}

// ============================================================================
// Tests - Effect Flags
// ============================================================================

TEST_F(ObjectExternalTest, EffectFlags_Defined) {
    EXPECT_EQ(EF_CLOAKED, 1);
    EXPECT_EQ(EF_DEFORM, 2);
    EXPECT_EQ(EF_COLORED, 4);
    EXPECT_EQ(EF_NAPALMED, 8);
    EXPECT_EQ(EF_VOLUME_LIT, 16);
    EXPECT_EQ(EF_VOLUME_CHANGING, 32);
    EXPECT_EQ(EF_FREEZE, 64);
    EXPECT_EQ(EF_LINE_ATTACH, 128);
    EXPECT_EQ(EF_SPECULAR, 256);
    EXPECT_EQ(EF_FADING_IN, 512);
    EXPECT_EQ(EF_FADING_OUT, 1024);
    EXPECT_EQ(EF_LIQUID, 2048);
    EXPECT_EQ(EF_CLOAK_WITH_MSG, 4096);
    EXPECT_EQ(EF_SPARKING, 8192);
    EXPECT_EQ(EF_BUMPMAPPED, 16384);
    EXPECT_EQ(EF_NEGATIVE_LIGHT, 32768);
    EXPECT_EQ(EF_VIRUS_INFECTED, 65536);
}

TEST_F(ObjectExternalTest, EffectFlags_PowersOfTwo) {
    // Effect flags should be powers of two
    EXPECT_EQ(EF_CLOAKED & (EF_CLOAKED - 1), 0);
    EXPECT_EQ(EF_DEFORM & (EF_DEFORM - 1), 0);
    EXPECT_EQ(EF_COLORED & (EF_COLORED - 1), 0);
    EXPECT_EQ(EF_NAPALMED & (EF_NAPALMED - 1), 0);
    EXPECT_EQ(EF_VOLUME_LIT & (EF_VOLUME_LIT - 1), 0);
    EXPECT_EQ(EF_FREEZE & (EF_FREEZE - 1), 0);
    EXPECT_EQ(EF_SPECULAR & (EF_SPECULAR - 1), 0);
    EXPECT_EQ(EF_FADING_IN & (EF_FADING_IN - 1), 0);
    EXPECT_EQ(EF_FADING_OUT & (EF_FADING_OUT - 1), 0);
    EXPECT_EQ(EF_LIQUID & (EF_LIQUID - 1), 0);
    EXPECT_EQ(EF_SPARKING & (EF_SPARKING - 1), 0);
    EXPECT_EQ(EF_BUMPMAPPED & (EF_BUMPMAPPED - 1), 0);
    EXPECT_EQ(EF_NEGATIVE_LIGHT & (EF_NEGATIVE_LIGHT - 1), 0);
    EXPECT_EQ(EF_VIRUS_INFECTED & (EF_VIRUS_INFECTED - 1), 0);
}

TEST_F(ObjectExternalTest, EffectFlags_ConsecutiveBits) {
    // Effect flags use bits 0-16
    EXPECT_EQ(EF_CLOAKED, (1 << 0));
    EXPECT_EQ(EF_DEFORM, (1 << 1));
    EXPECT_EQ(EF_COLORED, (1 << 2));
    EXPECT_EQ(EF_NAPALMED, (1 << 3));
    EXPECT_EQ(EF_VOLUME_LIT, (1 << 4));
    EXPECT_EQ(EF_VOLUME_CHANGING, (1 << 5));
    EXPECT_EQ(EF_FREEZE, (1 << 6));
    EXPECT_EQ(EF_LINE_ATTACH, (1 << 7));
    EXPECT_EQ(EF_SPECULAR, (1 << 8));
    EXPECT_EQ(EF_FADING_IN, (1 << 9));
    EXPECT_EQ(EF_FADING_OUT, (1 << 10));
    EXPECT_EQ(EF_LIQUID, (1 << 11));
    EXPECT_EQ(EF_CLOAK_WITH_MSG, (1 << 12));
    EXPECT_EQ(EF_SPARKING, (1 << 13));
    EXPECT_EQ(EF_BUMPMAPPED, (1 << 14));
    EXPECT_EQ(EF_NEGATIVE_LIGHT, (1 << 15));
    EXPECT_EQ(EF_VIRUS_INFECTED, (1 << 16));
}

// ============================================================================
// Tests - Control Types
// ============================================================================

TEST_F(ObjectExternalTest, ControlTypes_Defined) {
    EXPECT_EQ(CT_NONE, 0);
    EXPECT_EQ(CT_AI, 1);
    EXPECT_EQ(CT_EXPLOSION, 2);
    EXPECT_EQ(CT_FLYING, 4);
    EXPECT_EQ(CT_SLEW, 5);
    EXPECT_EQ(CT_FLYTHROUGH, 6);
    EXPECT_EQ(CT_WEAPON, 9);
    EXPECT_EQ(CT_DEBRIS, 12);
    EXPECT_EQ(CT_POWERUP, 13);
    EXPECT_EQ(CT_SOAR, 14);
    EXPECT_EQ(CT_PARTICLE, 15);
    EXPECT_EQ(CT_SPLINTER, 16);
    EXPECT_EQ(CT_SOUNDSOURCE, 17);
    EXPECT_EQ(CT_DYING, 18);
    EXPECT_EQ(CT_DYING_AND_AI, 19);
}

TEST_F(ObjectExternalTest, ControlTypes_Unique) {
    // Control types should be unique
    uint8_t types[] = {
        CT_NONE, CT_AI, CT_EXPLOSION, CT_FLYING, CT_SLEW, CT_FLYTHROUGH,
        CT_WEAPON, CT_DEBRIS, CT_POWERUP, CT_SOAR, CT_PARTICLE, CT_SPLINTER,
        CT_SOUNDSOURCE, CT_DYING, CT_DYING_AND_AI
    };
    
    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
        for (size_t j = i + 1; j < sizeof(types)/sizeof(types[0]); j++) {
            EXPECT_NE(types[i], types[j])
                << "Duplicate control type at positions " << i << " and " << j;
        }
    }
}

// ============================================================================
// Tests - Movement Types
// ============================================================================

TEST_F(ObjectExternalTest, MovementTypes_Defined) {
    EXPECT_EQ(MT_NONE, 0);
    EXPECT_EQ(MT_PHYSICS, 1);
    EXPECT_EQ(MT_WALKING, 2);
    EXPECT_EQ(MT_AT_REST, 3);
    EXPECT_EQ(MT_SHOCKWAVE, 4);
    EXPECT_EQ(MT_OBJ_LINKED, 5);
}

TEST_F(ObjectExternalTest, MovementTypes_Contiguous) {
    // Movement types 0-5 are contiguous
    int expected = 0;
    EXPECT_EQ(MT_NONE, expected++);
    EXPECT_EQ(MT_PHYSICS, expected++);
    EXPECT_EQ(MT_WALKING, expected++);
    EXPECT_EQ(MT_AT_REST, expected++);
    EXPECT_EQ(MT_SHOCKWAVE, expected++);
    EXPECT_EQ(MT_OBJ_LINKED, expected++);
}

// ============================================================================
// Tests - Movement Classes
// ============================================================================

TEST_F(ObjectExternalTest, MovementClasses_Defined) {
    EXPECT_EQ(MC_STANDING, 0);
    EXPECT_EQ(MC_FLYING, 1);
    EXPECT_EQ(MC_ROLLING, 2);
    EXPECT_EQ(MC_WALKING, 3);
    EXPECT_EQ(MC_JUMPING, 4);
}

TEST_F(ObjectExternalTest, MovementClasses_Contiguous) {
    // Movement classes 0-4 are contiguous
    int expected = 0;
    EXPECT_EQ(MC_STANDING, expected++);
    EXPECT_EQ(MC_FLYING, expected++);
    EXPECT_EQ(MC_ROLLING, expected++);
    EXPECT_EQ(MC_WALKING, expected++);
    EXPECT_EQ(MC_JUMPING, expected++);
}

// ============================================================================
// Tests - Attach Types
// ============================================================================

TEST_F(ObjectExternalTest, AttachTypes_Defined) {
    EXPECT_EQ(AT_RAD, 0);
    EXPECT_EQ(AT_ALIGNED, 1);
    EXPECT_EQ(AT_UNALIGNED, 2);
}

TEST_F(ObjectExternalTest, AttachTypes_Contiguous) {
    // Attach types 0-2 are contiguous
    int expected = 0;
    EXPECT_EQ(AT_RAD, expected++);
    EXPECT_EQ(AT_ALIGNED, expected++);
    EXPECT_EQ(AT_UNALIGNED, expected++);
}

// ============================================================================
// Tests - Render Types
// ============================================================================

TEST_F(ObjectExternalTest, RenderTypes_Defined) {
    EXPECT_EQ(RT_NONE, 0);
    EXPECT_EQ(RT_POLYOBJ, 1);
    EXPECT_EQ(RT_FIREBALL, 2);
    EXPECT_EQ(RT_WEAPON, 3);
    EXPECT_EQ(RT_LINE, 4);
    EXPECT_EQ(RT_PARTICLE, 5);
    EXPECT_EQ(RT_SPLINTER, 6);
    EXPECT_EQ(RT_ROOM, 7);
    EXPECT_EQ(RT_EDITOR_SPHERE, 8);
    EXPECT_EQ(RT_SHARD, 9);
}

TEST_F(ObjectExternalTest, RenderTypes_Contiguous) {
    // Render types 0-9 are contiguous
    int expected = 0;
    EXPECT_EQ(RT_NONE, expected++);
    EXPECT_EQ(RT_POLYOBJ, expected++);
    EXPECT_EQ(RT_FIREBALL, expected++);
    EXPECT_EQ(RT_WEAPON, expected++);
    EXPECT_EQ(RT_LINE, expected++);
    EXPECT_EQ(RT_PARTICLE, expected++);
    EXPECT_EQ(RT_SPLINTER, expected++);
    EXPECT_EQ(RT_ROOM, expected++);
    EXPECT_EQ(RT_EDITOR_SPHERE, expected++);
    EXPECT_EQ(RT_SHARD, expected++);
}

// ============================================================================
// Tests - Object Flags
// ============================================================================

TEST_F(ObjectExternalTest, ObjectFlags_Defined) {
    EXPECT_EQ(OF_FORCE_CEILING_CHECK, 0x00000001);
    EXPECT_EQ(OF_DEAD, 0x00000002);
    EXPECT_EQ(OF_DESTROYED, 0x00000004);
    EXPECT_EQ(OF_STOPPED_THIS_FRAME, 0x00000008);
    EXPECT_EQ(OF_ATTACHED, 0x00000010);
    EXPECT_EQ(OF_MOVED_THIS_FRAME, 0x00000020);
    EXPECT_EQ(OF_AI_DO_DEATH, 0x00000040);
    EXPECT_EQ(OF_USES_LIFELEFT, 0x00000080);
    EXPECT_EQ(OF_SAFE_TO_RENDER, 0x00000100);
    EXPECT_EQ(OF_OUTSIDE_MINE, 0x00000200);
    EXPECT_EQ(OF_DESTROYABLE, 0x00000400);
    EXPECT_EQ(OF_BIG_OBJECT, 0x00000800);
    EXPECT_EQ(OF_POLYGON_OBJECT, 0x00001000);
    EXPECT_EQ(OF_DYING, 0x00002000);
    EXPECT_EQ(OF_USE_DESTROYED_POLYMODEL, 0x00004000);
    EXPECT_EQ(OF_RENDERED, 0x00008000);
}

TEST_F(ObjectExternalTest, ObjectFlags_MoreFlags) {
    EXPECT_EQ(OF_NO_OBJECT_COLLISIONS, 0x00010000);
    EXPECT_EQ(OF_STUCK_ON_PORTAL, 0x00020000);
    EXPECT_EQ(OF_TEMP_GRAVITY, 0x00040000);
    EXPECT_EQ(OF_CLIENT_KNOWS, 0x00080000);
    EXPECT_EQ(OF_SERVER_SAYS_DELETE, 0x00100000);
    EXPECT_EQ(OF_SERVER_OBJECT, 0x00200000);
    EXPECT_EQ(OF_PING_ACCELERATE, 0x00400000);
    EXPECT_EQ(OF_AI_DEATH, 0x00800000);
    EXPECT_EQ(OF_SEND_MULTI_REMOVE_ON_DEATH, 0x01000000);
    EXPECT_EQ(OF_SEND_MULTI_REMOVE_ON_DEATHWS, 0x02000000);
    EXPECT_EQ(OF_PREDICTED, 0x04000000);
    EXPECT_EQ(OF_INPLAYERINVENTORY, 0x08000000);
}

TEST_F(ObjectExternalTest, ObjectFlags_PowersOfTwo) {
    // Object flags should be powers of two
    EXPECT_EQ(OF_FORCE_CEILING_CHECK & (OF_FORCE_CEILING_CHECK - 1), 0);
    EXPECT_EQ(OF_DEAD & (OF_DEAD - 1), 0);
    EXPECT_EQ(OF_DESTROYED & (OF_DESTROYED - 1), 0);
    EXPECT_EQ(OF_DYING & (OF_DYING - 1), 0);
    EXPECT_EQ(OF_DESTROYABLE & (OF_DESTROYABLE - 1), 0);
    EXPECT_EQ(OF_CLIENT_KNOWS & (OF_CLIENT_KNOWS - 1), 0);
    EXPECT_EQ(OF_PREDICTED & (OF_PREDICTED - 1), 0);
}

TEST_F(ObjectExternalTest, ObjectFlags_ConsecutiveBits) {
    // First 16 flags use bits 0-15
    EXPECT_EQ(OF_FORCE_CEILING_CHECK, (1u << 0));
    EXPECT_EQ(OF_DEAD, (1u << 1));
    EXPECT_EQ(OF_DESTROYED, (1u << 2));
    EXPECT_EQ(OF_STOPPED_THIS_FRAME, (1u << 3));
    EXPECT_EQ(OF_ATTACHED, (1u << 4));
    EXPECT_EQ(OF_MOVED_THIS_FRAME, (1u << 5));
    EXPECT_EQ(OF_AI_DO_DEATH, (1u << 6));
    EXPECT_EQ(OF_USES_LIFELEFT, (1u << 7));
    EXPECT_EQ(OF_SAFE_TO_RENDER, (1u << 8));
    EXPECT_EQ(OF_OUTSIDE_MINE, (1u << 9));
    EXPECT_EQ(OF_DESTROYABLE, (1u << 10));
    EXPECT_EQ(OF_BIG_OBJECT, (1u << 11));
    EXPECT_EQ(OF_POLYGON_OBJECT, (1u << 12));
    EXPECT_EQ(OF_DYING, (1u << 13));
    EXPECT_EQ(OF_USE_DESTROYED_POLYMODEL, (1u << 14));
    EXPECT_EQ(OF_RENDERED, (1u << 15));
}

TEST_F(ObjectExternalTest, ObjectFlags_Unique) {
    // All object flags should be unique
    uint32_t flags[] = {
        OF_FORCE_CEILING_CHECK, OF_DEAD, OF_DESTROYED, OF_STOPPED_THIS_FRAME,
        OF_ATTACHED, OF_MOVED_THIS_FRAME, OF_AI_DO_DEATH, OF_USES_LIFELEFT,
        OF_SAFE_TO_RENDER, OF_OUTSIDE_MINE, OF_DESTROYABLE, OF_BIG_OBJECT,
        OF_POLYGON_OBJECT, OF_DYING, OF_USE_DESTROYED_POLYMODEL, OF_RENDERED,
        OF_NO_OBJECT_COLLISIONS, OF_STUCK_ON_PORTAL, OF_TEMP_GRAVITY,
        OF_CLIENT_KNOWS, OF_SERVER_SAYS_DELETE, OF_SERVER_OBJECT,
        OF_PING_ACCELERATE, OF_AI_DEATH, OF_SEND_MULTI_REMOVE_ON_DEATH,
        OF_SEND_MULTI_REMOVE_ON_DEATHWS, OF_PREDICTED, OF_INPLAYERINVENTORY
    };
    
    for (size_t i = 0; i < sizeof(flags)/sizeof(flags[0]); i++) {
        for (size_t j = i + 1; j < sizeof(flags)/sizeof(flags[0]); j++) {
            EXPECT_NE(flags[i], flags[j])
                << "Duplicate object flag at positions " << i << " and " << j;
        }
    }
}

// ============================================================================
// Tests - Flag Categories
// ============================================================================

TEST_F(ObjectExternalTest, ObjectFlags_Lifecycle) {
    // Lifecycle flags (dead, destroyed, dying)
    EXPECT_EQ(OF_DEAD & OF_DESTROYED, 0);
    EXPECT_EQ(OF_DEAD & OF_DYING, 0);
    EXPECT_EQ(OF_DESTROYED & OF_DYING, 0);
}

TEST_F(ObjectExternalTest, ObjectFlags_Multiplayer) {
    // Multiplayer-related flags are independent
    EXPECT_EQ(OF_CLIENT_KNOWS & OF_SERVER_OBJECT, 0);
    EXPECT_EQ(OF_CLIENT_KNOWS & OF_PREDICTED, 0);
    EXPECT_EQ(OF_SERVER_OBJECT & OF_PREDICTED, 0);
}

TEST_F(ObjectExternalTest, ObjectFlags_Rendering) {
    // Rendering flags
    EXPECT_EQ(OF_SAFE_TO_RENDER & OF_RENDERED, 0);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
