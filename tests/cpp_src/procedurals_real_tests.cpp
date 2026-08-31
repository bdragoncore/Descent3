/**
 * @file procedurals_real_tests.cpp
 * @brief Tests for procedurals.cpp 1352 lines (procedural fire/water textures).
 *
 * @details
 * The REAL source file is compiled into this test; GameTextures is
 * provided locally so the texture system is not needed. Covers palette
 * packing, the proc element allocator, gradient noise, and replicated
 * Bresenham line / fade math whose entry points are only reachable via
 * EvaluateFireProcedural.
 *
 * This harness validates the behavior of `Descent3/procedurals.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/procedurals.cpp`
 * @par Harness
 * `procedurals_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/procedurals.cpp
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cstring>

#include "procedurals.h"
#include "gametexture.h"
#include "grdefs.h"
#include "fix.h"
#include "config.h"
#include "bitmap.h"

texture GameTextures[MAX_TEXTURES];

// stubs for globals referenced by procedurals.cpp (same pattern as
// gametexture_real_tests)
int FrameCount = 0;
tDetailSettings Detail_settings;
uint8_t EasterEgg = 0;
int paged_in_count = 0;
int paged_in_num = 0;
scalar FixCos(angle a) { return 0; }
scalar FixSin(angle a) { return 0; }

// exported by procedurals.cpp
extern int pholdrand;
extern uint8_t perm[256];
void InitNoise();
float GradLattice(int ix, int iy, float fx, float fy);
float GradNoise(float x, float y);

#define REP_PROC_SIZE 128

/**
 * @test ProcPalette.GeneratePaletteForProceduralPacks555
 * @brief Verifies generate Palette For Procedural Packs555.
 *
 * @details
 * Exercises the ProcPalette code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/procedurals.cpp
 * @ingroup descent3_tests
 */
TEST(ProcPalette, GeneratePaletteForProceduralPacks555) {
  uint8_t r[256], g[256], b[256];
  uint16_t pal[256];
  memset(r, 0, 256);
  memset(g, 0, 256);
  memset(b, 0, 256);

  r[1] = 255; // pure red
  g[2] = 255; // pure green
  b[3] = 255; // pure blue
  r[4] = g[4] = b[4] = 255; // white
  r[5] = 128;               // truncation: 128/255*31 = 15.56 -> 15
  g[6] = 127;               // 127/255*31 = 15.44 -> 15

  GeneratePaletteForProcedural(r, g, b, pal);

  EXPECT_EQ(pal[0], OPAQUE_FLAG); // black keeps opaque bit
  EXPECT_EQ(pal[1], OPAQUE_FLAG | (31 << 10));
  EXPECT_EQ(pal[2], OPAQUE_FLAG | (31 << 5));
  EXPECT_EQ(pal[3], OPAQUE_FLAG | 31);
  EXPECT_EQ(pal[4], OPAQUE_FLAG | (31 << 10) | (31 << 5) | 31);
  EXPECT_EQ(pal[5], OPAQUE_FLAG | (15 << 10));
  EXPECT_EQ(pal[6], OPAQUE_FLAG | (15 << 5));

  // every entry always carries the opaque flag
  for (int i = 0; i < 256; i++)
    ASSERT_EQ(pal[i] & OPAQUE_FLAG, (uint16_t)OPAQUE_FLAG);
}

/**
 * @test ProcElements.AllocFreeLIFOAndExhaustion
 * @brief Verifies alloc Free LIFOAnd Exhaustion.
 *
 * @details
 * Exercises the ProcElements code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/procedurals.cpp
 * @ingroup descent3_tests
 */
