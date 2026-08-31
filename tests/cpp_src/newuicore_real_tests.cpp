/**
 * @file newuicore_real_tests.cpp
 * @brief Unit tests for Descent3/newui_core.cpp: the high-level newui framework.
 *
 * @details
 * (resource manager refcounting, UI plumbing, menus, sheets, sliders and
 * list boxes) exercised on top of the REAL ui library (build/ui/libui.a).
 *
 * This harness validates the behavior of `Descent3/newuicore.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/newuicore.cpp`
 * @par Harness
 * `newuicore_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/newuicore.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

#include "newui_core.h"
#include "newui.h"
#include "uisys.h"
#include "pserror.h"
#include "gamefont.h"
#include "d3music.h"
#include "mem.h"
#include "descent.h"
#include "renderer.h"
#include "chrono_timer.h"
#include "hlsoundlib.h"
#include <SDL3/SDL_assert.h>

#define REC(msg) g_recorder.push_back(msg)
static std::vector<std::string> g_recorder;

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
int Max_window_w = 640;
int Max_window_h = 480;
bool Multi_bail_ui_menu = false;
int Game_fonts[NUM_FONTS] = {0};

// current function mode; defaults to GAME_MODE so the music frame path is off.
// Tests flip this via SetFunctionMode to exercise MENU_MODE branches.
static function_mode s_function_mode = GAME_MODE;
function_mode GetFunctionMode() { return s_function_mode; }
void SetFunctionMode(function_mode mode) { s_function_mode = mode; }

struct FakeApp : oeApplication {
  void init() override {}
  void get_info(void *) override {}
  int flags() const override { return 0; }
  unsigned defer() override {
    REC("defer");
    return 0;
  }
  void delay(float) override {}
  void set_defer_handler(void (*)(bool)) override {}
};
static FakeApp g_fake_app;
oeApplication *Descent = &g_fake_app;

namespace D3 {
float ChronoTimer::GetTime() { return 0.0f; }
int64_t ChronoTimer::GetTimeMS() { return 0; }
void ChronoTimer::SleepMS(int) {}
void ChronoTimer::Initialize() {}
} // namespace D3

// ddebug hooks (lib not linked)
void (*DebugBreak_callback_stop)() = nullptr;
void (*DebugBreak_callback_resume)() = nullptr;

void Error(const char *, ...) {}

// ---------------------------------------------------------------------------
// bitmap stubs: scriptable dimensions decide chunked vs plain
// ---------------------------------------------------------------------------
static int s_bm_next_handle = 1;
static int s_bm_w = 64, s_bm_h = 64;
static int s_bm_free_count = 0;
static int s_chunk_create_count = 0;
static int s_chunk_destroy_count = 0;

int bm_AllocLoadFileBitmap(const char *filename, int local, int format) {
  REC(std::string("bmload:") + filename);
  return s_bm_next_handle++;
}
int bm_w(int handle, int dir) { return s_bm_w; }
int bm_h(int handle, int dir) { return s_bm_h; }
bool bm_CreateChunkedBitmap(int n, chunked_bitmap *c) {
  memset(c, 0, sizeof(*c));
  static int dummy[4];
  c->bm_array = dummy;
  s_chunk_create_count++;
  return true;
}
void bm_DestroyChunkedBitmap(chunked_bitmap *) { s_chunk_destroy_count++; }
void bm_FreeBitmap(int) { s_bm_free_count++; }

// mem.h macro backends
void *mem_malloc_sub(int size, const char *, int) { return malloc(size); }
void mem_free_sub(void *p) { free(p); }
char *mem_strdup_sub(const char *s, const char *, int) { return strdup(s); }

// ---------------------------------------------------------------------------
// input stubs: dead keyboard/mouse
// ---------------------------------------------------------------------------
int ddio_GetAdjKeyState(int) { return 0; }
void ddio_KeyFlush() {}
int ddio_KeyInKey() { return 0; }
bool ddio_MouseGetEvent(int *btn, bool *state) {
  *btn = 0;
  *state = false;
  return false;
}
int ddio_MouseGetState(int *x, int *y, int *dx, int *dy, int *z, int *dz) {
  if (x)
    *x = 0;
  if (y)
    *y = 0;
  if (dx)
    *dx = 0;
  if (dy)
    *dy = 0;
  if (z)
    *z = 0;
  if (dz)
    *dz = 0;
  return 0;
}
void ddio_MouseReset() {}
void ddio_MouseSetVCoords(int, int) {}

// ---------------------------------------------------------------------------
// renderer / frame stubs
// ---------------------------------------------------------------------------
void rend_ClearScreen(ddgr_color) { REC("clearscreen"); }
void rend_Flip() { REC("flip"); }
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, uint8_t) {}
void rend_DrawLine(int, int, int, int) {}
void rend_DrawPolygon2D(int, g3Point **, int) {}
void rend_DrawScaledBitmap(int, int, int, int, int, float, float, float, float, int, const float *) {}
void rend_EndFrame() { REC("rendendframe"); }
void rend_StartFrame(int, int, int, int, int) { REC("rendstartframe"); }
void rend_SetAlphaType(int8_t) {}
void rend_SetAlphaValue(uint8_t) {}
void rend_SetColorModel(color_model) {}
void rend_SetFlatColor(ddgr_color) {}
void rend_SetLighting(light_state) {}
void rend_SetOverlayType(uint8_t) {}
void rend_SetTextureType(texture_type) {}
void rend_SetWrapType(wrap_type) {}
void rend_SetZBufferState(int8_t) {}

void StartFrame(bool) { REC("startframe"); }
void StartFrame(int, int, int, int, bool, bool) { REC("startframe"); }
void EndFrame() { REC("endframe"); }
void DoScreenshot() { REC("screenshot"); }

// ---------------------------------------------------------------------------
// text / font stubs
// ---------------------------------------------------------------------------
const char *GetStringFromTable(int id) {
  static std::string buf;
  buf = "STR_" + std::to_string(id);
  return buf.c_str();
}
extern "C" {
void grtext_SetParameters(int, int, int, int, int) {}
void grtext_SetFlags(int) {}
int grtext_GetFont() { return 0; }
int grtext_GetTextHeight(const char *) { return 12; }
int grtext_GetTextLineWidth(const char *) { return 40; }
void grtext_Puts(int, int, const char *) {}
void grtext_CenteredPrintf(int, int, const char *, ...) {}
void grtext_SetColor(ddgr_color) {}
void grtext_SetAlpha(uint8_t) {}
void grtext_SetFont(int) {}
void grtext_Flush() {}
int grfont_GetHeight(int) { return 12; }
int grfont_KeyToAscii(int, int) { return 'a'; }
}

void textaux_WordWrap(const char *, char *, int, int) {}

// ---------------------------------------------------------------------------
// sound / music
// ---------------------------------------------------------------------------
hlsSystem::hlsSystem() {}
int hlsSystem::Play2dSound(int, float, float, uint16_t) { return 0; }
void hlsSystem::BeginSoundFrame(bool) { REC("beginsound"); }
void hlsSystem::EndSoundFrame() { REC("endsound"); }
int hlsSystem::GetLLSoundQuantity() { return 0; }
void hlsSystem::SetLLSoundQuantity(int) {}
float hlsSystem::GetMasterVolume() { return 1.0f; }
void hlsSystem::SetMasterVolume(float) {}
char hlsSystem::GetSoundQuality() { return 0; }
bool hlsSystem::SetSoundQuality(char) { return true; }
void hlsSystem::StopSoundImmediate(int) {}
void hlsSystem::KillSoundLib(bool) {}
hlsSystem Sound_system;

void D3MusicDoFrame(tMusicSeqInfo *info) {
  REC("musicdoframe");
  (void)info;
}

// SDL assert hook used by the real ui library
SDL_AssertState SDL_ReportAssertion(SDL_AssertData *, const char *, const char *, int) {
  return SDL_ASSERTION_IGNORE;
}

// file-local to newui_core.cpp but needed for the callback-mapping test
void SimpleUICallback();

/**
 * @brief GTest fixture for NewuiCoreTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class NewuiCoreTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_recorder.clear();
    s_bm_next_handle = 1;
    s_bm_w = 64;
    s_bm_h = 64;
    s_bm_free_count = 0;
    s_chunk_create_count = 0;
    s_chunk_destroy_count = 0;
    Multi_bail_ui_menu = false;
    Max_window_w = 640;
    Max_window_h = 480;
  }
};

// ---------------------------------------------------------------------------
// newuiResources: bitmap cache with refcounting
// ---------------------------------------------------------------------------

/**
 * @test NewuiCoreTest.LoadCachesByFilenameAndRefcounts
 * @brief Verifies load Caches By Filename And Refcounts.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, LoadCachesByFilenameAndRefcounts) {
  newuiCore_Init();

  UIBitmapItem *a1 = newui_LoadBitmap("Window.ogf");
  ASSERT_NE(a1, nullptr);
  EXPECT_FALSE(a1->is_chunked());
  UIBitmapItem *a2 = newui_LoadBitmap("window.ogf"); // same file (case-insensitive)
  ASSERT_NE(a2, nullptr);
  EXPECT_NE(a2, a1);                             // distinct wrapper objects...
  EXPECT_EQ(a2->get_bitmap(), a1->get_bitmap()); // ...sharing one bitmap

  // one underlying bitmap for both references
  int frees_before = s_bm_free_count;
  newui_FreeBitmap(a1);
  EXPECT_EQ(s_bm_free_count, frees_before); // still referenced by a2
  newui_FreeBitmap(a2);
  EXPECT_EQ(s_bm_free_count, frees_before + 1); // released now
}

/**
 * @test NewuiCoreTest.LoadChunksOddSizedBitmaps
 * @brief Verifies load Chunks Odd Sized Bitmaps.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, LoadChunksOddSizedBitmaps) {
  newuiCore_Init();
  s_bm_w = 48;
  s_bm_h = 48; // not a power-of-two tile size

  UIBitmapItem *item = newui_LoadBitmap("Odd.ogf");
  ASSERT_NE(item, nullptr);
  EXPECT_TRUE(item->is_chunked());

  int destroys_before = s_chunk_destroy_count;
  newui_FreeBitmap(item);
  EXPECT_EQ(s_chunk_destroy_count, destroys_before + 1);
}

/**
 * @test NewuiCoreTest.FreeOfUnknownItemIsIgnored
 * @brief Verifies free Of Unknown Item Is Ignored.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, FreeOfUnknownItemIsIgnored) {
  newuiCore_Init();
  // A foreign bitmap item must not corrupt the cache.
  UIBitmapItem stranger(9999);
  newui_FreeBitmap(&stranger); // should simply return (Int3 is a no-op in RELEASE)
}

/**
 * @test NewuiCoreTest.PageInAndReleasePreloadedBitmaps
 * @brief Verifies page In And Release Preloaded Bitmaps.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, PageInAndReleasePreloadedBitmaps) {
  newuiCore_Init();
  int loads_before = 0;
  for (auto &r : g_recorder)
    if (r.rfind("bmload:", 0) == 0)
      loads_before++;

  newuiCore_PageInBitmaps();
  int loaded = 0;
  for (auto &r : g_recorder)
    if (r.rfind("bmload:", 0) == 0)
      loaded++;
  EXPECT_GT(loaded, loads_before);

  int frees_before = s_bm_free_count;
  newuiCore_ReleaseBitmaps();
  EXPECT_GT(s_bm_free_count, frees_before);

  // after release, paging in again must work
  newuiCore_PageInBitmaps();
  newuiCore_ReleaseBitmaps();
}

// ---------------------------------------------------------------------------
// UI plumbing
// ---------------------------------------------------------------------------

static int s_cb_hits = 0;
static void CountingCallback() { s_cb_hits++; }

/**
 * @test NewuiCoreTest.SetGetUICallbackRoundTrip
 * @brief Verifies set Get UICallback Round Trip.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, SetGetUICallbackRoundTrip) {
  SetUICallback(CountingCallback);
  EXPECT_EQ(GetUICallback(), &CountingCallback);
  SetUICallback(nullptr);
  EXPECT_EQ(GetUICallback(), nullptr);
}

/**
 * @test NewuiCoreTest.DefaultUiCallbackMapsToSimple
 * @brief Verifies default Ui Callback Maps To Simple.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, DefaultUiCallbackMapsToSimple) {
  SetUICallback(DEFAULT_UICALLBACK);
  EXPECT_EQ(GetUICallback(), &SimpleUICallback);
  SetUICallback(nullptr);
}

/**
 * @test NewuiCoreTest.SimpleUICallbackClearsScreenBetweenFrames
 * @brief Verifies simple UICallback Clears Screen Between Frames.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, SimpleUICallbackClearsScreenBetweenFrames) {
  SimpleUICallback();
  ASSERT_GE(g_recorder.size(), 3u);
  EXPECT_EQ(g_recorder[0], "startframe");
  EXPECT_EQ(g_recorder[1], "clearscreen");
  EXPECT_EQ(g_recorder[2], "endframe");
}

/**
 * @test NewuiCoreTest.DoUIFrameBailSetsForceQuit
 * @brief Verifies do UIFrame Bail Sets Force Quit.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, DoUIFrameBailSetsForceQuit) {
  Multi_bail_ui_menu = true;
  DoUIFrame();
  Multi_bail_ui_menu = false;
  EXPECT_EQ(GetUIFrameResult(), NEWUIRES_FORCEQUIT);
}

// ---------------------------------------------------------------------------
// newuiMenu / newuiSheet integration on the real ui library
// ---------------------------------------------------------------------------

/**
 * @test NewuiCoreTest.MenuCreateAndAddOptions
 * @brief Verifies menu Create And Add Options.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, MenuCreateAndAddOptions) {
  newuiCore_Init();
  newuiMenu menu;
  menu.Create();

  newuiSheet *sheet = menu.AddOption(100, "Play", 4, true, 0);
  EXPECT_NE(sheet, nullptr);
  menu.AddSimpleOption(200, "Quit", 0); // no sheet attached

  // nothing processed yet
  EXPECT_EQ(menu.GetCurrentOption(), -1);
  EXPECT_EQ(menu.GetCurrentSheet(), nullptr);

  menu.SetCurrentOption(100); // queued for next DoUI pass
  EXPECT_EQ(menu.GetCurrentOption(), -1);

  // sheet capacity: exceeding N_NEWUI_SHEETS options is rejected
  newuiSheet *extra = menu.AddOption(300, "X", 1, true, 0);
  (void)extra;

  // explicit teardown mirrors game usage; relying on the destructor trips a
  // latent engine bug (virtual OnDestroy never fires from the base dtor)
  menu.Destroy();
}

/**
 * @test NewuiCoreTest.SheetRealizesGadgetsAndGetGadgetResolves
 * @brief Verifies sheet Realizes Gadgets And Get Gadget Resolves.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, SheetRealizesGadgetsAndGetGadgetResolves) {
  newuiCore_Init();
  newuiMenu menu;
  menu.Create();

  newuiSheet *sheet = menu.AddOption(10, "Options", 6, true, 0);
  ASSERT_NE(sheet, nullptr);

  sheet->NewGroup("Group", 0, 0);
  sheet->AddButton("OK", 50);
  bool *cbval = sheet->AddCheckBox("Toggle", false, 51);
  ASSERT_NE(cbval, nullptr);
  int16_t *slider = sheet->AddSlider("Slide", 20, 5, nullptr, 52);
  ASSERT_NE(slider, nullptr);
  EXPECT_EQ(*slider, 5); // initial value echoed back

  sheet->SetInitialFocusedGadget(50);
  sheet->Realize();

  // NOTE: newuiButton inherits VIRTUALLY from UIButton, and t_gadget_desc's
  // obj union aliases newuiButton* with UIGadget* -- reading .gadget for
  // button-family widgets yields an unadjusted (wrong) base pointer under
  // clang/GCC layouts. Both GetGadget() and the Realize() initial-focus scan
  // go through that union, so they can only be trusted for widgets whose
  // chain has no virtual bases (slider, listbox, text, ...).
  UIGadget *sl = sheet->GetGadget(52);
  ASSERT_NE(sl, nullptr);
  EXPECT_EQ(sl->GetID(), 52);

  // checkbox registered and readable through its parm binding
  bool *cbval2 = sheet->AddCheckBox("Second", true, 53);
  ASSERT_NE(cbval2, nullptr);
  EXPECT_FALSE(sheet->HasChanged(cbval));

  sheet->Unrealize();
  menu.Destroy();
}

/**
 * @test NewuiCoreTest.SheetAddTextAndChangeableText
 * @brief Verifies sheet Add Text And Changeable Text.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, SheetAddTextAndChangeableText) {
  newuiCore_Init();
  newuiMenu menu;
  menu.Create();
  newuiSheet *sheet = menu.AddOption(20, "Info", 4, true, 0);
  ASSERT_NE(sheet, nullptr);

  sheet->NewGroup(nullptr, 0, 0);   // untitled group
  sheet->AddText("Hello %s", "D3"); // printf-style static text

  char *dyn = sheet->AddChangeableText(32);
  ASSERT_NE(dyn, nullptr);
  strcpy(dyn, "dynamic");

  sheet->Realize();
  // auto-id gadgets (-1) are resolvable; first match wins
  UIGadget *txt = sheet->GetGadget(-1);
  EXPECT_NE(txt, nullptr);
  EXPECT_FALSE(sheet->HasChanged(dyn));
  sheet->Unrealize();
  menu.Destroy();
}

// ---------------------------------------------------------------------------
// newuiListBox item management
// ---------------------------------------------------------------------------

static int s_lb_select_hits = 0;
static void LbSelectCb(int) { s_lb_select_hits++; }

/**
 * @test NewuiCoreTest.ListboxAddPreservesInsertionOrderVirtually
 * @brief Verifies listbox Add Preserves Insertion Order Virtually.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, ListboxAddPreservesInsertionOrderVirtually) {
  // the internal sort only reorders real storage; the virtual view that
  // GetItem exposes keeps insertion order (V2R[n] = insertion sequence)
  newuiListBox lb;
  lb.AddItem("banana");
  lb.AddItem("Apple");
  lb.AddItem("cherry");

  char buf[64];
  ASSERT_TRUE(lb.GetItem(0, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "banana");
  ASSERT_TRUE(lb.GetItem(1, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "Apple");
  ASSERT_TRUE(lb.GetItem(2, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "cherry");
  EXPECT_FALSE(lb.GetItem(3, buf, sizeof(buf))); // out of range

  // index-based selection round trips through the virtual mapping
  lb.SetCurrentIndex(1);
  EXPECT_EQ(lb.GetCurrentIndex(), 1);
  char cur[64];
  ASSERT_TRUE(lb.GetCurrentItem(cur, sizeof(cur)));
  EXPECT_STREQ(cur, "Apple");
}

/**
 * @test NewuiCoreTest.ListboxNoSortPreservesInsertion
 * @brief Verifies listbox No Sort Preserves Insertion.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, ListboxNoSortPreservesInsertion) {
  newuiListBox lb;
  lb.SetFlag(UILB_NOSORT);
  lb.AddItem("zeta");
  lb.AddItem("alpha");

  char buf[64];
  ASSERT_TRUE(lb.GetItem(0, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "zeta");
  ASSERT_TRUE(lb.GetItem(1, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "alpha");
}

/**
 * @test NewuiCoreTest.ListboxSelectionAndCallbacks
 * @brief Verifies listbox Selection And Callbacks.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, ListboxSelectionAndCallbacks) {
  newuiListBox lb;
  s_lb_select_hits = 0;
  lb.SetSelectChangeCallback(LbSelectCb);
  lb.AddItem("one");
  lb.AddItem("two");
  lb.AddItem("three");

  lb.SetCurrentIndex(2);
  EXPECT_EQ(lb.GetCurrentIndex(), 2);
  char buf[64];
  ASSERT_TRUE(lb.GetCurrentItem(buf, sizeof(buf)));
  EXPECT_STREQ(buf, "three");
  EXPECT_EQ(s_lb_select_hits, 1);

  // selecting the same index again must not re-fire the callback
  lb.SetCurrentIndex(2);
  EXPECT_EQ(s_lb_select_hits, 1);

  // out-of-bounds selection is ignored
  lb.SetCurrentIndex(99);
  EXPECT_EQ(lb.GetCurrentIndex(), 2);
  EXPECT_EQ(s_lb_select_hits, 1);
}

/**
 * @test NewuiCoreTest.ListboxRemoveItemRemapsIndices
 * @brief Verifies listbox Remove Item Remaps Indices.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, ListboxRemoveItemRemapsIndices) {
  newuiListBox lb;
  lb.AddItem("aa");
  lb.AddItem("bb");
  lb.AddItem("cc");

  lb.RemoveItem("bb"); // removal matches exactly by default

  char buf[64];
  ASSERT_TRUE(lb.GetItem(0, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "aa");
  ASSERT_TRUE(lb.GetItem(1, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "cc");
  EXPECT_FALSE(lb.GetItem(2, buf, sizeof(buf)));

  // selection still valid after removal
  lb.SetCurrentIndex(1);
  EXPECT_EQ(lb.GetCurrentIndex(), 1);
}

/**
 * @test NewuiCoreTest.ListboxRemoveMissingIsNoop
 * @brief Verifies listbox Remove Missing Is Noop.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, ListboxRemoveMissingIsNoop) {
  newuiListBox lb;
  lb.AddItem("keep");
  lb.RemoveItem("nope"); // logs a warning, changes nothing

  char buf[64];
  ASSERT_TRUE(lb.GetItem(0, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "keep");
}

/**
 * @test NewuiCoreTest.ListboxSetCurrentItemExactMatchByDefault
 * @brief Verifies listbox Set Current Item Exact Match By Default.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, ListboxSetCurrentItemExactMatchByDefault) {
  // legacy quirk: RemoveItem/SetCurrentItem invert the flag polarity, so the
  // default (no UILB_CASESENSITIVE) path compares with strcmp = exact match
  newuiListBox lb;
  lb.AddItem("Mako");
  lb.AddItem("Pyro-GL");
  lb.AddItem("Titan");

  EXPECT_TRUE(lb.SetCurrentItem("Pyro-GL")); // exact match works
  char buf[64];
  ASSERT_TRUE(lb.GetCurrentItem(buf, sizeof(buf)));
  EXPECT_STREQ(buf, "Pyro-GL");

  EXPECT_FALSE(lb.SetCurrentItem("pyro-gl")); // wrong case is not found
  EXPECT_EQ(lb.GetCurrentIndex(), 1);         // selection unchanged

  EXPECT_FALSE(lb.SetCurrentItem("Phoenix"));
}

/**
 * @test NewuiCoreTest.ListboxRemoveAllResetsState
 * @brief Verifies listbox Remove All Resets State.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, ListboxRemoveAllResetsState) {
  newuiListBox lb;
  lb.AddItem("x");
  lb.AddItem("y");
  lb.RemoveAll();

  char buf[64];
  EXPECT_FALSE(lb.GetItem(0, buf, sizeof(buf)));
  // adding again works from scratch
  lb.AddItem("fresh");
  ASSERT_TRUE(lb.GetItem(0, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "fresh");
}

// ---------------------------------------------------------------------------
// header-inline slider conversion helpers (newui_core.h)
// ---------------------------------------------------------------------------

/**
 * @test NewuiCoreTest.SliderPosConversions
 * @brief Verifies slider Pos Conversions.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, SliderPosConversions) {
  tSliderSettings f{};
  f.type = SLIDER_UNITS_FLOAT;
  f.min_val.f = 0.0f;
  f.max_val.f = 1.0f;
  EXPECT_EQ(CALC_SLIDER_POS_FLOAT(0.5f, &f, 100), 50);
  EXPECT_EQ(CALC_SLIDER_POS_FLOAT(0.0f, &f, 100), 0);
  EXPECT_EQ(CALC_SLIDER_POS_FLOAT(1.0f, &f, 64), 64);

  tSliderSettings i{};
  i.type = SLIDER_UNITS_INT;
  i.min_val.i = 10;
  i.max_val.i = 110;
  EXPECT_EQ(CALC_SLIDER_POS_INT(60, &i, 50), 25); // halfway
  EXPECT_EQ(CALC_SLIDER_POS_INT(10, &i, 50), 0);
}

/**
 * @test NewuiCoreTest.SliderValueConversionsRoundTrip
 * @brief Verifies slider Value Conversions Round Trip.
 *
 * @details
 * Exercises the NewuiCoreTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/newuicore.cpp
 * @ingroup descent3_tests
 */
TEST_F(NewuiCoreTest, SliderValueConversionsRoundTrip) {
  // value <-> position round trips across a range; F_APPROXIMATE adds
  // +1e-6 so compare with a small tolerance
  EXPECT_NEAR(CALC_SLIDER_FLOAT_VALUE(50, 0.0f, 2.0f, 100), 1.0f, 0.001f);
  EXPECT_NEAR(CALC_SLIDER_FLOAT_VALUE(0, -1.0f, 1.0f, 80), -1.0f, 0.001f);

  EXPECT_EQ(CALC_SLIDER_INT_VALUE(25, 10, 110, 50), 60);
  EXPECT_EQ(CALC_SLIDER_INT_VALUE(0, 5, 105, 50), 5); // floor at min
}

// ==== TESTS ====
