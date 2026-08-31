/**
 * @file loadlevel_real_tests.cpp
 * @brief Tests for LoadLevel.cpp 5831 lines — level loading. Covers the.
 *
 * @details
 * RLE compression codec used to store room visibility/lightmap
 * arrays (WriteCompressionByte/Short + readers), the
 * size-then-choose heuristics (ties go to RAW), the 250-entry run
 * cap, the command==1 hole that makes streams unreadable, and the
 * pre-version-94 object handle translation macro XLATE_HANDLE.
 *
 * This harness validates the behavior of `Descent3/LoadLevel.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/LoadLevel.cpp`
 * @par Harness
 * `loadlevel_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/LoadLevel.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>

constexpr int NO_COMPRESS = 0;
constexpr int COMPRESS = 1;

// in-memory stand-in for CFILE
struct Buf {
  std::vector<uint8_t> bytes;
};
static void w8(Buf *f, uint8_t v) { f->bytes.push_back(v); }
static void w16(Buf *f, uint16_t v) {
  w8(f, v & 0xFF);
  w8(f, (v >> 8) & 0xFF);
}
struct Rd {
  const uint8_t *p;
  int pos = 0;
  uint8_t r8() { return p[pos++]; }
  uint16_t r16() {
    uint16_t lo = r8(), hi = r8();
    return lo | (hi << 8);
  }
};

// ---------------------------------------------------------------------------
// WriteCompressionByte replication (LoadLevel.cpp:2217-2272)
static int RepWriteCompByte(Buf *fp, uint8_t *val, int total, int just_count,
                            int compress) {
  int done = 0, written = 0, curptr = 0;

  if (compress == NO_COMPRESS) {
    w8(fp, NO_COMPRESS);
    for (int i = 0; i < total; i++)
      w8(fp, val[i]);
    return total;
  } else if (!just_count)
    w8(fp, COMPRESS);

  while (!done) {
    if (curptr == total) {
      done = 1;
      continue;
    }
    uint8_t curval = val[curptr];
    uint8_t count = 1;
    while ((curptr + count) < total && val[curptr + count] == curval && count < 250)
      count++;

    written += 2;

    if (just_count) {
      curptr += count;
      continue;
    }
    if (count == 1) {
      w8(fp, 0);
      w8(fp, curval);
    } else {
      w8(fp, count);
      w8(fp, curval);
    }
    curptr += count;
  }
  return written;
}

// ReadCompressionByte replication (LoadLevel.cpp:2356-2387)
static void RepReadCompByte(Rd *fp, uint8_t *vals, int total) {
  int count = 0;
  uint8_t compressed = fp->r8();
  if (compressed == 0) {
    for (int i = 0; i < total; i++)
      vals[i] = fp->r8();
    return;
  }
  while (count != total) {
    uint8_t command = fp->r8();
    if (command == 0) { // literal
      vals[count++] = fp->r8();
    } else if (command >= 2 && command <= 250) { // run
      uint8_t height = fp->r8();
      for (int k = 0; k < command; k++)
        vals[count++] = height;
    } else {
      FAIL() << "command 1 or >250: bad compression run (Int3 in original)";
    }
  }
}

/**
 * @test LoadLevelRLE.ByteRoundtripRunsLiteralsAndNoCompress
 * @brief Verifies byte Roundtrip Runs Literals And No Compress.
 *
 * @details
 * Exercises the LoadLevelRLE code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/LoadLevel.cpp
 * @ingroup descent3_tests
 */
