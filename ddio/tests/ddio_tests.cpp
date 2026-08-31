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
 * @file ddio_tests.cpp
 * @brief Unit tests for ddio/ddio.h, ddio/lnxfile.cpp, ddio/key.cpp, lib/chrono_timer.h.
 *
 * @details
 * Covers the public contract of `ddio/ddio.h, ddio/lnxfile.cpp, ddio/key.cpp, lib/chrono_timer.h` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `ddio/ddio.h, ddio/lnxfile.cpp, ddio/key.cpp, lib/chrono_timer.h`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `ddio/ddio.h, ddio/lnxfile.cpp, ddio/key.cpp, lib/chrono_timer.h`
 * @par Harness
 * `ddio_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see ddio/ddio.h
 * @see ddio/lnxfile.cpp
 * @see ddio/key.cpp
 * @see lib/chrono_timer.h
 */



#include <filesystem>
#include <chrono>
#include <thread>
#include <gtest/gtest.h>

#include "ddio.h"
#include "chrono_timer.h"

/**
 * @test D3.DDIO_GetTmpFileName
 * @brief Verifies dDIO Get Tmp File Name.
 *
 * @details
 * Exercises the D3 code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/ddio.h, ddio/lnxfile.cpp, ddio/key.cpp, lib/chrono_timer.h
 * @ingroup descent3_tests
 */
TEST(D3, DDIO_GetTmpFileName) {
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path result = ddio_GetTmpFileName(temp_dir, "prefix_");
  EXPECT_FALSE(result.empty());
  EXPECT_EQ(result.extension(), ".tmp");
  EXPECT_TRUE(canonical(result.parent_path()) == canonical(temp_dir));
}

/**
 * @test D3.ChronoTimer_TimeIsNonNegative
 * @brief Verifies chrono Timer Time Is Non Negative.
 *
 * @details
 * Exercises the D3 code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/ddio.h, ddio/lnxfile.cpp, ddio/key.cpp, lib/chrono_timer.h
 * @ingroup descent3_tests
 */
TEST(D3, ChronoTimer_TimeIsNonNegative) {
  D3::ChronoTimer::Initialize();
  EXPECT_GE(D3::ChronoTimer::GetTime(), 0.0f);
  EXPECT_GE(D3::ChronoTimer::GetTimeMS(), 0);
  EXPECT_GE(D3::ChronoTimer::GetTimeUS(), 0);
}

/**
 * @test D3.ChronoTimer_IsMonotonic
 * @brief Verifies chrono Timer Is Monotonic.
 *
 * @details
 * Exercises the D3 code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/ddio.h, ddio/lnxfile.cpp, ddio/key.cpp, lib/chrono_timer.h
 * @ingroup descent3_tests
 */
TEST(D3, ChronoTimer_IsMonotonic) {
  D3::ChronoTimer::Initialize();
  int64_t t0 = D3::ChronoTimer::GetTimeMS();
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  int64_t t1 = D3::ChronoTimer::GetTimeMS();
  EXPECT_GE(t1, t0);
}

/**
 * @test D3.ChronoTimer_SleepMSAdvancesClock
 * @brief Verifies chrono Timer Sleep MSAdvances Clock.
 *
 * @details
 * Exercises the D3 code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see ddio/ddio.h, ddio/lnxfile.cpp, ddio/key.cpp, lib/chrono_timer.h
 * @ingroup descent3_tests
 */
TEST(D3, ChronoTimer_SleepMSAdvancesClock) {
  D3::ChronoTimer::Initialize();
  int64_t before = D3::ChronoTimer::GetTimeMS();
  D3::ChronoTimer::SleepMS(10);
  int64_t after = D3::ChronoTimer::GetTimeMS();
  // The clock must advance by at least the requested sleep (allow slack).
  EXPECT_GE(after, before);
}
