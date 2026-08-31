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

TEST_F(CFileRealTest, BasicOpenAndClose) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);
  EXPECT_EQ(controller.GetOpenCallCount(), 1);
  EXPECT_EQ(controller.GetLastOpenedPath(), "//test/file1.txt");

  cfclose(cf);
  EXPECT_EQ(controller.GetCloseCallCount(), 1);
}

TEST_F(CFileRealTest, ReadBytes) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  char buffer[64];
  int bytes_read = cf_ReadBytes(reinterpret_cast<uint8_t *>(buffer), 5, cf);
  EXPECT_EQ(bytes_read, 5);
  EXPECT_EQ(strncmp(buffer, "Hello", 5), 0);

  cfclose(cf);
}

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

TEST_F(CFileRealTest, ReadShort) {
  std::vector<uint8_t> short_data = {0x34, 0x12}; // 0x1234 little-endian
  controller.AddFile("//test/short_test.bin", short_data);

  CFILE *cf = cfopen("//test/short_test.bin", "rb");
  ASSERT_NE(cf, nullptr);

  int16_t val = cf_ReadShort(cf);
  EXPECT_EQ(val, 0x1234);

  cfclose(cf);
}

TEST_F(CFileRealTest, ReadByte) {
  CFILE *cf = cfopen("//test/file2.bin", "rb");
  ASSERT_NE(cf, nullptr);

  int8_t val = cf_ReadByte(cf);
  EXPECT_EQ(static_cast<uint8_t>(val), 0x01);

  cfclose(cf);
}

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

TEST_F(CFileRealTest, FileLength) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  uint32_t len = cfilelength(cf);
  EXPECT_EQ(len, 13); // "Hello, World!"

  cfclose(cf);
}

TEST_F(CFileRealTest, EndOfFile) {
  CFILE *cf = cfopen("//test/file1.txt", "rb");
  ASSERT_NE(cf, nullptr);

  EXPECT_EQ(cfeof(cf), 0); // Not at EOF

  cfseek(cf, 0, SEEK_END);
  EXPECT_NE(cfeof(cf), 0); // At EOF

  cfclose(cf);
}

TEST_F(CFileRealTest, FileExists) {
  int result = cfexist("//test/file1.txt");
  EXPECT_NE(result, 0); // File exists

  result = cfexist("//test/nonexistent.txt");
  EXPECT_EQ(result, 0); // File doesn't exist
}

// ============================================================================
// Write Operations
// ============================================================================

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

TEST_F(CFileRealTest, OpenNonExistentFile) {
  CFILE *cf = cfopen("//test/does_not_exist.txt", "rb");
  EXPECT_EQ(cf, nullptr);
}

TEST_F(CFileRealTest, SimulatedOpenFailure) {
  controller.SetOpenShouldFail(true, ENOENT);

  CFILE *cf = cfopen("//test/file1.txt", "rb");
  EXPECT_EQ(cf, nullptr);
}

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

TEST_F(CFileRealTest, LocatePathSuccess) {
  std::filesystem::path result = cf_LocatePath("file1.txt");
  EXPECT_FALSE(result.empty());
}

TEST_F(CFileRealTest, LocatePathNotFound) {
  std::filesystem::path result = cf_LocatePath("nonexistent.txt");
  EXPECT_TRUE(result.empty());
}

TEST_F(CFileRealTest, MultiplePaths) {
  cf_AddBaseDirectory("/other");
  controller.AddFile("/other/file3.txt", "Third file");

  auto results = cf_LocateMultiplePaths("file3.txt");
  EXPECT_FALSE(results.empty());
}

// ============================================================================
// Library Operations (HOG)
// ============================================================================

TEST_F(CFileRealTest, OpenLibrary) {
  int lib_handle = cf_OpenLibrary("//test/test.hog");
  EXPECT_NE(lib_handle, 0);

  if (lib_handle != 0) {
    cf_CloseLibrary(lib_handle);
  }
}

TEST_F(CFileRealTest, LibraryFileNotFound) {
  int lib_handle = cf_OpenLibrary("//test/nonexistent.hog");
  EXPECT_EQ(lib_handle, 0);
}

// ============================================================================
// Utility Functions
// ============================================================================

TEST_F(CFileRealTest, CopyFile) {
  bool result = cf_CopyFile("//test/copied.txt", "//test/file1.txt", 0);
  EXPECT_TRUE(result);

  // Verify copy exists
  int exists = cfexist("//test/copied.txt");
  EXPECT_NE(exists, 0);
}

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

TEST_F(CFileRealTest, EmptyFile) {
  CFILE *cf = cfopen("//test/empty.txt", "rb");
  ASSERT_NE(cf, nullptr);

  EXPECT_EQ(cfilelength(cf), 0);
  EXPECT_NE(cfeof(cf), 0);

  cfclose(cf);
}

TEST_F(CFileRealTest, ReadBeyondEOF) {
  CFILE *cf = cfopen("//test/file2.bin", "rb");
  ASSERT_NE(cf, nullptr);

  cfseek(cf, 0, SEEK_END);

  uint8_t buffer[10];
  int bytes_read = cf_ReadBytes(buffer, 10, cf);
  EXPECT_EQ(bytes_read, 0); // Nothing to read at EOF

  cfclose(cf);
}

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
