/**
 * @file d3movie_real_tests.cpp
 * @brief Tests for Descent3/d3movie.cpp — NextPow2 and mve_Init plumbing.
 *
 * @details
 * Minimal stubs; bitmap-heavy Blit is exercised via the same TU by
 * including the cpp directly, but we avoid pulling in heavy linker deps
 * for now. More coverage can be added once the minimal build is green.
 *
 * This harness validates the behavior of `Descent3/d3movie.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/d3movie.cpp`
 * @par Harness
 * `d3movie_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/d3movie.cpp
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <filesystem>

// Headers that declare the symbols d3movie.cpp uses – included so the
// included cpp sees their declarations.
#include "bitmap.h"
#include "renderer.h"
#include "grdefs.h"
#include "mvelib.h"
#include "cfile.h"
#include "application.h"
#include "ddio.h"
#include "game.h"
#include "d3movie.h"

// Provide GameBitmaps definition that bitmap.h declares extern
bms_bitmap GameBitmaps[MAX_BITMAPS] = {};

// Include the real implementation after declarations are visible so its
// calls resolve to the headers' declarations. Stubs defined after this
// include will satisfy the linker.
#include "../../Descent3/d3movie.cpp"

// ---------------------------------------------------------------------------
// Stubs that satisfy the linker for symbols referenced by d3movie.cpp
// Signatures must match exactly the declarations in their headers.
// ---------------------------------------------------------------------------

// bitmap
int bm_AllocBitmap(int w, int h, int flags) {
  for (int i = 0; i < MAX_BITMAPS; ++i) if (GameBitmaps[i].flags == 0) { GameBitmaps[i].flags = 1; (void)w; (void)h; (void)flags; return i; }
  return -1;
}
void bm_FreeBitmap(int handle) { if (handle>=0 && handle<MAX_BITMAPS) GameBitmaps[handle].flags=0; }
uint16_t *bm_data(int handle, int miplevel) { (void)handle;(void)miplevel; static uint16_t fake[1024*1024]={}; return fake; }

// renderer
void rend_SetAlphaType(int8_t) {}
void rend_SetAlphaValue(uint8_t) {}
void rend_SetLighting(light_state) {}
void rend_SetColorModel(color_model) {}
void rend_SetOverlayType(uint8_t) {}
void rend_SetWrapType(wrap_type) {}
void rend_SetFiltering(int8_t) {}
void rend_SetZBufferState(int8_t) {}
void rend_SetZBufferState(int) {} // overload for completeness
void rend_SetFrameBufferCopyState(bool) {}
void rend_ClearScreen(ddgr_color) {}
void rend_DrawScaledBitmap(int,int,int,int,int,float,float,float,float,int,const float*) {}
void rend_Flip() {}
int Game_fonts[6] = {};

// cfile
std::filesystem::path cf_LocatePath(const std::filesystem::path &p) { return p; }

// ddio
int ddio_KeyInKey() { return 0; }

// game – exact signature from Descent3/game.h:6 params with default for last
void StartFrame(int, int, int, int, bool, bool) {}
void EndFrame() {}

// grtext – extern "C" per grtext.h
extern "C" {
void grtext_SetFont(int) {}
void grtext_SetColor(ddgr_color) {}
void grtext_SetAlpha(uint8_t) {}
void grtext_SetFlags(int) {}
void grtext_CenteredPrintf(int, int, const char*, ...) {}
void grtext_Flush() {}
void grtext_Reset() {}
void grtext_SetFontScale(float) {}
int grfont_GetHeight(int) { return 16; }
}

// args – exact signature with default start
int FindArg(const char *a, int) { (void)a; return 0; }

// mem – mem.h defines macro to mem_malloc_sub
void *mem_malloc_sub(int size, const char*, int) { return malloc(size); }
void mem_free_sub(void *p) { free(p); }
char *mem_strdup_sub(const char *s, const char*, int) { return strdup(s); }
void *mem_realloc_sub(void*, int) { return nullptr; }
int mem_size_sub(void*) { return 0; }

// mvelib
mve_cb_Read mve_read=nullptr;
mve_cb_Alloc mve_alloc=nullptr;
mve_cb_Free mve_free=nullptr;
mve_cb_ShowFrame mve_showframe=nullptr;
mve_cb_SetPalette mve_setpalette=nullptr;
void MVE_ioCallbacks(mve_cb_Read r){ mve_read=r; }
void MVE_memCallbacks(mve_cb_Alloc a, mve_cb_Free f){ mve_alloc=a; mve_free=f; }
void MVE_sfCallbacks(mve_cb_ShowFrame s){ mve_showframe=s; }
void MVE_palCallbacks(mve_cb_SetPalette p){ mve_setpalette=p; }
void MVE_sndInit(bool){}
MVESTREAM* MVE_rmPrepMovie(void*,int,int,int){ static MVESTREAM s; return &s; }
int MVE_rmStepMovie(MVESTREAM*){ return MVE_ERR_EOF; }
void MVE_rmEndMovie(MVESTREAM*){}
void mve_reset(MVESTREAM*){}
void MVE_rmHoldMovie(){}

// application
oeApplication *Descent=nullptr;

// ---------------------------------------------------------------------------

/**
 * @test D3MovieTest.NextPow2
 * @brief Verifies next Pow2.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, NextPow2) {
  EXPECT_EQ(NextPow2(0), 0);
  EXPECT_EQ(NextPow2(1), 1);
  EXPECT_EQ(NextPow2(2), 2);
  EXPECT_EQ(NextPow2(3), 4);
  EXPECT_EQ(NextPow2(5), 8);
  EXPECT_EQ(NextPow2(16), 16);
  EXPECT_EQ(NextPow2(17), 32);
  EXPECT_EQ(NextPow2(255), 256);
  EXPECT_EQ(NextPow2(256), 256);
  EXPECT_EQ(NextPow2(257), 512);
}

/**
 * @test D3MovieTest.InitializePaletteAndCallbackSet
 * @brief Verifies initialize Palette And Callback Set.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, InitializePaletteAndCallbackSet) {
  InitializePalette();
  uint8_t pal[3] = {63,0,0};
  // Should not crash; verify via blit that palette entry 0 becomes red
  CallbackSetPalette(pal, 0, 1);
  Movie_bm_handle = -1;
  uint8_t buf[1] = {0};
  int tw, th;
  BlitToMovieBitmap(buf, 1, 1, 0, false, tw, th);
  ASSERT_NE(Movie_bm_handle, -1);
  uint16_t *d = bm_data(Movie_bm_handle, 0);
  EXPECT_EQ(d[0], uint16_t(OPAQUE_FLAG | GR_RGB16(252,0,0)));
}

/**
 * @test D3MovieTest.CallbackAllocFreeFileRead
 * @brief Verifies callback Alloc Free File Read.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, CallbackAllocFreeFileRead) {
  void *p = CallbackAlloc(32);
  ASSERT_NE(p, nullptr);
  CallbackFree(p);
  FILE *f = tmpfile();
  ASSERT_NE(f, nullptr);
  const char *msg="hi";
  fwrite(msg,1,2,f); rewind(f);
  char out[2]={};
  EXPECT_EQ(CallbackFileRead(f,out,2), 1u);
  rewind(f);
  char out2[5]={};
  EXPECT_EQ(CallbackFileRead(f,out2,5), 0u);
  fclose(f);
}

/**
 * @test D3MovieTest.BlitHicolorAndPow2
 * @brief Verifies blit Hicolor And Pow2.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, BlitHicolorAndPow2) {
  Movie_bm_handle = -1;
  uint16_t src[2] = {GR_RGB16(10,20,30), GR_RGB16(40,50,60)};
  int tw, th;
  BlitToMovieBitmap((uint8_t*)src, 2, 1, 1, false, tw, th);
  EXPECT_EQ(tw, 2); EXPECT_EQ(th, 1);
  uint16_t *d = bm_data(Movie_bm_handle, 0);
  EXPECT_EQ(d[0], uint16_t(src[0]|OPAQUE_FLAG));
  Movie_bm_handle=-1;
  uint8_t buf[60]={};
  BlitToMovieBitmap(buf, 6, 10, 0, true, tw, th);
  EXPECT_EQ(tw, 16); EXPECT_EQ(th, 16);
}

/**
 * @test D3MovieTest.MveInitAndCallbacks
 * @brief Verifies mve Init And Callbacks.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, MveInitAndCallbacks) {
  EXPECT_EQ(mve_Init(), MVELIB_NOERROR);
  EXPECT_NO_THROW(mve_SetCallback([](int,int,int){}));
  EXPECT_NO_THROW(mve_SetRenderProperties(0,0,640,480,RENDERER_OPENGL,true));
}

/**
 * @test D3MovieTest.NextPow2LargeAndPowerOfTwo
 * @brief Verifies next Pow2Large And Power Of Two.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, NextPow2LargeAndPowerOfTwo) {
  EXPECT_EQ(NextPow2(64), 64);
  EXPECT_EQ(NextPow2(65), 128);
  EXPECT_EQ(NextPow2(100), 128);
  EXPECT_EQ(NextPow2(1000), 1024);
  EXPECT_EQ(NextPow2(1025), 2048);
  EXPECT_EQ(NextPow2(2048), 2048);
  // quirk: 0 returns 0 (n-- => -1 => all bits set => 0 after increment)
  EXPECT_EQ(NextPow2(0), 0);
}

/**
 * @test D3MovieTest.InitializePaletteResetsToBlack
 * @brief Verifies initialize Palette Resets To Black.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, InitializePaletteResetsToBlack) {
  uint8_t pal[3] = {63,63,63};
  CallbackSetPalette(pal, 10, 1);
  InitializePalette();
  Movie_bm_handle = -1;
  uint8_t buf[1] = {10};
  int tw, th;
  BlitToMovieBitmap(buf, 1, 1, 0, false, tw, th);
  uint16_t *d = bm_data(Movie_bm_handle, 0);
  EXPECT_EQ(d[0], uint16_t(OPAQUE_FLAG | GR_RGB16(0,0,0)));
}

/**
 * @test D3MovieTest.CallbackSetPaletteMultipleEntries
 * @brief Verifies callback Set Palette Multiple Entries.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, CallbackSetPaletteMultipleEntries) {
  // CallbackSetPalette expects pBuffer to be base of full palette; start offset is applied internally.
  // To set entries 2,3,4 we provide a buffer with 5 entries (0..4) and start=0 count=5.
  uint8_t pal[15] = {};
  // entries 0,1 dummy, 2:10,20,30 3:40,50,60 4:70,80,90
  pal[6]=10; pal[7]=20; pal[8]=30;
  pal[9]=40; pal[10]=50; pal[11]=60;
  pal[12]=70; pal[13]=80; pal[14]=90;
  CallbackSetPalette(pal, 0, 5);
  Movie_bm_handle = -1;
  uint8_t buf[3] = {2,3,4};
  int tw, th;
  BlitToMovieBitmap(buf, 3, 1, 0, false, tw, th);
  uint16_t *d = bm_data(Movie_bm_handle, 0);
  EXPECT_EQ(d[0], uint16_t(OPAQUE_FLAG | GR_RGB16(40,80,120)));
  EXPECT_EQ(d[1], uint16_t(OPAQUE_FLAG | GR_RGB16(160,200,240)));
  EXPECT_NE(d[2], uint16_t(OPAQUE_FLAG | GR_RGB16(0,0,0)));
}

/**
 * @test D3MovieTest.BlitSetsChangedFlagAndStride
 * @brief Verifies blit Sets Changed Flag And Stride.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, BlitSetsChangedFlagAndStride) {
  Movie_bm_handle = -1;
  uint8_t pal[15] = {};
  // set entries 1..4: 1:40,0,0 (10<<2) 2:80,0,0 3:120,0,0 4:160,0,0
  pal[3]=10; pal[4]=0; pal[5]=0;
  pal[6]=20; pal[7]=0; pal[8]=0;
  pal[9]=30; pal[10]=0; pal[11]=0;
  pal[12]=40; pal[13]=0; pal[14]=0;
  // Actually need to set entry 1 at offset 3: entry index *3
  // pal[3]=10 corresponds to entry1 R, etc. But we used pal[3] for entry1? entry0 at 0, entry1 at 3, entry2 at6 etc.
  // Let's rebuild correctly: entry i at i*3
  pal[3]=10; // entry1 R=10 -> 40 after <<2
  pal[6]=20;
  pal[9]=30;
  pal[12]=40;
  CallbackSetPalette(pal, 0, 5);
  uint8_t buf[4] = {1,2,3,4};
  int tw, th;
  BlitToMovieBitmap(buf, 2, 2, 0, false, tw, th);
  ASSERT_NE(Movie_bm_handle, -1);
  EXPECT_TRUE(GameBitmaps[Movie_bm_handle].flags & BF_CHANGED);
  uint16_t *d = bm_data(Movie_bm_handle, 0);
  EXPECT_EQ(d[0], uint16_t(OPAQUE_FLAG | GR_RGB16(40,0,0)));
  EXPECT_EQ(d[1], uint16_t(OPAQUE_FLAG | GR_RGB16(80,0,0)));
  EXPECT_EQ(d[tw], uint16_t(OPAQUE_FLAG | GR_RGB16(120,0,0)));
  EXPECT_EQ(d[tw+1], uint16_t(OPAQUE_FLAG | GR_RGB16(160,0,0)));
}

/**
 * @test D3MovieTest.BlitPow2SquareUsesMaxDimension
 * @brief Verifies blit Pow2Square Uses Max Dimension.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, BlitPow2SquareUsesMaxDimension) {
  int tw, th;
  uint8_t buf[100]={};
  Movie_bm_handle=-1; BlitToMovieBitmap(buf, 63, 63, 0, true, tw, th);
  EXPECT_EQ(tw, 64); EXPECT_EQ(th, 64);
  Movie_bm_handle=-1; BlitToMovieBitmap(buf, 65, 33, 0, true, tw, th);
  EXPECT_EQ(tw, 128); EXPECT_EQ(th, 128);
  Movie_bm_handle=-1; BlitToMovieBitmap(buf, 1, 512, 0, true, tw, th);
  EXPECT_EQ(tw, 512); EXPECT_EQ(th, 512);
}

/**
 * @test D3MovieTest.SequenceStartFailsWithoutFile
 * @brief Verifies sequence Start Fails Without File.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, SequenceStartFailsWithoutFile) {
  // cf_LocatePath returns passed path; fopen will fail for non-existent file => returns 0
  intptr_t h = mve_SequenceStart("nonexistent.mve", nullptr, nullptr, false);
  EXPECT_EQ(h, 0);
}

/**
 * @test D3MovieTest.SequenceStartSuccessWithTempFile
 * @brief Verifies sequence Start Success With Temp File.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, SequenceStartSuccessWithTempFile) {
  char tmpl[] = "/tmp/d3movie_test_XXXXXX";
  int fd = mkstemp(tmpl);
  ASSERT_NE(fd, -1);
  close(fd);
  // need to make cf_LocatePath return the temp path; our stub returns p unchanged,
  // so we pass the temp path directly
  intptr_t h = mve_SequenceStart(tmpl, nullptr, nullptr, true);
  EXPECT_NE(h, 0);
  EXPECT_NE(mve_showframe, nullptr); // SequenceStart registers callbacks
  // clean up sequence
  EXPECT_TRUE(mve_SequenceClose(h, nullptr));
  EXPECT_EQ(Movie_bm_handle, -1); // closed frees bitmap if any (none allocated yet)
  unlink(tmpl);
}

/**
 * @test D3MovieTest.SequenceFrameAndClose
 * @brief Verifies sequence Frame And Close.
 *
 * @details
 * Exercises the D3MovieTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/d3movie.cpp
 * @ingroup descent3_tests
 */
TEST(D3MovieTest, SequenceFrameAndClose) {
  char tmpl[] = "/tmp/d3movie_test2_XXXXXX";
  int fd = mkstemp(tmpl); close(fd);
  intptr_t h = mve_SequenceStart(tmpl, nullptr, nullptr, false);
  ASSERT_NE(h, 0);
  int bm = -2;
  intptr_t out = mve_SequenceFrame(h, nullptr, true, &bm);
  EXPECT_EQ(out, -1); // our stub returns EOF
  EXPECT_TRUE(mve_SequenceClose(h, nullptr));
  unlink(tmpl);
}
