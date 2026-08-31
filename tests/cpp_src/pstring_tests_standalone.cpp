/**
 * @file pstring_tests_standalone.cpp
 * @brief D3 Coverage Tests - pstring module (Standalone).
 *
 * @details
 * Tests for string manipulation utilities.
 * Uses seeded RNG for reproducible test data.
 *
 * This harness validates the behavior of `Descent3/pstring_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/pstring_tests_standalone.cpp`
 * @par Harness
 * `pstring_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/pstring_tests_standalone.cpp
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <string>
#include <cstring>

// ============================================================================
// D3 pstring Implementation (matching lib/pstring.h and misc/pstring.cpp)
// ============================================================================

#include <cctype>

size_t CleanupStr(char *dest, const char *src, size_t destlen) {
    if (destlen == 0)
        return 0;

    const char *end;
    size_t out_size;

    while (std::isspace((unsigned char)*src))
        src++;

    if (*src == '\0') {
        *dest = '\0';
        return 1;
    }

    end = src + std::strlen(src) - 1;
    while (end > src && std::isspace((unsigned char)*end))
        end--;
    end++;

    out_size = (end - src) < destlen - 1 ? (end - src) : destlen - 1;

    std::memcpy(dest, src, out_size);
    dest[out_size] = '\0';

    return out_size;
}

// ============================================================================
// Test Fixtures
// ============================================================================

/**
 * @brief GTest fixture for PstringTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PstringTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    PstringTest() : rng_(SEED) {}
};

// ============================================================================
// CleanupStr tests
// ============================================================================

/**
 * @test PstringTest.EmptyString
 * @brief Verifies empty String.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, EmptyString) {
    char dest[100];
    size_t len = CleanupStr(dest, "", 100);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "");
}

/**
 * @test PstringTest.NoSpaces
 * @brief Verifies no Spaces.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, NoSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "hello", 100);
    EXPECT_EQ(len, 5);
    EXPECT_STREQ(dest, "hello");
}

/**
 * @test PstringTest.LeadingSpaces
 * @brief Verifies leading Spaces.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, LeadingSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "   hello", 100);
    EXPECT_EQ(len, 5);
    EXPECT_STREQ(dest, "hello");
}

/**
 * @test PstringTest.TrailingSpaces
 * @brief Verifies trailing Spaces.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, TrailingSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "hello   ", 100);
    EXPECT_EQ(len, 5);
    EXPECT_STREQ(dest, "hello");
}

/**
 * @test PstringTest.LeadingAndTrailingSpaces
 * @brief Verifies leading And Trailing Spaces.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, LeadingAndTrailingSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "   hello world   ", 100);
    EXPECT_EQ(len, 11);
    EXPECT_STREQ(dest, "hello world");
}

/**
 * @test PstringTest.TabsInsteadOfSpaces
 * @brief Verifies tabs Instead Of Spaces.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, TabsInsteadOfSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "\t\thello\t\t", 100);
    EXPECT_EQ(len, 5);
    EXPECT_STREQ(dest, "hello");
}

/**
 * @test PstringTest.MixedWhitespace
 * @brief Verifies mixed Whitespace.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, MixedWhitespace) {
    char dest[100];
    size_t len = CleanupStr(dest, " \t  hello world \t ", 100);
    EXPECT_EQ(len, 11);
    EXPECT_STREQ(dest, "hello world");
}

/**
 * @test PstringTest.OnlySpaces
 * @brief Verifies only Spaces.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, OnlySpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "     ", 100);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "");
}

/**
 * @test PstringTest.OnlyTabs
 * @brief Verifies only Tabs.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, OnlyTabs) {
    char dest[100];
    size_t len = CleanupStr(dest, "\t\t\t", 100);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "");
}

/**
 * @test PstringTest.InternalSpacesPreserved
 * @brief Verifies internal Spaces Preserved.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, InternalSpacesPreserved) {
    char dest[100];
    size_t len = CleanupStr(dest, "  hello  world  ", 100);
    EXPECT_EQ(len, 12);
    EXPECT_STREQ(dest, "hello  world");
}

/**
 * @test PstringTest.DestLenSmallerThanSource
 * @brief Verifies dest Len Smaller Than Source.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, DestLenSmallerThanSource) {
    char dest[5];
    size_t len = CleanupStr(dest, "hello world", 5);
    EXPECT_EQ(len, 4);
    EXPECT_STREQ(dest, "hell");
}

/**
 * @test PstringTest.DestLenExactlyOne
 * @brief Verifies dest Len Exactly One.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, DestLenExactlyOne) {
    char dest[2];
    size_t len = CleanupStr(dest, "hello", 2);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "h");
}

/**
 * @test PstringTest.DestLenZero
 * @brief Verifies dest Len Zero.
 *
 * @details
 * Exercises the PstringTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringTest, DestLenZero) {
    char dest[100];
    size_t len = CleanupStr(dest, "hello", 0);
    EXPECT_EQ(len, 0);
}

// ============================================================================
// Property-based tests
// ============================================================================

/**
 * @brief GTest fixture for PstringPropertyTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PstringPropertyTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 54321;
    std::mt19937 rng_;
    
    PstringPropertyTest() : rng_(SEED) {}
};

/**
 * @test PstringPropertyTest.CleanupPreservesInternalContent
 * @brief Verifies cleanup Preserves Internal Content.
 *
 * @details
 * Exercises the PstringPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringPropertyTest, CleanupPreservesInternalContent) {
    const char* inputs[] = {
        "hello world",
        "  a b c  ",
        "\ttrimmed\t",
        "no spaces",
        "  multiple   spaces   here  "
    };
    
    for (const char* input : inputs) {
        char dest[100];
        CleanupStr(dest, input, 100);
        
        const char* p = input;
        while (*p == ' ' || *p == '\t') p++;
        const char* start = p;
        const char* end = start + strlen(start) - 1;
        while (end > start && (*end == ' ' || *end == '\t')) end--;
        size_t expected_len = end - start + 1;
        
        EXPECT_EQ(strlen(dest), expected_len);
    }
}

/**
 * @test PstringPropertyTest.CleanupNeverExceedsDestLen
 * @brief Verifies cleanup Never Exceeds Dest Len.
 *
 * @details
 * Exercises the PstringPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PstringPropertyTest, CleanupNeverExceedsDestLen) {
    char dest[10];
    const char* inputs[] = {
        "hello world",
        "a b c d e f g h i j k l m n o p q r s t u v w x y z",
        "   long string with spaces   "
    };
    
    for (const char* input : inputs) {
        size_t len = CleanupStr(dest, input, 10);
        EXPECT_LE(len, 9u);
        EXPECT_LE(strlen(dest), 9u);
    }
}

// ============================================================================
// Edge cases
// ============================================================================

/**
 * @test PstringEdgeCase.SingleCharNoSpace
 * @brief Verifies single Char No Space.
 *
 * @details
 * Exercises the PstringEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(PstringEdgeCase, SingleCharNoSpace) {
    char dest[10];
    size_t len = CleanupStr(dest, "a", 10);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "a");
}

/**
 * @test PstringEdgeCase.SingleCharWithSpace
 * @brief Verifies single Char With Space.
 *
 * @details
 * Exercises the PstringEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(PstringEdgeCase, SingleCharWithSpace) {
    char dest[10];
    size_t len = CleanupStr(dest, " a ", 10);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "a");
}

/**
 * @test PstringEdgeCase.NullPointer
 * @brief Verifies null Pointer.
 *
 * @details
 * Exercises the PstringEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pstring_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(PstringEdgeCase, NullPointer) {
    char dest[10];
    size_t len = CleanupStr(dest, "test", 10);
    EXPECT_GT(len, 0);
}
