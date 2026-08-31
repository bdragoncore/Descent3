#include "gtest/gtest.h"
#include "pstypes.h"
#include <cstring>

#define HOTSPOT_DISPLAY "TelComon.HSM"
#define TELCOM_DISPLAY_TGA "TelComOff.TGA"
#define TELCOM_DISPLAY_OGF "TelComOff.OGF"
#define TELCOM_DISPLAY_OGF_ON "TelComOn.OGF"
#define TS_POWER 1
#define TS_OFF -3
#define POWER_BUTTON 1
#define NEON_LIGHT 2
#define NUMBER_OF_SYSTEMS 5
#define TS_MAINMENU TS_OFF
#define TS_MISSION 1
#define TS_CARGO 2
#define TS_MAP 3
#define TS_SHIPSELECT 4
#define TS_GOALS 5
#define MAX_TELCOM_SCREENS 20
#define MAX_TELCOM_EVENTS 10
#define DUMMY_SCREEN (MAX_TELCOM_SCREENS - 1)
#define MONITOR_MAIN 0
#define MONITOR_TOP 1
#define MAX_MONITOR 2
#define TCSYS_TAB 0
#define TCSYS_UP 1
#define TCSYS_DOWN 2
#define TCSYS_LEFT 3
#define TCSYS_RIGHT 4
#define TCSYS_ENTER 5
#define TCSYS_SPACE 6
#define TCSYS_Q 7
#define TCSYS_F1 8
#define TCSYS_F2 9
#define TCSYS_F3 10
#define TCSYS_MAXKEYS 21
#define SS_EMPTY 0
#define SS_BEING_MADE 1
#define SS_READY 2
#define TCS_POWEROFF 0
#define TCS_POWERON 1
#define TCSND_STARTUP 0
#define TCSND_STATIC 1
#define TCSND_SHUTDOWN 2
#define TCSND_RUNNING 3
#define TCSND_LIGHTBULB 4
#define TCSND_TYPING 5
#define TCSND_CLICK 6
#define TCSND_SOUNDCOUNT 7

TEST(TelComConstantsTest, DisplayFileNames) {
    EXPECT_STREQ(HOTSPOT_DISPLAY, "TelComon.HSM");
    EXPECT_STREQ(TELCOM_DISPLAY_TGA, "TelComOff.TGA");
    EXPECT_STREQ(TELCOM_DISPLAY_OGF, "TelComOff.OGF");
    EXPECT_STREQ(TELCOM_DISPLAY_OGF_ON, "TelComOn.OGF");
}

TEST(TelComConstantsTest, PowerState) {
    EXPECT_EQ(TS_POWER, 1);
    EXPECT_EQ(TS_OFF, -3);
}

TEST(TelComConstantsTest, ButtonTypes) {
    EXPECT_EQ(POWER_BUTTON, 1);
    EXPECT_EQ(NEON_LIGHT, 2);
}

TEST(TelComConstantsTest, NumberOfSystems) {
    EXPECT_EQ(NUMBER_OF_SYSTEMS, 5);
    EXPECT_GT(NUMBER_OF_SYSTEMS, 0);
}

TEST(TelComConstantsTest, SystemIndices) {
    EXPECT_EQ(TS_MAINMENU, -3);
    EXPECT_EQ(TS_MISSION, 1);
    EXPECT_EQ(TS_CARGO, 2);
    EXPECT_EQ(TS_MAP, 3);
    EXPECT_EQ(TS_SHIPSELECT, 4);
    EXPECT_EQ(TS_GOALS, 5);
}

TEST(TelComConstantsTest, MaxTelComScreens) {
    EXPECT_EQ(MAX_TELCOM_SCREENS, 20);
    EXPECT_GT(MAX_TELCOM_SCREENS, 0);
}

TEST(TelComConstantsTest, MaxTelComEvents) {
    EXPECT_EQ(MAX_TELCOM_EVENTS, 10);
    EXPECT_GT(MAX_TELCOM_EVENTS, 0);
}

TEST(TelComConstantsTest, DummyScreen) {
    EXPECT_EQ(DUMMY_SCREEN, MAX_TELCOM_SCREENS - 1);
}

TEST(TelComConstantsTest, MonitorTypes) {
    EXPECT_EQ(MONITOR_MAIN, 0);
    EXPECT_EQ(MONITOR_TOP, 1);
    EXPECT_EQ(MAX_MONITOR, 2);
}

TEST(TelComConstantsTest, SystemKeyTypes) {
    EXPECT_EQ(TCSYS_TAB, 0);
    EXPECT_EQ(TCSYS_UP, 1);
    EXPECT_EQ(TCSYS_DOWN, 2);
    EXPECT_EQ(TCSYS_LEFT, 3);
    EXPECT_EQ(TCSYS_RIGHT, 4);
    EXPECT_EQ(TCSYS_ENTER, 5);
    EXPECT_EQ(TCSYS_SPACE, 6);
    EXPECT_EQ(TCSYS_Q, 7);
    EXPECT_EQ(TCSYS_F1, 8);
    EXPECT_EQ(TCSYS_F2, 9);
    EXPECT_EQ(TCSYS_F3, 10);
    EXPECT_EQ(TCSYS_MAXKEYS, 21);
}

TEST(TelComConstantsTest, ScreenStates) {
    EXPECT_EQ(SS_EMPTY, 0);
    EXPECT_EQ(SS_BEING_MADE, 1);
    EXPECT_EQ(SS_READY, 2);
}

TEST(TelComConstantsTest, TelComState) {
    EXPECT_EQ(TCS_POWEROFF, 0);
    EXPECT_EQ(TCS_POWERON, 1);
}

TEST(TelComConstantsTest, TelComSounds) {
    EXPECT_EQ(TCSND_STARTUP, 0);
    EXPECT_EQ(TCSND_STATIC, 1);
    EXPECT_EQ(TCSND_SHUTDOWN, 2);
    EXPECT_EQ(TCSND_RUNNING, 3);
    EXPECT_EQ(TCSND_LIGHTBULB, 4);
    EXPECT_EQ(TCSND_TYPING, 5);
    EXPECT_EQ(TCSND_CLICK, 6);
    EXPECT_EQ(TCSND_SOUNDCOUNT, 7);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