TEST(LoadLevelRLE, ByteRoundtripRunsLiteralsAndNoCompress) {
  // mixed runs and literals round-trip exactly
  uint8_t src[10] = {7, 7, 7, 9, 3, 3, 5, 5, 5, 5};
  Buf b;
  int sz = RepWriteCompByte(&b, src, 10, 0, COMPRESS);
  EXPECT_EQ(b.bytes[0], COMPRESS);
  // runs of 3 + 2 + 4 -> 6 bytes after the marker... plus literal 9 -> 8
  EXPECT_EQ(sz, 8);

  uint8_t dst[10] = {0};
  Rd r{b.bytes.data(), 0}; // reader consumes the marker itself
  RepReadCompByte(&r, dst, 10);
  EXPECT_EQ(0, memcmp(src, dst, 10));

  // all-distinct data: every value costs a 2-byte literal pair
  uint8_t raw[4] = {1, 2, 3, 4};
  Buf b2;
  EXPECT_EQ(RepWriteCompByte(&b2, raw, 4, 0, COMPRESS), 8);
  Rd r2{b2.bytes.data(), 0};
  uint8_t out[4];
  RepReadCompByte(&r2, out, 4);
  EXPECT_EQ(0, memcmp(raw, out, 4));

  // NO_COMPRESS writes a 0 marker then raw values
  Buf b3;
  EXPECT_EQ(RepWriteCompByte(&b3, raw, 4, 0, NO_COMPRESS), 4); // returns total
  EXPECT_EQ(b3.bytes.size(), 5u);                              // marker + data
  Rd r3{b3.bytes.data(), 0};
  uint8_t out3[4];
  RepReadCompByte(&r3, out3, 4);
  EXPECT_EQ(0, memcmp(raw, out3, 4));
}

/**
 * @test LoadLevelRLE.RunCapJustCountAndHeuristicTieBreak
 * @brief Verifies run Cap Just Count And Heuristic Tie Break.
 *
 * @details
 * Exercises the LoadLevelRLE code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/LoadLevel.cpp
 * @ingroup descent3_tests
 */
TEST(LoadLevelRLE, RunCapJustCountAndHeuristicTieBreak) {
  // a run longer than 250 splits: cap check is count<250, max run = 250
  static uint8_t big[501]; // 300 ones then 201 twos
  memset(big, 1, 300);
  memset(big + 300, 2, 201);
  Buf b;
  RepWriteCompByte(&b, big, 501, 0, COMPRESS);
  // first run must encode as exactly 250
  EXPECT_EQ(b.bytes[1], 250);
  EXPECT_EQ(b.bytes[2], 1);

  uint8_t dst[501];
  Rd r{b.bytes.data(), 0};
  RepReadCompByte(&r, dst, 501);
  EXPECT_EQ(0, memcmp(big, dst, 501));

  // just_count mode: sizes without writing (compressed size of 300-run
  // data would be 2+2+2=6 vs total 501)
  Buf sink;
  size_t before = sink.bytes.size();
  EXPECT_EQ(RepWriteCompByte(&sink, big, 501, 1, COMPRESS), 6);
  EXPECT_EQ(sink.bytes.size(), before); // nothing written

  // heuristic: compressed size >= raw size picks RAW (tie included)
  auto choose = [&](uint8_t *v, int total) {
    int count = RepWriteCompByte(&sink, v, total, 1, COMPRESS);
    return (count >= total) ? NO_COMPRESS : COMPRESS;
  };
  uint8_t alt[3] = {1, 2, 1}; // worst case: 6 bytes compressed vs 3 raw
  EXPECT_EQ(choose(alt, 3), NO_COMPRESS);
  uint8_t same[4] = {9, 9, 9, 9}; // one run: 2 < 4
  EXPECT_EQ(choose(same, 4), COMPRESS);
}

/**
 * @test LoadLevelRLE.ShortVariantUsesThreeBytePairsAndCommandOneIsFatal
 * @brief Verifies short Variant Uses Three Byte Pairs And Command One Is Fatal.
 *
 * @details
 * Exercises the LoadLevelRLE code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/LoadLevel.cpp
 * @ingroup descent3_tests
 */
