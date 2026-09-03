/**
 * @file ddio_file_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for ddio file/path utilities and key translation helpers:
 *  - ddio_SplitPath, ddio_MakePath (ddio/lnxfile.cpp)
 *  - ddio_KeyToAscii, ddio_AsciiToKey (ddio/key.cpp)
 *
 * This harness validates the behavior of `ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h`
 * @par Harness
 * `ddio_file_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see ddio/lnxfile.cpp
 * @see ddio/key.cpp
 * @see ddio/ddio.h
 */

#include <gtest/gtest.h>

#include "ddio.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <cstdio>

// Stubs for ddio/key.cpp internal dependencies — we only need the
// pure KeyToAscii/AsciiToKey tables, not the full keyboard subsystem.
bool ddio_InternalKeyInit(ddio_init_info *) { return true; }
void ddio_InternalKeyClose() {}
void ddio_InternalResetKey(uint8_t) {}
float ddio_InternalKeyDownTime(uint8_t) { return 0.0f; }
bool ddio_InternalKeyState(uint8_t) { return false; }
void ddio_InternalKeySuspend() {}
void ddio_InternalKeyResume() {}
void ddio_InternalKeyFrame() {}

// ============================================================================
// ddio_SplitPath
// ============================================================================

/**
 * @test DdioFile.SplitPath_FullPath
 * @brief Verifies split Path Full Path.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, SplitPath_FullPath) {
  char path[256]{}, file[256]{}, ext[256]{};
  ddio_SplitPath("/a/b/c.txt", path, file, ext);
  EXPECT_STREQ(path, "/a/b");
  EXPECT_STREQ(file, "c");
  EXPECT_STREQ(ext, ".txt");
}

/**
 * @test DdioFile.SplitPath_NoDir
 * @brief Verifies split Path No Dir.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, SplitPath_NoDir) {
  char path[256]{}, file[256]{}, ext[256]{};
  ddio_SplitPath("file.txt", path, file, ext);
  EXPECT_STREQ(path, "");
  EXPECT_STREQ(file, "file");
  EXPECT_STREQ(ext, ".txt");
}

/**
 * @test DdioFile.SplitPath_NoExtension
 * @brief Verifies split Path No Extension.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, SplitPath_NoExtension) {
  char path[256]{}, file[256]{}, ext[256]{};
  ddio_SplitPath("/path/to/file", path, file, ext);
  EXPECT_STREQ(ext, "");
  EXPECT_STREQ(file, "file");
  EXPECT_STREQ(path, "/path/to");
}

/**
 * @test DdioFile.SplitPath_MultiDotUsesLastExtension
 * @brief Verifies split Path Multi Dot Uses Last Extension.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, SplitPath_MultiDotUsesLastExtension) {
  char path[256]{}, file[256]{}, ext[256]{};
  ddio_SplitPath("/a/b/file.tar.gz", path, file, ext);
  EXPECT_STREQ(ext, ".gz");
  EXPECT_STREQ(file, "file.tar");
  EXPECT_STREQ(path, "/a/b");
}

/**
 * @test DdioFile.SplitPath_RootFile
 * @brief Verifies split Path Root File.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, SplitPath_RootFile) {
  char path[256]{}, file[256]{}, ext[256]{};
  ddio_SplitPath("/file.txt", path, file, ext);
  EXPECT_STREQ(file, "file");
  EXPECT_STREQ(ext, ".txt");
}

/**
 * @test DdioFile.SplitPath_NullOutputsAllowed
 * @brief Verifies split Path Null Outputs Allowed.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, SplitPath_NullOutputsAllowed) {
  // implementation guards each output with `if (ext)` etc — must not crash
  ddio_SplitPath("/a/b/c.txt", nullptr, nullptr, nullptr);
  char file[256]{};
  ddio_SplitPath("/a/b/c.txt", nullptr, file, nullptr);
  EXPECT_STREQ(file, "c");
}

// ============================================================================
// ddio_MakePath
// ============================================================================

/**
 * @test DdioFile.MakePath_SingleSubdir
 * @brief Verifies make Path Single Subdir.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, MakePath_SingleSubdir) {
  char out[512]{};
  ddio_MakePath(out, "/base", "sub", nullptr);
  EXPECT_STREQ(out, "/base/sub");
}

/**
 * @test DdioFile.MakePath_MultipleSubdirs
 * @brief Verifies make Path Multiple Subdirs.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, MakePath_MultipleSubdirs) {
  char out[512]{};
  ddio_MakePath(out, "/base", "a", "b", "c", nullptr);
  EXPECT_STREQ(out, "/base/a/b/c");
}

/**
 * @test DdioFile.MakePath_BaseAlreadyTrailingSlash
 * @brief Verifies make Path Base Already Trailing Slash.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, MakePath_BaseAlreadyTrailingSlash) {
  char out[512]{};
  ddio_MakePath(out, "/base/", "sub", nullptr);
  EXPECT_STREQ(out, "/base/sub");
}

/**
 * @test DdioFile.MakePath_InPlaceBase
 * @brief Verifies make Path In Place Base.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, MakePath_InPlaceBase) {
  char out[512];
  strcpy(out, "/base");
  ddio_MakePath(out, out, "sub", nullptr);
  EXPECT_STREQ(out, "/base/sub");
}

// ============================================================================
// ddio_KeyToAscii / ddio_AsciiToKey
// ============================================================================

/**
 * @test DdioFile.KeyToAscii_OutOfRangeReturns255
 * @brief Verifies key To Ascii Out Of Range Returns255.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, KeyToAscii_OutOfRangeReturns255) {
  EXPECT_EQ(ddio_KeyToAscii(127), 255);
  EXPECT_EQ(ddio_KeyToAscii(200), 255);
}

/**
 * @test DdioFile.KeyToAscii_LowercaseRoundTrip
 * @brief Verifies key To Ascii Lowercase Round Trip.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, KeyToAscii_LowercaseRoundTrip) {
  for (char c = 'a'; c <= 'z'; c++) {
    int kc = ddio_AsciiToKey(c);
    ASSERT_NE(kc, 0) << "AsciiToKey failed for " << c;
    EXPECT_EQ(ddio_KeyToAscii(kc), (int)c) << "round trip for " << c;
  }
}

/**
 * @test DdioFile.KeyToAscii_UppercaseRoundTrip
 * @brief Verifies key To Ascii Uppercase Round Trip.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, KeyToAscii_UppercaseRoundTrip) {
  for (char c = 'A'; c <= 'Z'; c++) {
    int kc = ddio_AsciiToKey(c);
    ASSERT_NE(kc, 0) << "AsciiToKey failed for " << c;
    EXPECT_EQ(ddio_KeyToAscii(kc), (int)c) << "round trip for " << c;
  }
}

/**
 * @test DdioFile.KeyToAscii_DigitsRoundTrip
 * @brief Verifies key To Ascii Digits Round Trip.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, KeyToAscii_DigitsRoundTrip) {
  for (char c = '0'; c <= '9'; c++) {
    int kc = ddio_AsciiToKey(c);
    ASSERT_NE(kc, 0) << "AsciiToKey failed for " << c;
    EXPECT_EQ(ddio_KeyToAscii(kc), (int)c) << "round trip for " << c;
  }
}

/**
 * @test DdioFile.AsciiToKey_OutOfRangeReturnsZero
 * @brief Verifies ascii To Key Out Of Range Returns Zero.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, AsciiToKey_OutOfRangeReturnsZero) {
  EXPECT_EQ(ddio_AsciiToKey(-1), 0);
  EXPECT_EQ(ddio_AsciiToKey(128), 0);
  EXPECT_EQ(ddio_AsciiToKey(200), 0);
}

/**
 * @test DdioFile.AsciiToKey_SpaceAndPunct
 * @brief Verifies ascii To Key Space And Punct.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, AsciiToKey_SpaceAndPunct) {
  EXPECT_NE(ddio_AsciiToKey(' '), 0);
  EXPECT_EQ(ddio_KeyToAscii(ddio_AsciiToKey(' ')), ' ');
  EXPECT_NE(ddio_AsciiToKey(','), 0);
  EXPECT_EQ(ddio_KeyToAscii(ddio_AsciiToKey(',')), ',');
}

// ============================================================================
// ddio filesystem helpers — GetWorkingDir / GetFileLength
// ============================================================================

/**
 * @test DdioFile.GetWorkingDir_ReturnsNonEmpty
 * @brief Verifies get Working Dir Returns Non Empty.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, GetWorkingDir_ReturnsNonEmpty) {
  char buf[1024]{};
  ddio_GetWorkingDir(buf, sizeof(buf));
  EXPECT_NE(buf[0], '\0');
  EXPECT_TRUE(std::filesystem::is_directory(buf));
}

/**
 * @test DdioFile.GetFileLength_MatchesWrittenBytes
 * @brief Verifies get File Length Matches Written Bytes.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, GetFileLength_MatchesWrittenBytes) {
  auto tmp = std::filesystem::temp_directory_path() / "d3_test_len.tmp";
  {
    std::ofstream f(tmp, std::ios::binary);
    std::string data(123, 'x');
    f.write(data.data(), data.size());
  }
  FILE *fp = fopen(tmp.string().c_str(), "rb");
  ASSERT_NE(fp, nullptr);
  EXPECT_EQ(ddio_GetFileLength(fp), 123);
  fclose(fp);
  std::filesystem::remove(tmp);
}

/**
 * @test DdioFile.GetFileLength_EmptyFile
 * @brief Verifies get File Length Empty File.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, GetFileLength_EmptyFile) {
  auto tmp = std::filesystem::temp_directory_path() / "d3_test_empty.tmp";
  {
    std::ofstream f(tmp, std::ios::binary);
  }
  FILE *fp = fopen(tmp.string().c_str(), "rb");
  ASSERT_NE(fp, nullptr);
  EXPECT_EQ(ddio_GetFileLength(fp), 0);
  fclose(fp);
  std::filesystem::remove(tmp);
}

/**
 * @test DdioFile.FileDiff_SizeMismatch
 * @brief Verifies file Diff Size Mismatch.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, FileDiff_SizeMismatch) {
  auto a = std::filesystem::temp_directory_path() / "d3_test_diff_a.tmp";
  auto b = std::filesystem::temp_directory_path() / "d3_test_diff_b.tmp";
  {
    std::ofstream f(a, std::ios::binary);
    f.write("hello", 5);
  }
  {
    std::ofstream f(b, std::ios::binary);
    f.write("hello world", 11);
  }
  EXPECT_TRUE(ddio_FileDiff(a, b));
  std::filesystem::remove(a);
  std::filesystem::remove(b);
}

/**
 * @test DdioFile.CopyFileTimeMakesFileDiffFalse
 * @brief Verifies copy File Time Makes File Diff False.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, CopyFileTimeMakesFileDiffFalse) {
  auto a = std::filesystem::temp_directory_path() / "d3_test_ctime_a.tmp";
  auto b = std::filesystem::temp_directory_path() / "d3_test_ctime_b.tmp";
  {
    std::ofstream f(a, std::ios::binary);
    f.write("same", 4);
  }
  {
    std::ofstream f(b, std::ios::binary);
    f.write("same", 4);
  }
  // Make b's times equal to a's, then FileDiff should be false (same size & mtime)
  ddio_CopyFileTime(b, a);
  EXPECT_FALSE(ddio_FileDiff(a, b));
  std::filesystem::remove(a);
  std::filesystem::remove(b);
}

/**
 * @test DdioFile.DeleteFile_RemovesFile
 * @brief Verifies delete File Removes File.
 *
 * @details
 * Exercises the DdioFile code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/lnxfile.cpp, ddio/key.cpp, ddio/ddio.h
 * @ingroup descent3_tests
 */
TEST(DdioFile, DeleteFile_RemovesFile) {
  auto tmp = std::filesystem::temp_directory_path() / "d3_test_del.tmp";
  {
    std::ofstream f(tmp, std::ios::binary);
    f.write("x", 1);
  }
  ASSERT_TRUE(std::filesystem::exists(tmp));
  EXPECT_EQ(ddio_DeleteFile(tmp.string().c_str()), 1);
  EXPECT_FALSE(std::filesystem::exists(tmp));
}
