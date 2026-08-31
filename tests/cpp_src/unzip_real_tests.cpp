/**
 * @file unzip_real_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for unzip/CompareZipFileName (unzip/unzip.cpp): case-insensitive
 * basename comparison that ignores directory prefix in the zip entry name.
 *
 * This harness validates the behavior of `Descent3/unzip.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/unzip.cpp`
 * @par Harness
 * `unzip_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/unzip.cpp
 */

#include <gtest/gtest.h>

#include "unzip.h"

/**
 * @test Unzip.Compare_ExactMatch
 * @brief Verifies compare Exact Match.
 *
 * @details
 * Exercises the Unzip code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/unzip.cpp
 * @ingroup descent3_tests
 */
TEST(Unzip, Compare_ExactMatch) {
  EXPECT_TRUE(CompareZipFileName("file.txt", "file.txt"));
}

/**
 * @test Unzip.Compare_CaseInsensitive
 * @brief Verifies compare Case Insensitive.
 *
 * @details
 * Exercises the Unzip code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/unzip.cpp
 * @ingroup descent3_tests
 */
TEST(Unzip, Compare_CaseInsensitive) {
  EXPECT_TRUE(CompareZipFileName("FILE.TXT", "file.txt"));
  EXPECT_TRUE(CompareZipFileName("file.txt", "FILE.TXT"));
  EXPECT_TRUE(CompareZipFileName("FiLe.TxT", "fIlE.tXt"));
}

/**
 * @test Unzip.Compare_IgnoresDirectoryPrefix
 * @brief Verifies compare Ignores Directory Prefix.
 *
 * @details
 * Exercises the Unzip code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/unzip.cpp
 * @ingroup descent3_tests
 */
TEST(Unzip, Compare_IgnoresDirectoryPrefix) {
  EXPECT_TRUE(CompareZipFileName("dir/file.txt", "file.txt"));
  EXPECT_TRUE(CompareZipFileName("a/b/c/file.txt", "file.txt"));
  EXPECT_TRUE(CompareZipFileName("DIR/SUB/FILE.TXT", "file.txt"));
}

/**
 * @test Unzip.Compare_Mismatch
 * @brief Verifies compare Mismatch.
 *
 * @details
 * Exercises the Unzip code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/unzip.cpp
 * @ingroup descent3_tests
 */
TEST(Unzip, Compare_Mismatch) {
  EXPECT_FALSE(CompareZipFileName("file.txt", "other.txt"));
  EXPECT_FALSE(CompareZipFileName("dir/file.txt", "other.txt"));
  EXPECT_FALSE(CompareZipFileName("file.txt", "file.tx")); // prefix only
  EXPECT_FALSE(CompareZipFileName("file.txt", "file.txta"));
}

/**
 * @test Unzip.Compare_EmptyMismatch
 * @brief Verifies compare Empty Mismatch.
 *
 * @details
 * Exercises the Unzip code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/unzip.cpp
 * @ingroup descent3_tests
 */
TEST(Unzip, Compare_EmptyMismatch) {
  EXPECT_FALSE(CompareZipFileName("dir/file.txt", "file"));
  EXPECT_FALSE(CompareZipFileName("file.txt", ""));
}

/**
 * @test Unzip.Compare_OnlyBasenameCompared
 * @brief Verifies compare Only Basename Compared.
 *
 * @details
 * Exercises the Unzip code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/unzip.cpp
 * @ingroup descent3_tests
 */
TEST(Unzip, Compare_OnlyBasenameCompared) {
  // "a/b.txt" basename is "b.txt", not "a"
  EXPECT_FALSE(CompareZipFileName("a/b.txt", "a"));
  EXPECT_TRUE(CompareZipFileName("a/b.txt", "b.txt"));
}