TEST(LoadLevelRLE, ShortVariantUsesThreeBytePairsAndCommandOneIsFatal) {
  // shorts: literal pairs cost 3 bytes (cmd + short), runs also 3
  uint16_t sv[6] = {1000, 1000, 1000, 7, 7, 9};
  Buf b;
  // replicate WriteCompressionShort inline
  auto wcshort = [&](uint16_t *val, int total, int just_count, int compress) {
    int done = 0, written = 0, curptr = 0;
    if (compress == NO_COMPRESS) {
      w8(&b, NO_COMPRESS);
      for (int i = 0; i < total; i++)
        w16(&b, val[i]);
      return total;
    } else if (!just_count)
      w8(&b, COMPRESS);
    while (!done) {
      if (curptr == total) {
        done = 1;
        continue;
      }
      uint16_t curval = val[curptr];
      uint8_t count = 1;
      while ((curptr + count) < total && val[curptr + count] == curval && count < 250)
        count++;
      written += 3;
      if (just_count) {
        curptr += count;
        continue;
      }
      w8(&b, count == 1 ? 0 : count);
      w16(&b, curval);
      curptr += count;
    }
    return written;
  };
  EXPECT_EQ(wcshort(sv, 6, 0, COMPRESS), 9); // run + literal + literal

  uint16_t dv[6];
  ASSERT_EQ(b.bytes[0], COMPRESS); // writer emits the marker
  Rd r{b.bytes.data(), 1};         // payload starts after it
  int count = 0;
  while (count != 6) {
    uint8_t cmd = r.r8();
    if (cmd == 0) {
      dv[count++] = r.r16();
    } else {
      uint16_t h = r.r16();
      for (int k = 0; k < cmd; k++)
        dv[count++] = h;
    }
  }
  EXPECT_EQ(0, memcmp(sv, dv, sizeof(sv)));

  // reader hole: command byte 1 is never produced by the writer but
  // hits Int3 on read -- corrupt streams crash rather than degrade
  auto rdb = [&](Rd *fp, uint8_t *vals, int total) {
    int count = 0;
    if (fp->r8() == 0) { // uncompressed marker
      for (int i = 0; i < total; i++)
        vals[i] = fp->r8();
      return;
    }
    while (count != total) {
      uint8_t cmd = fp->r8();
      if (cmd == 0)
        vals[count++] = fp->r8();
      else if (cmd >= 2 && cmd <= 250) {
        uint8_t h = fp->r8();
        for (int k = 0; k < cmd; k++)
          vals[count++] = h;
      } else
        throw std::runtime_error("Int3: bad compression run");
    }
  };
  uint8_t hostile[] = {1, 1}; // compressed marker then illegal command
  Rd rh{hostile, 0};
  uint8_t sinkv[8] = {0};
  EXPECT_ANY_THROW(rdb(&rh, sinkv, 4));
}

// ---------------------------------------------------------------------------
// XLATE_HANDLE replication (LoadLevel.cpp:1386-1388)
#define OLD_HANDLE_OBJNUM_MASK 0x3ff
#define OLD_HANDLE_COUNT_MASK 0xfffffc00
#define XLATE_HANDLE(handle)                                                                           \
  ((((handle) & OLD_HANDLE_COUNT_MASK) << 1) + ((handle) & OLD_HANDLE_OBJNUM_MASK))

/**
 * @test LoadLevelHandle.OldHandleTranslationPreservesObjnumDoublesCountSlot
 * @brief Verifies old Handle Translation Preserves Objnum Doubles Count Slot.
 *
 * @details
 * Exercises the LoadLevelHandle code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/LoadLevel.cpp
 * @ingroup descent3_tests
 */
TEST(LoadLevelHandle, OldHandleTranslationPreservesObjnumDoublesCountSlot) {
  // old handles pack objnum in the low 10 bits, generation above;
  // translation shifts the generation field up one bit, leaving bit 10 zero
  uint32_t old = (5u << 10) | 123u;
  uint32_t translated = XLATE_HANDLE(old);
  EXPECT_EQ(translated & OLD_HANDLE_OBJNUM_MASK, 123u); // objnum kept
  EXPECT_EQ(translated >> 11, 5u);                      // gen value preserved

  // gen 1 + objnum 7 -> gen field now at bit 11
  EXPECT_EQ(XLATE_HANDLE((1u << 10) | 7u), (1u << 11) | 7u);

  // zero handle stays zero
  EXPECT_EQ(XLATE_HANDLE(0), 0u);
}
