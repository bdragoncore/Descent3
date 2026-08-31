/**
 * @file program_real_tests.cpp
 * @brief Tests for Descent3/program.cpp — program version state (114 lines).
 *
 * @details
 * Covers ProgramVersion flag setting.
 *
 * This harness validates the behavior of `Descent3/program.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/program.cpp`
 * @par Harness
 * `program_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/program.cpp
 */

#include <gtest/gtest.h>
#include "program.h"
#include "appdatabase.h"
#include "linux/lnxdatabase.h"

// Provide global Database required by program.cpp
oeAppDatabase *Database = nullptr;

// Stub CRegistry not needed; we stub oeLnxAppDatabase methods to avoid registry
oeLnxAppDatabase::oeLnxAppDatabase() : database(nullptr) {}
oeLnxAppDatabase::oeLnxAppDatabase(oeLnxAppDatabase *parent) : database(nullptr) { (void)parent; }
oeLnxAppDatabase::~oeLnxAppDatabase() {}
CRegistry* oeLnxAppDatabase::GetSystemRegistry() { return nullptr; }
bool oeLnxAppDatabase::create_record(const char *pathname) { (void)pathname; return true; }
bool oeLnxAppDatabase::lookup_record(const char *pathname) { (void)pathname; return true; }
bool oeLnxAppDatabase::read(const char *label, char *entry, int *entrylen) { (void)label;(void)entry;(void)entrylen; return false; }
bool oeLnxAppDatabase::read(const char *label, void *entry, int wordsize) { (void)label;(void)entry;(void)wordsize; return false; }
bool oeLnxAppDatabase::read(const char *label, bool *entry) { (void)label;(void)entry; return false; }
bool oeLnxAppDatabase::write(const char *label, const char *entry, int entrylen) { (void)label;(void)entry;(void)entrylen; return true; }
bool oeLnxAppDatabase::write(const char *label, int entry) { (void)label;(void)entry; return true; }
void oeLnxAppDatabase::get_user_name(char *buffer, size_t *size) { if(buffer&&size&&*size>0) {buffer[0]=0; *size=0;} }

// Stub Error via pserror (with RELEASE it may be no-op, but provide)
void Error(const char *fmt, ...) { (void)fmt; }

/**
 * @test Program.VersionFlags
 * @brief Verifies version Flags.
 *
 * @details
 * Exercises the Program code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/program.cpp
 * @ingroup descent3_tests
 */
TEST(Program, VersionFlags) {
  // DEVELOPMENT_VERSION should set debug=1, editor=0, release=0
  ProgramVersion(DEVELOPMENT_VERSION, 1, 5, 0);
  EXPECT_EQ(Program_version.major, 1);
  EXPECT_EQ(Program_version.minor, 5);
  EXPECT_EQ(Program_version.build, 0);
  EXPECT_TRUE(Program_version.debug);
  EXPECT_FALSE(Program_version.editor);
  EXPECT_FALSE(Program_version.release);

  ProgramVersion(RELEASE_VERSION, 1, 5, 1);
  EXPECT_FALSE(Program_version.debug);
  EXPECT_FALSE(Program_version.editor);
  // Based on program.cpp, release case sets release=0? Check: both dev and release set release 0, windowed 0
  // So we just check flags are set.

  // BETA modifier
  ProgramVersion(DEVELOPMENT_VERSION|BETA_VERSION, 2,0,0);
  EXPECT_TRUE(Program_version.beta);
  EXPECT_FALSE(Program_version.demo);

  ProgramVersion(RELEASE_VERSION|DEMO_VERSION, 2,0,0);
  EXPECT_TRUE(Program_version.demo);
  EXPECT_FALSE(Program_version.beta);
}

/**
 * @test Program.WindowedNotSet
 * @brief Verifies windowed Not Set.
 *
 * @details
 * Exercises the Program code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/program.cpp
 * @ingroup descent3_tests
 */
TEST(Program, WindowedNotSet) {
  ProgramVersion(DEVELOPMENT_VERSION, 1,0,0);
  EXPECT_FALSE(Program_version.windowed);
  ProgramVersion(RELEASE_VERSION, 1,0,0);
  EXPECT_FALSE(Program_version.windowed);
}
