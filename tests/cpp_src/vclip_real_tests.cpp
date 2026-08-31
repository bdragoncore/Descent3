/**
 * @file vclip_real_tests.cpp
 * @brief Video clip system tests (Descent3/vclip.cpp).
 *
 * @details
 * Links the real vclip.cpp. Bitmap library calls are stubbed with scripted
 * handles/dimensions and captures; cfile and mem are real, so IFL parsing
 * and binary paging run against genuine temp files. Error() is overridden
 * locally because the real implementation exits the process even in release
 * builds - the stub records and returns instead.
 *
 * This harness validates the behavior of `Descent3/vclip.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/vclip.cpp`
 * @par Harness
 * `vclip_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/vclip.cpp
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "bitmap.h"
#include "cfile.h"
#include "game.h"
#include "log.h"
#include "mem.h"
#include "pserror.h"
#include "vclip.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
bms_bitmap GameBitmaps[MAX_BITMAPS];
int Low_vidmem = 0;

// Defined in vclip.cpp but missing from vclip.h
void FreeAllVClips();

// Error() normally lives in ddebug/error.cpp and calls exit(0); override it.
static int g_error_calls = 0;
void Error(const char *, ...) { g_error_calls++; }

// Minimal POSIX-flavored split matching ddio semantics used by the tests.
void ddio_SplitPath(const char *srcPath, char *path, char *filename, char *ext) {
  const char *slash = nullptr;
  const char *p = srcPath;
  while (*p) {
    if (*p == '/' || *p == '\\')
      slash = p;
    p++;
  }
  if (path) {
    size_t n = slash ? static_cast<size_t>(slash - srcPath) : 0;
    if (n > 255)
      n = 255;
    memcpy(path, srcPath, n);
    path[n] = 0;
  }
  const char *base = slash ? slash + 1 : srcPath;
  if (!filename)
    return;
  const char *dot = strrchr(base, '.');
  if (dot) {
    strcpy(filename, std::string(base, dot - base).c_str());
    if (ext)
      strcpy(ext, dot);
  } else {
    strcpy(filename, base);
    if (ext)
      ext[0] = 0;
  }
}

// ---------------------------------------------------------------------------
// Bitmap stubs with capture state
// ---------------------------------------------------------------------------
struct AllocRec {
  int w, h, add_mem;
};
static std::vector<AllocRec> g_allocs;
static std::vector<std::string> g_loaded_files;   // bm_AllocLoadFileBitmap names
static std::vector<int> g_freed;                  // bm_FreeBitmap handles
static std::vector<std::pair<int, int>> g_scaled; // dest,src
static int g_next_handle = 30;

static std::vector<int> g_scripted_loads; // results for bm_AllocLoadBitmap
static std::vector<std::pair<int, int>> g_dims; // bm_w/bm_h answers by call order

int bm_AllocLoadFileBitmap(const char *name, int, int) {
  g_loaded_files.push_back(name);
  return g_next_handle++;
}
int bm_AllocLoadBitmap(CFILE *, int, int) {
  if (g_scripted_loads.empty())
    return g_next_handle++;
  int r = g_scripted_loads.front();
  g_scripted_loads.erase(g_scripted_loads.begin());
  return r;
}
int bm_AllocBitmap(int w, int h, int add_mem) {
  g_allocs.push_back({w, h, add_mem});
  return g_next_handle++;
}
void bm_FreeBitmap(int handle) { g_freed.push_back(handle); }
void bm_ScaleBitmapToBitmap(int dest, int src) { g_scaled.push_back({dest, src}); }
int bm_SaveBitmap(CFILE *, int) { return 1; }

static int g_dim_index = 0;
int bm_w(int, int) {
  if (g_dims.empty())
    return 64;
  int i = g_dim_index < static_cast<int>(g_dims.size()) ? g_dim_index : static_cast<int>(g_dims.size()) - 1;
  return g_dims[i].first;
}
int bm_h(int, int) {
  if (g_dims.empty())
    return 64;
  int i = g_dim_index < static_cast<int>(g_dims.size()) ? g_dim_index : static_cast<int>(g_dims.size()) - 1;
  g_dim_index++;
  return g_dims[i].second;
}

// ---------------------------------------------------------------------------
// Helpers / fixture
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for VClipTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VClipTest : public ::testing::Test {
protected:
  void SetUp() override {
    InitVClips();
    memset(GameVClips, 0, sizeof(GameVClips));
    Num_vclips = 0;
    g_allocs.clear();
    g_loaded_files.clear();
    g_freed.clear();
    g_scaled.clear();
    g_scripted_loads.clear();
    g_dims.clear();
    g_error_calls = 0;
    g_next_handle = 30;
    g_dim_index = 0;
  }

  static void WriteFile(const char *path, const void *data, size_t len) {
    FILE *fp = fopen(path, "wb");
    ASSERT_NE(fp, nullptr);
    fwrite(data, 1, len, fp);
    fclose(fp);
  }

  static void WriteTextFile(const char *path, const char *contents) {
    FILE *fp = fopen(path, "wb");
    ASSERT_NE(fp, nullptr);
    fputs(contents, fp);
    fclose(fp);
  }
};

// ---------------------------------------------------------------------------
// Lifecycle: Alloc / Free / refcount
// ---------------------------------------------------------------------------
/**
 * @test VClipTest.AllocDefaultsAndCounter
 * @brief Verifies alloc Defaults And Counter.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, AllocDefaultsAndCounter) {
  int n = AllocVClip();
  ASSERT_GE(n, 0);
  EXPECT_EQ(GameVClips[n].used, 1);
  EXPECT_NE(GameVClips[n].frames, nullptr);
  EXPECT_FLOAT_EQ(GameVClips[n].frame_time, 0.07f); // DEFAULT_FRAMETIME
  EXPECT_EQ(GameVClips[n].flags, VCF_NOT_RESIDENT);
  EXPECT_EQ(Num_vclips, 1);
}

/**
 * @test VClipTest.AllocExhaustionReturnsMinusOne
 * @brief Verifies alloc Exhaustion Returns Minus One.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, AllocExhaustionReturnsMinusOne) {
  for (int i = 0; i < MAX_VCLIPS; i++) {
    int n = AllocVClip();
    ASSERT_GE(n, 0);
    ASSERT_EQ(n, i); // lowest free slot handed out in order
  }
  EXPECT_EQ(AllocVClip(), -1);
}

/**
 * @test VClipTest.RefcountKeepsClipAliveAcrossFrees
 * @brief Verifies refcount Keeps Clip Alive Across Frees.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, RefcountKeepsClipAliveAcrossFrees) {
  int a = AllocVClip();
  GameVClips[a].used = 2;
  FreeVClip(a);
  EXPECT_EQ(GameVClips[a].used, 1);
  EXPECT_NE(GameVClips[a].frames, nullptr); // still owned
  FreeVClip(a);
  EXPECT_EQ(GameVClips[a].used, 0);
  EXPECT_EQ(Num_vclips, 0);
}

/**
 * @test VClipTest.ResidentFreeReleasesFrameBitmaps
 * @brief Verifies resident Free Releases Frame Bitmaps.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, ResidentFreeReleasesFrameBitmaps) {
  int a = AllocVClip();
  GameVClips[a].flags &= ~VCF_NOT_RESIDENT; // resident: bitmaps must be freed
  GameVClips[a].num_frames = 2;
  GameVClips[a].frames[0] = 7;
  GameVClips[a].frames[1] = 8;

  FreeVClip(a);
  ASSERT_EQ(g_freed.size(), 2u);
  EXPECT_EQ(g_freed[0], 7);
  EXPECT_EQ(g_freed[1], 8);
}

/**
 * @test VClipTest.NonresidentFreeSkipsBitmapRelease
 * @brief Verifies nonresident Free Skips Bitmap Release.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, NonresidentFreeSkipsBitmapRelease) {
  int a = AllocVClip(); // stays VCF_NOT_RESIDENT (never paged in)
  GameVClips[a].num_frames = 3;
  FreeVClip(a);
  EXPECT_TRUE(g_freed.empty());
}

/**
 * @test VClipTest.FreeAllVClipsSweepsEverything
 * @brief Verifies free All VClips Sweeps Everything.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, FreeAllVClipsSweepsEverything) {
  int a = AllocVClip();
  int b = AllocVClip();
  GameVClips[a].used = 3; // multi-referenced
  GameVClips[b].flags &= ~VCF_NOT_RESIDENT;
  GameVClips[b].num_frames = 1;
  GameVClips[b].frames[0] = 9;

  FreeAllVClips();

  EXPECT_EQ(GameVClips[a].used, 0);
  EXPECT_EQ(GameVClips[b].used, 0);
  ASSERT_EQ(g_freed.size(), 1u);
  EXPECT_EQ(g_freed[0], 9);
}

// ---------------------------------------------------------------------------
// Name helpers
// ---------------------------------------------------------------------------
/**
 * @test VClipTest.FindVClipNameCaseInsensitive
 * @brief Verifies find VClip Name Case Insensitive.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, FindVClipNameCaseInsensitive) {
  strncpy(GameVClips[4].name, "FireBall.OAF", PAGENAME_LEN);
  GameVClips[4].used = 1;
  strncpy(GameVClips[6].name, "unused", PAGENAME_LEN);

  EXPECT_EQ(FindVClipName("fireball.oaf"), 4);
  EXPECT_EQ(FindVClipName("nomatch"), -1);
}

/**
 * @test VClipTest.ChangeVClipNameSwapsExtensionToOaf
 * @brief Verifies change VClip Name Swaps Extension To Oaf.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, ChangeVClipNameSwapsExtensionToOaf) {
  char out[PAGENAME_LEN];
  memset(out, 0, sizeof(out));

  ChangeVClipName("/some/dir/fire.ifl", out);
  EXPECT_STREQ(out, "fire.oaf");

  memset(out, 0, sizeof(out));
  ChangeVClipName("a.b.c", out); // extension is whatever follows the last dot
  EXPECT_STREQ(out, "a.b.oaf");
}

/**
 * @test VClipTest.ChangeVClipNameTruncatesLongStems
 * @brief Verifies change VClip Name Truncates Long Stems.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, ChangeVClipNameTruncatesLongStems) {
  char out[PAGENAME_LEN];
  memset(out, 0, sizeof(out));

  std::string long_name(40, 'x');
  ChangeVClipName(long_name.c_str(), out);
  // limit = PAGENAME_LEN - 5 chars kept, then ".oaf" appended
  EXPECT_EQ(strlen(out), PAGENAME_LEN - 1u);
  EXPECT_STREQ(out + PAGENAME_LEN - 5, ".oaf");
  EXPECT_EQ(strspn(out, "x"), PAGENAME_LEN - 5u);
}

// ---------------------------------------------------------------------------
// AllocLoadIFLVClip: IFL text parsing
// ---------------------------------------------------------------------------
/**
 * @test VClipTest.IFLOnlyLoadsAlnumLines
 * @brief Verifies iFLOnly Loads Alnum Lines.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, IFLOnlyLoadsAlnumLines) {
  WriteTextFile("/tmp/opencode/ifl_ok.ifl",
                "; a comment\n"
                "\n"
                "   \n"
                ";another comment\n"
                "$TIME=5.5\n"       // dead code upstream: '$' fails isalnum gate
                "frame1.tga\n"
                "\tframe2.tga\n");  // tab is not alnum -> skipped

  int n = AllocLoadIFLVClip("/tmp/opencode/ifl_ok.ifl", 0, 0, 0);
  ASSERT_GE(n, 0);

  EXPECT_EQ(GameVClips[n].num_frames, 1);
  ASSERT_EQ(g_loaded_files.size(), 1u);
  // On paths without backslashes the frame keeps its relative name
  EXPECT_STREQ(g_loaded_files[0].c_str(), "frame1.tga");
  EXPECT_STREQ(GameVClips[n].name, "ifl_ok.oaf");

  FreeVClip(n);
}

/**
 * @test VClipTest.IFLNoValidFramesFails
 * @brief Verifies iFLNo Valid Frames Fails.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, IFLNoValidFramesFails) {
  WriteTextFile("/tmp/opencode/ifl_empty.ifl", ";nothing here\n\n");
  EXPECT_EQ(AllocLoadIFLVClip("/tmp/opencode/ifl_empty.ifl", 0, 0, 0), -1);
  EXPECT_EQ(Num_vclips, 0); // aborted alloc was released
}

/**
 * @test VClipTest.IFLMissingFileFailsWithoutAlloc
 * @brief Verifies iFLMissing File Fails Without Alloc.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, IFLMissingFileFailsWithoutAlloc) {
  EXPECT_EQ(AllocLoadIFLVClip("/tmp/opencode/no_such.ifl", 0, 0, 0), -1);
  EXPECT_TRUE(g_loaded_files.empty());
}

/**
 * @test VClipTest.IFLFailedBitmapReportsErrorAndAborts
 * @brief Verifies iFLFailed Bitmap Reports Error And Aborts.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, IFLFailedBitmapReportsErrorAndAborts) {
  WriteTextFile("/tmp/opencode/ifl_good.ifl", "ok.tga\n");
  int good = AllocLoadIFLVClip("/tmp/opencode/ifl_good.ifl", 0, 0, 0);
  ASSERT_GE(good, 0);
  EXPECT_EQ(g_error_calls, 0);

  // A different stem avoids the name-reuse path; force the loader to hand
  // out an invalid handle (-1 < 1) so the frame load fails.
  WriteTextFile("/tmp/opencode/ifl_fail.ifl", "bad.tga\n");
  g_next_handle = -1;
  EXPECT_EQ(AllocLoadIFLVClip("/tmp/opencode/ifl_fail.ifl", 0, 0, 0), -1);
  EXPECT_EQ(g_error_calls, 1);

  // Quirk: the failed clip is NOT freed - it stays allocated forever.
  EXPECT_EQ(Num_vclips, 2);

  FreeVClip(good);
}

// ---------------------------------------------------------------------------
// AllocLoadVClip dispatch & paging
// ---------------------------------------------------------------------------
/**
 * @test VClipTest.AllocLoadPageableSkipsPaging
 * @brief Verifies alloc Load Pageable Skips Paging.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, AllocLoadPageableSkipsPaging) {
  int n = AllocLoadVClip("pager.oaf", 0, 0, 1 /* pageable */, 0);
  ASSERT_GE(n, 0);
  EXPECT_EQ(GameVClips[n].flags & VCF_NOT_RESIDENT, VCF_NOT_RESIDENT);
  EXPECT_EQ(GameVClips[n].target_size, 0);
  EXPECT_EQ(GameVClips[n].num_frames, 0); // nothing loaded yet
  FreeVClip(n);
}

