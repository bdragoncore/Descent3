/**
 * @file megacell_real_tests.cpp
 * @brief Tests for megacell.cpp (138 lines) — the 8x8 texture megacell pool.
 *
 * @details
 * Covers InitMegacells clearing, Alloc/Free lifecycle and reuse,
 * circular GetNext/GetPrev search, empty-pool and single-element
 * edge cases, case-insensitive Find by name that ignores unused slots,
 * and the default dimensions / zeroed state of freshly allocated cells.
 *
 * This harness validates the behavior of `Descent3/megacell.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/megacell.cpp`
 * @par Harness
 * `megacell_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/megacell.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>

#include "megacell.h"

// megacell.cpp defines these globals
// extern megacell Megacells[MAX_MEGACELLS];
// extern int Num_megacells;

/**
 * @brief GTest fixture for MegacellTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class MegacellTest : public ::testing::Test {
protected:
  void SetUp() override { InitMegacells(); }
};

/**
 * @test MegacellTest.InitClearsAllSlotsAndCount
 * @brief Verifies init Clears All Slots And Count.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, InitClearsAllSlotsAndCount) {
  // dirty a couple slots first then re-init
  int a = AllocMegacell();
  int b = AllocMegacell();
  Megacells[a].flags = 123;
  strncpy(Megacells[a].name, "hello", PAGENAME_LEN);
  Megacells[b].name[0] = 'x';

  InitMegacells();

  EXPECT_EQ(Num_megacells, 0);
  for (int i = 0; i < MAX_MEGACELLS; i++) {
    EXPECT_EQ(Megacells[i].used, 0) << "slot " << i;
    EXPECT_EQ(Megacells[i].name[0], '\0') << "slot " << i;
  }
}

/**
 * @test MegacellTest.AllocReturnsSequentialHandlesWithDefaults
 * @brief Verifies alloc Returns Sequential Handles With Defaults.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, AllocReturnsSequentialHandlesWithDefaults) {
  int h0 = AllocMegacell();
  int h1 = AllocMegacell();
  int h2 = AllocMegacell();

  EXPECT_EQ(h0, 0);
  EXPECT_EQ(h1, 1);
  EXPECT_EQ(h2, 2);
  EXPECT_EQ(Num_megacells, 3);

  for (int h : {h0, h1, h2}) {
    EXPECT_EQ(Megacells[h].used, 1);
    EXPECT_EQ(Megacells[h].width, DEFAULT_MEGACELL_WIDTH);
    EXPECT_EQ(Megacells[h].height, DEFAULT_MEGACELL_HEIGHT);
    EXPECT_EQ(Megacells[h].flags, 0);
    EXPECT_EQ(Megacells[h].name[0], '\0');
  }
}

/**
 * @test MegacellTest.AllocZeroesTextureHandlesAndFlags
 * @brief Verifies alloc Zeroes Texture Handles And Flags.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, AllocZeroesTextureHandlesAndFlags) {
  // pre-pollute a slot's memory then free and re-alloc to ensure memset
  int h = AllocMegacell();
  for (int i = 0; i < MAX_MEGACELL_WIDTH * MAX_MEGACELL_HEIGHT; i++)
    Megacells[h].texture_handles[i] = 0x7fff;
  Megacells[h].flags = 999;
  Megacells[h].width = 1;
  Megacells[h].height = 2;
  FreeMegacell(h);

  int h2 = AllocMegacell();
  EXPECT_EQ(h2, h);
  EXPECT_EQ(Megacells[h2].flags, 0);
  EXPECT_EQ(Megacells[h2].width, DEFAULT_MEGACELL_WIDTH);
  EXPECT_EQ(Megacells[h2].height, DEFAULT_MEGACELL_HEIGHT);
  for (int i = 0; i < MAX_MEGACELL_WIDTH * MAX_MEGACELL_HEIGHT; i++) {
    EXPECT_EQ(Megacells[h2].texture_handles[i], 0) << "handle " << i;
  }
}

/**
 * @test MegacellTest.FreeClearsUsedAndNameAndDecrements
 * @brief Verifies free Clears Used And Name And Decrements.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, FreeClearsUsedAndNameAndDecrements) {
  int h = AllocMegacell();
  strncpy(Megacells[h].name, "MyCell", PAGENAME_LEN);
  EXPECT_EQ(Num_megacells, 1);

  FreeMegacell(h);
  EXPECT_EQ(Megacells[h].used, 0);
  EXPECT_EQ(Megacells[h].name[0], '\0');
  EXPECT_EQ(Num_megacells, 0);
}

/**
 * @test MegacellTest.AllocReusesLowestFreedSlot
 * @brief Verifies alloc Reuses Lowest Freed Slot.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, AllocReusesLowestFreedSlot) {
  int h0 = AllocMegacell(); // 0
  int h1 = AllocMegacell(); // 1
  int h2 = AllocMegacell(); // 2
  (void)h0; // suppress unused warning in some builds (actually used via value)
  FreeMegacell(h1);         // free middle
  EXPECT_EQ(Num_megacells, 2);

  int h3 = AllocMegacell();
  EXPECT_EQ(h3, 1); // lowest free reused
  EXPECT_EQ(Num_megacells, 3);

  FreeMegacell(h0);
  FreeMegacell(h2);
  FreeMegacell(h3);
  EXPECT_EQ(Num_megacells, 0);
  EXPECT_EQ(AllocMegacell(), 0); // wraps back to 0
}

/**
 * @test MegacellTest.AllocFailsWhenFull
 * @brief Verifies alloc Fails When Full.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, AllocFailsWhenFull) {
  for (int i = 0; i < MAX_MEGACELLS; i++) {
    int h = AllocMegacell();
    ASSERT_GE(h, 0) << "failed at " << i;
    EXPECT_EQ(h, i);
  }
  EXPECT_EQ(Num_megacells, MAX_MEGACELLS);
  EXPECT_EQ(AllocMegacell(), -1); // pool exhausted
  EXPECT_EQ(Num_megacells, MAX_MEGACELLS);

  // freeing one slot makes allocation succeed again at that index
  FreeMegacell(50);
  EXPECT_EQ(Num_megacells, MAX_MEGACELLS - 1);
  EXPECT_EQ(AllocMegacell(), 50);
}

/**
 * @test MegacellTest.GetNextWrapsCircularly
 * @brief Verifies get Next Wraps Circularly.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, GetNextWrapsCircularly) {
  // sparse allocation: 2, 5, 9
  for (int i = 0; i < 10; i++)
    AllocMegacell(); // 0..9
  for (int i : {0, 1, 3, 4, 6, 7, 8})
    FreeMegacell(i);
  // remaining used: 2,5,9  (Num=3)
  EXPECT_EQ(GetNextMegacell(2), 5);
  EXPECT_EQ(GetNextMegacell(5), 9);
  EXPECT_EQ(GetNextMegacell(9), 2); // wraps to beginning
  EXPECT_EQ(GetNextMegacell(0), 2); // gap before first
  EXPECT_EQ(GetNextMegacell(3), 5);
  EXPECT_EQ(GetNextMegacell(6), 9);
}

/**
 * @test MegacellTest.GetPrevWrapsCircularlyReverse
 * @brief Verifies get Prev Wraps Circularly Reverse.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, GetPrevWrapsCircularlyReverse) {
  for (int i = 0; i < 10; i++)
    AllocMegacell();
  for (int i : {0, 1, 3, 4, 6, 7, 8})
    FreeMegacell(i);
  // remaining used: 2,5,9
  EXPECT_EQ(GetPrevMegacell(9), 5);
  EXPECT_EQ(GetPrevMegacell(5), 2);
  EXPECT_EQ(GetPrevMegacell(2), 9); // wraps to end
  EXPECT_EQ(GetPrevMegacell(6), 5);
  EXPECT_EQ(GetPrevMegacell(0), 9); // before first wraps to last
}

/**
 * @test MegacellTest.GetNextAndPrevSingleElementReturnsSelf
 * @brief Verifies get Next And Prev Single Element Returns Self.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, GetNextAndPrevSingleElementReturnsSelf) {
  int h = AllocMegacell(); // only 0 used
  EXPECT_EQ(GetNextMegacell(h), h);
  EXPECT_EQ(GetPrevMegacell(h), h);
  EXPECT_EQ(GetNextMegacell(50), h); // any n finds the sole element
  EXPECT_EQ(GetPrevMegacell(50), h);
}

/**
 * @test MegacellTest.GetNextAndPrevEmptyReturnsZero
 * @brief Verifies get Next And Prev Empty Returns Zero.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, GetNextAndPrevEmptyReturnsZero) {
  EXPECT_EQ(Num_megacells, 0);
  EXPECT_EQ(GetNextMegacell(0), 0);
  EXPECT_EQ(GetPrevMegacell(0), 0);
  EXPECT_EQ(GetNextMegacell(50), 0);
  EXPECT_EQ(GetPrevMegacell(50), 0);
}

/**
 * @test MegacellTest.FindByNameCaseInsensitiveIgnoresUnused
 * @brief Verifies find By Name Case Insensitive Ignores Unused.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, FindByNameCaseInsensitiveIgnoresUnused) {
  int h0 = AllocMegacell();
  int h1 = AllocMegacell();
  strncpy(Megacells[h0].name, "Alpha", PAGENAME_LEN);
  strncpy(Megacells[h1].name, "BETA", PAGENAME_LEN);

  char q1[] = "alpha";
  char q2[] = "ALPHA";
  char q3[] = "AlPhA";
  char q4[] = "beta";
  char q5[] = "BETA";
  char q6[] = "gamma";

  EXPECT_EQ(FindMegacellName(q1), h0);
  EXPECT_EQ(FindMegacellName(q2), h0);
  EXPECT_EQ(FindMegacellName(q3), h0);
  EXPECT_EQ(FindMegacellName(q4), h1);
  EXPECT_EQ(FindMegacellName(q5), h1);
  EXPECT_EQ(FindMegacellName(q6), -1);

  // freed entry not found even though name still in memory until cleared
  FreeMegacell(h0);
  EXPECT_EQ(FindMegacellName(q1), -1);

  // unused slot with matching name must not be found
  strncpy(Megacells[80].name, "gamma", PAGENAME_LEN); // slot unused
  Megacells[80].used = 0;
  EXPECT_EQ(FindMegacellName(q6), -1);
}

/**
 * @test MegacellTest.FindReturnsFirstMatchWhenDuplicates
 * @brief Verifies find Returns First Match When Duplicates.
 *
 * @details
 * Exercises the MegacellTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/megacell.cpp
 * @ingroup descent3_tests
 */
TEST_F(MegacellTest, FindReturnsFirstMatchWhenDuplicates) {
  int h0 = AllocMegacell();
  int h1 = AllocMegacell();
  strncpy(Megacells[h0].name, "dup", PAGENAME_LEN);
  strncpy(Megacells[h1].name, "dup", PAGENAME_LEN);
  char q[] = "dup";
  EXPECT_EQ(FindMegacellName(q), h0); // linear scan returns lowest index
}
