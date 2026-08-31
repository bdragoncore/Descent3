/**
 * @file cinematics_real_tests.cpp
 * @brief Tests for cinematics.cpp (~171 lines).
 *
 * @details
 * Covers the movie playback front end: init gating (dedicated server,
 * mve library failure), extension normalization (.mve/.mv8), subtitle
 * lifecycle around playback, render-property propagation, the
 * StartMovie/FrameMovie/EndMovie sequence API including looped-handle
 * replacement and centered blitting, and null/uninitialized guards.
 * The mve_* library, subtitle system, bitmap sizing and renderer calls
 * are stubbed with recorders.
 *
 * This harness validates the behavior of `Descent3/cinematics.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/cinematics.cpp`
 * @par Harness
 * `cinematics_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/cinematics.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>

#include "cinematics.h"
#include "d3movie.h"
#include "bitmap.h"
#include "renderer.h"
#include "subtitles.h"
#include "dedicated_server.h"
#include "game.h"

// ---------------------------------------------------------------------------
// globals normally provided by other TUs
// ---------------------------------------------------------------------------
bool Dedicated_server = false;
int Max_window_w = 1024, Max_window_h = 768;
renderer_type Renderer_type = RENDERER_OPENGL; // any value
oeApplication *Descent = nullptr;

// ---------------------------------------------------------------------------
// mve_* stubs with recorders
// ---------------------------------------------------------------------------
static int g_mve_init_result = MVELIB_NOERROR;
static std::vector<std::string> g_play_requests;
static int g_play_result = MVELIB_NOERROR;
struct SeqStartRec {
  std::string name;
  bool looping;
};
static std::vector<SeqStartRec> g_seq_starts;
static intptr_t g_seq_start_handle = 0;
struct FrameRec {
  intptr_t in_handle;
  bool sequence;
  intptr_t out_handle;
  int bm_handle;
};
static std::vector<FrameRec> g_frame_calls;
static intptr_t g_next_frame_out = -2; // -2 = passthrough (no loop)
static int g_frame_bm_handle = -1;
static bool g_frame_end = false; // SequenceFrame reports -1
static std::vector<std::pair<intptr_t, void *>> g_seq_closes;
static MovieFrameCallback_fp g_set_callback = nullptr;

int mve_Init() { return g_mve_init_result; }
void mve_SetCallback(MovieFrameCallback_fp cb) { g_set_callback = cb; }

int mve_PlayMovie(const std::filesystem::path &pMovieName, oeApplication *) {
  g_play_requests.push_back(pMovieName.string());
  return g_play_result;
}

intptr_t mve_SequenceStart(const char *mvename, void *, oeApplication *, bool looping) {
  g_seq_starts.push_back({mvename, looping});
  return g_seq_start_handle;
}

intptr_t mve_SequenceFrame(intptr_t handle, void *, bool sequence, int *bm_handle) {
  FrameRec r{handle, sequence, g_next_frame_out, g_frame_bm_handle};
  g_frame_calls.push_back(r);
  *bm_handle = g_frame_bm_handle;
  if (g_frame_end)
    return -1; // end of stream / error
  if (g_next_frame_out != -2)
    return g_next_frame_out; // movie looped: new handle
  return handle;             // not looped: same handle passes through
}

bool mve_SequenceClose(intptr_t handle, void *fhandle) {
  g_seq_closes.push_back({handle, fhandle});
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// subtitle / renderer / bitmap stubs with recorders
// ---------------------------------------------------------------------------
struct PropsRec {
  int16_t x, y, w, h;
  renderer_type type;
  bool hicolor;
};
static std::vector<PropsRec> g_props;
static std::vector<std::pair<int, int>> g_blits; // (bm_handle, packed x<<12|y)

void mve_SetRenderProperties(int16_t x, int16_t y, int16_t w, int16_t h, renderer_type type, bool hicolor) {
  g_props.push_back({x, y, w, h, type, hicolor});
}
static std::vector<std::string> g_subt_inits;
static int g_subt_closes = 0;
static std::vector<int> g_subt_draws;
static int g_alpha_sets = 0;
static std::vector<int> g_bm_sizes;              // per-handle: w*1000+h
static std::vector<chunked_bitmap *> g_destroyed_chunks;

void SubtInitSubtitles(const std::filesystem::path &filename) { g_subt_inits.push_back(filename.string()); }
void SubtCloseSubtitles() { g_subt_closes++; }
void SubtDrawSubtitles(int frame_num) { g_subt_draws.push_back(frame_num); }

void rend_SetAlphaType(int8_t) { g_alpha_sets++; }
void rend_CopyBitmapToFramebuffer(int bm_handle, int x, int y) {
  g_blits.push_back({bm_handle, (x << 12) | y});
}
int bm_w(int handle, int) {
  int v = handle > 0 && handle <= (int)g_bm_sizes.size() ? g_bm_sizes[handle - 1] : 0;
  return v / 1000;
}
int bm_h(int handle, int) {
  int v = handle > 0 && handle <= (int)g_bm_sizes.size() ? g_bm_sizes[handle - 1] : 0;
  return v % 1000;
}
void bm_DestroyChunkedBitmap(chunked_bitmap *chunk) { g_destroyed_chunks.push_back(chunk); }

// ---------------------------------------------------------------------------
// Fixture helpers
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for CinematicsTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class CinematicsTest : public ::testing::Test {
protected:
  void SetUp() override {
    Dedicated_server = false;
    Max_window_w = 640;
    Max_window_h = 480;
    Renderer_type = RENDERER_OPENGL;
    Descent = nullptr;
    g_mve_init_result = MVELIB_NOERROR;
    g_play_requests.clear();
    g_play_result = MVELIB_NOERROR;
    g_seq_starts.clear();
    g_seq_start_handle = 0;
    g_frame_calls.clear();
    g_next_frame_out = -2;
    g_frame_bm_handle = -1;
    g_frame_end = false;
    g_seq_closes.clear();
    g_set_callback = nullptr;
    g_subt_inits.clear();
    g_subt_closes = 0;
    g_subt_draws.clear();
    g_alpha_sets = 0;
    g_blits.clear();
    g_bm_sizes.clear();
    g_destroyed_chunks.clear();
    g_props.clear();
    InitCinematics(); // default: initialized for most tests
  }

  void TearDown() override { Dedicated_server = false; }
};

/**
 * @test CinematicsTest.InitOnDedicatedServerDisablesPlayback
 * @brief Verifies init On Dedicated Server Disables Playback.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, InitOnDedicatedServerDisablesPlayback) {
  Dedicated_server = true;
  EXPECT_TRUE(InitCinematics()); // succeeds but leaves lib uninitialized

  EXPECT_FALSE(PlayMovie("intro"));
  EXPECT_TRUE(g_play_requests.empty());
  EXPECT_EQ(StartMovie("intro"), nullptr);
}

/**
 * @test CinematicsTest.ReinitFailureReportsFalseButStaleInitPersists
 * @brief Verifies reinit Failure Reports False But Stale Init Persists.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, ReinitFailureReportsFalseButStaleInitPersists) {
  g_mve_init_result = MVELIB_INIT_ERROR;
  EXPECT_FALSE(InitCinematics()); // reports failure
  // KNOWN QUIRK: the early return never resets Cinematic_lib_init, so
  // a previously initialized library remains fully usable
  EXPECT_TRUE(PlayMovie("intro"));
}

/**
 * @test CinematicsTest.InitRegistersCallbackWhichDrawsSubtitles
 * @brief Verifies init Registers Callback Which Draws Subtitles.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, InitRegistersCallbackWhichDrawsSubtitles) {
  // SetUp already ran InitCinematics
  ASSERT_NE(g_set_callback, nullptr);
  EXPECT_TRUE(g_subt_draws.empty());
  g_set_callback(10, 20, 7);
  ASSERT_EQ(g_subt_draws.size(), 1u);
  EXPECT_EQ(g_subt_draws[0], 7);
}

/**
 * @test CinematicsTest.PlayMovieNormalizesMissingExtension
 * @brief Verifies play Movie Normalizes Missing Extension.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, PlayMovieNormalizesMissingExtension) {
  EXPECT_TRUE(PlayMovie("intro"));
  ASSERT_EQ(g_play_requests.size(), 1u);
  EXPECT_STREQ(g_play_requests[0].c_str(), "intro.mve");

  // subtitles initialized with the ORIGINAL name, closed afterwards
  ASSERT_EQ(g_subt_inits.size(), 1u);
  EXPECT_STREQ(g_subt_inits[0].c_str(), "intro");
  EXPECT_EQ(g_subt_closes, 1);
}

/**
 * @test CinematicsTest.PlayMovieKeepsValidExtensionsCaseInsensitive
 * @brief Verifies play Movie Keeps Valid Extensions Case Insensitive.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, PlayMovieKeepsValidExtensionsCaseInsensitive) {
  EXPECT_TRUE(PlayMovie("a.mv8"));
  EXPECT_STREQ(g_play_requests.back().c_str(), "a.mv8");

  EXPECT_TRUE(PlayMovie("b.MVE"));
  EXPECT_STREQ(g_play_requests.back().c_str(), "b.MVE");

  EXPECT_TRUE(PlayMovie("c.txt"));
  EXPECT_STREQ(g_play_requests.back().c_str(), "c.mve"); // wrong ext replaced
}

/**
 * @test CinematicsTest.PlayMovieUsesWindowPropertiesAndRendererType
 * @brief Verifies play Movie Uses Window Properties And Renderer Type.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, PlayMovieUsesWindowPropertiesAndRendererType) {
  PlayMovie("x");
  ASSERT_FALSE(g_props.empty());
  auto p = g_props.back();
  EXPECT_EQ(p.x, 0);
  EXPECT_EQ(p.y, 0);
  EXPECT_EQ(p.w, 640);
  EXPECT_EQ(p.h, 480);
  EXPECT_EQ(p.type, RENDERER_OPENGL);
  EXPECT_TRUE(p.hicolor); // hardcoded kHiColorEnabled = true

  // SetMovieProperties forwards verbatim
  SetMovieProperties(11, 22, 33, 44, RENDERER_DIRECT3D);
  p = g_props.back();
  EXPECT_EQ(p.x, 11);
  EXPECT_EQ(p.y, 22);
  EXPECT_EQ(p.w, 33);
  EXPECT_EQ(p.h, 44);
  EXPECT_EQ(p.type, RENDERER_DIRECT3D);
}

/**
 * @test CinematicsTest.PlayMovieReportsMveErrorsAsFailure
 * @brief Verifies play Movie Reports Mve Errors As Failure.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, PlayMovieReportsMveErrorsAsFailure) {
  g_play_result = MVELIB_PLAYBACK_ERROR;
  EXPECT_FALSE(PlayMovie("broken"));
  EXPECT_EQ(g_subt_closes, 1); // subtitles still shut down
}

/**
 * @test CinematicsTest.StartMovieAppendsExtensionAndWrapsHandle
 * @brief Verifies start Movie Appends Extension And Wraps Handle.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, StartMovieAppendsExtensionAndWrapsHandle) {
  g_seq_start_handle = 42;
  tCinematic *mve = StartMovie("loop");
  ASSERT_NE(mve, nullptr);
  EXPECT_EQ(mve->mvehandle, 42);
  EXPECT_EQ(mve->frame_chunk.bm_array, nullptr);

  ASSERT_EQ(g_seq_starts.size(), 1u);
  EXPECT_STREQ(g_seq_starts[0].name.c_str(), "loop.mve");
  EXPECT_FALSE(g_seq_starts[0].looping); // default

  EndMovie(mve);
}

/**
 * @test CinematicsTest.StartMovieKeepsExtensionsAndLoopingFlag
 * @brief Verifies start Movie Keeps Extensions And Looping Flag.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, StartMovieKeepsExtensionsAndLoopingFlag) {
  g_seq_start_handle = 9;
  tCinematic *mve = StartMovie("clip.mv8", true);
  ASSERT_NE(mve, nullptr);
  EXPECT_STREQ(g_seq_starts[0].name.c_str(), "clip.mv8");
  EXPECT_TRUE(g_seq_starts[0].looping);
  EndMovie(mve);
}

/**
 * @test CinematicsTest.StartMovieReturnsNullWhenHandleIsZero
 * @brief Verifies start Movie Returns Null When Handle Is Zero.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, StartMovieReturnsNullWhenHandleIsZero) {
  g_seq_start_handle = 0;
  EXPECT_EQ(StartMovie("dead"), nullptr);
}

/**
 * @test CinematicsTest.FrameMovieCentersBlitWhenAsked
 * @brief Verifies frame Movie Centers Blit When Asked.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, FrameMovieCentersBlitWhenAsked) {
  g_seq_start_handle = 5;
  tCinematic *mve = StartMovie("m");
  ASSERT_NE(mve, nullptr);

  g_frame_bm_handle = 1;
  g_bm_sizes.assign(1, 200000 + 100); // w=200 h=100

  // properties set to full window by PlayMovie/StartMovie path? use explicit:
  SetMovieProperties(0, 0, 640, 480, Renderer_type);

  EXPECT_TRUE(FrameMovie(mve, -1, -1)); // center both axes

  ASSERT_EQ(g_blits.size(), 1u);
  int packed = g_blits[0].second;
  int x = packed >> 12, y = packed & 0xfff;
  EXPECT_EQ(x, (640 - 200) / 2); // 220
  EXPECT_EQ(y, (480 - 100) / 2); // 190
  EXPECT_EQ(g_blits[0].first, 1);
  EXPECT_EQ(g_alpha_sets, 1); // AT_ALWAYS before blit

  EndMovie(mve);
}

// FrameMovie's bool means "sequence did NOT end" (-1 from SequenceFrame),
// not "frame rendered" - normal and looped frames both return true.
/**
 * @test CinematicsTest.FrameMovieHonorsExplicitCoordsAndNoBitmap
 * @brief Verifies frame Movie Honors Explicit Coords And No Bitmap.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, FrameMovieHonorsExplicitCoordsAndNoBitmap) {
  g_seq_start_handle = 5;
  tCinematic *mve = StartMovie("m");
  ASSERT_NE(mve, nullptr);

  g_frame_bm_handle = -1; // no frame bitmap this call
  EXPECT_TRUE(FrameMovie(mve, 30, 40));
  EXPECT_TRUE(g_blits.empty()); // nothing to blit

  g_frame_bm_handle = 2;
  EXPECT_TRUE(FrameMovie(mve, 30, 40));
  ASSERT_EQ(g_blits.size(), 1u);
  EXPECT_EQ(g_blits[0].second >> 12, 30); // x used as-is
  EXPECT_EQ(g_blits[0].second & 0xfff, 40);

  EndMovie(mve);
}

// End of stream: SequenceFrame yields -1 -> returns false, but the
// frame bitmap is STILL blitted - the loop check and blit check are
// independent (final frames are drawn before reporting the end).
/**
 * @test CinematicsTest.FrameMovieReturnsFalseAtEndOfStreamButStillBlits
 * @brief Verifies frame Movie Returns False At End Of Stream But Still Blits.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, FrameMovieReturnsFalseAtEndOfStreamButStillBlits) {
  g_seq_start_handle = 5;
  tCinematic *mve = StartMovie("m");
  ASSERT_NE(mve, nullptr);

  g_frame_end = true;
  g_frame_bm_handle = 4;
  EXPECT_FALSE(FrameMovie(mve, 10, 10));
  ASSERT_EQ(g_blits.size(), 1u); // last frame still drawn
  EXPECT_EQ(g_blits[0].first, 4);
  EXPECT_EQ(mve->mvehandle, 5); // handle NOT replaced on -1

  EndMovie(mve);
}

/**
 * @test CinematicsTest.FrameMovieReplacesHandleOnLoopAndReportsTrue
 * @brief Verifies frame Movie Replaces Handle On Loop And Reports True.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, FrameMovieReplacesHandleOnLoopAndReportsTrue) {
  g_seq_start_handle = 5;
  tCinematic *mve = StartMovie("m");
  ASSERT_NE(mve, nullptr);

  g_next_frame_out = 99; // movie wrapped around
  g_frame_bm_handle = -1;
  EXPECT_TRUE(FrameMovie(mve, 0, 0, false)); // sequence=false passthrough
  EXPECT_EQ(mve->mvehandle, 99);             // struct updated to new handle
  ASSERT_EQ(g_frame_calls.size(), 1u);
  EXPECT_FALSE(g_frame_calls[0].sequence);

  EndMovie(mve);
  ASSERT_EQ(g_seq_closes.size(), 1u);
  EXPECT_EQ((intptr_t)g_seq_closes[0].first, 99);
}

/**
 * @test CinematicsTest.EndMovieDestroysChunkedBitmapIfPresent
 * @brief Verifies end Movie Destroys Chunked Bitmap If Present.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, EndMovieDestroysChunkedBitmapIfPresent) {
  g_seq_start_handle = 3;
  tCinematic *mve = StartMovie("m");
  ASSERT_NE(mve, nullptr);

  static int dummy_chunks[4];
  mve->frame_chunk.bm_array = dummy_chunks;
  EndMovie(mve);

  ASSERT_EQ(g_destroyed_chunks.size(), 1u);
  EXPECT_EQ(g_destroyed_chunks[0]->bm_array, dummy_chunks);
  EXPECT_EQ(g_seq_closes.size(), 1u);
}

/**
 * @test CinematicsTest.GuardsAgainstNullAndUninitializedStates
 * @brief Verifies guards Against Null And Uninitialized States.
 *
 * @details
 * Exercises the CinematicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/cinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CinematicsTest, GuardsAgainstNullAndUninitializedStates) {
  // uninitialized: everything is a safe no-op / failure
  Dedicated_server = true;
  InitCinematics();
  EXPECT_FALSE(FrameMovie(nullptr, 0, 0));
  tCinematic fake{};
  EXPECT_FALSE(FrameMovie(&fake, 0, 0));
  EndMovie(&fake); // must not crash or close anything
  EXPECT_TRUE(g_seq_closes.empty());
}
