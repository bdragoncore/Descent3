/*
 * Descent 3
 * Copyright (C) 2024 Descent Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file md5_tests.cpp
 * @brief Unit tests for md5/md5.cpp, md5/md5.h.
 *
 * @details
 * Covers the public contract of `md5/md5.cpp, md5/md5.h` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `md5/md5.cpp, md5/md5.h`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `md5/md5.cpp, md5/md5.h`
 * @par Harness
 * `md5_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see md5/md5.cpp
 * @see md5/md5.h
 */



#include <gtest/gtest.h>

#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

#include "md5.h"

namespace {

std::string hexdigest_after_n_rounds_of_fuzz(int n) {
  MD5 md5;
  for (int i = 0; i < n; ++i) {
    md5.update((float)1.0f);
    md5.update((int)2);
    md5.update((int16_t)3);
    md5.update((uint32_t)4U);
    md5.update((uint8_t)5);
  }

  std::ostringstream hexdigest;
  hexdigest << std::hex;
  auto digest = md5.digest();

  for (int i = 0; i < 16; ++i)
    hexdigest << std::setw(2) << std::setfill('0') << std::nouppercase << static_cast<uint32_t>(digest[i]);

  return hexdigest.str();
}

}; // namespace

/**
 * @test D3.MD5
 * @brief Verifies mD5.
 *
 * @details
 * Exercises the D3 code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see md5/md5.cpp, md5/md5.h
 * @ingroup descent3_tests
 */
TEST(D3, MD5) {
  EXPECT_EQ(hexdigest_after_n_rounds_of_fuzz(0), "d41d8cd98f00b204e9800998ecf8427e");
  EXPECT_EQ(hexdigest_after_n_rounds_of_fuzz(1), "c56c005504f5b9b17df9d83f1106e9b2");
  EXPECT_EQ(hexdigest_after_n_rounds_of_fuzz(2), "98327a3f4ee311c9bdc8613508922c95");
  EXPECT_EQ(hexdigest_after_n_rounds_of_fuzz(4), "f2343ce526646c087df92d708e883675");
  EXPECT_EQ(hexdigest_after_n_rounds_of_fuzz(8), "73396b338a0a769c4e6ae5b7b0bc187e");
  EXPECT_EQ(hexdigest_after_n_rounds_of_fuzz(100), "99b30c1dcc42d97c3d914c26a14bb1d7");
  EXPECT_EQ(hexdigest_after_n_rounds_of_fuzz(1999), "82eeff8c7d574c8232b0ca6ca2c9dd40");
}

std::string hexdigest_of_bytes(const uint8_t *data, std::size_t n) {
  MD5 md5;
  md5.update(data, n);
  std::ostringstream hexdigest;
  hexdigest << std::hex;
  auto digest = md5.digest();
  for (int i = 0; i < 16; ++i)
    hexdigest << std::setw(2) << std::setfill('0') << std::nouppercase << static_cast<uint32_t>(digest[i]);
  return hexdigest.str();
}

// RFC 1321 test suite: known answers for standard inputs.
/**
 * @test D3.MD5_RFC1321Vectors
 * @brief Verifies mD5 RFC1321Vectors.
 *
 * @details
 * Exercises the D3 code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see md5/md5.cpp, md5/md5.h
 * @ingroup descent3_tests
 */
TEST(D3, MD5_RFC1321Vectors) {
  EXPECT_EQ(hexdigest_of_bytes(reinterpret_cast<const uint8_t *>(""), 0), "d41d8cd98f00b204e9800998ecf8427e");
  EXPECT_EQ(hexdigest_of_bytes(reinterpret_cast<const uint8_t *>("a"), 1), "0cc175b9c0f1b6a831c399e269772661");
  EXPECT_EQ(hexdigest_of_bytes(reinterpret_cast<const uint8_t *>("abc"), 3), "900150983cd24fb0d6963f7d28e17f72");
  EXPECT_EQ(hexdigest_of_bytes(reinterpret_cast<const uint8_t *>("message digest"), 14), "f96b697d7cb7938d525a2f31aaf161d0");
  EXPECT_EQ(hexdigest_of_bytes(reinterpret_cast<const uint8_t *>("abcdefghijklmnopqrstuvwxyz"), 26), "c3fcd3d76192e4007dfb496cca67e13b");
  EXPECT_EQ(hexdigest_of_bytes(reinterpret_cast<const uint8_t *>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"), 62), "d174ab98d277d9f5a5611c2c9f419d9f");
  const char *eighty =
      "1234567890123456789012345678901234567890"
      "1234567890123456789012345678901234567890";
  EXPECT_EQ(hexdigest_of_bytes(reinterpret_cast<const uint8_t *>(eighty), 80), "57edf4a22be3c955ac49da2e2107b67a");
}

/**
 * @test D3.MD5_ChunkedUpdatesMatchOneShot
 * @brief Verifies mD5 Chunked Updates Match One Shot.
 *
 * @details
 * Exercises the D3 code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see md5/md5.cpp, md5/md5.h
 * @ingroup descent3_tests
 */
TEST(D3, MD5_ChunkedUpdatesMatchOneShot) {
  const char *msg = "The quick brown fox jumps over the lazy dog";
  const std::size_t n = std::strlen(msg);
  uint8_t one_shot[16], chunked[16];

  MD5 a;
  a.update(reinterpret_cast<const uint8_t *>(msg), n);
  a.digest(one_shot);

  MD5 b;
  // Update in irregular chunks (1, 2, 5, ... bytes) to exercise buffering.
  std::size_t off = 0;
  std::size_t chunk = 1;
  while (off < n) {
    std::size_t take = std::min(chunk, n - off);
    b.update(reinterpret_cast<const uint8_t *>(msg) + off, take);
    off += take;
    chunk = (chunk * 2 + 1) % 7 + 1;
  }
  b.digest(chunked);

  for (int i = 0; i < 16; ++i)
    EXPECT_EQ(chunked[i], one_shot[i]) << "digest byte " << i;
}
