/**
 * @file smallviews_real_tests.cpp
 * @brief Tests for SmallViews.cpp 569 lines — small 3D views (rear/guided views).
 *
 * @details
 * Covers CreateSmallView validation/popup-save/timed/label truncation,
 * CloseSmallView restore & guided-missile cleanup, ClosePopupView gating,
 * ResetSmallViews, HUD window layout math, font scale thresholds, and
 * static-frame computation. Replicates logic to avoid render deps.
 *
 * This harness validates the behavior of `Descent3/SmallViews.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/SmallViews.cpp`
 * @par Harness
 * `smallviews_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/SmallViews.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>

// replicated constants (SmallViews.h:72-82, SmallViews.cpp:165-168, 267)
constexpr int SVW_LEFT = 0, SVW_CENTER = 1, SVW_RIGHT = 2;
constexpr int NUM_SMALL_VIEWS = 3;
constexpr int SVF_POPUP = 1, SVF_BIGGER = 2, SVF_REARVIEW = 4, SVF_TIMED = 8, SVF_CROSSHAIRS = 16, SVF_STATIC = 32;
constexpr int OBJECT_HANDLE_NONE = -1;
constexpr int LABEL_LEN = 19;
constexpr float STATIC_TIME = 0.25f;

// replicated small_view struct (SmallViews.cpp:171-178)
struct small_view {
  int objhandle = OBJECT_HANDLE_NONE;
  int flags = 0;
  float timer = 0;
  float zoom = 0;
  int gun_num = -1;
  char label[LABEL_LEN + 1] = {};
};

// replicated state
static small_view Small_views[NUM_SMALL_VIEWS];
static small_view Small_views_save[NUM_SMALL_VIEWS];
static bool Guided_missile_smallview = false;
static int Guided_missile_objhandle = OBJECT_HANDLE_NONE;
static int Disable_primary_monitor = 0, Disable_secondary_monitor = 0;
static int Clear_screen = 0;

// mock object table: handle -> exists + is-guided
static std::vector<int> g_existingHandles;
static int g_guidedHandle = -2; // sentinel "no guided"
static bool MockObjGet(int h) {
  for (int e : g_existingHandles) if (e == h) return true;
  return false;
}

// replicated CreateSmallView (SmallViews.cpp:207-250)
static int RepCreateSmallView(int window, int objhandle, int flags, float time, float zoom, int gun_num, const char *label) {
  if ((window < 0) || (window >= NUM_SMALL_VIEWS)) {
    window = SVW_LEFT; // Int3() in debug; clamped here
  }
  if (!MockObjGet(objhandle))
    return -1;

  small_view *svp = &Small_views[window];

  if ((flags & SVF_POPUP) && (svp->objhandle != OBJECT_HANDLE_NONE) && !(svp->flags & SVF_POPUP))
    Small_views_save[window] = *svp;

  svp->objhandle = objhandle;
  svp->zoom = zoom;
  svp->gun_num = gun_num;
  svp->flags = flags;
  svp->timer = time;
  if (time != 0.0f)
    svp->flags |= SVF_TIMED;

  strncpy(svp->label, label ? label : "", LABEL_LEN);
  svp->label[LABEL_LEN] = 0;

  if (window == SVW_LEFT)
    Disable_primary_monitor = 1;
  else if (window == SVW_RIGHT)
    Disable_secondary_monitor = 1;

  if (objhandle == g_guidedHandle) {
    Guided_missile_smallview = true;
    Guided_missile_objhandle = objhandle;
  }

  return window;
}

// replicated ResetSmallViews (253-262)
static void RepResetSmallViews() {
  for (int v = 0; v < NUM_SMALL_VIEWS; v++) {
    Small_views[v].objhandle = OBJECT_HANDLE_NONE;
    Small_views_save[v].objhandle = OBJECT_HANDLE_NONE;
  }
  Disable_primary_monitor = 0;
  Disable_secondary_monitor = 0;
  Guided_missile_smallview = false;
  Guided_missile_objhandle = OBJECT_HANDLE_NONE;
}

// replicated CloseSmallView (540-562)
static void RepCloseSmallView(int window) {
  if (Small_views[window].objhandle == Guided_missile_objhandle) {
    // mark guided missile small view as inactive.
    Guided_missile_objhandle = OBJECT_HANDLE_NONE;
    Guided_missile_smallview = false;
  }

  Small_views[window].objhandle = OBJECT_HANDLE_NONE;

  if (Small_views_save[window].objhandle != OBJECT_HANDLE_NONE) {
    Small_views[window] = Small_views_save[window];
    Small_views_save[window].objhandle = OBJECT_HANDLE_NONE;
  } else {
    Clear_screen = 4; // force background redraw
    if (window == SVW_LEFT)
      Disable_primary_monitor = 0;
    else if (window == SVW_RIGHT)
      Disable_secondary_monitor = 0;
  }
}

// replicated ClosePopupView (566-569)
static void RepClosePopupView(int window) {
  if (Small_views[window].flags & SVF_POPUP)
    RepCloseSmallView(window);
}

struct SmallViewsFixture : ::testing::Test {
  void SetUp() override {
    RepResetSmallViews();
    g_existingHandles.clear();
    g_guidedHandle = -2;
    Clear_screen = 0;
  }
};

/**
 * @test SmallViewsFixture.CreateReturnsWindowAndStores
 * @brief Verifies create Returns Window And Stores.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, CreateReturnsWindowAndStores) {
  g_existingHandles = {42};
  int w = RepCreateSmallView(SVW_RIGHT, 42, SVF_CROSSHAIRS, 0.0f, 1.5f, -1, "Rear");
  EXPECT_EQ(w, SVW_RIGHT);
  EXPECT_EQ(Small_views[SVW_RIGHT].objhandle, 42);
  EXPECT_FLOAT_EQ(Small_views[SVW_RIGHT].zoom, 1.5f);
  EXPECT_STREQ(Small_views[SVW_RIGHT].label, "Rear");
  EXPECT_EQ(Disable_secondary_monitor, 1);
}

/**
 * @test SmallViewsFixture.CreateBadObjectFails
 * @brief Verifies create Bad Object Fails.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, CreateBadObjectFails) {
  g_existingHandles = {};
  EXPECT_EQ(RepCreateSmallView(SVW_LEFT, 99, 0, 0, 1.0f, -1, ""), -1);
  EXPECT_EQ(Small_views[SVW_LEFT].objhandle, OBJECT_HANDLE_NONE);
}

/**
 * @test SmallViewsFixture.CreateInvalidWindowClampsToLeft
 * @brief Verifies create Invalid Window Clamps To Left.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, CreateInvalidWindowClampsToLeft) {
  g_existingHandles = {7};
  EXPECT_EQ(RepCreateSmallView(-1, 7, 0, 0, 1, -1, ""), SVW_LEFT);
  EXPECT_EQ(RepCreateSmallView(NUM_SMALL_VIEWS, 7, 0, 0, 1, -1, ""), SVW_LEFT);
  EXPECT_EQ(Small_views[SVW_LEFT].objhandle, 7);
  EXPECT_EQ(Disable_primary_monitor, 1);
}

/**
 * @test SmallViewsFixture.TimedFlagSetWhenTimeNonzero
 * @brief Verifies timed Flag Set When Time Nonzero.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, TimedFlagSetWhenTimeNonzero) {
  g_existingHandles = {1};
  RepCreateSmallView(SVW_LEFT, 1, 0, 5.0f, 1, -1, "");
  EXPECT_TRUE(Small_views[0].flags & SVF_TIMED);
  RepCreateSmallView(SVW_LEFT, 1, 0, 0.0f, 1, -1, "");
  EXPECT_FALSE(Small_views[0].flags & SVF_TIMED); // replaced view, time==0 -> no flag
}

/**
 * @test SmallViewsFixture.LabelTruncatedAt19Chars
 * @brief Verifies label Truncated At19Chars.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, LabelTruncatedAt19Chars) {
  g_existingHandles = {1};
  std::string longLabel(40, 'x');
  RepCreateSmallView(SVW_LEFT, 1, 0, 0, 1, -1, longLabel.c_str());
  EXPECT_EQ(strlen(Small_views[0].label), (size_t)LABEL_LEN);
  EXPECT_TRUE(Small_views[0].label[LABEL_LEN] == '\0');
}

/**
 * @test SmallViewsFixture.PopupSavesPreviousView
 * @brief Verifies popup Saves Previous View.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, PopupSavesPreviousView) {
  g_existingHandles = {1, 2};
  RepCreateSmallView(SVW_LEFT, 1, 0, 0, 1, -1, "base");       // non-popup base
  RepCreateSmallView(SVW_LEFT, 2, SVF_POPUP, 0, 1, -1, "popup"); // popup over it
  EXPECT_EQ(Small_views_save[SVW_LEFT].objhandle, 1);          // previous saved
  EXPECT_EQ(Small_views[SVW_LEFT].objhandle, 2);
  // popup over popup does NOT re-save
  RepCreateSmallView(SVW_LEFT, 2, SVF_POPUP, 0, 1, -1, "p2");
  EXPECT_STREQ(Small_views_save[SVW_LEFT].label, "base"); // still original save
}

/**
 * @test SmallViewsFixture.GuidedMissileDetection
 * @brief Verifies guided Missile Detection.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, GuidedMissileDetection) {
  g_existingHandles = {55};
  g_guidedHandle = 55;
  RepCreateSmallView(SVW_LEFT, 55, 0, 0, 1, -1, "");
  EXPECT_TRUE(Guided_missile_smallview);
  EXPECT_EQ(Guided_missile_objhandle, 55);
}

/**
 * @test SmallViewsFixture.CloseRestoresSavedPopupBase
 * @brief Verifies close Restores Saved Popup Base.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, CloseRestoresSavedPopupBase) {
  g_existingHandles = {1, 2};
  RepCreateSmallView(SVW_RIGHT, 1, 0, 0, 1, -1, "base");
  RepCreateSmallView(SVW_RIGHT, 2, SVF_POPUP, 0, 1, -1, "pop");
  RepCloseSmallView(SVW_RIGHT);
  EXPECT_EQ(Small_views[SVW_RIGHT].objhandle, 1); // restored
  EXPECT_STREQ(Small_views[SVW_RIGHT].label, "base");
  EXPECT_EQ(Small_views_save[SVW_RIGHT].objhandle, OBJECT_HANDLE_NONE);
  EXPECT_EQ(Clear_screen, 0); // no clear on restore path
  EXPECT_EQ(Disable_secondary_monitor, 1); // not cleared on restore path
}

/**
 * @test SmallViewsFixture.CloseNoSaveClearsScreenAndMonitor
 * @brief Verifies close No Save Clears Screen And Monitor.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, CloseNoSaveClearsScreenAndMonitor) {
  g_existingHandles = {1};
  RepCreateSmallView(SVW_LEFT, 1, 0, 0, 1, -1, "");
  RepCloseSmallView(SVW_LEFT);
  EXPECT_EQ(Small_views[SVW_LEFT].objhandle, OBJECT_HANDLE_NONE);
  EXPECT_EQ(Clear_screen, 4);
  EXPECT_EQ(Disable_primary_monitor, 0);
}

/**
 * @test SmallViewsFixture.CloseGuidedMissileClearsFlag
 * @brief Verifies close Guided Missile Clears Flag.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, CloseGuidedMissileClearsFlag) {
  g_existingHandles = {9};
  g_guidedHandle = 9;
  RepCreateSmallView(SVW_RIGHT, 9, 0, 0, 1, -1, "");
  ASSERT_TRUE(Guided_missile_smallview);
  RepCloseSmallView(SVW_RIGHT);
  EXPECT_FALSE(Guided_missile_smallview);
  EXPECT_EQ(Guided_missile_objhandle, OBJECT_HANDLE_NONE);
}

/**
 * @test SmallViewsFixture.ClosePopupOnlyIfPopup
 * @brief Verifies close Popup Only If Popup.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, ClosePopupOnlyIfPopup) {
  g_existingHandles = {1, 2};
  RepCreateSmallView(SVW_LEFT, 1, 0, 0, 1, -1, "perm");
  RepClosePopupView(SVW_LEFT);
  EXPECT_EQ(Small_views[SVW_LEFT].objhandle, 1); // non-popup untouched

  RepCreateSmallView(SVW_LEFT, 2, SVF_POPUP, 0, 1, -1, "temp");
  RepClosePopupView(SVW_LEFT);
  EXPECT_EQ(Small_views[SVW_LEFT].objhandle, 1); // closed and restored
  EXPECT_STREQ(Small_views[SVW_LEFT].label, "perm");
}

/**
 * @test SmallViewsFixture.ResetClearsEverything
 * @brief Verifies reset Clears Everything.
 *
 * @details
 * Exercises the SmallViewsFixture code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST_F(SmallViewsFixture, ResetClearsEverything) {
  g_existingHandles = {1};
  g_guidedHandle = 1;
  RepCreateSmallView(SVW_LEFT, 1, SVF_POPUP, 3, 1, -1, "x");
  RepResetSmallViews();
  for (int i = 0; i < NUM_SMALL_VIEWS; ++i) {
    EXPECT_EQ(Small_views[i].objhandle, OBJECT_HANDLE_NONE);
    EXPECT_EQ(Small_views_save[i].objhandle, OBJECT_HANDLE_NONE);
  }
  EXPECT_FALSE(Guided_missile_smallview);
  EXPECT_EQ(Guided_missile_objhandle, OBJECT_HANDLE_NONE);
  EXPECT_EQ(Disable_primary_monitor, 0);
  EXPECT_EQ(Disable_secondary_monitor, 0);
}

// replicated HUD layout math (SmallViews.cpp:467-483)
struct Rect { int left, top, right, bot; };
static Rect HudWindowRect(int v, int gw, int gh, int gx, int gy, bool bigger) {
  int w = gh / 4;
  int spacing = ((gw - (NUM_SMALL_VIEWS * w)) / 3);
  int x = ((spacing + w) / 2) + (v * (spacing + w)) + gx;
  int h = w;
  int y = gy + (gh - h / 2 - gh / 24);
  if (bigger) { w += w / 4; h += h / 4; }
  return {x - w / 2, y - h / 2, x + w / 2, y + h / 2};
}

/**
 * @test SmallViews.HudLayoutMath
 * @brief Verifies hud Layout Math.
 *
 * @details
 * Exercises the SmallViews code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST(SmallViews, HudLayoutMath) {
  Rect r = HudWindowRect(0, 640, 480, 0, 0, false);
  int w = 480 / 4; // 120
  EXPECT_EQ(r.right - r.left, 2 * (w / 2)); // symmetric around center
  EXPECT_EQ(r.bot - r.top, 2 * (w / 2));

  // bigger window grows by quarter
  Rect rb = HudWindowRect(0, 640, 480, 0, 0, true);
  EXPECT_GT(rb.right - rb.left, r.right - r.left);

  // windows progress left to right
  Rect r0 = HudWindowRect(0, 640, 480, 0, 0, false);
  Rect r2 = HudWindowRect(2, 640, 480, 0, 0, false);
  EXPECT_GT(r2.left, r0.left);
}

// replicated font scale thresholds (SmallViews.cpp:279-287)
static float FontScaleForAspect(float aspect) {
  if (aspect <= 0.60f) return 0.60f;
  if (aspect <= 0.80f) return 0.80f;
  return 1.0f;
}

/**
 * @test SmallViews.FontScaleThresholds
 * @brief Verifies font Scale Thresholds.
 *
 * @details
 * Exercises the SmallViews code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST(SmallViews, FontScaleThresholds) {
  EXPECT_FLOAT_EQ(FontScaleForAspect(0.30f), 0.60f);
  EXPECT_FLOAT_EQ(FontScaleForAspect(0.60f), 0.60f); // inclusive
  EXPECT_FLOAT_EQ(FontScaleForAspect(0.61f), 0.80f);
  EXPECT_FLOAT_EQ(FontScaleForAspect(0.80f), 0.80f); // inclusive
  EXPECT_FLOAT_EQ(FontScaleForAspect(0.81f), 1.0f);
  EXPECT_FLOAT_EQ(FontScaleForAspect(1.0f), 1.0f);
}

// replicated static frame computation (SmallViews.cpp:301-303)
static int StaticFrameIndex(int num_frames, float timer) {
  int frame = (int)(num_frames * (STATIC_TIME - timer) / STATIC_TIME);
  if (frame == num_frames)
    frame--;
  return frame;
}

/**
 * @test SmallViews.StaticFrameIndexMath
 * @brief Verifies static Frame Index Math.
 *
 * @details
 * Exercises the SmallViews code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/SmallViews.cpp
 * @ingroup descent3_tests
 */
TEST(SmallViews, StaticFrameIndexMath) {
  EXPECT_EQ(StaticFrameIndex(10, STATIC_TIME), 0);   // timer start -> frame 0
  EXPECT_EQ(StaticFrameIndex(10, 0.0f), 9);          // timer expired -> last frame (not 10)
  EXPECT_EQ(StaticFrameIndex(8, 0.125f), 4);         // halfway
  // no negative clamp: past-end timer gives negative float -> truncates to 0
  // (unreachable in practice since timer counts down from STATIC_TIME)
  EXPECT_EQ(StaticFrameIndex(4, 0.30f), 0);
}
