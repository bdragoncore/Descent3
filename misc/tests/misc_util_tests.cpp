/**
 * @file misc_util_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for the real misc library string utilities:
 * CleanupStr, StringJoin, StringSplit (misc/pstring.cpp).
 *
 * This harness validates the behavior of `misc/pstring.cpp, misc/pstring.h`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `misc/pstring.cpp, misc/pstring.h`
 * @par Harness
 * `misc_util_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see misc/pstring.cpp
 * @see misc/pstring.h
 */

#include <gtest/gtest.h>

#include "pstring.h"

// ============================================================================
// CleanupStr — trims leading/trailing whitespace into a fixed buffer
// ============================================================================

/**
 * @test Pstring.CleanupStr_TrimsBothEnds
 * @brief Verifies cleanup Str Trims Both Ends.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, CleanupStr_TrimsBothEnds) {
  char out[64];
  std::size_t n = CleanupStr(out, "  hello world  \t", sizeof(out));
  EXPECT_EQ(std::string(out), "hello world");
  EXPECT_EQ(n, std::strlen("hello world"));
}

/**
 * @test Pstring.CleanupStr_NoWhitespacePassthrough
 * @brief Verifies cleanup Str No Whitespace Passthrough.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, CleanupStr_NoWhitespacePassthrough) {
  char out[64];
  std::size_t n = CleanupStr(out, "plain", sizeof(out));
  EXPECT_EQ(std::string(out), "plain");
  EXPECT_EQ(n, 5);
}

/**
 * @test Pstring.CleanupStr_EmptyString
 * @brief Verifies cleanup Str Empty String.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, CleanupStr_EmptyString) {
  char out[64];
  std::size_t n = CleanupStr(out, "", sizeof(out));
  EXPECT_EQ(std::string(out), "");
  EXPECT_EQ(n, 1);
}

/**
 * @test Pstring.CleanupStr_AllWhitespace
 * @brief Verifies cleanup Str All Whitespace.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, CleanupStr_AllWhitespace) {
  char out[64];
  std::size_t n = CleanupStr(out, "   \t\n ", sizeof(out));
  EXPECT_EQ(std::string(out), "");
  EXPECT_EQ(n, 1);
}

/**
 * @test Pstring.CleanupStr_ZeroLengthDest
 * @brief Verifies cleanup Str Zero Length Dest.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, CleanupStr_ZeroLengthDest) {
  char out[4] = {'x', 'x', 'x', 'x'};
  std::size_t n = CleanupStr(out, "hello", 0);
  EXPECT_EQ(n, 0);
  EXPECT_EQ(out[0], 'x'); // untouched
}

/**
 * @test Pstring.CleanupStr_TruncatesToDestLen
 * @brief Verifies cleanup Str Truncates To Dest Len.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, CleanupStr_TruncatesToDestLen) {
  char out[6];
  std::size_t n = CleanupStr(out, "hello world", sizeof(out));
  EXPECT_EQ(std::string(out), "hello");
  EXPECT_EQ(n, 5);
}

// ============================================================================
// StringJoin
// ============================================================================

/**
 * @test Pstring.StringJoin_EmptyVector
 * @brief Verifies string Join Empty Vector.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringJoin_EmptyVector) {
  EXPECT_EQ(StringJoin({}, ","), "");
}

/**
 * @test Pstring.StringJoin_SingleElement
 * @brief Verifies string Join Single Element.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringJoin_SingleElement) {
  EXPECT_EQ(StringJoin({"solo"}, ","), "solo");
}

/**
 * @test Pstring.StringJoin_MultipleWithDelimiter
 * @brief Verifies string Join Multiple With Delimiter.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringJoin_MultipleWithDelimiter) {
  EXPECT_EQ(StringJoin({"a", "b", "c"}, ", "), "a, b, c");
}

/**
 * @test Pstring.StringJoin_EmptyDelimiter
 * @brief Verifies string Join Empty Delimiter.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringJoin_EmptyDelimiter) {
  EXPECT_EQ(StringJoin({"a", "b", "c"}, ""), "abc");
}

// ============================================================================
// StringSplit
// ============================================================================

/**
 * @test Pstring.StringSplit_Basic
 * @brief Verifies string Split Basic.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringSplit_Basic) {
  auto parts = StringSplit("a,b,c", ",");
  ASSERT_EQ(parts.size(), 3);
  EXPECT_EQ(parts[0], "a");
  EXPECT_EQ(parts[1], "b");
  EXPECT_EQ(parts[2], "c");
}

/**
 * @test Pstring.StringSplit_NoDelimiter
 * @brief Verifies string Split No Delimiter.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringSplit_NoDelimiter) {
  auto parts = StringSplit("solo", ",");
  ASSERT_EQ(parts.size(), 1);
  EXPECT_EQ(parts[0], "solo");
}

/**
 * @test Pstring.StringSplit_MultiCharDelimiter
 * @brief Verifies string Split Multi Char Delimiter.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringSplit_MultiCharDelimiter) {
  auto parts = StringSplit("a::b::c", "::");
  ASSERT_EQ(parts.size(), 3);
  EXPECT_EQ(parts[0], "a");
  EXPECT_EQ(parts[1], "b");
  EXPECT_EQ(parts[2], "c");
}

/**
 * @test Pstring.StringSplit_TrailingDelimiterKeepsEmpty
 * @brief Verifies string Split Trailing Delimiter Keeps Empty.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringSplit_TrailingDelimiterKeepsEmpty) {
  auto parts = StringSplit("a,b,", ",");
  ASSERT_EQ(parts.size(), 3);
  EXPECT_EQ(parts[0], "a");
  EXPECT_EQ(parts[1], "b");
  EXPECT_EQ(parts[2], "");
}

/**
 * @test Pstring.StringSplit_EmptyString
 * @brief Verifies string Split Empty String.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringSplit_EmptyString) {
  auto parts = StringSplit("", ",");
  ASSERT_EQ(parts.size(), 1);
  EXPECT_EQ(parts[0], "");
}

/**
 * @test Pstring.StringSplit_JoinRoundTrip
 * @brief Verifies string Split Join Round Trip.
 *
 * @details
 * Exercises the Pstring code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see misc/pstring.cpp, misc/pstring.h
 * @ingroup descent3_tests
 */
TEST(Pstring, StringSplit_JoinRoundTrip) {
  std::vector<std::string> parts = {"alpha", "beta", "gamma"};
  std::string joined = StringJoin(parts, ";");
  auto split = StringSplit(joined, ";");
  EXPECT_EQ(split, parts);
}
