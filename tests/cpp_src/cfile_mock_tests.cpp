/**
 * @file cfile_mock_tests.cpp
 * @brief D3 CFile Mock Tests.
 *
 * @details
 * Demonstrates using the in-memory file system for testing.
 *
 * This harness validates the behavior of `Descent3/cfile_mock.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/cfile_mock.cpp`
 * @par Harness
 * `cfile_mock_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/cfile_mock.cpp
 */

#include <gtest/gtest.h>
#include "mock_cfile.hpp"

/**
 * @brief GTest fixture for CFileMockTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class CFileMockTest : public ::testing::Test {
protected:
    InMemoryFileSystem inmem_fs;
    
    void SetUp() override {
        GetInMemoryFileSystem() = &inmem_fs;
    }
    
    void TearDown() override {
        GetInMemoryFileSystem() = nullptr;
        inmem_fs.Clear();
    }
};

/**
 * @test CFileMockTest.ReadIntWithEndianness
 * @brief Verifies read Int With Endianness.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, ReadIntWithEndianness) {
    // Inject test data: little-endian 0x12345678
    std::vector<uint8_t> data = {0x78, 0x56, 0x34, 0x12};
    inmem_fs.AddFile("test.bin", data);
    
    CFILE* cf = cfopen("test.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    // cf_ReadInt applies INTEL_INT - should return 0x12345678
    int32_t val = cf_ReadInt(cf);
    EXPECT_EQ(val, 0x12345678);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.ReadMultipleInts
 * @brief Verifies read Multiple Ints.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, ReadMultipleInts) {
    std::vector<uint8_t> data = {
        0x01, 0x00, 0x00, 0x00,  // 1
        0x02, 0x00, 0x00, 0x00,  // 2
        0x03, 0x00, 0x00, 0x00   // 3
    };
    inmem_fs.AddFile("nums.bin", data);
    
    CFILE* cf = cfopen("nums.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cf_ReadInt(cf), 1);
    EXPECT_EQ(cf_ReadInt(cf), 2);
    EXPECT_EQ(cf_ReadInt(cf), 3);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.ReadShort
 * @brief Verifies read Short.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, ReadShort) {
    std::vector<uint8_t> data = {0x34, 0x12};  // little-endian 0x1234
    inmem_fs.AddFile("short.bin", data);
    
    CFILE* cf = cfopen("short.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    int16_t val = cf_ReadShort(cf);
    EXPECT_EQ(val, 0x1234);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.ReadByte
 * @brief Verifies read Byte.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, ReadByte) {
    std::vector<uint8_t> data = {0x42};
    inmem_fs.AddFile("byte.bin", data);
    
    CFILE* cf = cfopen("byte.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    int8_t val = cf_ReadByte(cf);
    EXPECT_EQ(val, 0x42);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.ReadFloat
 * @brief Verifies read Float.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, ReadFloat) {
    // Float 3.14 in little-endian
    std::vector<uint8_t> data = {
        0xc3, 0xf5, 0x48, 0x40
    };
    inmem_fs.AddFile("float.bin", data);
    
    CFILE* cf = cfopen("float.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    float val = cf_ReadFloat(cf);
    EXPECT_NEAR(val, 3.14f, 0.001f);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.FileNotFound
 * @brief Verifies file Not Found.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, FileNotFound) {
    CFILE* cf = cfopen("nonexistent.bin", "rb");
    EXPECT_EQ(cf, nullptr);
}

/**
 * @test CFileMockTest.FileExists
 * @brief Verifies file Exists.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, FileExists) {
    std::vector<uint8_t> data = {1, 2, 3, 4};
    inmem_fs.AddFile("exists.bin", data);
    
    EXPECT_EQ(cfexist("exists.bin"), 2);  // CFES_ON_DISK
    EXPECT_EQ(cfexist("nonexistent.bin"), 0);  // CFES_NOT_FOUND
}

/**
 * @test CFileMockTest.SeekAndTell
 * @brief Verifies seek And Tell.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, SeekAndTell) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    inmem_fs.AddFile("seek.bin", data);
    
    CFILE* cf = cfopen("seek.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cftell(cf), 0);
    
    cfseek(cf, 2, SEEK_SET);
    EXPECT_EQ(cftell(cf), 2);
    
    cfseek(cf, 1, SEEK_CUR);
    EXPECT_EQ(cftell(cf), 3);
    
    cfseek(cf, -1, SEEK_END);
    EXPECT_EQ(cftell(cf), 4);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.EndOfFile
 * @brief Verifies end Of File.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, EndOfFile) {
    std::vector<uint8_t> data = {1, 2};
    inmem_fs.AddFile("eof.bin", data);
    
    CFILE* cf = cfopen("eof.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cfeof(cf), 0);
    
    cfseek(cf, 0, SEEK_END);
    EXPECT_NE(cfeof(cf), 0);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.Rewind
 * @brief Verifies rewind.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, Rewind) {
    std::vector<uint8_t> data = {1, 2, 3};
    inmem_fs.AddFile("rewind.bin", data);
    
    CFILE* cf = cfopen("rewind.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    cfseek(cf, 2, SEEK_SET);
    EXPECT_EQ(cftell(cf), 2);
    
    cf_Rewind(cf);
    EXPECT_EQ(cftell(cf), 0);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.ReadBytes
 * @brief Verifies read Bytes.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, ReadBytes) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    inmem_fs.AddFile("bytes.bin", data);
    
    CFILE* cf = cfopen("bytes.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    ubyte buf[4];
    int bytes_read = cf_ReadBytes(buf, 4, cf);
    
    EXPECT_EQ(bytes_read, 4);
    EXPECT_EQ(buf[0], 0x01);
    EXPECT_EQ(buf[1], 0x02);
    EXPECT_EQ(buf[2], 0x03);
    EXPECT_EQ(buf[3], 0x04);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.ReadString
 * @brief Verifies read String.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, ReadString) {
    std::string data = "Hello, World!";
    inmem_fs.AddFile("string.txt", data);
    
    CFILE* cf = cfopen("string.txt", "rt");  // text mode
    ASSERT_NE(cf, nullptr);
    
    char buf[64];
    int len = cf_ReadString(buf, sizeof(buf), cf);
    
    EXPECT_GT(len, 0);
    EXPECT_STREQ(buf, "Hello, World!");
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.WriteAndRead
 * @brief Verifies write And Read.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, WriteAndRead) {
    // Pre-add file for writing (or use "wb" to create new)
    CFILE* cf = cfopen("write.bin", "wb");
    ASSERT_NE(cf, nullptr);
    
    cf_WriteInt(cf, 0x12345678);
    cf_WriteShort(cf, 0x1234);
    cf_WriteByte(cf, 0x42);
    cf_WriteFloat(cf, 3.14f);
    
    cfclose(cf);
    
    // Now read it back
    cf = cfopen("write.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cf_ReadInt(cf), 0x12345678);
    EXPECT_EQ(cf_ReadShort(cf), 0x1234);
    EXPECT_EQ(cf_ReadByte(cf), 0x42);
    
    float f = cf_ReadFloat(cf);
    EXPECT_NEAR(f, 3.14f, 0.001f);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.WriteString
 * @brief Verifies write String.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, WriteString) {
    // Open for writing - creates new file
    CFILE* cf = cfopen("writestr.txt", "wt");
    ASSERT_NE(cf, nullptr);
    
    cf_WriteString(cf, "Test string");
    
    cfclose(cf);
    
    // Add the file for reading (it was created in write mode)
    auto* f = inmem_fs.GetFile("writestr.txt");
    ASSERT_NE(f, nullptr);
    
    // Read back in text mode
    cf = cfopen("writestr.txt", "rt");
    ASSERT_NE(cf, nullptr);
    
    char buf[64];
    cf_ReadString(buf, sizeof(buf), cf);
    EXPECT_STREQ(buf, "Test string");
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.FileLength
 * @brief Verifies file Length.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, FileLength) {
    std::vector<uint8_t> data(100, 0xAB);
    inmem_fs.AddFile("length.bin", data);
    
    CFILE* cf = cfopen("length.bin", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cfilelength(cf), 100);
    
    cfclose(cf);
}

/**
 * @test CFileMockTest.MultipleFiles
 * @brief Verifies multiple Files.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, MultipleFiles) {
    inmem_fs.AddFile("file1.bin", std::vector<uint8_t>{1, 2, 3});
    inmem_fs.AddFile("file2.bin", std::vector<uint8_t>{4, 5, 6});
    inmem_fs.AddFile("file3.bin", std::vector<uint8_t>{7, 8, 9});
    
    CFILE* cf1 = cfopen("file1.bin", "rb");
    CFILE* cf2 = cfopen("file2.bin", "rb");
    CFILE* cf3 = cfopen("file3.bin", "rb");
    
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

/**
 * @test CFileMockTest.CopyFile
 * @brief Verifies copy File.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, CopyFile) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    inmem_fs.AddFile("source.bin", data);
    
    bool result = cf_CopyFile("dest.bin", "source.bin", 0);
    EXPECT_TRUE(result);
    
    // Verify copy exists
    CFILE* cf = cfopen("dest.bin", "rb");
    ASSERT_NE(cf, nullptr);
    EXPECT_EQ(cfilelength(cf), 5);
    cfclose(cf);
}

/**
 * @test CFileMockTest.BinaryVsTextMode
 * @brief Verifies binary Vs Text Mode.
 *
 * @details
 * Exercises the CFileMockTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cfile_mock.cpp
 * @ingroup descent3_tests
 */
TEST_F(CFileMockTest, BinaryVsTextMode) {
    // Pre-add file for writing
    inmem_fs.AddFile("binary.bin", std::vector<uint8_t>{});
    
    // Write in binary mode - should preserve exact bytes
    CFILE* cf = cfopen("binary.bin", "wb");
    cf_WriteByte(cf, '\n');
    cfclose(cf);
    
    cf = cfopen("binary.bin", "rb");
    auto* infile = inmem_fs.GetFile("binary.bin");
    ASSERT_NE(infile, nullptr);
    EXPECT_EQ(infile->data.size(), 1);
    EXPECT_EQ(infile->data[0], '\n');
    cfclose(cf);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
