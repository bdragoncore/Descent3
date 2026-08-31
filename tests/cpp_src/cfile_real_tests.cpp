/*
 * Descent 3
 * Copyright (C) 2024 Descent Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file cfile_real_tests.cpp
 * @brief Unit tests for Descent3/cfile.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/cfile.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/cfile.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/cfile.cpp`
 * @par Harness
 * `cfile_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/cfile.cpp
 */



/*
 * CFile Real Unit Tests - Using Linker-Wrapped C IO
 *
 * These tests exercise the REAL cfile library (cfile.cpp, cfile.h)
 * but with C stdio functions (fopen, fread, etc.) intercepted
 * via linker wrapping (--wrap flag).
 *
 * This allows testing:
 * - Real cfile logic and error handling
 * - HOG file operations
 * - Case-insensitive path resolution
 * - Error conditions (simulated IO failures)
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <cstring>

// Include the real cfile header
#include "cfile.h"

// Include our IO wrappers
#include "cfile_io_wrappers.hpp"

// Test fixture for real cfile tests
/**
 * @brief GTest fixture for CFileRealTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class CFileRealTest : public ::testing::Test {
protected:
  CFileIOController controller;

  void SetUp() override {
    // Set this controller as the global instance for wrapped functions
    CFileIOController::SetInstance(&controller);

    // Set up base directories
    cf_ClearBaseDirectories();
    cf_AddBaseDirectory("test");

    // Clear search paths
    cf_ClearAllSearchPaths();

    // Add test files to mock filesystem
    SetupTestData();
  }

  void TearDown() override {
    CFileIOController::SetInstance(nullptr);
    controller.ClearFiles();
    controller.ResetFailures();
    controller.ResetCallCounts();
  }

  void SetupTestData() {
    // Create test files
    controller.AddFile("//test/file1.txt", "Hello, World!");
    controller.AddFile("//test/file2.bin", std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04, 0x05});
    controller.AddFile("//test/empty.txt", "");

    // Create a simple HOG-like structure (simplified for testing)
    // In real HOG, we'd have a proper header, but for now we test basic library operations
    std::vector<uint8_t> hog_data = {                        // Simplified HOG header and data
                                     'H', 'O', 'G', '2',     // Magic
                                     0x01, 0x00, 0x00, 0x00, // Version
                                     0x01, 0x00, 0x00, 0x00, // File count
                                                             // File entry
                                     't', 'e', 's', 't', '.', 't', 'x', 't', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Name (64 bytes)
                                     0x05, 0x00, 0x00, 0x00,       // Size
                                     0x20, 0x00, 0x00, 0x00,       // Offset
                                                                   // File data
                                     'H', 'E', 'L', 'L', 'O'};
    controller.AddFile("//test/test.hog", hog_data);
  }
};

// ============================================================================
// Basic File Operations
// ============================================================================

/**
 * @test CFileRealTest.BasicOpenAndClose
 * @brief Verifies basic Open And Close.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, BasicOpenAndClose) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);
  EXPECT_EQ(controller.GetOpenCallCount(), 1);
  EXPECT_EQ(controller.GetLastOpenedPath(), "//test/file1.txt");

  cfclose(cf);
  EXPECT_EQ(controller.GetCloseCallCount(), 1);
}

/**
 * @test CFileRealTest.ReadBytes
 * @brief Verifies read Bytes.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, ReadBytes) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  char buffer[64];
  int bytes_read = cf_ReadBytes(reinterpret_cast<uint8_t *>(buffer), 5, cf);
  EXPECT_EQ(bytes_read, 5);
  EXPECT_EQ(strncmp(buffer, "Hello", 5), 0);

  cfclose(cf);
}

/**
 * @test CFileRealTest.ReadInt
 * @brief Verifies read Int.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, ReadInt) {
  // Create file with known integer data
  std::vector<uint8_t> int_data = {0x78, 0x56, 0x34, 0x12}; // 0x12345678 little-endian
  controller.AddFile("//test/int_test.bin", int_data);

  CFILE *cf = cfopen("//test/int_test.bin", "rb");
  ASSERT_NE(cf, nullptr);

  int32_t val = cf_ReadInt(cf);
  EXPECT_EQ(val, 0x12345678);

  cfclose(cf);
}

/**
 * @test CFileRealTest.ReadShort
 * @brief Verifies read Short.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, ReadShort) {
  std::vector<uint8_t> short_data = {0x34, 0x12}; // 0x1234 little-endian
  controller.AddFile("//test/short_test.bin", short_data);

  CFILE *cf = cfopen("//test/short_test.bin", "rb");
  ASSERT_NE(cf, nullptr);

  int16_t val = cf_ReadShort(cf);
  EXPECT_EQ(val, 0x1234);

  cfclose(cf);
}

/**
 * @test CFileRealTest.ReadByte
 * @brief Verifies read Byte.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, ReadByte) {
  CFILE *cf = cfopen("//test/file2.bin", "rb");
  ASSERT_NE(cf, nullptr);

  int8_t val = cf_ReadByte(cf);
  EXPECT_EQ(static_cast<uint8_t>(val), 0x01);

  cfclose(cf);
}

/**
 * @test CFileRealTest.ReadString
 * @brief Verifies read String.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, ReadString) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  char buffer[64];
  int len = cf_ReadString(buffer, sizeof(buffer), cf);
  EXPECT_EQ(len, 13); // "Hello, World!" without null terminator
  EXPECT_STREQ(buffer, "Hello, World!");

  cfclose(cf);
}

// ============================================================================
// File Position Operations
// ============================================================================

/**
 * @test CFileRealTest.SeekSet
 * @brief Verifies seek Set.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, SeekSet) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  int result = cfseek(cf, 7, SEEK_SET);
  EXPECT_EQ(result, 0);
  EXPECT_EQ(cftell(cf), 7);

  char c = cf_ReadByte(cf);
  EXPECT_EQ(c, 'W'); // Position 7 in "Hello, World!"

  cfclose(cf);
}

/**
 * @test CFileRealTest.SeekCur
 * @brief Verifies seek Cur.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, SeekCur) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  cf_ReadByte(cf); // Position 1
  cf_ReadByte(cf); // Position 2

  int result = cfseek(cf, 3, SEEK_CUR);
  EXPECT_EQ(result, 0);
  EXPECT_EQ(cftell(cf), 5);

  cfclose(cf);
}

/**
 * @test CFileRealTest.SeekEnd
 * @brief Verifies seek End.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, SeekEnd) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  int result = cfseek(cf, -6, SEEK_END);
  EXPECT_EQ(result, 0);

  char buffer[8];
  cf_ReadBytes(reinterpret_cast<uint8_t *>(buffer), 6, cf);
  buffer[6] = '\0';
  EXPECT_STREQ(buffer, "World!");

  cfclose(cf);
}

/**
 * @test CFileRealTest.Rewind
 * @brief Verifies rewind.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, Rewind) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  cf_ReadBytes(nullptr, 10, cf); // Move position
  EXPECT_GT(cftell(cf), 0);

  cf_Rewind(cf);
  EXPECT_EQ(cftell(cf), 0);

  char c = cf_ReadByte(cf);
  EXPECT_EQ(c, 'H'); // Back at start

  cfclose(cf);
}

// ============================================================================
// File Information
// ============================================================================

/**
 * @test CFileRealTest.FileLength
 * @brief Verifies file Length.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, FileLength) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  uint32_t len = cfilelength(cf);
  EXPECT_EQ(len, 13); // "Hello, World!"

  cfclose(cf);
}

/**
 * @test CFileRealTest.EndOfFile
 * @brief Verifies end Of File.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, EndOfFile) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  EXPECT_EQ(cfeof(cf), 0); // Not at EOF

  cfseek(cf, 0, SEEK_END);
  EXPECT_NE(cfeof(cf), 0); // At EOF

  cfclose(cf);
}

/**
 * @test CFileRealTest.FileExists
 * @brief Verifies file Exists.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, FileExists) {
  int result = cfexist("//test/file1.txt");
  EXPECT_NE(result, 0); // File exists

  result = cfexist("//test/nonexistent.txt");
  EXPECT_EQ(result, 0); // File doesn't exist
}

// ============================================================================
// Write Operations
// ============================================================================

/**
 * @test CFileRealTest.WriteAndReadBack
 * @brief Verifies write And Read Back.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, WriteAndReadBack) {
  // Write data
  CFILE *cf = cfopen("//test/write_test.bin", "wb");
  ASSERT_NE(cf, nullptr);

  cf_WriteInt(cf, 0xDEADBEEF);
  cf_WriteShort(cf, 0x1234);
  cf_WriteByte(cf, 0x42);

  cfclose(cf);
  EXPECT_EQ(controller.GetWriteCallCount(), 3);

  // Read it back
  cf = cfopen("//test/write_test.bin", "rb");
  ASSERT_NE(cf, nullptr);

  EXPECT_EQ(cf_ReadInt(cf), 0xDEADBEEF);
  EXPECT_EQ(cf_ReadShort(cf), 0x1234);
  EXPECT_EQ(cf_ReadByte(cf), 0x42);

  cfclose(cf);
}

/**
 * @test CFileRealTest.WriteString
 * @brief Verifies write String.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, WriteString) {
  CFILE *cf = cfopen("//test/string_test.txt", "wt");
  ASSERT_NE(cf, nullptr);

  cf_WriteString(cf, "Test Line");
  cfclose(cf);

  cf = cfopen("//test/string_test.txt", "rt");
  ASSERT_NE(cf, nullptr);

  char buffer[64];
  cf_ReadString(buffer, sizeof(buffer), cf);
  EXPECT_STREQ(buffer, "Test Line");

  cfclose(cf);
}

// ============================================================================
// Error Handling
// ============================================================================

/**
 * @test CFileRealTest.OpenNonExistentFile
 * @brief Verifies open Non Existent File.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, OpenNonExistentFile) {
  CFILE *cf = cfopen("//test/does_not_exist.txt", "rb");
  EXPECT_EQ(cf, nullptr);
}

/**
 * @test CFileRealTest.SimulatedOpenFailure
 * @brief Verifies simulated Open Failure.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, SimulatedOpenFailure) {
  controller.SetOpenShouldFail(true, ENOENT);

  CFILE *cf = cfopen("//test/file1.txt", "rb");
  EXPECT_EQ(cf, nullptr);
}

/**
 * @test CFileRealTest.SimulatedReadFailure
 * @brief Verifies simulated Read Failure.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, SimulatedReadFailure) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  controller.SetReadShouldFail(true, EIO);

  char buffer[64];
  int bytes_read = cf_ReadBytes(reinterpret_cast<uint8_t *>(buffer), 10, cf);
  EXPECT_EQ(bytes_read, 0); // Should fail

  cfclose(cf);
}

// ============================================================================
// Path Resolution
// ============================================================================

/**
 * @test CFileRealTest.LocatePathSuccess
 * @brief Verifies locate Path Success.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, LocatePathSuccess) {
  std::filesystem::path result = cf_LocatePath("file1.txt");
  EXPECT_FALSE(result.empty());
}

/**
 * @test CFileRealTest.LocatePathNotFound
 * @brief Verifies locate Path Not Found.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, LocatePathNotFound) {
  std::filesystem::path result = cf_LocatePath("nonexistent.txt");
  EXPECT_TRUE(result.empty());
}

/**
 * @test CFileRealTest.MultiplePaths
 * @brief Verifies multiple Paths.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, MultiplePaths) {
  cf_AddBaseDirectory("/other");
  controller.AddFile("/other/file3.txt", "Third file");

  auto results = cf_LocateMultiplePaths("file3.txt");
  EXPECT_FALSE(results.empty());
}

// ============================================================================
// Library Operations (HOG)
// ============================================================================

/**
 * @test CFileRealTest.OpenLibrary
 * @brief Verifies open Library.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, OpenLibrary) {
  int lib_handle = cf_OpenLibrary("//test/test.hog");
  EXPECT_NE(lib_handle, 0);

  if (lib_handle != 0) {
    cf_CloseLibrary(lib_handle);
  }
}

/**
 * @test CFileRealTest.LibraryFileNotFound
 * @brief Verifies library File Not Found.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, LibraryFileNotFound) {
  int lib_handle = cf_OpenLibrary("//test/nonexistent.hog");
  EXPECT_EQ(lib_handle, 0);
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @test CFileRealTest.CopyFile
 * @brief Verifies copy File.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, CopyFile) {
  bool result = cf_CopyFile("//test/copied.txt", "//test/file1.txt", 0);
  EXPECT_TRUE(result);

  // Verify copy exists
  int exists = cfexist("//test/copied.txt");
  EXPECT_NE(exists, 0);
}

/**
 * @test CFileRealTest.DiffFiles
 * @brief Verifies diff Files.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, DiffFiles) {
  controller.AddFile("//test/file_a.txt", "Content A");
  controller.AddFile("//test/file_b.txt", "Content B");

  bool result = cf_Diff("//test/file_a.txt", "//test/file_b.txt");
  EXPECT_TRUE(result); // Files are different

  // Same file should not be different
  result = cf_Diff("//test/file_a.txt", "//test/file_a.txt");
  EXPECT_FALSE(result);
}

// ============================================================================
// Float and Double Tests
// ============================================================================

/**
 * @test CFileRealTest.ReadWriteFloat
 * @brief Verifies read Write Float.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, ReadWriteFloat) {
  CFILE *cf = cfopen("//test/float_test.bin", "wb");
  ASSERT_NE(cf, nullptr);

  cf_WriteFloat(cf, 3.14159f);
  cfclose(cf);

  cf = cfopen("//test/float_test.bin", "rb");
  ASSERT_NE(cf, nullptr);

  float val = cf_ReadFloat(cf);
  EXPECT_NEAR(val, 3.14159f, 0.0001f);

  cfclose(cf);
}

/**
 * @test CFileRealTest.ReadWriteDouble
 * @brief Verifies read Write Double.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, ReadWriteDouble) {
  CFILE *cf = cfopen("//test/double_test.bin", "wb");
  ASSERT_NE(cf, nullptr);

  cf_WriteDouble(cf, 2.718281828);
  cfclose(cf);

  cf = cfopen("//test/double_test.bin", "rb");
  ASSERT_NE(cf, nullptr);

  double val = cf_ReadDouble(cf);
  EXPECT_NEAR(val, 2.718281828, 0.0001);

  cfclose(cf);
}

// ============================================================================
// Edge Cases
// ============================================================================

/**
 * @test CFileRealTest.EmptyFile
 * @brief Verifies empty File.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, EmptyFile) {
  CFILE *cf = cfopen("//test/empty.txt", "rb");
  ASSERT_NE(cf, nullptr);

  EXPECT_EQ(cfilelength(cf), 0);
  EXPECT_NE(cfeof(cf), 0);

  cfclose(cf);
}

/**
 * @test CFileRealTest.ReadBeyondEOF
 * @brief Verifies read Beyond EOF.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, ReadBeyondEOF) {
  CFILE *cf = cfopen("//test/file2.bin", "rb");
  ASSERT_NE(cf, nullptr);

  cfseek(cf, 0, SEEK_END);

  uint8_t buffer[10];
  int bytes_read = cf_ReadBytes(buffer, 10, cf);
  EXPECT_EQ(bytes_read, 0); // Nothing to read at EOF

  cfclose(cf);
}

/**
 * @test CFileRealTest.MultipleOpens
 * @brief Verifies multiple Opens.
 *
 * @details
 * Exercises the CFileRealTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileRealTest, MultipleOpens) {
  CFILE *cf1 = cfopen("//test/file1.txt", "rb");
  CFILE *cf2 = cfopen("//test/file2.bin", "rb");

  ASSERT_NE(cf1, nullptr);
  ASSERT_NE(cf2, nullptr);

  char c1 = cf_ReadByte(cf1);
  char c2 = cf_ReadByte(cf2);

  EXPECT_EQ(c1, 'H');
  EXPECT_EQ(static_cast<uint8_t>(c2), 0x01);

  cfclose(cf1);
  cfclose(cf2);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
