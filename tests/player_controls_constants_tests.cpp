#include "gtest/gtest.h"
#include "pstypes.h"
#include <cstdint>

// Player/controls constants (from Descent3/player.h, controls.h) for standalone tests
#define MAX_WAYPOINTS 25
#define INITIAL_LIVES 3
#define CONVERTER_RATE 20.0f
#define CONVERTER_SCALE 2.0f
#define CONVERTER_SOUND_DELAY 0.5f
#define AFTERBURN_TIME 5000.0
#define PSPF_RED 1
#define PSPF_BLUE 2
#define PSPF_GREEN 4
#define PSPF_YELLOW 8
#define PLAYER_POS_HACK_TIME 10
#define DEATH_RESPAWN_TIME 3.0f
#define SCORE_ADDED_TIME 2.0
#define READF_MOUSE 0x2
#define READF_JOY 0x1
#define JOY_AXIS_SENS_RANGE 4.0f
#define MSE_AXIS_SENS_RANGE 4.0f
#define HEADING_AXIS 0
#define PITCH_AXIS 1
#define BANK_AXIS 2
#define HORIZONTAL_AXIS 3
#define VERTICAL_AXIS 4
#define THROTTLE_AXIS 5

TEST(PlayerConstantsTest, WaypointLimit) {
    EXPECT_EQ(MAX_WAYPOINTS, 25);
    EXPECT_GT(MAX_WAYPOINTS, 0);
}

TEST(PlayerConstantsTest, InitialLives) {
    EXPECT_EQ(INITIAL_LIVES, 3);
    EXPECT_GT(INITIAL_LIVES, 0);
}

TEST(PlayerConstantsTest, ConverterRates) {
    EXPECT_EQ(CONVERTER_RATE, 20.0f);
    EXPECT_EQ(CONVERTER_SCALE, 2.0f);
    EXPECT_EQ(CONVERTER_SOUND_DELAY, 0.5f);
    
    EXPECT_GT(CONVERTER_RATE, CONVERTER_SCALE);
    EXPECT_GT(CONVERTER_SOUND_DELAY, 0.0f);
}

TEST(PlayerConstantsTest, AfterburnTime) {
    EXPECT_EQ(AFTERBURN_TIME, 5000.0);
    EXPECT_GT(AFTERBURN_TIME, 0.0);
}

TEST(PlayerConstantsTest, PSPFFlags) {
    EXPECT_EQ(PSPF_RED, 1);
    EXPECT_EQ(PSPF_BLUE, 2);
    EXPECT_EQ(PSPF_GREEN, 4);
    EXPECT_EQ(PSPF_YELLOW, 8);
    
    int all_pspf = PSPF_RED | PSPF_BLUE | PSPF_GREEN | PSPF_YELLOW;
    EXPECT_EQ(all_pspf, 0xF);
    
    EXPECT_EQ(PSPF_RED, 1 << 0);
    EXPECT_EQ(PSPF_BLUE, 1 << 1);
    EXPECT_EQ(PSPF_GREEN, 1 << 2);
    EXPECT_EQ(PSPF_YELLOW, 1 << 3);
}

TEST(PlayerConstantsTest, PlayerPosHackTime) {
    EXPECT_EQ(PLAYER_POS_HACK_TIME, 10);
    EXPECT_GT(PLAYER_POS_HACK_TIME, 0);
}

TEST(PlayerConstantsTest, DeathRespawnTime) {
    EXPECT_EQ(DEATH_RESPAWN_TIME, 3.0f);
    EXPECT_GT(DEATH_RESPAWN_TIME, 0.0f);
}

TEST(PlayerConstantsTest, ScoreAddedTime) {
    EXPECT_EQ(SCORE_ADDED_TIME, 2.0);
    EXPECT_GT(SCORE_ADDED_TIME, 0.0);
}

TEST(PlayerConstantsTest, ConverterRateGreaterThanScale) {
    EXPECT_GT(CONVERTER_RATE, CONVERTER_SCALE);
}

TEST(ControlsConstantsTest, ReadFlags) {
    EXPECT_EQ(READF_MOUSE, 0x2);
    EXPECT_EQ(READF_JOY, 0x1);
    
    int all_readf = READF_MOUSE | READF_JOY;
    EXPECT_EQ(all_readf, 0x3);
}

TEST(ControlsConstantsTest, AxisSensitivity) {
    EXPECT_EQ(JOY_AXIS_SENS_RANGE, 4.0f);
    EXPECT_EQ(MSE_AXIS_SENS_RANGE, 4.0f);
    
    EXPECT_EQ(JOY_AXIS_SENS_RANGE, MSE_AXIS_SENS_RANGE);
    EXPECT_GT(JOY_AXIS_SENS_RANGE, 0.0f);
}

TEST(ControlsConstantsTest, AxisIdentifiers) {
    EXPECT_EQ(HEADING_AXIS, 0);
    EXPECT_EQ(PITCH_AXIS, 1);
    EXPECT_EQ(BANK_AXIS, 2);
    EXPECT_EQ(HORIZONTAL_AXIS, 3);
    EXPECT_EQ(VERTICAL_AXIS, 4);
    EXPECT_EQ(THROTTLE_AXIS, 5);
    
    EXPECT_EQ(THROTTLE_AXIS - HEADING_AXIS, 5);
}

TEST(ControlsConstantsTest, AxisValuesAreSequential) {
    EXPECT_EQ(HEADING_AXIS + 1, PITCH_AXIS);
    EXPECT_EQ(PITCH_AXIS + 1, BANK_AXIS);
    EXPECT_EQ(BANK_AXIS + 1, HORIZONTAL_AXIS);
    EXPECT_EQ(HORIZONTAL_AXIS + 1, VERTICAL_AXIS);
    EXPECT_EQ(VERTICAL_AXIS + 1, THROTTLE_AXIS);
}

TEST(ControlsConstantsTest, ReadFlagsArePowersOfTwo) {
    EXPECT_EQ(READF_JOY, 1 << 0);
    EXPECT_EQ(READF_MOUSE, 1 << 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
