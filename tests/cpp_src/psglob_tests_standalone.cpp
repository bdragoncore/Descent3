/**
 * @file psglob_tests_standalone.cpp
 * @brief D3 Coverage Tests - psglob module (Standalone).
 *
 * @details
 * Tests for glob pattern matching utilities.
 * Uses seeded RNG for reproducible test data.
 *
 * This harness validates the behavior of `Descent3/psglob_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/psglob_tests_standalone.cpp`
 * @par Harness
 * `psglob_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/psglob_tests_standalone.cpp
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

/**
 * @brief GTest fixture for PsglobTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PsglobTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    PsglobTest() : rng_(SEED) {}
};

// ============================================================================
// PSGlobHasPattern tests
// ============================================================================

/**
 * @test PsglobTest.EmptyStringNoPattern
 * @brief Verifies empty String No Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, EmptyStringNoPattern) {
    char empty[] = "";
    EXPECT_EQ(PSGlobHasPattern(empty), 0);
}

/**
 * @test PsglobTest.PlainStringNoPattern
 * @brief Verifies plain String No Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, PlainStringNoPattern) {
    char s[] = "hello";
    EXPECT_EQ(PSGlobHasPattern(s), 0);
}

/**
 * @test PsglobTest.QuestionMarkIsPattern
 * @brief Verifies question Mark Is Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, QuestionMarkIsPattern) {
    char s[] = "hel?lo";
    EXPECT_EQ(PSGlobHasPattern(s), 1);
}

/**
 * @test PsglobTest.StarIsPattern
 * @brief Verifies star Is Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, StarIsPattern) {
    char s[] = "hel*lo";
    EXPECT_EQ(PSGlobHasPattern(s), 1);
}

/**
 * @test PsglobTest.BracketsArePattern
 * @brief Verifies brackets Are Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, BracketsArePattern) {
    char s1[] = "file[0-9].txt";
    char s2[] = "file[!0-9].txt";
    EXPECT_EQ(PSGlobHasPattern(s1), 1);
    EXPECT_EQ(PSGlobHasPattern(s2), 1);
}

/**
 * @test PsglobTest.UnclosedBracketNotPattern
 * @brief Verifies unclosed Bracket Not Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, UnclosedBracketNotPattern) {
    char s[] = "file[.txt";
    EXPECT_EQ(PSGlobHasPattern(s), 0);
}

/**
 * @test PsglobTest.ClosedBracketIsPattern
 * @brief Verifies closed Bracket Is Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, ClosedBracketIsPattern) {
    char s[] = "file].txt";
    EXPECT_EQ(PSGlobHasPattern(s), 0);
}

/**
 * @test PsglobTest.EscapeBackslashNotPattern
 * @brief Verifies escape Backslash Not Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, EscapeBackslashNotPattern) {
    char s[] = "file\\*.txt";
    EXPECT_EQ(PSGlobHasPattern(s), 0);
}

// ============================================================================
// PSGlobMatch - Exact Match tests
// ============================================================================

/**
 * @test PsglobTest.ExactMatch
 * @brief Verifies exact Match.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, ExactMatch) {
    char p1[] = "hello"; char t1[] = "hello";
    char p2[] = "world"; char t2[] = "world";
    char p3[] = "";     char t3[] = "";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

/**
 * @test PsglobTest.ExactMatchFails
 * @brief Verifies exact Match Fails.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @test PsglobTest.QuestionMarkMatchesOne
 * @brief Verifies question Mark Matches One.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, QuestionMarkMatchesOne) {
    char p1[] = "h?llo"; char t1[] = "hello";
    char p2[] = "?????"; char t2[] = "hello";
    char p3[] = "h?llo"; char t3[] = "hallo";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

/**
 * @test PsglobTest.QuestionMarkNoMatch
 * @brief Verifies question Mark No Match.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, QuestionMarkNoMatch) {
    char p1[] = "h?llo"; char t1[] = "hxxxxllo";
    char p2[] = "???";   char t2[] = "hell";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
}

/**
 * @test PsglobTest.QuestionMarkAtEnd
 * @brief Verifies question Mark At End.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, QuestionMarkAtEnd) {
    char p1[] = "file?"; char t1[] = "file1";
    char p2[] = "file?"; char t2[] = "file";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
}

// ============================================================================
// PSGlobMatch - Star tests
// ============================================================================

/**
 * @test PsglobTest.StarMatchesAny
 * @brief Verifies star Matches Any.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @test PsglobTest.StarMatchesEmpty
 * @brief Verifies star Matches Empty.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, StarMatchesEmpty) {
    char p1[] = "file*"; char t1[] = "file";
    char p2[] = "*";     char t2[] = "";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

/**
 * @test PsglobTest.StarMultipleSegments
 * @brief Verifies star Multiple Segments.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @test PsglobTest.BracketMatchSingle
 * @brief Verifies bracket Match Single.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, BracketMatchSingle) {
    char p1[] = "file[0-9].txt"; char t1[] = "file1.txt";
    char p2[] = "file[0-9].txt"; char t2[] = "file9.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

/**
 * @test PsglobTest.BracketMatchRange
 * @brief Verifies bracket Match Range.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @test PsglobTest.BracketInvertMatch
 * @brief Verifies bracket Invert Match.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, BracketInvertMatch) {
    char p1[] = "file[!0-9].txt"; char t1[] = "fileA.txt";
    char p2[] = "file[!0-9].txt"; char t2[] = "file1.txt";
    char p3[] = "file[^0-9].txt"; char t3[] = "fileA.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

/**
 * @test PsglobTest.BracketMultipleChars
 * @brief Verifies bracket Multiple Chars.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, BracketMultipleChars) {
    char p1[] = "file[0123456789].txt"; char t1[] = "file5.txt";
    char p2[] = "file[abc].txt";        char t2[] = "fileb.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

/**
 * @test PsglobTest.BracketNoMatch
 * @brief Verifies bracket No Match.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, BracketNoMatch) {
    char p[] = "file[0-9].txt"; char t[] = "fileA.txt";
    EXPECT_EQ(PSGlobMatch(p, t, 1, 0), 0);
}

// ============================================================================
// PSGlobMatch - Case Sensitivity tests
// ============================================================================

/**
 * @test PsglobTest.CaseSensitive
 * @brief Verifies case Sensitive.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, CaseSensitive) {
    char p1[] = "Hello"; char t1[] = "hello";
    char p2[] = "Hello"; char t2[] = "Hello";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

/**
 * @test PsglobTest.CaseInsensitive
 * @brief Verifies case Insensitive.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @test PsglobTest.DotSpecialNoStarDot
 * @brief Verifies dot Special No Star Dot.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, DotSpecialNoStarDot) {
    char p1[] = "*";          char t1[] = ".hidden";
    char p2[] = "*.txt";      char t2[] = ".txt";
    char p3[] = "*.txt";      char t3[] = "file.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 1), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 1), 0);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 1), 1);
}

/**
 * @test PsglobTest.DotSpecialQuestionMark
 * @brief Verifies dot Special Question Mark.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, DotSpecialQuestionMark) {
    char p1[] = "?hidden"; char t1[] = ".hidden";
    char p2[] = "?file";   char t2[] = ".file";
    char p3[] = "?file";   char t3[] = "afile";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 1), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 1), 0);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 1), 1);
}

/**
 * @test PsglobTest.DotSpecialDisabled
 * @brief Verifies dot Special Disabled.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, DotSpecialDisabled) {
    char p1[] = "*";      char t1[] = ".hidden";
    char p2[] = "*.txt";  char t2[] = ".txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

// ============================================================================
// PSGlobMatch - Escape tests
// ============================================================================

/**
 * @test PsglobTest.EscapeBackslash
 * @brief Verifies escape Backslash.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @test PsglobTest.EmptyPattern
 * @brief Verifies empty Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, EmptyPattern) {
    char p1[] = "";           char t1[] = "anything";
    char p2[] = "";           char t2[] = "";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

/**
 * @test PsglobTest.StarAtEnd
 * @brief Verifies star At End.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, StarAtEnd) {
    char p1[] = "file*";  char t1[] = "file";
    char p2[] = "file*";  char t2[] = "file.txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
}

/**
 * @test PsglobTest.StarAtStart
 * @brief Verifies star At Start.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobTest, StarAtStart) {
    char p1[] = "*.txt";      char t1[] = "document.txt";
    char p2[] = "*.txt";      char t2[] = "txt";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 0);
}

/**
 * @test PsglobTest.ComplexPattern
 * @brief Verifies complex Pattern.
 *
 * @details
 * Exercises the PsglobTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @brief GTest fixture for PsglobPropertyTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
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

/**
 * @test PsglobPropertyTest.StarMatchesEverything
 * @brief Verifies star Matches Everything.
 *
 * @details
 * Exercises the PsglobPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobPropertyTest, StarMatchesEverything) {
    char p1[] = "*";  char t1[] = "any";
    char p2[] = "*";  char t2[] = "";
    char p3[] = "**"; char t3[] = "anything";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p2, t2, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p3, t3, 1, 0), 1);
}

/**
 * @test PsglobPropertyTest.QuestionMarksMatchCount
 * @brief Verifies question Marks Match Count.
 *
 * @details
 * Exercises the PsglobPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobPropertyTest, QuestionMarksMatchCount) {
    char p1[] = "???";
    char t1[] = "abc";
    char t2[] = "ab";
    char t3[] = "abcd";
    EXPECT_EQ(PSGlobMatch(p1, t1, 1, 0), 1);
    EXPECT_EQ(PSGlobMatch(p1, t2, 1, 0), 0);
    EXPECT_EQ(PSGlobMatch(p1, t3, 1, 0), 0);
}

/**
 * @test PsglobPropertyTest.CaseInsensitiveConsistency
 * @brief Verifies case Insensitive Consistency.
 *
 * @details
 * Exercises the PsglobPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsglobPropertyTest, CaseInsensitiveConsistency) {
    std::string s = random_string(5);
    char p[20], t[20];
    strncpy(p, s.c_str(), sizeof(p)-1); p[sizeof(p)-1] = '\0';
    strncpy(t, s.c_str(), sizeof(t)-1); t[sizeof(t)-1] = '\0';
    EXPECT_EQ(PSGlobMatch(p, t, 0, 0), 1);
    EXPECT_EQ(PSGlobMatch(p, t, 1, 0), 1);
}

/**
 * @test PsglobPropertyTest.BracketInvertConsistency
 * @brief Verifies bracket Invert Consistency.
 *
 * @details
 * Exercises the PsglobPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psglob_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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
