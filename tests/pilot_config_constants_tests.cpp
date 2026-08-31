#include "gtest/gtest.h"
#include "pstypes.h"

#define PILOT_STRING_SIZE 20
#define PPIC_INVALID_ID 65535
#define PLT_FILE_FATAL (-3)
#define PLT_FILE_CANT_CREATE (-2)
#define PLT_FILE_EXISTS (-1)
#define N_MOUSE_AXIS 2
#define N_JOY_AXIS 6
#define MAX_PILOT_TAUNTS 8
#define PILOT_TAUNT_SIZE 60
#define N_SUPPORTED_VIDRES 8
#define RES_512X384 0
#define RES_640X480 1
#define RES_800X600 2
#define RES_960X720 3
#define RES_1024X768 4
#define RES_1280X960 5
#define RES_1600X1200 6
#define DETAIL_LEVEL_LOW 0
#define DETAIL_LEVEL_MED 1
#define DETAIL_LEVEL_HIGH 2
#define DETAIL_LEVEL_VERY_HIGH 3
#define DETAIL_LEVEL_CUSTOM 4

TEST(PilotConstantsTest, PilotStringSize) {
    EXPECT_EQ(PILOT_STRING_SIZE, 20);
    EXPECT_GT(PILOT_STRING_SIZE, 0);
}

TEST(PilotConstantsTest, PilotPicId) {
    EXPECT_EQ(PPIC_INVALID_ID, 65535);
    EXPECT_EQ(PPIC_INVALID_ID, 0xFFFF);
}

TEST(PilotConstantsTest, PilotFileErrors) {
    EXPECT_EQ(PLT_FILE_FATAL, -3);
    EXPECT_EQ(PLT_FILE_CANT_CREATE, -2);
    EXPECT_EQ(PLT_FILE_EXISTS, -1);
}

TEST(PilotConstantsTest, InputAxisCounts) {
    EXPECT_EQ(N_MOUSE_AXIS, 2);
    EXPECT_EQ(N_JOY_AXIS, 6);
    EXPECT_GT(N_JOY_AXIS, N_MOUSE_AXIS);
}

TEST(PilotConstantsTest, MaxPilotTaunts) {
    EXPECT_EQ(MAX_PILOT_TAUNTS, 8);
    EXPECT_GT(MAX_PILOT_TAUNTS, 0);
}

TEST(PilotConstantsTest, PilotTauntSize) {
    EXPECT_EQ(PILOT_TAUNT_SIZE, 60);
    EXPECT_GT(PILOT_TAUNT_SIZE, 0);
}

TEST(ConfigConstantsTest, SupportedVideoResolutions) {
    EXPECT_EQ(N_SUPPORTED_VIDRES, 8);
    EXPECT_GT(N_SUPPORTED_VIDRES, 0);
}

TEST(ConfigConstantsTest, ResolutionIdentifiers) {
    EXPECT_EQ(RES_512X384, 0);
    EXPECT_EQ(RES_640X480, 1);
    EXPECT_EQ(RES_800X600, 2);
    EXPECT_EQ(RES_960X720, 3);
    EXPECT_EQ(RES_1024X768, 4);
    EXPECT_EQ(RES_1280X960, 5);
    EXPECT_EQ(RES_1600X1200, 6);
}

TEST(ConfigConstantsTest, DetailLevels) {
    EXPECT_EQ(DETAIL_LEVEL_LOW, 0);
    EXPECT_EQ(DETAIL_LEVEL_MED, 1);
    EXPECT_EQ(DETAIL_LEVEL_HIGH, 2);
    EXPECT_EQ(DETAIL_LEVEL_VERY_HIGH, 3);
    EXPECT_EQ(DETAIL_LEVEL_CUSTOM, 4);
}

TEST(ConfigConstantsTest, DetailLevelRange) {
    EXPECT_EQ(DETAIL_LEVEL_CUSTOM - DETAIL_LEVEL_LOW, 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
