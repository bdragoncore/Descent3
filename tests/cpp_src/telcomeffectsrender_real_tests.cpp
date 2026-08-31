/**
 * @file telcomeffectsrender_real_tests.cpp
 * @brief Tests for TelcomEffectsRender.cpp 998 lines — the real per-type.
 *
 * @details
 * renderers, linked together with the REAL TelComEffects.cpp so the
 * effect tables, event queues, and focus plumbing are live. The
 * renderer backend (bitmaps, blits, grtext) is stubbed; bitmap data
 * flows through fake in-memory buffers so blur math and stage
 * progression are observable.
 *
 * This harness validates the behavior of `Descent3/TelcomEffectsRender.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/TelcomEffectsRender.cpp`
 * @par Harness
 * `telcomeffectsrender_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/TelcomEffectsRender.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>

#include "TelComEffects.h"
#include "TelCom.h"
#include "bitmap.h"
#include "hlsoundlib.h"
#include "cinematics.h"
#include "vecmat.h"

// public in TelcomEffectsRender.cpp but missing from headers
void BlurBitmapArea(uint16_t *srcbm, uint16_t *dstbm, int16_t width, int16_t height, int16_t startx, int16_t starty,
                    int16_t bmw);
int GetTextEffectWithTabStopsOnScreen(int screen);

// ---- renderer backend stubs ----
static uint16_t g_fake_bm_data[8][256 * 16]; // handle -> pixels (16x16 max used)
static int g_bm_w[8], g_bm_h[8];
static int g_clear_count = -1;
static int g_blit_count = 0;

void bm_ClearBitmap(int handle) { g_clear_count++; }
uint16_t *bm_data(int handle, int miplevel) { return g_fake_bm_data[handle]; }
int bm_w(int handle, int miplevel) { return g_bm_w[handle]; }
int bm_h(int handle, int miplevel) { return g_bm_h[handle]; }
// BltBmpToScreen/UploadChunk are real (TelComEffects.cpp); count the
// blits they forward into the rend_ stub
void rend_DrawChunkedBitmap(chunked_bitmap *cb, int dx, int dy, unsigned char) { g_blit_count++; }
void rend_SetColorModel(color_model) {}
void rend_SetLighting(light_state) {}
void rend_SetWrapType(wrap_type) {}
void rend_SetAlphaType(signed char) {}
void rend_SetAlphaValue(unsigned char) {}
void rend_SetFiltering(signed char) {}
void rend_SetOverlayType(unsigned char) {}
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, int) {}
void rend_DrawScaledBitmap(int, int, int, int, int, float, float, float, float, int, float const *) {}
void rend_FillRect(unsigned int, int, int, int, int) {}
int bm_AllocBitmap(int, int, int) { return -1; }
int bm_AllocLoadFileBitmap(const char *, int, int) { return -1; }
bool bm_CreateChunkedBitmap(int, chunked_bitmap *) { return false; }
void bm_DestroyChunkedBitmap(chunked_bitmap *) {}
void bm_FreeBitmap(int) {}
int FindSoundName(const char *) { return -1; }
bool FrameMovie(tCinematic *, int, int, bool) { return false; }
bms_bitmap GameBitmaps[MAX_BITMAPS];
int Game_fonts[8] = {};

// hlsSystem backend — sound playback not under test
hlsSystem::hlsSystem() {}
void hlsSystem::KillSoundLib(bool) {}
int hlsSystem::Play2dSound(int sound_index, float volume, float pan, unsigned short) { return -1; }

extern "C" {
uint8_t grtext_GetAlpha(void) { return 255; }
void grtext_SetAlpha(uint8_t) {}
void grtext_SetColor(ddgr_color) {}
void grtext_SetFont(int) {}
void grtext_Printf(int, int, const char *, ...) {}
int grfont_GetHeight(int) { return 20; }
}

void textaux_WordWrap(const char *, char *, int, int) {}
// single-line source: first call copies into dest and reports done
const char *textaux_CopyTextLine(const char *src, char *dest) {
  if (!src || !*src)
    return NULL;
  while (*src && *src != '\n')
    *dest++ = *src++;
  *dest = '\0';
  return (*src == '\n') ? (src + 1) : NULL;
}
void TelcomStartSound(int) {}
void TelcomStopSound(int) {}
tCinematic *StartMovie(const char *, bool) { return nullptr; }
void EndMovie(tCinematic *) {}
bool FrameMovie(tCinematic *, int, int) { return false; }

hlsSystem Sound_system;
tTelComInfo Telcom_system;

// Faithful replicas of two TelCom.cpp hooks (kept out to avoid linking
// the whole telcom UI): SendEvent appends to the system queue,
// SetFocus strips focus from the current holder on that screen and
// grants it to a tab stop.
void TelComSendEvent(int event_num, int parm1, int parm2) {
  for (int i = 0; i < MAX_TELCOM_EVENTS; i++) {
    if (Telcom_system.Telcom_event_queue[i].id == -1) {
      Telcom_system.Telcom_event_queue[i].id = event_num;
      Telcom_system.Telcom_event_queue[i].parms[0] = parm1;
      Telcom_system.Telcom_event_queue[i].parms[1] = parm2;
      return;
    }
  }
}
void TelComSetFocusOnEffect(int efxnum) {
  if (TCEffects[efxnum].type == EFX_NONE)
    return;
  if (TCEffects[efxnum].has_focus)
    return;
  if (!TCEffects[efxnum].tab_stop)
    return;
  int screen = TCEffects[efxnum].screen;
  for (int i = 0; i < MAX_TCEFFECTS; i++) {
    if (TCEffects[i].type != EFX_NONE && TCEffects[i].screen == screen)
      TCEffects[i].has_focus = false;
  }
  TCEffects[efxnum].has_focus = true;
}

/**
 * @test TelcomRenderEfx.BlurBitmapAreaAveragesOnlyOpaquePixelsOverWholeBlock
 * @brief Verifies blur Bitmap Area Averages Only Opaque Pixels Over Whole Block.
 *
 * @details
 * Exercises the TelcomRenderEfx code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelcomEffectsRender.cpp
 * @ingroup descent3_tests
 */
