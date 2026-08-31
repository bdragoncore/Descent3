#include "gtest/gtest.h"
#include "pstypes.h"

#define AI_SOUND_SHORT_DIST 60.0f

TEST(AIConstantsTest, AISoundShortDistance) {
    EXPECT_EQ(AI_SOUND_SHORT_DIST, 60.0f);
    EXPECT_GT(AI_SOUND_SHORT_DIST, 0.0f);
}

TEST(AIConstantsTest, AISoundShortDistance_Reasonable) {
    EXPECT_LT(AI_SOUND_SHORT_DIST, 1000.0f);
    EXPECT_GT(AI_SOUND_SHORT_DIST, 1.0f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