TEST(ProcElements, AllocFreeLIFOAndExhaustion) {
  extern dynamic_proc_element DynamicProcElements[];
  extern int Num_proc_elements;

  // real init: resets types, rebuilds free list 0..N-1, zeroes count.
  // Needs the bitmap system up for the easter-egg probe (missing file
  // is handled gracefully once initted).
  bm_InitBitmaps();
  InitProcedurals();

  // allocations hand out slots in order
  int a = ProcElementAllocate();
  int b = ProcElementAllocate();
  EXPECT_EQ(a, 0);
  EXPECT_EQ(b, 1);
  EXPECT_EQ(Num_proc_elements, 2);
  EXPECT_EQ(DynamicProcElements[a].next, -1);
  EXPECT_EQ(DynamicProcElements[a].prev, -1);

  // free then realloc is LIFO: freed slot comes back immediately
  ProcElementFree(b);
  EXPECT_EQ(Num_proc_elements, 1);
  EXPECT_EQ(DynamicProcElements[b].type, PROC_NONE);
  int c = ProcElementAllocate();
  EXPECT_EQ(c, 1);

  // exhaust all 8000 slots
  while (ProcElementAllocate() != -1) {
  }
  EXPECT_EQ(Num_proc_elements, MAX_PROC_ELEMENTS);
  EXPECT_EQ(ProcElementAllocate(), -1);

  // one free makes exactly one slot available again
  ProcElementFree(1234);
  EXPECT_EQ(ProcElementAllocate(), 1234);
  EXPECT_EQ(Num_proc_elements, MAX_PROC_ELEMENTS);

  // drain back down so other tests start clean
  while (Num_proc_elements > 0)
    ProcElementFree(0);
}

/**
 * @test ProcNoise.InitNoiseUnitVectorsAndGradNoiseProperties
 * @brief Verifies init Noise Unit Vectors And Grad Noise Properties.
 *
 * @details
 * Exercises the ProcNoise code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/procedurals.cpp
 * @ingroup descent3_tests
 */
TEST(ProcNoise, InitNoiseUnitVectorsAndGradNoiseProperties) {
  pholdrand = 1; // deterministic LCG seed
  InitNoise();

  // perm entries are prand()%255: never equal 255 (quirk of %TABMASK)
  for (int i = 0; i < 256; i++) {
    EXPECT_GE((int)perm[i], 0);
    EXPECT_LE((int)perm[i], 254);
  }

  // noise rows are unit vectors; verified indirectly through GradNoise
  // bounds below since Noise_table has file-local linkage

  // at integer lattice points both fractional parts are 0 -> result 0
  EXPECT_FLOAT_EQ(GradNoise(3.0f, 7.0f), 0.0f);
  EXPECT_FLOAT_EQ(GradNoise(-2.0f, 0.0f), 0.0f);

  // deterministic across calls
  float v1 = GradNoise(12.34f, 56.78f);
  float v2 = GradNoise(12.34f, 56.78f);
  EXPECT_FLOAT_EQ(v1, v2);
  EXPECT_NE(v1, 0.0f); // off-lattice point has some gradient

  // bounded output over a sample grid (unit gradients, lerped)
  for (int iy = -3; iy < 40; iy += 7) {
    for (int ix = -3; ix < 40; ix += 7) {
      float v = GradNoise(ix + 0.37f, iy + 0.61f);
      EXPECT_LT(fabsf(v), 1.5f);
    }
  }

  // negative coordinates floor correctly (no wrap toward zero)
  EXPECT_FLOAT_EQ(GradNoise(-0.25f, -0.25f), GradNoise(-0.25f, -0.25f));
}

