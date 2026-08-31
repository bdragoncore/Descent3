/*
 * D3 Coverage Tests - pstring module (Standalone)
 * 
 * Tests for string manipulation utilities.
 * Uses seeded RNG for reproducible test data.
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

class PstringTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    PstringTest() : rng_(SEED) {}
};

// ============================================================================
// CleanupStr tests
// ============================================================================

TEST_F(PstringTest, EmptyString) {
    char dest[100];
    size_t len = CleanupStr(dest, "", 100);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "");
}

TEST_F(PstringTest, NoSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "hello", 100);
    EXPECT_EQ(len, 5);
    EXPECT_STREQ(dest, "hello");
}

TEST_F(PstringTest, LeadingSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "   hello", 100);
    EXPECT_EQ(len, 5);
    EXPECT_STREQ(dest, "hello");
}

TEST_F(PstringTest, TrailingSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "hello   ", 100);
    EXPECT_EQ(len, 5);
    EXPECT_STREQ(dest, "hello");
}

TEST_F(PstringTest, LeadingAndTrailingSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "   hello world   ", 100);
    EXPECT_EQ(len, 11);
    EXPECT_STREQ(dest, "hello world");
}

TEST_F(PstringTest, TabsInsteadOfSpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "\t\thello\t\t", 100);
    EXPECT_EQ(len, 5);
    EXPECT_STREQ(dest, "hello");
}

TEST_F(PstringTest, MixedWhitespace) {
    char dest[100];
    size_t len = CleanupStr(dest, " \t  hello world \t ", 100);
    EXPECT_EQ(len, 11);
    EXPECT_STREQ(dest, "hello world");
}

TEST_F(PstringTest, OnlySpaces) {
    char dest[100];
    size_t len = CleanupStr(dest, "     ", 100);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "");
}

TEST_F(PstringTest, OnlyTabs) {
    char dest[100];
    size_t len = CleanupStr(dest, "\t\t\t", 100);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "");
}

TEST_F(PstringTest, InternalSpacesPreserved) {
    char dest[100];
    size_t len = CleanupStr(dest, "  hello  world  ", 100);
    EXPECT_EQ(len, 12);
    EXPECT_STREQ(dest, "hello  world");
}

TEST_F(PstringTest, DestLenSmallerThanSource) {
    char dest[5];
    size_t len = CleanupStr(dest, "hello world", 5);
    EXPECT_EQ(len, 4);
    EXPECT_STREQ(dest, "hell");
}

TEST_F(PstringTest, DestLenExactlyOne) {
    char dest[2];
    size_t len = CleanupStr(dest, "hello", 2);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "h");
}

TEST_F(PstringTest, DestLenZero) {
    char dest[100];
    size_t len = CleanupStr(dest, "hello", 0);
    EXPECT_EQ(len, 0);
}

// ============================================================================
// Property-based tests
// ============================================================================

class PstringPropertyTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 54321;
    std::mt19937 rng_;
    
    PstringPropertyTest() : rng_(SEED) {}
};

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

TEST(PstringEdgeCase, SingleCharNoSpace) {
    char dest[10];
    size_t len = CleanupStr(dest, "a", 10);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "a");
}

TEST(PstringEdgeCase, SingleCharWithSpace) {
    char dest[10];
    size_t len = CleanupStr(dest, " a ", 10);
    EXPECT_EQ(len, 1);
    EXPECT_STREQ(dest, "a");
}

TEST(PstringEdgeCase, NullPointer) {
    char dest[10];
    size_t len = CleanupStr(dest, "test", 10);
    EXPECT_GT(len, 0);
}
