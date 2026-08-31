/*
 * D3 Coverage Tests - psglob module (Standalone)
 * 
 * Tests for glob pattern matching utilities.
 * Uses seeded RNG for reproducible test data.
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <string>
#include <cstring>

// ============================================================================
// D3 psglob Implementation (matching misc/psglob.cpp)
// ============================================================================

int PSGlobHasPattern(char *string);
int PSGlobMatch(char *pattern, char *text, int case_sensitive, int dot_special);
int PSGlobMatchAfterStar(char *pattern, int case_sensitive, char *text);

// ============================================================================
// Test Fixtures
// ============================================================================

class PsglobTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    PsglobTest() : rng_(SEED) {}
};

// ============================================================================
// PSGlobHasPattern tests
// ============================================================================

TEST_F(PsglobTest, EmptyStringNoPattern) {
    char empty[] = "";
    EXPECT_EQ(PSGlobHasPattern(empty), 0);
}

TEST_F(PsglobTest, PlainStringNoPattern) {
    char s[] = "hello";
    EXPECT_EQ(PSGlobHasPattern(s), 0);
}

TEST_F(PsglobTest, QuestionMarkIsPattern) {
    char s[] = "hel?lo";
    EXPECT_EQ(PSGlobHasPattern(s), 1);
}

TEST_F(PsglobTest, StarIsPattern) {
    char s[] = "hel*lo";
    EXPECT_EQ(PSGlobHasPattern(s), 1);
}

TEST_F(PsglobTest, BracketsArePattern) {
    char s1[] = "file[0-9].txt";
    char s2[] = "file[!0-9].txt";
    EXPECT_EQ(PSGlobHasPattern(s1), 1);
    EXPECT_EQ(PSGlobHasPattern(s2), 1);
}

TEST_F(PsglobTest, UnclosedBracketNotPattern) {
    char s[] = "file[.txt";
    EXPECT_EQ(PSGlobHasPattern(s), 0);
}

TEST_F(PsglobTest, ClosedBracketIsPattern) {
    char s[] = "file].txt";
    EXPECT_EQ(PSGlobHasPattern(s), 0);
}

TEST_F(PsglobTest, EscapeBackslashNotPattern) {
    char s[] = "file\\*.txt";
    EXPECT_EQ(PSGlobHasPattern(s), 0);
}

// ============================================================================
// PSGlobMatch - Exact Match tests
// ============================================================================

TEST_F(PsglobTest, ExactMatch) {
    char p1[] = "hello"; char t1[] = "hello";
    char p2[] = "world"; char t2[] = "world";
    char p3[] = "";     char t3[] = "";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

TEST_F(PsglobTest, ExactMatchFails) {
    char p1[] = "hello"; char t1[] = "world";
    char p2[] = "hello"; char t2[] = "hell";
    char p3[] = "hello"; char t3[] = "helloo";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 0);
}

// ============================================================================
// PSGlobMatch - Question Mark tests
// ============================================================================

TEST_F(PsglobTest, QuestionMarkMatchesOne) {
    char p1[] = "h?llo"; char t1[] = "hello";
    char p2[] = "?????"; char t2[] = "hello";
    char p3[] = "h?llo"; char t3[] = "hallo";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

TEST_F(PsglobTest, QuestionMarkNoMatch) {
    char p1[] = "h?llo"; char t1[] = "hxxxxllo";
    char p2[] = "???";   char t2[] = "hell";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
}

TEST_F(PsglobTest, QuestionMarkAtEnd) {
    char p1[] = "file?"; char t1[] = "file1";
    char p2[] = "file?"; char t2[] = "file";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
}

// ============================================================================
// PSGlobMatch - Star tests
// ============================================================================

TEST_F(PsglobTest, StarMatchesAny) {
    char p1[] = "*";          char t1[] = "anything";
    char p2[] = "*";          char t2[] = "";
    char p3[] = "file*";      char t3[] = "file.txt";
    char p4[] = "*.txt";      char t4[] = "document.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p4, t4, 1, 0), 1);
}

TEST_F(PsglobTest, StarMatchesEmpty) {
    char p1[] = "file*"; char t1[] = "file";
    char p2[] = "*";     char t2[] = "";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

TEST_F(PsglobTest, StarMultipleSegments) {
    char p1[] = "a*b*c"; char t1[] = "axbyc";
    char p2[] = "a*b*c"; char t2[] = "abbc";
    char p3[] = "a*b*c"; char t3[] = "axxbxc";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

// ============================================================================
// PSGlobMatch - Character Set tests
// ============================================================================

TEST_F(PsglobTest, BracketMatchSingle) {
    char p1[] = "file[0-9].txt"; char t1[] = "file1.txt";
    char p2[] = "file[0-9].txt"; char t2[] = "file9.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

TEST_F(PsglobTest, BracketMatchRange) {
    char p1[] = "file[0-5].txt"; char t1[] = "file3.txt";
    char p2[] = "file[0-5].txt"; char t2[] = "file6.txt";
    char p3[] = "file[a-z].txt"; char t3[] = "filem.txt";
    char p4[] = "file[a-z].txt"; char t4[] = "fileM.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p4, t4, 1, 0), 0);
}

TEST_F(PsglobTest, BracketInvertMatch) {
    char p1[] = "file[!0-9].txt"; char t1[] = "fileA.txt";
    char p2[] = "file[!0-9].txt"; char t2[] = "file1.txt";
    char p3[] = "file[^0-9].txt"; char t3[] = "fileA.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

TEST_F(PsglobTest, BracketMultipleChars) {
    char p1[] = "file[0123456789].txt"; char t1[] = "file5.txt";
    char p2[] = "file[abc].txt";        char t2[] = "fileb.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

TEST_F(PsglobTest, BracketNoMatch) {
    char p[] = "file[0-9].txt"; char t[] = "fileA.txt";
    EXPECT_EQ(PSGlobMatch(p, t, 1, 0), 0);
}

// ============================================================================
// PSGlobMatch - Case Sensitivity tests
// ============================================================================

TEST_F(PsglobTest, CaseSensitive) {
    char p1[] = "Hello"; char t1[] = "hello";
    char p2[] = "Hello"; char t2[] = "Hello";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

TEST_F(PsglobTest, CaseInsensitive) {
    char p1[] = "Hello"; char t1[] = "hello";
    char p2[] = "Hello"; char t2[] = "HELLO";
    char p3[] = "HeLLo"; char t3[] = "hEllO";
    EXPECT_EQ(PSGlobMatch(p1, t1, 0, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 0, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 0, 0), 1);
}

// ============================================================================
// PSGlobMatch - Dot Special tests
// ============================================================================

TEST_F(PsglobTest, DotSpecialNoStarDot) {
    char p1[] = "*";          char t1[] = ".hidden";
    char p2[] = "*.txt";      char t2[] = ".txt";
    char p3[] = "*.txt";      char t3[] = "file.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 1), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 1), 0);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 1), 1);
}

TEST_F(PsglobTest, DotSpecialQuestionMark) {
    char p1[] = "?hidden"; char t1[] = ".hidden";
    char p2[] = "?file";   char t2[] = ".file";
    char p3[] = "?file";   char t3[] = "afile";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 1), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 1), 0);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 1), 1);
}

TEST_F(PsglobTest, DotSpecialDisabled) {
    char p1[] = "*";      char t1[] = ".hidden";
    char p2[] = "*.txt";  char t2[] = ".txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

// ============================================================================
// PSGlobMatch - Escape tests
// ============================================================================

TEST_F(PsglobTest, EscapeBackslash) {
    char p1[] = "file\\*.txt"; char t1[] = "file*.txt";
    char p2[] = "file\\?.txt"; char t2[] = "file?.txt";
    char p3[] = "file\\*.txt"; char t3[] = "fileX.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 0);
}

// ============================================================================
// PSGlobMatch - Edge Cases
// ============================================================================

TEST_F(PsglobTest, EmptyPattern) {
    char p1[] = "";           char t1[] = "anything";
    char p2[] = "";           char t2[] = "";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

TEST_F(PsglobTest, StarAtEnd) {
    char p1[] = "file*";  char t1[] = "file";
    char p2[] = "file*";  char t2[] = "file.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

TEST_F(PsglobTest, StarAtStart) {
    char p1[] = "*.txt";      char t1[] = "document.txt";
    char p2[] = "*.txt";      char t2[] = "txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
}

TEST_F(PsglobTest, ComplexPattern) {
    char p1[] = "test*1.txt";  char t1[] = "testfile1.txt";
    char p2[] = "test*.txt";   char t2[] = "testfile.txt";
    char p3[] = "a*b";         char t3[] = "ab";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

// ============================================================================
// Property-based tests
// ============================================================================

class PsglobPropertyTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 54321;
    std::mt19937 rng_;
    
    PsglobPropertyTest() : rng_(SEED) {}
    
    std::string random_string(int len) {
        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-";
        std::uniform_int_distribution<int> dist(0, sizeof(charset) - 2);
        std::string s;
        for (int i = 0; i < len; i++) {
            s += charset[dist(rng_)];
        }
        return s;
    }
};

TEST_F(PsglobPropertyTest, StarMatchesEverything) {
    char p1[] = "*";  char t1[] = "any";
    char p2[] = "*";  char t2[] = "";
    char p3[] = "**"; char t3[] = "anything";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

TEST_F(PsglobPropertyTest, QuestionMarksMatchCount) {
    char p1[] = "???";
    char t1[] = "abc";
    char t2[] = "ab";
    char t3[] = "abcd";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p1, t2, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p1, t3, 1, 0), 0);
}

TEST_F(PsglobPropertyTest, CaseInsensitiveConsistency) {
    std::string s = random_string(5);
    char p[20], t[20];
    strncpy(p, s.c_str(), sizeof(p)-1); p[sizeof(p)-1] = '\0';
    strncpy(t, s.c_str(), sizeof(t)-1); t[sizeof(t)-1] = '\0';
    EXPECT_EQ(PSGlobMatch(p, t, 0, 0), 1);
    EXPECT_EQ(PSGlobMatch(p, t, 1, 0), 1);
}

TEST_F(PsglobPropertyTest, BracketInvertConsistency) {
    char p1[] = "[!0-9]";
    char p2[] = "[0-9]";
    for (int i = 0; i < 10; i++) {
        char s[2] = {(char)('0' + i), '\0'};
        int match_pos = PSGlobMatch(p1, s, 1, 0);
        int match_neg = PSGlobMatch(p2, s, 1, 0);
        EXPECT_EQ(match_pos + match_neg, 1);
    }
}
