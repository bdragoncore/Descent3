/*
 * Descent 3
 * Copyright (C) 2024 Descent Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
 * CFile Integration Tests - Mock-Based
 *
 * These tests replicate the original cfile/tests/cfile_tests.cpp functionality
 * but use the enhanced mock infrastructure instead of real filesystem access.
 *
 * Tests cover:
 * - Basic file I/O operations (read/write byte, short, int, string)
 * - HOG library operations (open library, read files from library)
 * - Case-insensitive path resolution
 * - Search paths and base directories
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <cstring>
#include "mock_cfile_full.hpp"

// Test fixture for CFile integration tests
class CFileIntegrationTest : public ::testing::Test {
protected:
  InMemoryFileSystem fs;

  void SetUp() override {
    GetInMemoryFileSystem() = &fs;
    SetupTestData();
  }

  void TearDown() override {
    GetInMemoryFileSystem() = nullptr;
    fs.Clear();
  }

  void SetupTestData() {
    // Create test files simulating the original TestDir contents
    // lowercase.txt contains "TEST" (4 bytes)
    fs.AddFile("TestDir/lowercase.txt", std::vector<uint8_t>{'T', 'E', 'S', 'T'});

    // UPPERCASE.TXT (empty for this test)
    fs.AddFile("TestDir/UPPERCASE.TXT", std::vector<uint8_t>{});

    // CamelCase.txt (empty for this test)
    fs.AddFile("TestDir/CamelCase.txt", std::vector<uint8_t>{});

    // Create a mock HOG library with lowercase.txt containing "TEST"
    fs.AddFileToLibrary("TestDir/test.hog", "lowercase.txt", std::vector<uint8_t>{'T', 'E', 'S', 'T'});
  }

  // Helper: Create little-endian bytes from uint32_t
  std::vector<uint8_t> le_bytes(uint32_t val) {
    return {static_cast<uint8_t>(val & 0xFF), static_cast<uint8_t>((val >> 8) & 0xFF),
            static_cast<uint8_t>((val >> 16) & 0xFF), static_cast<uint8_t>((val >> 24) & 0xFF)};
  }
};

// ============================================================================
// CFileIO Tests - Basic file I/O operations
// ============================================================================

TEST_F(CFileIntegrationTest, CFileIO_ReadString) {
  // Open the file and read as string (simulates original test)
  CFILE *file_handle = cfopen("TestDir/lowercase.txt", "rb");
  ASSERT_NE(file_handle, nullptr);

  char buf[5];
  int len = cf_ReadString(buf, 5, file_handle);
  EXPECT_EQ(len, 4);
  EXPECT_STREQ(buf, "TEST");

  cfclose(file_handle);
}

TEST_F(CFileIntegrationTest, CFileIO_ReadByte) {
  CFILE *file_handle = cfopen("TestDir/lowercase.txt", "rb");
  ASSERT_NE(file_handle, nullptr);

  // First byte should be 'T' (84)
  EXPECT_EQ(cf_ReadByte(file_handle), 84);
  cf_Rewind(file_handle);

  cfclose(file_handle);
}

TEST_F(CFileIntegrationTest, CFileIO_ReadShort) {
  CFILE *file_handle = cfopen("TestDir/lowercase.txt", "rb");
  ASSERT_NE(file_handle, nullptr);

  // 'T' (84) and 'E' (69) as little-endian short: 0x4554 = 17748
  int16_t val = cf_ReadShort(file_handle);
  EXPECT_EQ(val, 17748);

  cfclose(file_handle);
}

TEST_F(CFileIntegrationTest, CFileIO_ReadInt) {
  CFILE *file_handle = cfopen("TestDir/lowercase.txt", "rb");
  ASSERT_NE(file_handle, nullptr);

  // 'T', 'E', 'S', 'T' as little-endian int: 0x54455354 = 1414743380
  int32_t val = cf_ReadInt(file_handle);
  EXPECT_EQ(val, 1414743380);

  cfclose(file_handle);
}

TEST_F(CFileIntegrationTest, CFileIO_Rewind) {
  CFILE *file_handle = cfopen("TestDir/lowercase.txt", "rb");
  ASSERT_NE(file_handle, nullptr);

  // Read first byte
  cf_ReadByte(file_handle);
  EXPECT_EQ(cftell(file_handle), 1);

  // Rewind and check position
  cf_Rewind(file_handle);
  EXPECT_EQ(cftell(file_handle), 0);

  // Read first byte again
  EXPECT_EQ(cf_ReadByte(file_handle), 84);

  cfclose(file_handle);
}

// ============================================================================
// CFileLibrary Tests - HOG library operations
// ============================================================================

TEST_F(CFileIntegrationTest, CFileLibrary_OpenAndRead) {
  // Open the library (simulates original test behavior)
  int lib_handle = cf_OpenLibrary("TestDir/test.hog");
  EXPECT_NE(lib_handle, 0);

  // Open file from library
  CFILE *file_handle = cfopen("lowercase.txt", "rb");
  ASSERT_NE(file_handle, nullptr);

  // Read string
  char buf[5];
  cf_ReadString(buf, 5, file_handle);
  EXPECT_STREQ(buf, "TEST");

  // Check file length (should be 4 in library)
  EXPECT_EQ(cfilelength(file_handle), 4);

  cfclose(file_handle);
  cf_CloseLibrary(lib_handle);
}

TEST_F(CFileIntegrationTest, CFileLibrary_OpenFileInLibrary) {
  int lib_handle = cf_OpenLibrary("TestDir/test.hog");
  EXPECT_NE(lib_handle, 0);

  // Open file specifically from library
  CFILE *file_handle = cf_OpenFileInLibrary("lowercase.txt", lib_handle);
  ASSERT_NE(file_handle, nullptr);

  char buf[5];
  cf_ReadString(buf, 5, file_handle);
  EXPECT_STREQ(buf, "TEST");

  cfclose(file_handle);
  cf_CloseLibrary(lib_handle);
}

TEST_F(CFileIntegrationTest, CFileLibrary_WithSearchPath) {
  // Test with search path enabled (second pass in original test)
  cf_SetSearchPath("TestDir", nullptr);

  int lib_handle = cf_OpenLibrary("TestDir/test.hog");
  EXPECT_NE(lib_handle, 0);

  // File should be found
  CFILE *file_handle = cfopen("lowercase.txt", "rb");
  EXPECT_NE(file_handle, nullptr);

  if (file_handle) {
    cfclose(file_handle);
  }

  cf_ClearAllSearchPaths();
  cf_CloseLibrary(lib_handle);
}

TEST_F(CFileIntegrationTest, CFileLibrary_FileNotFound) {
  // Try to open without library - should fail
  CFILE *file_handle = cfopen("nonexistent.txt", "rb");
  EXPECT_EQ(file_handle, nullptr);
}

TEST_F(CFileIntegrationTest, CFileLibrary_CRC) {
  int lib_handle = cf_OpenLibrary("TestDir/test.hog");
  EXPECT_NE(lib_handle, 0);

  CFILE *file_handle = cfopen("lowercase.txt", "rb");
  ASSERT_NE(file_handle, nullptr);

  // Calculate CRC
  unsigned int crc = cf_CalculateFileCRC(file_handle);
  EXPECT_NE(crc, 0); // Should have some CRC value

  cfclose(file_handle);
  cf_CloseLibrary(lib_handle);
}

// ============================================================================
// CFileLocatePath Tests - Case-insensitive path resolution
// ============================================================================

TEST_F(CFileIntegrationTest, CFileLocatePath_Lowercase) {
  // Should find lowercase.txt
  std::filesystem::path result = cf_LocatePath("lowercase.txt");
  EXPECT_FALSE(result.empty());
  EXPECT_NE(result.string().find("lowercase.txt"), std::string::npos);
}

TEST_F(CFileIntegrationTest, CFileLocatePath_Uppercase) {
  // Should find UPPERCASE.TXT via case-insensitive match
  std::filesystem::path result = cf_LocatePath("uppercase.txt");
  EXPECT_FALSE(result.empty());
}

TEST_F(CFileIntegrationTest, CFileLocatePath_CamelCase) {
  // Should find CamelCase.txt
  std::filesystem::path result = cf_LocatePath("camelcase.txt");
  EXPECT_FALSE(result.empty());
}

TEST_F(CFileIntegrationTest, CFileLocatePath_NotFound) {
  // Should return empty for non-existent file
  std::filesystem::path result = cf_LocatePath("no-exist-file.txt");
  EXPECT_TRUE(result.empty());
}

TEST_F(CFileIntegrationTest, CFileLocatePath_WithPath) {
  // Should find with path prefix
  std::filesystem::path result = cf_LocatePath("TestDir/lowercase.txt");
  EXPECT_FALSE(result.empty());
}

TEST_F(CFileIntegrationTest, CFileLocatePath_CaseInsensitiveMixed) {
  // Test various case combinations
  std::vector<std::string> variations = {"LOWERCASE.TXT", "Lowercase.txt", "LoWeRcAsE.tXt", "lowercase.TXT"};

  for (const auto &var : variations) {
    std::filesystem::path result = cf_LocatePath(var);
    EXPECT_FALSE(result.empty()) << "Failed to find: " << var;
  }
}

// ============================================================================
// Base Directory Tests
// ============================================================================

TEST_F(CFileIntegrationTest, BaseDirectory_AddAndClear) {
  cf_ClearBaseDirectories();
  EXPECT_TRUE(fs.GetBaseDirectories().empty());

  cf_AddBaseDirectory("/test/path");
  EXPECT_EQ(fs.GetBaseDirectories().size(), 1);
  EXPECT_EQ(fs.GetBaseDirectories()[0], "/test/path");

  cf_AddBaseDirectory("/another/path");
  EXPECT_EQ(fs.GetBaseDirectories().size(), 2);

  cf_ClearBaseDirectories();
  EXPECT_TRUE(fs.GetBaseDirectories().empty());
}

TEST_F(CFileIntegrationTest, BaseDirectory_LocateWithBaseDir) {
  // Add a base directory
  cf_AddBaseDirectory("TestDir");

  // Should be able to locate file in base directory
  std::filesystem::path result = cf_LocatePath("lowercase.txt");
  EXPECT_FALSE(result.empty());

  cf_ClearBaseDirectories();
}

// ============================================================================
// Search Path Tests
// ============================================================================

TEST_F(CFileIntegrationTest, SearchPath_AddAndClear) {
  cf_ClearAllSearchPaths();
  EXPECT_TRUE(fs.GetSearchPaths().empty());

  cf_SetSearchPath("TestDir", nullptr);
  EXPECT_EQ(fs.GetSearchPaths().size(), 1);
  EXPECT_EQ(fs.GetSearchPaths()[0].path, "TestDir");

  cf_ClearAllSearchPaths();
  EXPECT_TRUE(fs.GetSearchPaths().empty());
}

TEST_F(CFileIntegrationTest, SearchPath_FindWithExtensionFilter) {
  // Add search path with extension filter
  cf_SetSearchPath("TestDir", ".txt", nullptr);

  // Should find .txt files
  CFILE *cf = cfopen("lowercase.txt", "rb");
  EXPECT_NE(cf, nullptr);
  if (cf)
    cfclose(cf);

  cf_ClearAllSearchPaths();
}

// ============================================================================
// File Existence Tests
// ============================================================================

TEST_F(CFileIntegrationTest, FileExist_OnDisk) {
  // Should return 2 (CFES_ON_DISK) for regular files
  int result = cfexist("TestDir/lowercase.txt");
  EXPECT_EQ(result, 2);
}

TEST_F(CFileIntegrationTest, FileExist_InLibrary) {
  int lib_handle = cf_OpenLibrary("TestDir/test.hog");
  EXPECT_NE(lib_handle, 0);

  // Should return 1 (CFES_IN_LIBRARY) for files in library
  int result = cfexist("lowercase.txt");
  EXPECT_EQ(result, 1);

  cf_CloseLibrary(lib_handle);
}

TEST_F(CFileIntegrationTest, FileExist_NotFound) {
  // Should return 0 (CFES_NOT_FOUND)
  int result = cfexist("nonexistent.txt");
  EXPECT_EQ(result, 0);
}

// ============================================================================
// Multi-File Tests
// ============================================================================

TEST_F(CFileIntegrationTest, MultipleFiles_Open) {
  // Create multiple files
  fs.AddFile("file1.dat", std::vector<uint8_t>{1, 2, 3});
  fs.AddFile("file2.dat", std::vector<uint8_t>{4, 5, 6});
  fs.AddFile("file3.dat", std::vector<uint8_t>{7, 8, 9});

  CFILE *cf1 = cfopen("file1.dat", "rb");
  CFILE *cf2 = cfopen("file2.dat", "rb");
  CFILE *cf3 = cfopen("file3.dat", "rb");

  ASSERT_NE(cf1, nullptr);
  ASSERT_NE(cf2, nullptr);
  ASSERT_NE(cf3, nullptr);

  EXPECT_EQ(cf_ReadByte(cf1), 1);
  EXPECT_EQ(cf_ReadByte(cf2), 4);
  EXPECT_EQ(cf_ReadByte(cf3), 7);

  cfclose(cf1);
  cfclose(cf2);
  cfclose(cf3);
}

// ============================================================================
// Read/Write Tests
// ============================================================================

TEST_F(CFileIntegrationTest, ReadWrite_Roundtrip) {
  // Write data
  CFILE *cf = cfopen("roundtrip.dat", "wb");
  ASSERT_NE(cf, nullptr);

  cf_WriteInt(cf, 0x12345678);
  cf_WriteShort(cf, 0x1234);
  cf_WriteByte(cf, 0x42);
  cf_WriteFloat(cf, 3.14f);
  cf_WriteString(cf, "Test String");

  cfclose(cf);

  // Read it back
  cf = cfopen("roundtrip.dat", "rb");
  ASSERT_NE(cf, nullptr);

  EXPECT_EQ(cf_ReadInt(cf), 0x12345678);
  EXPECT_EQ(cf_ReadShort(cf), 0x1234);
  EXPECT_EQ(cf_ReadByte(cf), 0x42);

  float f = cf_ReadFloat(cf);
  EXPECT_NEAR(f, 3.14f, 0.001f);

  char buf[64];
  cf_ReadString(buf, sizeof(buf), cf);
  EXPECT_STREQ(buf, "Test String");

  cfclose(cf);
}

TEST_F(CFileIntegrationTest, Seek_Operations) {
  fs.AddFile("seek.dat", std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04, 0x05});

  CFILE *cf = cfopen("seek.dat", "rb");
  ASSERT_NE(cf, nullptr);

  // SEEK_SET
  cfseek(cf, 2, SEEK_SET);
  EXPECT_EQ(cftell(cf), 2);
  EXPECT_EQ(cf_ReadByte(cf), 0x03);

  // SEEK_CUR
  cfseek(cf, -1, SEEK_CUR);
  EXPECT_EQ(cftell(cf), 2);

  // SEEK_END
  cfseek(cf, -2, SEEK_END);
  EXPECT_EQ(cftell(cf), 3);
  EXPECT_EQ(cf_ReadByte(cf), 0x04);

  cfclose(cf);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(CFileIntegrationTest, Edge_EmptyFile) {
  fs.AddFile("empty.dat", std::vector<uint8_t>{});

  CFILE *cf = cfopen("empty.dat", "rb");
  ASSERT_NE(cf, nullptr);

  EXPECT_EQ(cfeof(cf), 1);
  EXPECT_EQ(cfilelength(cf), 0);

  cfclose(cf);
}

TEST_F(CFileIntegrationTest, Edge_ReadBeyondEOF) {
  fs.AddFile("short.dat", std::vector<uint8_t>{0x01, 0x02});

  CFILE *cf = cfopen("short.dat", "rb");
  ASSERT_NE(cf, nullptr);

  cfseek(cf, 0, SEEK_END);

  ubyte buf[10];
  int count = cf_ReadBytes(buf, 10, cf);
  EXPECT_EQ(count, 0);

  cfclose(cf);
}

TEST_F(CFileIntegrationTest, Edge_CopyFile) {
  fs.AddFile("source.dat", std::vector<uint8_t>{1, 2, 3, 4, 5});

  bool result = cf_CopyFile("dest.dat", "source.dat", 0);
  EXPECT_TRUE(result);

  CFILE *cf = cfopen("dest.dat", "rb");
  ASSERT_NE(cf, nullptr);
  EXPECT_EQ(cfilelength(cf), 5);
  cfclose(cf);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
