/**
 * @file gametexture_real_tests.cpp
 * @brief Tests for gametexture.cpp — texture paging pool (870 lines).
 *
 * @details
 * Covers InitTextures, AllocTexture defaults, FindTextureName
 * case-insensitive with Num_textures optimization, FreeTexture,
 * and circular GetNext/GetPrevious.
 *
 * This harness validates the behavior of `Descent3/gametexture.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gametexture.cpp`
 * @par Harness
 * `gametexture_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gametexture.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>

// Provide headers for structs so we can define globals expected by gametexture.o
#include "bitmap.h"
#include "vclip.h"
#include "gametexture.h"

// Globals & stubs required to link Descent3/gametexture.cpp
// Some are defined in other libs; we provide trivial definitions here.

// bitmap/vclip globals
bms_bitmap GameBitmaps[MAX_BITMAPS];
uint32_t Bitmap_memory_used = 0;
vclip GameVClips[MAX_VCLIPS];
int Num_vclips = 0;

// procedural / bump stubs
void InitProcedurals() {}
int bump_AllocBumpmap(int w, int h) { (void)w; (void)h; return -1; }
void bump_FreeBumpmap(int handle) { (void)handle; }
void *bump_data(int handle) { (void)handle; return nullptr; }

// vclip paging
void PageInVClip(int vcnum) { (void)vcnum; }
int AllocLoadVClip(const char *filename, int texture_size, int mipped, int pageable, int format) {
  (void)filename; (void)texture_size; (void)mipped; (void)pageable; (void)format; return -1;
}

// bitmap helpers used by gametexture's PageInTexture, LoadTextureImage etc.
// We won't exercise those paths, but linker needs them.
int bm_AllocBitmap(int w, int h, int add_mem) { (void)w; (void)h; (void)add_mem; return -1; }
void bm_FreeBitmap(int handle) { (void)handle; }
int bm_w(int handle, int miplevel) { (void)handle; (void)miplevel; return 0; }
int bm_h(int handle, int miplevel) { (void)handle; (void)miplevel; return 0; }
uint16_t *bm_data(int handle, int miplevel) { (void)handle; (void)miplevel; return nullptr; }
int bm_mipped(int handle) { (void)handle; return 0; }
void bm_ScaleBitmapToBitmap(int dest, int src) { (void)dest; (void)src; }
int bm_SetBitmapIfTransparent(int handle) { (void)handle; return 0; }
int bm_AllocLoadFileBitmap(const char *filename, int mipped, int format) { (void)filename; (void)mipped; (void)format; return -1; }
int bm_AllocLoadFileNoMemBitmap(const char *filename, int mipped, int format) { (void)filename; (void)mipped; (void)format; return -1; }

// args
int FindArg(const char *which, int start) { (void)which; (void)start; return 0; }
int FindArgChar(const char *which, char singleCharArg) { (void)which; (void)singleCharArg; return 0; }

// timers / game globals — gametexture uses timer_GetTime via macro to D3::ChronoTimer::GetTime()
#include "chrono_timer.h"
namespace D3 {
float ChronoTimer::GetTime() { return 0.0f; }
void ChronoTimer::Initialize() {}
int64_t ChronoTimer::GetTimeMS() { return 0; }
int64_t ChronoTimer::GetTimeUS() { return 0; }
void ChronoTimer::SleepMS(int d) { (void)d; }
}
void EvaluateProcedural(int handle) { (void)handle; }
uint16_t DefaultProcPalette[512] = {0};
int Mem_superlow_memory_mode = 0;
int Mem_low_memory_mode = 0;
int Low_vidmem = 0;
float Gametime = 0;
int FrameCount = 0;

// Detail_settings defined in config.h — need struct definition
#include "config.h"
tDetailSettings Detail_settings;

// helper to reset pool to empty without calling InitTextures
static void ResetPoolEmpty() {
  for (int i = 0; i < MAX_TEXTURES; ++i) GameTextures[i].used = 0;
  Num_textures = 0;
}

/**
 * @brief GTest fixture for GametextureTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class GametextureTest : public ::testing::Test {
protected:
  void SetUp() override { ResetPoolEmpty(); }
};

/**
 * @test GametextureTest.InitTexturesCreatesSampleTexture
 * @brief Verifies init Textures Creates Sample Texture.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, InitTexturesCreatesSampleTexture) {
  ResetPoolEmpty();
  int ret = InitTextures();
  EXPECT_EQ(ret, 1);
  EXPECT_EQ(Num_textures, 1);
  EXPECT_EQ(GameTextures[0].used, 1);
  EXPECT_STREQ(GameTextures[0].name, "SAMPLE TEXTURE");
  EXPECT_EQ(GameTextures[0].bm_handle, BAD_BITMAP_HANDLE);
  EXPECT_EQ(GameTextures[0].bumpmap, -1);
  EXPECT_EQ(GameTextures[0].procedural, nullptr);
  // cleanup for next tests: reset to empty
  ResetPoolEmpty();
  // undo atexit registration side-effect is harmless
}

/**
 * @test GametextureTest.AllocReturnsSequentialWithDefaults
 * @brief Verifies alloc Returns Sequential With Defaults.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, AllocReturnsSequentialWithDefaults) {
  int h0 = AllocTexture();
  int h1 = AllocTexture();
  int h2 = AllocTexture();
  EXPECT_EQ(h0, 0);
  EXPECT_EQ(h1, 1);
  EXPECT_EQ(h2, 2);
  EXPECT_EQ(Num_textures, 3);
  for (int h : {h0, h1, h2}) {
    EXPECT_EQ(GameTextures[h].used, 1);
    EXPECT_EQ(GameTextures[h].bumpmap, -1);
    EXPECT_EQ(GameTextures[h].procedural, nullptr);
    EXPECT_EQ(GameTextures[h].flags, 0);
    EXPECT_FLOAT_EQ(GameTextures[h].alpha, 1.0f);
    EXPECT_FLOAT_EQ(GameTextures[h].speed, 1.0f);
    EXPECT_FLOAT_EQ(GameTextures[h].reflectivity, 0.6f);
    EXPECT_EQ(GameTextures[h].bm_handle, -1);
    EXPECT_EQ(GameTextures[h].destroy_handle, -1);
    EXPECT_EQ(GameTextures[h].corona_type, 0);
    EXPECT_EQ(GameTextures[h].name[0], '\0');
  }
}

/**
 * @test GametextureTest.AllocZeroesAndInitializes
 * @brief Verifies alloc Zeroes And Initializes.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, AllocZeroesAndInitializes) {
  int h = AllocTexture();
  GameTextures[h].flags = 999;
  GameTextures[h].alpha = 9.0f;
  strncpy(GameTextures[h].name, "tmp", PAGENAME_LEN);
  FreeTexture(h);
  EXPECT_EQ(Num_textures, 0);
  int h2 = AllocTexture();
  EXPECT_EQ(h2, h);
  EXPECT_EQ(GameTextures[h2].flags, 0);
  EXPECT_FLOAT_EQ(GameTextures[h2].alpha, 1.0f);
  EXPECT_EQ(GameTextures[h2].name[0], '\0');
}

/**
 * @test GametextureTest.FreeClearsUsedAndDecrements
 * @brief Verifies free Clears Used And Decrements.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, FreeClearsUsedAndDecrements) {
  int h = AllocTexture();
  strncpy(GameTextures[h].name, "MyTex", PAGENAME_LEN);
  EXPECT_EQ(Num_textures, 1);
  FreeTexture(h);
  EXPECT_EQ(GameTextures[h].used, 0);
  EXPECT_EQ(GameTextures[h].name[0], '\0');
  EXPECT_EQ(Num_textures, 0);
}

/**
 * @test GametextureTest.AllocReusesLowestFreedSlot
 * @brief Verifies alloc Reuses Lowest Freed Slot.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, AllocReusesLowestFreedSlot) {
  int h0 = AllocTexture(); //0
  int h1 = AllocTexture(); //1
  int h2 = AllocTexture(); //2
  (void)h0;
  FreeTexture(h1);
  EXPECT_EQ(Num_textures, 2);
  int h3 = AllocTexture();
  EXPECT_EQ(h3, 1);
  EXPECT_EQ(Num_textures, 3);
  FreeTexture(h0);
  FreeTexture(h2);
  FreeTexture(h3);
  EXPECT_EQ(Num_textures, 0);
  EXPECT_EQ(AllocTexture(), 0);
}

/**
 * @test GametextureTest.FindTextureNameCaseInsensitiveIgnoresUnused
 * @brief Verifies find Texture Name Case Insensitive Ignores Unused.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, FindTextureNameCaseInsensitiveIgnoresUnused) {
  int h0 = AllocTexture();
  int h1 = AllocTexture();
  strncpy(GameTextures[h0].name, "Alpha", PAGENAME_LEN);
  strncpy(GameTextures[h1].name, "BETA", PAGENAME_LEN);
  EXPECT_EQ(FindTextureName("alpha"), h0);
  EXPECT_EQ(FindTextureName("ALPHA"), h0);
  EXPECT_EQ(FindTextureName("AlPhA"), h0);
  EXPECT_EQ(FindTextureName("beta"), h1);
  EXPECT_EQ(FindTextureName("BETA"), h1);
  EXPECT_EQ(FindTextureName("gamma"), -1);
  FreeTexture(h0);
  EXPECT_EQ(FindTextureName("alpha"), -1);
  // unused slot with matching name must not be found
  strncpy(GameTextures[80].name, "gamma", PAGENAME_LEN);
  GameTextures[80].used = 0;
  EXPECT_EQ(FindTextureName("gamma"), -1);
}

/**
 * @test GametextureTest.FindReturnsFirstMatch
 * @brief Verifies find Returns First Match.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, FindReturnsFirstMatch) {
  int h0 = AllocTexture();
  int h1 = AllocTexture();
  strncpy(GameTextures[h0].name, "dup", PAGENAME_LEN);
  strncpy(GameTextures[h1].name, "dup", PAGENAME_LEN);
  EXPECT_EQ(FindTextureName("dup"), h0);
}

/**
 * @test GametextureTest.FindRespectsNumTexturesOptimization
 * @brief Verifies find Respects Num Textures Optimization.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, FindRespectsNumTexturesOptimization) {
  // Allocate sparse: used at 0 and 50, Num=2, ensure search stops early but finds later entry
  ResetPoolEmpty();
  GameTextures[0].used = 1;
  strncpy(GameTextures[0].name, "first", PAGENAME_LEN);
  GameTextures[50].used = 1;
  strncpy(GameTextures[50].name, "later", PAGENAME_LEN);
  Num_textures = 2;
  EXPECT_EQ(FindTextureName("later"), 50);
  EXPECT_EQ(FindTextureName("first"), 0);
  EXPECT_EQ(FindTextureName("missing"), -1);
}

/**
 * @test GametextureTest.GetNextWrapsCircularly
 * @brief Verifies get Next Wraps Circularly.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, GetNextWrapsCircularly) {
  for (int i = 0; i < 10; ++i) AllocTexture(); //0..9
  for (int i : {0,1,3,4,6,7,8}) FreeTexture(i);
  // remaining 2,5,9 Num=3
  EXPECT_EQ(GetNextTexture(2), 5);
  EXPECT_EQ(GetNextTexture(5), 9);
  EXPECT_EQ(GetNextTexture(9), 2);
  EXPECT_EQ(GetNextTexture(0), 2);
  EXPECT_EQ(GetNextTexture(3), 5);
}

/**
 * @test GametextureTest.GetPreviousWrapsCircularly
 * @brief Verifies get Previous Wraps Circularly.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, GetPreviousWrapsCircularly) {
  for (int i = 0; i < 10; ++i) AllocTexture();
  for (int i : {0,1,3,4,6,7,8}) FreeTexture(i);
  EXPECT_EQ(GetPreviousTexture(9), 5);
  EXPECT_EQ(GetPreviousTexture(5), 2);
  EXPECT_EQ(GetPreviousTexture(2), 9);
  EXPECT_EQ(GetPreviousTexture(6), 5);
  EXPECT_EQ(GetPreviousTexture(0), 9);
}

/**
 * @test GametextureTest.GetNextPrevSingleReturnsSelf
 * @brief Verifies get Next Prev Single Returns Self.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, GetNextPrevSingleReturnsSelf) {
  int h = AllocTexture();
  EXPECT_EQ(GetNextTexture(h), h);
  EXPECT_EQ(GetPreviousTexture(h), h);
  EXPECT_EQ(GetNextTexture(50), h);
  EXPECT_EQ(GetPreviousTexture(50), h);
}

/**
 * @test GametextureTest.GetNextPrevEmptyReturnsMinusOne
 * @brief Verifies get Next Prev Empty Returns Minus One.
 *
 * @details
 * Exercises the GametextureTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gametexture.cpp
 * @ingroup descent3_tests
 */
TEST_F(GametextureTest, GetNextPrevEmptyReturnsMinusOne) {
  EXPECT_EQ(Num_textures, 0);
  EXPECT_EQ(GetNextTexture(0), -1);
  EXPECT_EQ(GetPreviousTexture(0), -1);
  EXPECT_EQ(GetNextTexture(50), -1);
  EXPECT_EQ(GetPreviousTexture(50), -1);
}
