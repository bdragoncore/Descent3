/*
 * D3 Coverage Tests - cfile module (Standalone)
 * 
 * Tests for cfile functions using the mock infrastructure.
 * Uses seeded RNG for reproducible test data.
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>

typedef unsigned char ubyte;
typedef int8_t sbyte;
typedef int16_t sshort;
typedef int32_t slong;

#include "mock_cfile.hpp"

static const uint32_t DEFAULT_SEED = 12345;

class CFileTest : public ::testing::Test {
protected:
    InMemoryFileSystem inmem_fs;
    std::mt19937 rng_;
    
    CFileTest() : rng_(DEFAULT_SEED) {}
    
    void SetUp() override {
        GetInMemoryFileSystem() = &inmem_fs;
    }
    
    void TearDown() override {
        GetInMemoryFileSystem() = nullptr;
        inmem_fs.Clear();
    }
    
    std::vector<uint8_t> random_bytes(size_t count) {
        std::uniform_int_distribution<int> dist(0, 255);
        std::vector<uint8_t> result(count);
        for (size_t i = 0; i < count; i++) {
            result[i] = static_cast<uint8_t>(dist(rng_));
        }
        return result;
    }
    
    std::vector<uint8_t> le_bytes(uint32_t val) {
        std::vector<uint8_t> result(4);
        result[0] = static_cast<uint8_t>(val & 0xFF);
        result[1] = static_cast<uint8_t>((val >> 8) & 0xFF);
        result[2] = static_cast<uint8_t>((val >> 16) & 0xFF);
        result[3] = static_cast<uint8_t>((val >> 24) & 0xFF);
        return result;
    }
    
    std::vector<uint8_t> le_bytes16(uint16_t val) {
        std::vector<uint8_t> result(2);
        result[0] = static_cast<uint8_t>(val & 0xFF);
        result[1] = static_cast<uint8_t>((val >> 8) & 0xFF);
        return result;
    }
    
    void append_bytes(std::vector<uint8_t>& dest, const std::vector<uint8_t>& src) {
        dest.insert(dest.end(), src.begin(), src.end());
    }
};

TEST_F(CFileTest, OpenExistingFile) {
    inmem_fs.AddFile("test.dat", std::vector<uint8_t>{1, 2, 3, 4});
    CFILE* cf = cfopen("test.dat", "rb");
    ASSERT_NE(cf, nullptr);
    cfclose(cf);
}

TEST_F(CFileTest, OpenNonExistentFile) {
    CFILE* cf = cfopen("nonexistent.dat", "rb");
    EXPECT_EQ(cf, nullptr);
}

TEST_F(CFileTest, FileExists) {
    inmem_fs.AddFile("exists.dat", std::vector<uint8_t>{1});
    EXPECT_EQ(cfexist("exists.dat"), 2);
    EXPECT_EQ(cfexist("nonexistent.dat"), 0);
}

TEST_F(CFileTest, ReadInt) {
    std::vector<uint8_t> data = le_bytes(0x12345678);
    inmem_fs.AddFile("int.dat", data);
    
    CFILE* cf = cfopen("int.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    int32_t val = cf_ReadInt(cf);
    EXPECT_EQ(val, 0x12345678);
    
    cfclose(cf);
}

TEST_F(CFileTest, ReadMultipleInts) {
    std::vector<uint8_t> data;
    append_bytes(data, le_bytes(100));
    append_bytes(data, le_bytes(200));
    append_bytes(data, le_bytes(300));
    append_bytes(data, le_bytes(400));
    inmem_fs.AddFile("multi.dat", data);
    
    CFILE* cf = cfopen("multi.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cf_ReadInt(cf), 100);
    EXPECT_EQ(cf_ReadInt(cf), 200);
    EXPECT_EQ(cf_ReadInt(cf), 300);
    EXPECT_EQ(cf_ReadInt(cf), 400);
    
    cfclose(cf);
}

TEST_F(CFileTest, ReadShort) {
    std::vector<uint8_t> data = le_bytes(static_cast<uint16_t>(0x1234));
    inmem_fs.AddFile("short.dat", data);
    
    CFILE* cf = cfopen("short.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    int16_t val = cf_ReadShort(cf);
    EXPECT_EQ(val, 0x1234);
    
    cfclose(cf);
}

TEST_F(CFileTest, ReadByte) {
    std::vector<uint8_t> data = {0x42};
    inmem_fs.AddFile("byte.dat", data);
    
    CFILE* cf = cfopen("byte.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    int8_t val = cf_ReadByte(cf);
    EXPECT_EQ(val, 0x42);
    
    cfclose(cf);
}

TEST_F(CFileTest, ReadFloat) {
    union { float f; uint32_t i; } val;
    val.f = 3.14159f;
    std::vector<uint8_t> data = le_bytes(val.i);
    inmem_fs.AddFile("float.dat", data);
    
    CFILE* cf = cfopen("float.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    float read_val = cf_ReadFloat(cf);
    EXPECT_NEAR(read_val, 3.14159f, 0.0001f);
    
    cfclose(cf);
}

TEST_F(CFileTest, ReadDouble) {
    union { double d; uint64_t i; } val;
    val.d = 2.718281828;
    std::vector<uint8_t> data;
    for (int i = 0; i < 8; i++) {
        data.push_back(static_cast<uint8_t>((val.i >> (i * 8)) & 0xFF));
    }
    inmem_fs.AddFile("double.dat", data);
    
    CFILE* cf = cfopen("double.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    double read_val = cf_ReadDouble(cf);
    EXPECT_NEAR(read_val, 2.718281828, 0.0001);
    
    cfclose(cf);
}

TEST_F(CFileTest, ReadBytes) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    inmem_fs.AddFile("bytes.dat", data);
    
    CFILE* cf = cfopen("bytes.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    ubyte buf[4];
    int count = cf_ReadBytes(buf, 4, cf);
    
    EXPECT_EQ(count, 4);
    EXPECT_EQ(buf[0], 0x01);
    EXPECT_EQ(buf[1], 0x02);
    EXPECT_EQ(buf[2], 0x03);
    EXPECT_EQ(buf[3], 0x04);
    
    cfclose(cf);
}

TEST_F(CFileTest, ReadString) {
    std::string test_str = "Hello, World!";
    std::vector<uint8_t> data(test_str.begin(), test_str.end());
    data.push_back(0);
    inmem_fs.AddFile("string.dat", data);
    
    CFILE* cf = cfopen("string.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    char buf[64];
    int len = cf_ReadString(buf, sizeof(buf), cf);
    
    EXPECT_GT(len, 0);
    EXPECT_STREQ(buf, "Hello, World!");
    
    cfclose(cf);
}

TEST_F(CFileTest, WriteAndReadInt) {
    CFILE* cf = cfopen("write_int.dat", "wb");
    ASSERT_NE(cf, nullptr);
    
    cf_WriteInt(cf, 0xDEADBEEF);
    cf_WriteInt(cf, 0xCAFEBABE);
    
    cfclose(cf);
    
    cf = cfopen("write_int.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cf_ReadInt(cf), 0xDEADBEEF);
    EXPECT_EQ(cf_ReadInt(cf), 0xCAFEBABE);
    
    cfclose(cf);
}

TEST_F(CFileTest, WriteAndReadShort) {
    CFILE* cf = cfopen("write_short.dat", "wb");
    ASSERT_NE(cf, nullptr);
    
    cf_WriteShort(cf, 0x1234);
    cf_WriteShort(cf, 0x5678);
    
    cfclose(cf);
    
    cf = cfopen("write_short.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cf_ReadShort(cf), 0x1234);
    EXPECT_EQ(cf_ReadShort(cf), 0x5678);
    
    cfclose(cf);
}

TEST_F(CFileTest, WriteAndReadByte) {
    CFILE* cf = cfopen("write_byte.dat", "wb");
    ASSERT_NE(cf, nullptr);
    
    cf_WriteByte(cf, 0x00);
    cf_WriteByte(cf, 0xFF);
    cf_WriteByte(cf, 0x42);
    
    cfclose(cf);
    
    cf = cfopen("write_byte.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(static_cast<uint8_t>(cf_ReadByte(cf)), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(cf_ReadByte(cf)), 0xFF);
    EXPECT_EQ(static_cast<uint8_t>(cf_ReadByte(cf)), 0x42);
    
    cfclose(cf);
}

TEST_F(CFileTest, WriteAndReadFloat) {
    CFILE* cf = cfopen("write_float.dat", "wb");
    ASSERT_NE(cf, nullptr);
    
    cf_WriteFloat(cf, 1.5f);
    cf_WriteFloat(cf, -2.25f);
    cf_WriteFloat(cf, 0.0f);
    
    cfclose(cf);
    
    cf = cfopen("write_float.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_NEAR(cf_ReadFloat(cf), 1.5f, 0.001f);
    EXPECT_NEAR(cf_ReadFloat(cf), -2.25f, 0.001f);
    EXPECT_NEAR(cf_ReadFloat(cf), 0.0f, 0.001f);
    
    cfclose(cf);
}

TEST_F(CFileTest, WriteAndReadDouble) {
    CFILE* cf = cfopen("write_double.dat", "wb");
    ASSERT_NE(cf, nullptr);
    
    cf_WriteDouble(cf, 1.23456789);
    cf_WriteDouble(cf, -987.654321);
    
    cfclose(cf);
    
    cf = cfopen("write_double.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_NEAR(cf_ReadDouble(cf), 1.23456789, 0.0001);
    EXPECT_NEAR(cf_ReadDouble(cf), -987.654321, 0.0001);
    
    cfclose(cf);
}

TEST_F(CFileTest, WriteAndReadBytes) {
    CFILE* cf = cfopen("write_bytes.dat", "wb");
    ASSERT_NE(cf, nullptr);
    
    ubyte data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    cf_WriteBytes(data, 5, cf);
    
    cfclose(cf);
    
    cf = cfopen("write_bytes.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    ubyte read_buf[5];
    int count = cf_ReadBytes(read_buf, 5, cf);
    
    EXPECT_EQ(count, 5);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(read_buf[i], data[i]);
    }
    
    cfclose(cf);
}

TEST_F(CFileTest, WriteAndReadString) {
    CFILE* cf = cfopen("write_string.dat", "wb");
    ASSERT_NE(cf, nullptr);
    
    cf_WriteString(cf, "Test String");
    cf_WriteString(cf, "");
    cf_WriteString(cf, "ABC");
    
    cfclose(cf);
    
    cf = cfopen("write_string.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    char buf[64];
    
    cf_ReadString(buf, sizeof(buf), cf);
    EXPECT_STREQ(buf, "Test String");
    
    cf_ReadString(buf, sizeof(buf), cf);
    EXPECT_STREQ(buf, "");
    
    cf_ReadString(buf, sizeof(buf), cf);
    EXPECT_STREQ(buf, "ABC");
    
    cfclose(cf);
}

TEST_F(CFileTest, SeekSet) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    inmem_fs.AddFile("seek.dat", data);
    
    CFILE* cf = cfopen("seek.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    cfseek(cf, 2, SEEK_SET);
    EXPECT_EQ(cftell(cf), 2);
    EXPECT_EQ(cf_ReadByte(cf), 0x03);
    
    cfclose(cf);
}

TEST_F(CFileTest, SeekCur) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    inmem_fs.AddFile("seek_cur.dat", data);
    
    CFILE* cf = cfopen("seek_cur.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    cfseek(cf, 2, SEEK_SET);
    cfseek(cf, 1, SEEK_CUR);
    EXPECT_EQ(cftell(cf), 3);
    
    cfclose(cf);
}

TEST_F(CFileTest, SeekEnd) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    inmem_fs.AddFile("seek_end.dat", data);
    
    CFILE* cf = cfopen("seek_end.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    cfseek(cf, -2, SEEK_END);
    EXPECT_EQ(cftell(cf), 3);
    
    cfclose(cf);
}

TEST_F(CFileTest, Rewind) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    inmem_fs.AddFile("rewind.dat", data);
    
    CFILE* cf = cfopen("rewind.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    cfseek(cf, 2, SEEK_SET);
    EXPECT_EQ(cftell(cf), 2);
    
    cf_Rewind(cf);
    EXPECT_EQ(cftell(cf), 0);
    
    cfclose(cf);
}

TEST_F(CFileTest, EndOfFile) {
    std::vector<uint8_t> data = {0x01, 0x02};
    inmem_fs.AddFile("eof.dat", data);
    
    CFILE* cf = cfopen("eof.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cfeof(cf), 0);
    
    cfseek(cf, 0, SEEK_END);
    EXPECT_NE(cfeof(cf), 0);
    
    cfclose(cf);
}

TEST_F(CFileTest, FileLength) {
    std::vector<uint8_t> data(100, 0xAB);
    inmem_fs.AddFile("length.dat", data);
    
    CFILE* cf = cfopen("length.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cfilelength(cf), 100);
    
    cfclose(cf);
}

TEST_F(CFileTest, CopyFile) {
    std::vector<uint8_t> source_data = {0x01, 0x02, 0x03, 0x04, 0x05};
    inmem_fs.AddFile("source_copy.dat", source_data);
    
    bool result = cf_CopyFile("dest_copy.dat", "source_copy.dat", 0);
    EXPECT_TRUE(result);
    
    CFILE* cf = cfopen("dest_copy.dat", "rb");
    ASSERT_NE(cf, nullptr);
    EXPECT_EQ(cfilelength(cf), 5);
    cfclose(cf);
}

TEST_F(CFileTest, MultipleFiles) {
    inmem_fs.AddFile("file1.dat", std::vector<uint8_t>{1});
    inmem_fs.AddFile("file2.dat", std::vector<uint8_t>{2});
    inmem_fs.AddFile("file3.dat", std::vector<uint8_t>{3});
    
    CFILE* cf1 = cfopen("file1.dat", "rb");
    CFILE* cf2 = cfopen("file2.dat", "rb");
    CFILE* cf3 = cfopen("file3.dat", "rb");
    
    ASSERT_NE(cf1, nullptr);
    ASSERT_NE(cf2, nullptr);
    ASSERT_NE(cf3, nullptr);
    
    EXPECT_EQ(cf_ReadByte(cf1), 1);
    EXPECT_EQ(cf_ReadByte(cf2), 2);
    EXPECT_EQ(cf_ReadByte(cf3), 3);
    
    cfclose(cf1);
    cfclose(cf2);
    cfclose(cf3);
}

TEST_F(CFileTest, ZeroValue) {
    std::vector<uint8_t> data;
    append_bytes(data, le_bytes(0));
    append_bytes(data, le_bytes16(0));
    data.push_back(0);
    inmem_fs.AddFile("zero.dat", data);
    
    CFILE* cf = cfopen("zero.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cf_ReadInt(cf), 0);
    EXPECT_EQ(cf_ReadShort(cf), 0);
    EXPECT_EQ(cf_ReadByte(cf), 0);
    
    cfclose(cf);
}

TEST_F(CFileTest, MaxValues) {
    std::vector<uint8_t> data;
    append_bytes(data, le_bytes(0xFFFFFFFF));
    append_bytes(data, le_bytes16(0xFFFF));
    data.push_back(0xFF);
    inmem_fs.AddFile("max.dat", data);
    
    CFILE* cf = cfopen("max.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cf_ReadInt(cf), 0xFFFFFFFF);
    EXPECT_EQ(static_cast<uint16_t>(cf_ReadShort(cf)), 0xFFFF);
    EXPECT_EQ(static_cast<uint8_t>(cf_ReadByte(cf)), 0xFF);
    
    cfclose(cf);
}

TEST_F(CFileTest, NegativeValues) {
    std::vector<uint8_t> data;
    append_bytes(data, le_bytes(static_cast<uint32_t>(-1)));
    append_bytes(data, le_bytes16(static_cast<uint16_t>(-1)));
    inmem_fs.AddFile("neg.dat", data);
    
    CFILE* cf = cfopen("neg.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cf_ReadInt(cf), -1);
    EXPECT_EQ(cf_ReadShort(cf), -1);
    
    cfclose(cf);
}

TEST_F(CFileTest, ReadBeyondEOF) {
    std::vector<uint8_t> data = {0x01, 0x02};
    inmem_fs.AddFile("short.dat", data);
    
    CFILE* cf = cfopen("short.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    cfseek(cf, 0, SEEK_END);
    
    ubyte buf[10];
    int count = cf_ReadBytes(buf, 10, cf);
    EXPECT_EQ(count, 0);
    
    cfclose(cf);
}

TEST_F(CFileTest, EmptyFile) {
    std::vector<uint8_t> data;
    inmem_fs.AddFile("empty.dat", data);
    
    CFILE* cf = cfopen("empty.dat", "rb");
    ASSERT_NE(cf, nullptr);
    
    EXPECT_EQ(cfeof(cf), 1);
    EXPECT_EQ(cfilelength(cf), 0);
    
    cfclose(cf);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