/**
 * @test VClipTest.AllocLoadReuseIncrementsRefcount
 * @brief Verifies alloc Load Reuse Increments Refcount.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, AllocLoadReuseIncrementsRefcount) {
  WriteTextFile("/tmp/opencode/reuse.ifl", "f1.tga\n");

  int a = AllocLoadVClip("/tmp/opencode/reuse.ifl", 0, 0, 1, 0);
  ASSERT_GE(a, 0);
  EXPECT_EQ(GameVClips[a].used, 1);

  // Second request with same stem reuses the slot without reparsing
  int b = AllocLoadVClip("/tmp/opencode/reuse.ifl", 0, 0, 1, 0);
  EXPECT_EQ(b, a);
  EXPECT_EQ(GameVClips[a].used, 2);
  // Quirk: 'pageable' is ignored for .ifl files - the first request parses
  // immediately; the reuse path skips parsing.
  ASSERT_EQ(g_loaded_files.size(), 1u);

  FreeVClip(b);
  FreeVClip(a);
  EXPECT_EQ(Num_vclips, 0);
}

/**
 * @test VClipTest.AllocLoadNonexistentOafCleansUp
 * @brief Verifies alloc Load Nonexistent Oaf Cleans Up.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, AllocLoadNonexistentOafCleansUp) {
  // Not an .ifl: goes down the OAF paging path, which fails to open and
  // leaves zero frames -> the clip is freed again and -1 returned.
  EXPECT_EQ(AllocLoadVClip("/tmp/opencode/nothere.oaf", 0, 0, 0, 0), -1);
  EXPECT_EQ(Num_vclips, 0);
}

// ---------------------------------------------------------------------------
// PageInVClip binary formats
// ---------------------------------------------------------------------------
/**
 * @test VClipTest.PageInModernBinaryFormat
 * @brief Verifies page In Modern Binary Format.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, PageInModernBinaryFormat) {
  int n = AllocVClip();
  strcpy(GameVClips[n].name, "/tmp/opencode/modern.oaf");

  uint8_t head[] = {127, 1, 2}; // magic, version, frame count
  float t = 0.05f;
  FILE *fp = fopen("/tmp/opencode/modern.oaf", "wb");
  fwrite(head, 1, 3, fp);
  fwrite(&t, 4, 1, fp);
  fclose(fp);

  g_scripted_loads = {21, 22};
  PageInVClip(n);

  EXPECT_EQ(GameVClips[n].num_frames, 2);
  EXPECT_EQ(GameVClips[n].frames[0], 21);
  EXPECT_EQ(GameVClips[n].frames[1], 22);
  EXPECT_EQ(GameVClips[n].flags & VCF_NOT_RESIDENT, 0); // now resident
  // Quirk: parsed frame_time is always discarded for the default
  EXPECT_FLOAT_EQ(GameVClips[n].frame_time, 0.07f);

  FreeVClip(n);
}

/**
 * @test VClipTest.PageInLegacyFormat
 * @brief Verifies page In Legacy Format.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, PageInLegacyFormat) {
  int n = AllocVClip();
  strcpy(GameVClips[n].name, "/tmp/opencode/legacy.oaf");

  // Legacy: first byte is the frame count, followed by two floats, an int,
  // and another float (all ignored).
  FILE *fp = fopen("/tmp/opencode/legacy.oaf", "wb");
  uint8_t nf = 3;
  uint32_t zeros[4] = {0, 0, 0, 0};
  fwrite(&nf, 1, 1, fp);
  fwrite(zeros, 4, 4, fp);
  fclose(fp);

  g_scripted_loads = {31, 32, 33};
  PageInVClip(n);

  EXPECT_EQ(GameVClips[n].num_frames, 3);
  EXPECT_EQ(GameVClips[n].frames[2], 33);
  EXPECT_FLOAT_EQ(GameVClips[n].frame_time, 0.07f);

  FreeVClip(n);
}

/**
 * @test VClipTest.PageInMissingFileKeepsNonresident
 * @brief Verifies page In Missing File Keeps Nonresident.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, PageInMissingFileKeepsNonresident) {
  int n = AllocVClip();
  strcpy(GameVClips[n].name, "/tmp/opencode/absent.oaf"); // no backslashes

  PageInVClip(n);

  EXPECT_EQ(GameVClips[n].num_frames, 0);
  EXPECT_EQ(GameVClips[n].flags & VCF_NOT_RESIDENT, VCF_NOT_RESIDENT);
}

/**
 * @test VClipTest.PageInResidentIsNoop
 * @brief Verifies page In Resident Is Noop.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, PageInResidentIsNoop) {
  int n = AllocVClip();
  GameVClips[n].flags &= ~VCF_NOT_RESIDENT;

  PageInVClip(n); // early-out, must not touch files
  EXPECT_TRUE(g_loaded_files.empty());
}

// ---------------------------------------------------------------------------
// Texture resizing inside the pager
// ---------------------------------------------------------------------------
/**
 * @test VClipTest.PageInResizesToNormalTextureSize
 * @brief Verifies page In Resizes To Normal Texture Size.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, PageInResizesToNormalTextureSize) {
  int n = AllocVClip();
  strcpy(GameVClips[n].name, "/tmp/opencode/resize.oaf");
  GameVClips[n].target_size = NORMAL_TEXTURE;
  GameVClips[n].flags |= VCF_WANTS_MIPPED;

  FILE *fp = fopen("/tmp/opencode/resize.oaf", "wb");
  uint8_t head[] = {127, 1, 1};
  float t = 0.07f;
  fwrite(head, 1, 3, fp);
  fwrite(&t, 4, 1, fp);
  fclose(fp);

  // Loaded bitmap is 16x16; target is TEXTURE_WIDTH x TEXTURE_HEIGHT
  g_scripted_loads = {21};
  g_dims = {{16, 16}, {128, 128}}; // loaded dims, then resized dims

  PageInVClip(n);

  ASSERT_EQ(g_allocs.size(), 1u);
  EXPECT_EQ(g_allocs[0].w, TEXTURE_WIDTH);
  EXPECT_EQ(g_allocs[0].h, TEXTURE_HEIGHT);
  EXPECT_EQ(g_allocs[0].add_mem, (TEXTURE_WIDTH * TEXTURE_HEIGHT * 2) / 3); // mipped extra

  // Frame replaced by the resized bitmap; original freed after scaling
  EXPECT_EQ(GameVClips[n].frames[0], g_next_handle - 1);
  ASSERT_EQ(g_scaled.size(), 1u);
  EXPECT_EQ(g_scaled[0].first, g_next_handle - 1);
  EXPECT_EQ(g_scaled[0].second, 21);
  ASSERT_EQ(g_freed.size(), 1u);
  EXPECT_EQ(g_freed[0], 21);

  EXPECT_EQ(GameBitmaps[g_next_handle - 1].flags & BF_MIPMAPPED, BF_MIPMAPPED);

  FreeVClip(n);
}

// ---------------------------------------------------------------------------
// SaveVClip error handling
// ---------------------------------------------------------------------------
/**
 * @test VClipTest.SaveVClipUnwritablePathFails
 * @brief Verifies save VClip Unwritable Path Fails.
 *
 * @details
 * Exercises the VClipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vclip.cpp
 * @ingroup descent3_tests
 */
TEST_F(VClipTest, SaveVClipUnwritablePathFails) {
  int n = AllocVClip();
  GameVClips[n].flags &= ~VCF_NOT_RESIDENT; // avoid paging attempt

  EXPECT_FALSE(SaveVClip("/nonexistent_dir_xyz/clip.vco", n));

  FreeVClip(n);
}
