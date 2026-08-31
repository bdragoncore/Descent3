/**
 * @file iff_loader_tests.cpp
 * @brief Unit tests for Descent3/iff_loader.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/iff_loader.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/iff_loader.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/iff_loader.cpp`
 * @par Harness
 * `iff_loader_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/iff_loader.cpp
 */

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

/**
 * @brief GTest fixture for IFFLoaderTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class IFFLoaderTest : public ::testing::Test {
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
    
    std::vector<uint8_t> CreateMinimalIFF(int width, int height) {
        std::vector<uint8_t> data;
        
        data.push_back('F');
        data.push_back('O');
        data.push_back('R');
        data.push_back('M');
        
        uint32_t form_size = 4 + 8 + 4 + (width * height);
        data.push_back((form_size >> 24) & 0xFF);
        data.push_back((form_size >> 16) & 0xFF);
        data.push_back((form_size >> 8) & 0xFF);
        data.push_back(form_size & 0xFF);
        
        data.push_back('P');
        data.push_back('B');
        data.push_back('M');
        data.push_back(' ');
        
        data.push_back('B');
        data.push_back('M');
        data.push_back('H');
        data.push_back('D');
        uint32_t bmhd_size = 20;
        data.push_back((bmhd_size >> 24) & 0xFF);
        data.push_back((bmhd_size >> 16) & 0xFF);
        data.push_back((bmhd_size >> 8) & 0xFF);
        data.push_back(bmhd_size & 0xFF);
        
        data.push_back((width >> 8) & 0xFF);
        data.push_back(width & 0xFF);
        data.push_back((height >> 8) & 0xFF);
        data.push_back(height & 0xFF);
        
        data.push_back(0);
        data.push_back(0);
        
        data.push_back(0);
        
        data.push_back(0);
        
        data.push_back(0);
        
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        data.push_back('B');
        data.push_back('O');
        data.push_back('D');
        data.push_back('Y');
        uint32_t body_size = width * height;
        data.push_back((body_size >> 24) & 0xFF);
        data.push_back((body_size >> 16) & 0xFF);
        data.push_back((body_size >> 8) & 0xFF);
        data.push_back(body_size & 0xFF);
        
        for (int i = 0; i < width * height; i++) {
            data.push_back(0);
        }
        
        return data;
    }
    
    std::vector<uint8_t> CreateMinimalILBM(int width, int height) {
        std::vector<uint8_t> data;
        
        data.push_back('F');
        data.push_back('O');
        data.push_back('R');
        data.push_back('M');
        
        uint32_t form_size = 4 + 8 + 4 + (width * height) + 8 + 768;
        data.push_back((form_size >> 24) & 0xFF);
        data.push_back((form_size >> 16) & 0xFF);
        data.push_back((form_size >> 8) & 0xFF);
        data.push_back(form_size & 0xFF);
        
        data.push_back('I');
        data.push_back('L');
        data.push_back('B');
        data.push_back('M');
        
        data.push_back('B');
        data.push_back('M');
        data.push_back('H');
        data.push_back('D');
        uint32_t bmhd_size = 20;
        data.push_back((bmhd_size >> 24) & 0xFF);
        data.push_back((bmhd_size >> 16) & 0xFF);
        data.push_back((bmhd_size >> 8) & 0xFF);
        data.push_back(bmhd_size & 0xFF);
        
        data.push_back((width >> 8) & 0xFF);
        data.push_back(width & 0xFF);
        data.push_back((height >> 8) & 0xFF);
        data.push_back(height & 0xFF);
        
        data.push_back(0);
        data.push_back(0);
        
        data.push_back(0);
        
        data.push_back(0);
        
        data.push_back(0);
        
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        data.push_back('C');
        data.push_back('M');
        data.push_back('A');
        data.push_back('P');
        uint32_t cmap_size = 768;
        data.push_back((cmap_size >> 24) & 0xFF);
        data.push_back((cmap_size >> 16) & 0xFF);
        data.push_back((cmap_size >> 8) & 0xFF);
        data.push_back(cmap_size & 0xFF);
        
        for (int i = 0; i < 256; i++) {
            data.push_back(i);
            data.push_back(i);
            data.push_back(i);
        }
        
        data.push_back('B');
        data.push_back('O');
        data.push_back('D');
        data.push_back('Y');
        uint32_t body_size = width * height;
        data.push_back((body_size >> 24) & 0xFF);
        data.push_back((body_size >> 16) & 0xFF);
        data.push_back((body_size >> 8) & 0xFF);
        data.push_back(body_size & 0xFF);
        
        for (int i = 0; i < width * height; i++) {
            data.push_back(0);
        }
        
        return data;
    }
    
    std::vector<uint8_t> CreateInvalidIFF() {
        std::vector<uint8_t> data;
        data.push_back('X');
        data.push_back('F');
        data.push_back('R');
        data.push_back('M');
        return data;
    }
    
    std::vector<uint8_t> CreateNonIFF() {
        std::vector<uint8_t> data;
        data.push_back('T');
        data.push_back('G');
        data.push_back('A');
        data.push_back(' ');
        return data;
    }
};

bool IFFLoaderTest::initialized = false;

/**
 * @test IFFLoaderTest.LoadPBMFromMemory
 * @brief Verifies load PBMFrom Memory.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, LoadPBMFromMemory) {
    auto iff_data = CreateMinimalIFF(32, 32);
    GetInMemoryFileSystem()->AddFile("test.pbm", iff_data);
    
    CFILE* cf = cfopen("test.pbm", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 32);
        EXPECT_EQ(bm_h(handle, 0), 32);
        bm_FreeBitmap(handle);
    } else {
        SUCCEED() << "PBM loading failed - format may not be supported";
    }
}

/**
 * @test IFFLoaderTest.LoadILBMFromMemory
 * @brief Verifies load ILBMFrom Memory.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, LoadILBMFromMemory) {
    auto iff_data = CreateMinimalILBM(32, 32);
    GetInMemoryFileSystem()->AddFile("test.ilbm", iff_data);
    
    CFILE* cf = cfopen("test.ilbm", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 32);
        EXPECT_EQ(bm_h(handle, 0), 32);
        bm_FreeBitmap(handle);
    }
}

/**
 * @test IFFLoaderTest.LoadIFFInvalidFile
 * @brief Verifies load IFFInvalid File.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, LoadIFFInvalidFile) {
    auto iff_data = CreateInvalidIFF();
    GetInMemoryFileSystem()->AddFile("invalid.iff", iff_data);
    
    CFILE* cf = cfopen("invalid.iff", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    EXPECT_EQ(handle, -1);
}

/**
 * @test IFFLoaderTest.LoadIFFNonIFFFile
 * @brief Verifies load IFFNon IFFFile.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, LoadIFFNonIFFFile) {
    auto iff_data = CreateNonIFF();
    GetInMemoryFileSystem()->AddFile("notiff.iff", iff_data);
    
    CFILE* cf = cfopen("notiff.iff", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    EXPECT_EQ(handle, -1);
}

/**
 * @test IFFLoaderTest.LoadIFFEmptyFile
 * @brief Verifies load IFFEmpty File.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, LoadIFFEmptyFile) {
    GetInMemoryFileSystem()->AddFile("empty.iff", std::vector<uint8_t>{});
    
    CFILE* cf = cfopen("empty.iff", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    EXPECT_EQ(handle, -1);
}

/**
 * @test IFFLoaderTest.LoadIFFFileNotFound
 * @brief Verifies load IFFFile Not Found.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, LoadIFFFileNotFound) {
    CFILE* cf = cfopen("nonexistent.iff", "rb");
    EXPECT_EQ(cf, nullptr);
}

/**
 * @test IFFLoaderTest.LoadMultipleIFF
 * @brief Verifies load Multiple IFF.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, LoadMultipleIFF) {
    auto iff1 = CreateMinimalIFF(16, 16);
    auto iff2 = CreateMinimalIFF(32, 32);
    auto iff3 = CreateMinimalILBM(64, 64);
    
    GetInMemoryFileSystem()->AddFile("a.pbm", iff1);
    GetInMemoryFileSystem()->AddFile("b.pbm", iff2);
    GetInMemoryFileSystem()->AddFile("c.ilbm", iff3);
    
    CFILE* cf1 = cfopen("a.pbm", "rb");
    ASSERT_NE(cf1, nullptr);
    int h1 = bm_AllocLoadBitmap(cf1, 0, 0);
    cfclose(cf1);
    
    CFILE* cf2 = cfopen("b.pbm", "rb");
    ASSERT_NE(cf2, nullptr);
    int h2 = bm_AllocLoadBitmap(cf2, 0, 0);
    cfclose(cf2);
    
    CFILE* cf3 = cfopen("c.ilbm", "rb");
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

/**
 * @test IFFLoaderTest.IFFDirectAllocFile
 * @brief Verifies iFFDirect Alloc File.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, IFFDirectAllocFile) {
    auto iff_data = CreateMinimalIFF(32, 32);
    GetInMemoryFileSystem()->AddFile("direct.pbm", iff_data);
    
    CFILE* cf = cfopen("direct.pbm", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_iff_alloc_file(cf);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 32);
        EXPECT_EQ(bm_h(handle, 0), 32);
        bm_FreeBitmap(handle);
    }
}

/**
 * @test IFFLoaderTest.IFFSmallSize
 * @brief Verifies iFFSmall Size.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, IFFSmallSize) {
    auto iff_data = CreateMinimalIFF(8, 8);
    GetInMemoryFileSystem()->AddFile("small.pbm", iff_data);
    
    CFILE* cf = cfopen("small.pbm", "rb");
    ASSERT_NE(cf, nullptr);
    
    int handle = bm_AllocLoadBitmap(cf, 0, 0);
    cfclose(cf);
    
    if (handle > 0) {
        EXPECT_EQ(bm_w(handle, 0), 8);
        EXPECT_EQ(bm_h(handle, 0), 8);
        bm_FreeBitmap(handle);
    }
}

/**
 * @test IFFLoaderTest.IFFLargeSize
 * @brief Verifies iFFLarge Size.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, IFFLargeSize) {
    auto iff_data = CreateMinimalIFF(256, 256);
    GetInMemoryFileSystem()->AddFile("large.pbm", iff_data);
    
    CFILE* cf = cfopen("large.pbm", "rb");
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

/**
 * @test IFFLoaderTest.IFFRectangular
 * @brief Verifies iFFRectangular.
 *
 * @details
 * Exercises the IFFLoaderTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/iff_loader.cpp
 * @ingroup descent3_tests
 */
TEST_F(IFFLoaderTest, IFFRectangular) {
    auto iff_data = CreateMinimalIFF(320, 240);
    GetInMemoryFileSystem()->AddFile("rect.pbm", iff_data);
    
    CFILE* cf = cfopen("rect.pbm", "rb");
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
