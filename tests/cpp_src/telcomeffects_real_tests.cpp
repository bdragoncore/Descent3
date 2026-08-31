/**
 * @file telcomeffects_real_tests.cpp
 * @brief Tests for TelComEffects.cpp 1292 lines — the TelCom effect list.
 *
 * @details
 * system. The REAL source is compiled in; bitmap, sound, and movie
 * backends are stubbed. Covers creation ordering (buttons sort to
 * the tail), focus hand-off on free, event queue FIFO with overflow
 * drop, id lookup, and button hit testing.
 *
 * This harness validates the behavior of `Descent3/TelComEffects.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/TelComEffects.cpp`
 * @par Harness
 * `telcomeffects_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/TelComEffects.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "TelComEffects.h"
#include "mem.h"
#include "TelCom.h"

int glitch_dx = 0, glitch_dy = 0; // normally in TelcomEffectsRender.cpp

// ---- backend stubs ----
void bm_FreeBitmap(int) {}
void bm_DestroyChunkedBitmap(chunked_bitmap *) {}
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, int) {}
void rend_DrawChunkedBitmap(chunked_bitmap *, int, int, unsigned char) {}

// per-type renderers live in TelcomEffectsRender.cpp
#define STUB_RENDER(name) void name(tceffect *, float, int, int, bool) {}
STUB_RENDER(RenderBackground)
STUB_RENDER(RenderBmpBlur)
STUB_RENDER(RenderBmpInvert)
STUB_RENDER(RenderBmpScanline)
STUB_RENDER(RenderBmpStatic)
STUB_RENDER(RenderBmpStretch)
STUB_RENDER(RenderButton)
STUB_RENDER(RenderMovie)
STUB_RENDER(RenderPolyModel)
STUB_RENDER(RenderSound)
STUB_RENDER(RenderTextFade)
STUB_RENDER(RenderTextStatic)
STUB_RENDER(RenderTextType)
void EndMovie(tCinematic *) {}
tCinematic *StartMovie(const char *, bool) { return nullptr; }
int FindSoundName(const char *) { return -1; }
void TelcomStopSound(int) {}
void textaux_WordWrap(const char *, char *, int, int) {}
tTelComInfo Telcom_system;
void vm_MakeIdentity(matrix *m) {
  m->rvec = {1.0f, 0.0f, 0.0f};
  m->uvec = {0.0f, 1.0f, 0.0f};
  m->fvec = {0.0f, 0.0f, 1.0f};
}

// bitmap touchpoints
int bm_AllocBitmap(int, int, int) { return -1; }
int bm_AllocLoadFileBitmap(const char *, int, int) { return -1; }
bool bm_CreateChunkedBitmap(int, chunked_bitmap *) { return false; }
uint16_t *bm_data(int, int) { return nullptr; }
int bm_w(int, int) { return 0; }
int bm_h(int, int) { return 0; }

// misc globals
int Game_fonts[8] = {};
bms_bitmap GameBitmaps[MAX_BITMAPS];

static const int SCR = 3; // screen under test

/**
 * @test TelcomEfx.CreateOrdersButtonsLastAndGrantsFirstFocus
 * @brief Verifies create Orders Buttons Last And Grants First Focus.
 *
 * @details
 * Exercises the TelcomEfx code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComEffects.cpp
 * @ingroup descent3_tests
 */
TEST(TelcomEfx, CreateOrdersButtonsLastAndGrantsFirstFocus) {
  EfxInit();

  int text0 = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 100);
  ASSERT_EQ(text0, 0);
  EXPECT_EQ(Screen_roots[SCR], 0);
  EXPECT_EQ(TCEffects[0].next, -1);

  // first tab stop on the screen takes focus
  int btn1 = EfxCreate(EFX_BUTTON, 0, SCR, 101, true);
  int btn2 = EfxCreate(EFX_BUTTON, 0, SCR, 102, true); // second tab stop: no focus
  int text3 = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 103); // non-button after buttons

  // chain order: text0 -> btn1 -> btn2 -> text3? No — non-buttons insert BEFORE buttons
  EXPECT_EQ(TCEffects[text0].next, text3);
  EXPECT_EQ(TCEffects[text3].prev, text0);
  EXPECT_EQ(TCEffects[text3].next, btn1);
  EXPECT_EQ(TCEffects[btn1].next, btn2);
  EXPECT_EQ(TCEffects[btn2].next, -1);
  EXPECT_EQ(Screen_roots[SCR], text0);

  EXPECT_TRUE(TCEffects[btn1].has_focus);  // first tab stop won
  EXPECT_FALSE(TCEffects[btn2].has_focus); // later tab stops wait

  // per-effect init defaults
  EXPECT_EQ(TCEffects[btn1].buttoninfo.sibling, -1);
  EXPECT_FLOAT_EQ(TCEffects[text0].speed, 1.0f);
  EXPECT_EQ(TCEffects[text0].color, GR_GREEN);
}

/**
 * @test TelcomEfx.FreeUnlinksAndHandsOffFocus
 * @brief Verifies free Unlinks And Hands Off Focus.
 *
 * @details
 * Exercises the TelcomEfx code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComEffects.cpp
 * @ingroup descent3_tests
 */
