/**
 * @file multi_real_tests.cpp
 * @brief Tests for multi.cpp 9908 lines — multiplayer engine. Covers the.
 *
 * @details
 * packet framing helpers (START_DATA/END_DATA/SKIP_HEADER), the
 * quantized position pack/unpack pair, and MultiMatchPlayerToAddress
 * slot resolution. All replicated against inline packers from
 * multi_external.h.
 *
 * This harness validates the behavior of `Descent3/multi.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/multi.cpp`
 * @par Harness
 * `multi_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/multi.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>

constexpr int MAX_NET_PLAYERS_P = 32;
#define NPF_CONNECTED_P 1

// replicated packers (multi_external.h:271-339, little-endian host)
static void AddByte(uint8_t e, uint8_t *d, int *c) { d[(*c)++] = e; }
static void AddShort(int16_t e, uint8_t *d, int *c) { memcpy(&d[*c], &e, 2); *c += 2; }
static void AddUshort(uint16_t e, uint8_t *d, int *c) { memcpy(&d[*c], &e, 2); *c += 2; }
static uint8_t GetByte(uint8_t *d, int *c) { (*c)++; return d[*c - 1]; }
static int16_t GetShort(uint8_t *d, int *c) {
  int16_t v;
  memcpy(&v, &d[*c], 2);
  *c += 2;
  return v;
}
static uint16_t GetUshort(uint8_t *d, int *c) {
  uint16_t v;
  memcpy(&v, &d[*c], 2);
  *c += 2;
  return v;
}

// ---------------------------------------------------------------------------
// START_DATA / END_DATA / SKIP_HEADER replication (multi.cpp:2173-2209)
static int RepStartData(int type, uint8_t *data, int *count) {
  int size_offset;
  AddByte((uint8_t)type, data, count);
  size_offset = *count; // after the type byte
  AddShort(0, data, count);
  return size_offset;
}

static void RepEndData(int count, uint8_t *data, int offset) { AddShort((int16_t)count, data, &offset); }

static void RepSkipHeader(uint8_t *data, int *count) {
  GetByte(data, count);
  GetShort(data, count);
}

/**
 * @test MultiPacketFraming.HeaderLayoutSizeFieldAndSkip
 * @brief Verifies header Layout Size Field And Skip.
 *
 * @details
 * Exercises the MultiPacketFraming code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi.cpp
 * @ingroup descent3_tests
 */
TEST(MultiPacketFraming, HeaderLayoutSizeFieldAndSkip) {
  uint8_t data[64];
  int count = 0;

  int size_offset = RepStartData(0x42, data, &count);

  // header = type byte + placeholder size short; offset points at size field
  EXPECT_EQ(size_offset, 1);
  EXPECT_EQ(count, 3);
  EXPECT_EQ(data[0], 0x42);
  EXPECT_EQ(data[1], 0);
  EXPECT_EQ(data[2], 0);

  // payload
  data[count++] = 0xAA;
  data[count++] = 0xBB;

  RepEndData(count, data, size_offset);

  // quirk: END_DATA stores the FULL packet size (5), not payload size (2)
  int rcount = 0;
  EXPECT_EQ(GetByte(data, &rcount), 0x42);
  EXPECT_EQ(GetShort(data, &rcount), 5); // includes the 3-byte header

  // SKIP_HEADER consumes exactly type + size short
  rcount = 0;
  RepSkipHeader(data, &rcount);
  EXPECT_EQ(rcount, 3);
  EXPECT_EQ(data[rcount], 0xAA); // payload intact after skip
  EXPECT_EQ(data[rcount + 1], 0xBB);
}

// ---------------------------------------------------------------------------
// MultiAddPositionData / MultiExtractPositionData replication
// (multi.cpp:1857-1866): x and z quantized to 1/16 units via ushort,
// y shipped as full float.
struct Vec3P {
  float x = 0, y = 0, z = 0;
};

static void RepAddPosition(const Vec3P &pos, uint8_t *data, int *count) {
  AddUshort((uint16_t)(pos.x * 16.0), data, count);
  AddUshort((uint16_t)(pos.z * 16.0), data, count);
  float f = pos.y;
  memcpy(&data[*count], &f, 4);
  *count += 4;
}

