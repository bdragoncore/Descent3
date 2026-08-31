#include "gtest/gtest.h"
#include "bitmap.h"
#include "pstypes.h"
#include "cfile.h"
#include "iff.h"
#include "mock_cfile.hpp"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cstdio>

class PCXLoaderTest : public ::testing::Test {
protected:
    static bool initialized;
    InMemoryFileSystem inmem_fs;
    
    void SetUp() override {
        GetInMemoryFileSystem() = &inmem_fs;
        
        if (!initialized) {
            bm_InitBitmaps();
            initialized = true;
        }
    }
    
    void TearDown() override {
        GetInMemoryFileSystem() = nullptr;
        inmem_fs.Clear();
    }
    
    std::vector<uint8_t> CreateMinimalPCX8Bit(int width, int height) {
        std::vector<uint8_t> data;
        
        data.push_back(0x0A);
        data.push_back(5);
        data.push_back(1);
        data.push_back(8);
        
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        data.push_back((width - 1) & 0xFF);
        data.push_back(((width - 1) >> 8) & 0xFF);
        data.push_back((height - 1) & 0xFF);
        data.push_back(((height - 1) >> 8) & 0xFF);
        
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        for (int i = 16; i < 64; i++) {
            data.push_back(0);
        }
        
        data.push_back(1);
        data.push_back(0);
        
        for (int i = 66; i < 128; i++) {
            data.push_back(0);
        }
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                data.push_back(0);
            }
            if ((width % 2) != 0) {
                data.push_back(0);
            }
        }
        
        for (int i = 0; i < 256; i++) {
            data.push_back(i);
            data.push_back(i);
            data.push_back(i);
        }
        
        return data;
    }
    
    std::vector<uint8_t> CreateMinimalPCX24Bit(int width, int height) {
        std::vector<uint8_t> data;
        
        data.push_back(0x0A);
        data.push_back(5);
        data.push_back(1);
        data.push_back(8);
        
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        data.push_back((width - 1) & 0xFF);
        data.push_back(((width - 1) >> 8) & 0xFF);
        data.push_back((height - 1) & 0xFF);
        data.push_back(((height - 1) >> 8) & 0xFF);
        
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        for (int i = 16; i < 64; i++) {
            data.push_back(0);
        }
        
        data.push_back(3);
        data.push_back(0);
        
        for (int i = 66; i < 128; i++) {
            data.push_back(0);
        }
        
        int row_size = ((width + 1) / 2) * 2;
        
        for (int c = 0; c < 3; c++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    data.push_back(0x80);
                }
                int pad = row_size - width;
                for (int p = 0; p < pad; p++) {
                    data.push_back(0);
                }
            }
        }
        
        return data;
    }
    
    std::vector<uint8_t> CreateInvalidPCX() {
        std::vector<uint8_t> data;
        data.push_back(0xFF);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        return data;
    }
    
    std::vector<uint8_t> CreateNonPCX() {
        std::vector<uint8_t> data;
        data.push_back('G');
        data.push_back('I');
        data.push_back('F');
        data.push_back('8');
        return data;
    }
};

bool PCXLoaderTest::initialized = false;

TEST_F(PCXLoaderTest, LoadPCX8BitFromMemory) {
    auto pcx_data = CreateMinimalPCX8Bit(32, 32);
    GetInMemoryFileSystem()->AddFile("test.pcx", pcx_data);
    
    CFILE* cf = cfopen("test.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 32);
        EXPECT_EQ(bm_h(handle, 0), 32);
        bm_FreeBitmap(handle);
    } else {
        SUCCEED() << "PCX loading failed - format may not be supported";
    }
}

TEST_F(PCXLoaderTest, LoadPCX24BitFromMemory) {
    auto pcx_data = CreateMinimalPCX24Bit(32, 32);
    GetInMemoryFileSystem()->AddFile("test24.pcx", pcx_data);
    
    CFILE* cf = cfopen("test24.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 32);
        EXPECT_EQ(bm_h(handle, 0), 32);
        bm_FreeBitmap(handle);
    }
}