TEST(TelcomEfx, FreeUnlinksAndHandsOffFocus) {
  EfxInit();

  int a = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 200);
  int b = EfxCreate(EFX_BUTTON, 0, SCR, 201, true); // gets focus
  int c = EfxCreate(EFX_BUTTON, 0, SCR, 202, true);

  ASSERT_TRUE(TCEffects[b].has_focus);

  // freeing the focused middle node relinks c after a and moves focus to c
  EfxFreeEffect(&TCEffects[b]);
  TCEffects[b].type = EFX_NONE;
  TCEffects[b].has_focus = false;

  EXPECT_EQ(TCEffects[a].next, c);
  EXPECT_EQ(TCEffects[c].prev, a);
  EXPECT_TRUE(TCEffects[c].has_focus);

  // free everything: root slot clears when the last node goes
  EfxFreeEffect(&TCEffects[c]);
  TCEffects[c].type = EFX_NONE;
  EfxFreeEffect(&TCEffects[a]);
  TCEffects[a].type = EFX_NONE;
  EXPECT_EQ(Screen_roots[SCR], -1);
}

/**
 * @test TelcomEfx.EventQueueFIFOAndOverflowDrop
 * @brief Verifies event Queue FIFOAnd Overflow Drop.
 *
 * @details
 * Exercises the TelcomEfx code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComEffects.cpp
 * @ingroup descent3_tests
 */
TEST(TelcomEfx, EventQueueFIFOAndOverflowDrop) {
  EfxInit();
  int e = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 300);
  tceffect *tce = &TCEffects[e];
  tTCEvent evt;

  EXPECT_FALSE(PopEvent(tce, &evt)); // empty

  for (int i = 0; i < MAX_EFFECT_EVENTS; i++)
    SendEventToEffect(e, 50 + i, i, 1000 + i);

  // FIFO order across the 5 slots
  for (int i = 0; i < MAX_EFFECT_EVENTS; i++) {
    ASSERT_TRUE(PopEvent(tce, &evt));
    EXPECT_EQ(evt.id, 50 + i);
    EXPECT_EQ(evt.parms[0], i);
    EXPECT_EQ(evt.parms[1], 1000 + i);
  }
  EXPECT_FALSE(PopEvent(tce, &evt));

  // overflow: filling the queue then sending one more drops the NEWEST
  for (int i = 0; i < MAX_EFFECT_EVENTS + 1; i++)
    SendEventToEffect(e, 70 + i, i, 0);
  for (int i = 0; i < MAX_EFFECT_EVENTS; i++) {
    ASSERT_TRUE(PopEvent(tce, &evt));
    EXPECT_EQ(evt.id, 70 + i); // 76 never queued
  }
  EXPECT_FALSE(PopEvent(tce, &evt));
}

/**
 * @test TelcomEfx.IdLookupAndButtonHitTest
 * @brief Verifies id Lookup And Button Hit Test.
 *
 * @details
 * Exercises the TelcomEfx code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComEffects.cpp
 * @ingroup descent3_tests
 */
TEST(TelcomEfx, IdLookupAndButtonHitTest) {
  EfxInit();

  int t = EfxCreate(EFX_TEXT_STATIC, 0, SCR, 400);
  int b = EfxCreate(EFX_BUTTON, 0, SCR, 401);
  tceffect *tcb = &TCEffects[b];
  tcb->pos_x = 10;
  tcb->pos_y = 20;
  tcb->w = 30;
  tcb->h = 10;

  EXPECT_EQ(GetEfxNumFromID(400, SCR), t);
  EXPECT_EQ(GetEfxNumFromID(401, SCR), b);
  EXPECT_EQ(GetEfxNumFromID(999, SCR), -1);   // unknown id
  EXPECT_EQ(GetEfxNumFromID(400, SCR + 1), -1); // other screen

  // corners inclusive; only buttons participate
  EXPECT_EQ(FindButtonEffectByXY(10, 20, SCR), b);
  EXPECT_EQ(FindButtonEffectByXY(40, 30, SCR), b); // x+w, y+h inclusive
  EXPECT_EQ(FindButtonEffectByXY(9, 25, SCR), -1);
  EXPECT_EQ(FindButtonEffectByXY(41, 25, SCR), -1);

  // the text effect at same coords is invisible to hit testing
  TCEffects[t].pos_x = 10;
  TCEffects[t].pos_y = 20;
  TCEffects[t].w = 30;
  TCEffects[t].h = 10;
  EXPECT_NE(FindButtonEffectByXY(15, 25, SCR), t);
}

/**
 * @test TelcomEfx.DestroyScreenClearsOnlyThatScreen
 * @brief Verifies destroy Screen Clears Only That Screen.
 *
 * @details
 * Exercises the TelcomEfx code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/TelComEffects.cpp
 * @ingroup descent3_tests
 */
TEST(TelcomEfx, DestroyScreenClearsOnlyThatScreen) {
  EfxInit();
  int keep = EfxCreate(EFX_TEXT_STATIC, 0, SCR + 1, 500);
  int gone = EfxCreate(EFX_BUTTON, 0, SCR, 501, true);

  EfxDestroyScreen(SCR);
  EXPECT_EQ(Screen_roots[SCR], -1);
  EXPECT_EQ(TCEffects[gone].type, EFX_NONE);
  EXPECT_EQ(TCEffects[gone].id, INVALID_EFFECT_ID);
  EXPECT_EQ(TCEffects[keep].type, EFX_TEXT_STATIC); // neighbor untouched
}