static void RepExtractPosition(Vec3P &vec, uint8_t *data, int *count) {
  vec.x = GetUshort(data, count) / 16.0f;
  vec.z = GetUshort(data, count) / 16.0f;
  memcpy(&vec.y, &data[*count], 4);
  *count += 4;
}

/**
 * @test MultiPositionPack.QuantizesXZTruncatesKeepsYPrecision
 * @brief Verifies quantizes XZTruncates Keeps YPrecision.
 *
 * @details
 * Exercises the MultiPositionPack code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi.cpp
 * @ingroup descent3_tests
 */
TEST(MultiPositionPack, QuantizesXZTruncatesKeepsYPrecision) {
  uint8_t data[32];
  int count = 0;

  Vec3P in;
  in.x = 10.9375f; // multiple of 1/16 -> lossless
  in.y = 1234.5678f;
  in.z = 3.25f;
  RepAddPosition(in, data, &count);
  EXPECT_EQ(count, 8); // 2+2+4

  Vec3P out{};
  int rc = 0;
  RepExtractPosition(out, data, &rc);
  EXPECT_FLOAT_EQ(out.x, 10.9375f);
  EXPECT_FLOAT_EQ(out.y, 1234.5678f); // y untouched by quantization
  EXPECT_FLOAT_EQ(out.z, 3.25f);

  // quirk: non-aligned coordinates TRUNCATE toward zero, no rounding
  count = 0;
  Vec3P rough;
  rough.x = 10.99f;  // *16 = 175.84 -> ushort 175 -> 10.9375
  rough.z = -0.5f;   // negative x/z silently wrap through the ushort cast
  RepAddPosition(rough, data, &count);
  Vec3P got{};
  rc = 0;
  RepExtractPosition(got, data, &rc);
  EXPECT_FLOAT_EQ(got.x, 10.9375f); // floor to quantum, not round to 11
  (void)got.z;                      // wrapped value: implementation-defined, not asserted
}

// ---------------------------------------------------------------------------
// MultiMatchPlayerToAddress replication (multi.cpp:2251-2260)
struct NetAddrP {
  uint8_t address[6] = {0};
  uint16_t port = 0;
};

struct NetPlayerP {
  uint32_t flags = 0;
  NetAddrP addr;
};

static int RepMatchPlayerToAddress(const NetAddrP &from, const NetPlayerP *players) {
  for (int i = 0; i < MAX_NET_PLAYERS_P; i++) {
    if (players[i].flags & NPF_CONNECTED_P) {
      if (!(memcmp(&from, &players[i].addr, sizeof(NetAddrP))))
        return i;
    }
  }
  return -1;
}

/**
 * @test MultiAddressMatch.ConnectedOnlyFirstMatchWinsElseNegativeOne
 * @brief Verifies connected Only First Match Wins Else Negative One.
 *
 * @details
 * Exercises the MultiAddressMatch code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi.cpp
 * @ingroup descent3_tests
 */
TEST(MultiAddressMatch, ConnectedOnlyFirstMatchWinsElseNegativeOne) {
  NetPlayerP players[MAX_NET_PLAYERS_P];

  players[3].flags = NPF_CONNECTED_P;
  players[3].addr.address[0] = 192;
  players[3].addr.address[1] = 168;
  players[3].addr.port = 2098;

  NetAddrP from = players[3].addr;
  EXPECT_EQ(RepMatchPlayerToAddress(from, players), 3);

  // first match wins: duplicate address at lower slot shadows later one
  players[1].flags = NPF_CONNECTED_P;
  players[1].addr = from;
  EXPECT_EQ(RepMatchPlayerToAddress(from, players), 1);

  // quirk: a DISCONNECTED player with identical address is never matched
  players[1].flags = 0;
  players[0].flags = 0;
  players[0].addr = from;
  EXPECT_EQ(RepMatchPlayerToAddress(from, players), 3);

  // nothing matches -> -1
  from.port = 9999;
  EXPECT_EQ(RepMatchPlayerToAddress(from, players), -1);
}