TEST(TelcomRenderEfx, BlurBitmapAreaAveragesOnlyOpaquePixelsOverWholeBlock) {
  const uint16_t SENTINEL = 0x1234;

  // 2x2 block inside an 8-wide bitmap: two opaque reds, two transparent
  uint16_t src[32];
  uint16_t dst[32];
  memset(src, 0, sizeof(src));
  for (auto &p : dst)
    p = SENTINEL;

  uint16_t red = OPAQUE_FLAG | GR_RGB16(31, 0, 0);
  src[0 * 8 + 0] = red; // (0,0)
  src[0 * 8 + 1] = red; // (1,0)
  // (0,1),(1,1) stay transparent (no OPAQUE_FLAG)

  BlurBitmapArea(src, dst, 2, 2, 0, 0, 8);

  // the average divides by the FULL block area, not by opaque count:
  // both pixels contribute their channel sum before the divide
  ddgr_color c = GR_16_TO_COLOR(red);
  uint16_t expected =
      OPAQUE_FLAG | GR_RGB16(GR_COLOR_RED(c) * 2 / 4, GR_COLOR_GREEN(c) * 2 / 4, GR_COLOR_BLUE(c) * 2 / 4);
  EXPECT_EQ(dst[0 * 8 + 0], expected);
  EXPECT_EQ(dst[0 * 8 + 1], expected);
  EXPECT_EQ(dst[1 * 8 + 0], expected);
  EXPECT_EQ(dst[1 * 8 + 1], expected);
  // neighbours untouched
  EXPECT_EQ(dst[0 * 8 + 2], SENTINEL);
  EXPECT_EQ(dst[2 * 8 + 0], SENTINEL);

  // fully transparent block collapses to zero rather than dividing by nothing
  BlurBitmapArea(src, dst, 2, 1, 4, 0, 8); // row 0 cols 4-5: all transparent
  EXPECT_EQ(dst[0 * 8 + 4], 0);
  EXPECT_EQ(dst[0 * 8 + 5], 0);

  // degenerate block writes nothing
  BlurBitmapArea(src, dst, 0, 0, 6, 0, 8);
  EXPECT_EQ(dst[0 * 8 + 6], SENTINEL); // untouched by the empty region
}

/**
 * @test TelcomRenderEfx.TabStopCounterIgnoresButtonsAndUntabbableText
 * @brief Verifies tab Stop Counter Ignores Buttons And Untabbable Text.
 *
 * @details
 * Exercises the TelcomRenderEfx code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelcomEffectsRender.cpp
 * @ingroup descent3_tests
 */
TEST(TelcomRenderEfx, TabStopCounterIgnoresButtonsAndUntabbableText) {
  EfxInit();
  EfxDestroyAllScreens();
  const int SCR = 3;

  int t1 = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 100, true);  // text tab stop
  int b1 = EfxCreate(EFX_BUTTON, 0, SCR, 101, true);       // button tab stop: not a text type
  int t2 = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 102, false); // text without tab stop
  ASSERT_GE(t1, 0);

  EXPECT_EQ(GetTextEffectWithTabStopsOnScreen(SCR), 1);

  // a second tabbable text item doubles the count
  int t3 = EfxCreate(EFX_TEXT_FADE, 0, SCR, 103, true);
  EXPECT_EQ(GetTextEffectWithTabStopsOnScreen(SCR), 2);

  // other screens report nothing
  EXPECT_EQ(GetTextEffectWithTabStopsOnScreen(SCR + 1), 0);
}

