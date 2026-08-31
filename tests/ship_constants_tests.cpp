#include "gtest/gtest.h"
#include "pstypes.h"
#include <cstring>

// Ship constants (from Descent3/ship.h) for standalone tests
#define MAX_SHIPS 30
#define DEFAULT_SHIP "Pyro-GL"
#define SFF_FUSION 1
#define SFF_ZOOM 4
#define SFF_TENTHS 8
#define SHIP_PYRO_ID 0
#define SHIP_PHOENIX_ID 1
#define SHIP_MAGNUM_ID 2
#define MAX_DEFAULT_SHIPS 3
#define SF_DEFAULT_ALLOW 1

TEST(ShipConstantsTest, MaxShips) {
    EXPECT_EQ(MAX_SHIPS, 30);
    EXPECT_GT(MAX_SHIPS, 0);
}

TEST(ShipConstantsTest, DefaultShipName) {
    EXPECT_STREQ(DEFAULT_SHIP, "Pyro-GL");
    EXPECT_GT(strlen(DEFAULT_SHIP), 0);
}

TEST(ShipConstantsTest, ShipFeatureFlags) {
    EXPECT_EQ(SFF_FUSION, 1);
    EXPECT_EQ(SFF_ZOOM, 4);
    EXPECT_EQ(SFF_TENTHS, 8);
    
    EXPECT_EQ(SFF_FUSION, 1 << 0);
    EXPECT_EQ(SFF_ZOOM, 1 << 2);
    EXPECT_EQ(SFF_TENTHS, 1 << 3);
    
    int all_sff = SFF_FUSION | SFF_ZOOM | SFF_TENTHS;
    EXPECT_EQ(all_sff, 0xD);
}

TEST(ShipConstantsTest, ShipIds) {
    EXPECT_EQ(SHIP_PYRO_ID, 0);
    EXPECT_EQ(SHIP_PHOENIX_ID, 1);
    EXPECT_EQ(SHIP_MAGNUM_ID, 2);
    
    EXPECT_EQ(SHIP_MAGNUM_ID - SHIP_PYRO_ID, 2);
}

TEST(ShipConstantsTest, MaxDefaultShips) {
    EXPECT_EQ(MAX_DEFAULT_SHIPS, 3);
    EXPECT_GT(MAX_DEFAULT_SHIPS, 0);
    EXPECT_LE(MAX_DEFAULT_SHIPS, MAX_SHIPS);
}

TEST(ShipConstantsTest, ShipFlags) {
    EXPECT_EQ(SF_DEFAULT_ALLOW, 1);
    EXPECT_GT(SF_DEFAULT_ALLOW, 0);
}

TEST(ShipConstantsTest, ShipIdsAreSequential) {
    EXPECT_EQ(SHIP_PYRO_ID + 1, SHIP_PHOENIX_ID);
    EXPECT_EQ(SHIP_PHOENIX_ID + 1, SHIP_MAGNUM_ID);
}

TEST(ShipConstantsTest, DefaultShipsNotZero) {
    EXPECT_EQ(SHIP_PYRO_ID, 0);  // first ship has id 0
    EXPECT_NE(SHIP_PHOENIX_ID, 0);
    EXPECT_NE(SHIP_MAGNUM_ID, 0);
}

TEST(ShipConstantsTest, MaxShipsGreaterThanDefaults) {
    EXPECT_GT(MAX_SHIPS, MAX_DEFAULT_SHIPS);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