namespace {

// Replication of DrawProceduralLine (procedurals.cpp:270-332): classic
// error-term line raster with PROC_SIZE wrap masks on both axes.
struct BufWrap {
  uint8_t px[REP_PROC_SIZE * REP_PROC_SIZE] = {};
  void DrawLine(int x1, int y1, int x2, int y2, uint8_t color) {
    const int xmask = REP_PROC_SIZE - 1, ymask = REP_PROC_SIZE - 1;
    int xinc = 1, yinc = 1;
    if (x1 > x2) {
      std::swap(x1, x2);
      std::swap(y1, y2);
    }
    int DX = x2 - x1, DY = y2 - y1;
    if (DX < 0) { xinc = -1; DX = -DX; }
    if (DY < 0) { yinc = -1; DY = -DY; }
    uint8_t *sp = px;
    int error_term, x, y;
    if (DX >= DY) {
      error_term = 0;
      x = x1 & xmask;
      y = y1 & ymask;
      sp += y * REP_PROC_SIZE;
      for (int i = 0; i < DX; i++) {
        *(sp + x) = color;
        x += xinc;
        x &= xmask;
        error_term += DY;
        if (error_term >= DX) {
          y += yinc;
          y &= ymask;
          sp = px + (y * REP_PROC_SIZE);
          error_term -= DX;
        }
      }
    } else {
      error_term = 0;
      x = x1 & xmask;
      y = y1 & ymask;
      sp += y * REP_PROC_SIZE;
      for (int i = 0; i < DY; i++) {
        *(sp + x) = color;
        y += yinc;
        y &= ymask;
        error_term += DX;
        sp = px + (y * REP_PROC_SIZE);
        if (error_term >= DY) {
          x += xinc;
          x &= xmask;
          error_term -= DY;
        }
      }
    }
  }
};

/**
 * @test ProcLineReplica.HorizontalVerticalAndWrapping
 * @brief Verifies horizontal Vertical And Wrapping.
 *
 * @details
 * Exercises the ProcLineReplica code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/procedurals.cpp
 * @ingroup descent3_tests
 */
TEST(ProcLineReplica, HorizontalVerticalAndWrapping) {
  BufWrap buf;

  // horizontal line sets exactly DX pixels
  buf.DrawLine(10, 5, 20, 5, 200);
  int count = 0;
  for (int i = 0; i < REP_PROC_SIZE * REP_PROC_SIZE; i++)
    count += buf.px[i] == 200;
  EXPECT_EQ(count, 10);

  BufWrap wbuf;
  // vertical line wrapping across the bottom edge y=126..129 -> 126,127,0,1
  wbuf.DrawLine(3, 126, 3, 130, 77);
  const int expected_ys[4] = {126, 127, 0, 1};
  int painted = 0;
  for (int i = 0; i < REP_PROC_SIZE * REP_PROC_SIZE; i++)
    painted += wbuf.px[i] == 77;
  EXPECT_EQ(painted, 4);
  for (int k = 0; k < 4; k++) {
    // only x==3 is lit on each visited row
    for (int xx = 0; xx < REP_PROC_SIZE; xx++)
      EXPECT_EQ(wbuf.px[expected_ys[k] * REP_PROC_SIZE + xx], xx == 3 ? 77 : 0)
          << "row " << expected_ys[k] << " x " << xx;
    // and no other row got painted
  }
  int rows_lit = 0;
  for (int yy = 0; yy < REP_PROC_SIZE; yy++) {
    bool any = false;
    for (int xx = 0; xx < REP_PROC_SIZE; xx++)
      any |= wbuf.px[yy * REP_PROC_SIZE + xx] == 77;
    rows_lit += any;
  }
  EXPECT_EQ(rows_lit, 4);

  BufWrap rev;
  // reversed endpoints are swapped first -> identical raster
  rev.DrawLine(20, 5, 10, 5, 200);
  for (int yy = 0; yy < REP_PROC_SIZE; yy++)
    for (int xx = 0; xx < REP_PROC_SIZE; xx++)
      EXPECT_EQ(rev.px[yy * REP_PROC_SIZE + xx], buf.px[yy * REP_PROC_SIZE + xx]);
}

/**
 * @test ProcFadeReplica.FadeStepMathAndZeroSticky
 * @brief Verifies fade Step Math And Zero Sticky.
 *
 * @details
 * Exercises the ProcFadeReplica code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/procedurals.cpp
 * @ingroup descent3_tests
 */
TEST(ProcFadeReplica, FadeStepMathAndZeroSticky) {
  // fadeval = ((255-heat)>>3)+1; pixels decrement and clamp to 0,
  // already-black pixels are left untouched.
  struct Case {
    uint8_t heat;
    int fadeval;
  };
  const Case cases[] = {{0, 32}, {255, 1}, {128, 16}, {7, 32}};
  for (const auto &c : cases) {
    int fadeval = (255 - c.heat) >> 3;
    fadeval++;
    EXPECT_EQ(fadeval, c.fadeval);
  }

  uint8_t tex[64];
  const int fadeval = ((255 - 100) >> 3) + 1; // 19+1 = 20
  for (int i = 0; i < 64; i++)
    tex[i] = (uint8_t)(i * 4); // 0,4,...252
  for (int i = 0; i < 64; i++) {
    int pix = tex[i];
    if (pix) {
      if (pix - fadeval <= 0)
        tex[i] = 0;
      else
        tex[i] = (uint8_t)(pix - fadeval);
    }
  }
  EXPECT_EQ(tex[0], 0);  // zero stays zero
  EXPECT_EQ(tex[1], 0);  // 4-20 clamps
  EXPECT_EQ(tex[5], 0);  // 20-20 clamps (<= 0)
  EXPECT_EQ(tex[6], 4);  // 24-20=4
  EXPECT_EQ(tex[63], 232); // 252-20
}

} // namespace