class TelcomRender : public ::testing::Test {
protected:
  void SetUp() override {
    EfxInit();
    EfxDestroyAllScreens();
    memset(&Telcom_system, 0, sizeof(Telcom_system));
    for (auto &e : Telcom_system.Telcom_event_queue)
      e.id = -1;
    memset(g_fake_bm_data, 0, sizeof(g_fake_bm_data));
    g_clear_count = -1;
    g_blit_count = 0;
    glitch_dx = glitch_dy = 0;
  }
};

/**
 * @test TelcomRender.TextStaticScrollEventsClampAndRecomputeScrollFlags
 * @brief Verifies text Static Scroll Events Clamp And Recompute Scroll Flags.
 *
 * @details
 * Exercises the TelcomRender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelcomEffectsRender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TelcomRender, TextStaticScrollEventsClampAndRecomputeScrollFlags) {
  const int SCR = 5;
  int num = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 200);
  ASSERT_GE(num, 0);
  tceffect *tce = &TCEffects[num];

  static char buf[] = "hello";
  tce->text_buffer = buf;
  tce->pos_x = 0;
  tce->pos_y = 0;
  tce->h = 10; // shorter than one line: content overflows -> scroll_d stays true
  tce->start_time = 0;

  Telcom_system.Monitor_coords[tce->monitor] = {0, 100, 0, 100};

  // SCROLLUP clamps at the first line
  tce->textinfo.line_index = 0;
  tce->textinfo.scroll_d = true;
  SendEventToEffect(num, TEVT_SCROLLUP);
  RenderScreen(SCR, &Telcom_system, 0.01f);
  EXPECT_EQ(tce->textinfo.line_index, 0);

  // SCROLLDOWN advances while allowed...
  SendEventToEffect(num, TEVT_SCROLLDOWN);
  RenderScreen(SCR, &Telcom_system, 0.01f);
  EXPECT_EQ(tce->textinfo.line_index, 1);

  // ...and the geometry pass re-enables scrolling here (10 < line height)
  EXPECT_TRUE(tce->textinfo.scroll_d);

  // with a tall box the same content fits: scroll_d flips off afterwards
  int num2 = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 201);
  ASSERT_GE(num2, 0);
  tceffect *tce2 = &TCEffects[num2];
  tce2->text_buffer = buf;
  tce2->pos_x = 0;
  tce2->pos_y = 0;
  tce2->h = 5000;
  tce2->start_time = 0;
  tce2->textinfo.scroll_d = true;
  SendEventToEffect(num2, TEVT_SCROLLDOWN);
  RenderScreen(SCR, &Telcom_system, 0.01f);
  EXPECT_EQ(tce2->textinfo.line_index, 1); // moved while still enabled
  EXPECT_FALSE(tce2->textinfo.scroll_d);   // recomputed from layout

  // deferred start renders nothing but burns down the timer
  int num3 = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 202);
  ASSERT_GE(num3, 0);
  TCEffects[num3].start_time = 1.0f;
  TCEffects[num3].text_buffer = buf;
  int blits_before = g_blit_count;
  RenderScreen(SCR, &Telcom_system, 0.4f);
  EXPECT_FLOAT_EQ(TCEffects[num3].start_time, 0.6f);
  EXPECT_FALSE(TCEffects[num3].has_focus);
}

/**
 * @test TelcomRender.BmpBlurAdvancesStagesThenFreezesAtCompletion
 * @brief Verifies bmp Blur Advances Stages Then Freezes At Completion.
 *
 * @details
 * Exercises the TelcomRender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelcomEffectsRender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TelcomRender, BmpBlurAdvancesStagesThenFreezesAtCompletion) {
  const int SCR = 6;
  const int SRC = 40, DST = 41, PIECE = 42;
  g_bm_w[SRC] = g_bm_h[SRC] = 16;
  g_bm_w[DST] = g_bm_h[DST] = 16;
  g_bm_w[PIECE] = g_bm_h[PIECE] = 16;

  // uniform opaque field: every blur stage yields the identical colour,
  // making the piece-copy easy to verify without redoing the math
  uint16_t col = OPAQUE_FLAG | GR_RGB16(31, 10, 0);
  for (int i = 0; i < 256; i++)
    g_fake_bm_data[SRC][i] = col;

  int num = EfxCreate(EFX_BMP_BLUR, 0, SCR, 300);
  ASSERT_GE(num, 0);
  tceffect *tce = &TCEffects[num];
  tce->flags = TC_BMPF_IN;
  tce->speed = 1.0f;
  tce->bmpinfo.bm_handle = SRC;
  tce->bmpinfo.temp_bmhandle = DST;
  tce->bmpinfo.stage = 0;
  static int piece_array[1] = {PIECE};
  tce->bmpinfo.chunk_bmp.bm_array = piece_array;
  tce->bmpinfo.chunk_bmp.w = 1;
  tce->bmpinfo.chunk_bmp.h = 1;
  tce->bmpinfo.chunk_bmp.pw = 16;
  tce->bmpinfo.chunk_bmp.ph = 16;
  tce->start_time = 0;
  Telcom_system.Monitor_coords[tce->monitor] = {0, 100, 0, 100};

  float frametime = 1.0f;
  // done is checked BEFORE the pass body: stage climbs one past
  // BLUR_STAGES (5) and only freezes once a pass starts above it
  for (int pass = 1; pass <= BLUR_STAGES + 1; pass++) {
    RenderScreen(SCR, &Telcom_system, frametime);
    EXPECT_FLOAT_EQ(tce->bmpinfo.stage, (float)pass);
  }

  // every pixel landed in the single chunk piece, colour preserved
  for (int i = 0; i < 256; i++)
    ASSERT_EQ(g_fake_bm_data[PIECE][i], col);

  // stage 6+ is "done": blits happen but stage no longer moves; the
  // temp bitmap is still cleared every active pass (even done ones)
  int clears_before_done = g_clear_count;
  float age_before = tce->age;
  RenderScreen(SCR, &Telcom_system, frametime);
  EXPECT_FLOAT_EQ(tce->bmpinfo.stage, (float)(BLUR_STAGES + 1));
  EXPECT_GT(g_blit_count, 0);
  EXPECT_GT(tce->age, age_before);              // age still ticks on done passes
  EXPECT_EQ(g_clear_count, clears_before_done + 1); // clear happens pre-done-check

  // TC_BMPF_OUT below zero also freezes
  int num2 = EfxCreate(EFX_BMP_BLUR, 0, SCR, 301);
  ASSERT_GE(num2, 0);
  TCEffects[num2].flags = TC_BMPF_OUT;
  TCEffects[num2].bmpinfo.stage = -0.5f;
  TCEffects[num2].bmpinfo.temp_bmhandle = DST;
  static int piece2[1] = {PIECE};
  TCEffects[num2].bmpinfo.chunk_bmp.bm_array = piece2;
  TCEffects[num2].bmpinfo.chunk_bmp.w = 1;
  TCEffects[num2].bmpinfo.chunk_bmp.pw = 16;
  TCEffects[num2].bmpinfo.chunk_bmp.ph = 16;
  TCEffects[num2].start_time = 0;
  RenderScreen(SCR, &Telcom_system, frametime);
  EXPECT_FLOAT_EQ(TCEffects[num2].bmpinfo.stage, -0.5f); // untouched
}

/**
 * @test TelcomRender.ButtonMouseEnterStealsFocusWhenFlagged
 * @brief Verifies button Mouse Enter Steals Focus When Flagged.
 *
 * @details
 * Exercises the TelcomRender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelcomEffectsRender.cpp
 * @ingroup descent3_tests
 */