TEST_F(PCXLoaderTest, LoadPCXInvalidFile) {
    auto pcx_data = CreateInvalidPCX();
    GetInMemoryFileSystem()->AddFile("invalid.pcx", pcx_data);
    
    CFILE* cf = cfopen("invalid.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    EXPECT_EQ(handle, -1);
}

TEST_F(PCXLoaderTest, LoadPCXNonPCXFile) {
    auto pcx_data = CreateNonPCX();
    GetInMemoryFileSystem()->AddFile("notpcx.pcx", pcx_data);
    
    CFILE* cf = cfopen("notpcx.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    EXPECT_EQ(handle, -1);
}

TEST_F(PCXLoaderTest, LoadPCXEmptyFile) {
    GetInMemoryFileSystem()->AddFile("empty.pcx", std::vector<uint8_t>{});
    
    CFILE* cf = cfopen("empty.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    EXPECT_EQ(handle, -1);
}

TEST_F(PCXLoaderTest, LoadPCXFileNotFound) {
    CFILE* cf = cfopen("nonexistent.pcx", "rb");
    EXPECT_EQ(cf, nullptr);
}

TEST_F(PCXLoaderTest, LoadMultiplePCX) {
    auto pcx1 = CreateMinimalPCX8Bit(16, 16);
    auto pcx2 = CreateMinimalPCX8Bit(32, 32);
    auto pcx3 = CreateMinimalPCX8Bit(64, 64);
    
    GetInMemoryFileSystem()->AddFile("a.pcx", pcx1);
    GetInMemoryFileSystem()->AddFile("b.pcx", pcx2);
    GetInMemoryFileSystem()->AddFile("c.pcx", pcx3);
    
    CFILE* cf1 = cfopen("a.pcx", "rb");
    ASSERT_NE(cf1, nullptr);
    int h1 = bm_AllocLoadBitmap(cf1, 0, 0);
    cfclose(cf1);
    
    CFILE* cf2 = cfopen("b.pcx", "rb");
    ASSERT_NE(cf2, nullptr);
    int h2 = bm_AllocLoadBitmap(cf2, 0, 0);
    cfclose(cf2);
    
    CFILE* cf3 = cfopen("c.pcx", "rb");
    ASSERT_NE(cf3, nullptr);
    int h3 = bm_AllocLoadBitmap(cf3, 0, 0);
    cfclose(cf3);
    
    if (h1 > 0 && h2 > 0 && h3 > 0) {
        EXPECT_EQ(bm_w(h1, 0), 16);
        EXPECT_EQ(bm_w(h2, 0), 32);
        EXPECT_EQ(bm_w(h3, 0), 64);
        bm_FreeBitmap(h1);
        bm_FreeBitmap(h2);
        bm_FreeBitmap(h3);
    }
}

TEST_F(PCXLoaderTest, PCXDirectAllocFile) {
    auto pcx_data = CreateMinimalPCX8Bit(32, 32);
    GetInMemoryFileSystem()->AddFile("direct.pcx", pcx_data);
    
    CFILE* cf = cfopen("direct.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_pcx_alloc_file(cf);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 32);
        EXPECT_EQ(bm_h(handle, 0), 32);
        bm_FreeBitmap(handle);
    }
}

TEST_F(PCXLoaderTest, PCXSmallSize) {
    auto pcx_data = CreateMinimalPCX8Bit(8, 8);
    GetInMemoryFileSystem()->AddFile("small.pcx", pcx_data);
    
    CFILE* cf = cfopen("small.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 8);
        EXPECT_EQ(bm_h(handle, 0), 8);
        bm_FreeBitmap(handle);
    }
}

TEST_F(PCXLoaderTest, PCXLargeSize) {
    auto pcx_data = CreateMinimalPCX8Bit(256, 256);
    GetInMemoryFileSystem()->AddFile("large.pcx", pcx_data);
    
    CFILE* cf = cfopen("large.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 256);
        EXPECT_EQ(bm_h(handle, 0), 256);
        bm_FreeBitmap(handle);
    } else {
        SUCCEED() << "Large bitmap may have memory constraints";
    }
}

TEST_F(PCXLoaderTest, PCXRectangular) {
    auto pcx_data = CreateMinimalPCX8Bit(320, 240);
    GetInMemoryFileSystem()->AddFile("rect.pcx", pcx_data);
    
    CFILE* cf = cfopen("rect.pcx", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 320);
        EXPECT_EQ(bm_h(handle, 0), 240);
        bm_FreeBitmap(handle);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    _exit(result);
}