TEST_F(TelcomRender, ButtonMouseEnterStealsFocusWhenFlagged) {
  const int SCR = 7;
  int a = EfxCreate(EFX_BUTTON, 0, SCR, 400, true);
  int b = EfxCreate(EFX_BUTTON, 0, SCR, 401, true);
  ASSERT_GE(a, 0);
  ASSERT_GE(b, 0);
  EXPECT_TRUE(TCEffects[a].has_focus);  // first tab stop owns focus
  EXPECT_FALSE(TCEffects[b].has_focus);

  // mouse-over only steals focus for buttons flagged OBF_MOUSEOVERFOCUS
  TCEffects[b].flags |= OBF_MOUSEOVERFOCUS;
  SendEventToEffect(b, TEVT_MOUSEENTER);
  RenderScreen(SCR, &Telcom_system, 0.01f);
  EXPECT_TRUE(TCEffects[b].has_focus);
  EXPECT_FALSE(TCEffects[a].has_focus);

  // without the flag the hover is ignored
  int c = EfxCreate(EFX_BUTTON, 0, SCR, 402, true);
  ASSERT_GE(c, 0);
  TCEffects[c].flags &= ~OBF_MOUSEOVERFOCUS;
  SendEventToEffect(c, TEVT_MOUSEENTER);
  RenderScreen(SCR, &Telcom_system, 0.01f);
  EXPECT_FALSE(TCEffects[c].has_focus);
  EXPECT_TRUE(TCEffects[b].has_focus);
}
