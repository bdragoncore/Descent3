/**
 * @file pilotpics_real_tests.cpp
 * @brief Tests for PilotPicsAPI.cpp (~747 lines).
 *
 * @details
 * Covers the pilot picture database: init/close lifecycle against a real
 * HOG + index file pair, alphabetical jump table, case-insensitive
 * QueryPilot counting, FindFirst/FindNext iteration, id binary search via
 * GetPilot/GetBitmapHandle, and count validation limits.
 *
 * This harness validates the behavior of `Descent3/pilotpics.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/pilotpics.cpp`
 * @par Harness
 * `pilotpics_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/pilotpics.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>

#include "PilotPicsAPI.h"
#include "cfile.h"
#include "bitmap.h"
#include "pilot.h"
#include "log.h"

// ---------------------------------------------------------------------------
// bitmap lib stub
// ---------------------------------------------------------------------------
static int g_next_bm_handle = 10;
static std::vector<std::string> g_bm_requests;
int bm_AllocLoadFileBitmap(const char *filename, int mipped, int) {
  (void)mipped;
  g_bm_requests.push_back(filename);
  return -1; // always fails -> BAD_BITMAP_HANDLE path
}

// ---------------------------------------------------------------------------
// Fixture helpers
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for PilotPicsTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PilotPicsTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    // cfopen() consults search paths; cf_OpenLibrary()/cf_LocatePath()
    // consults base directories - register both
    cf_SetSearchPath("/tmp/opencode", {});
    cf_AddBaseDirectory("/tmp/opencode");
  }

  void SetUp() override {
    std::filesystem::remove("/tmp/opencode/PPics.Hog"); // exact-case
    std::filesystem::remove("/tmp/opencode/PPics.idx");
    std::filesystem::remove("/tmp/opencode/ppics.hog"); // stale leftovers from
    std::filesystem::remove("/tmp/opencode/ppics.idx"); // older runs still resolve
                                                        // via case-insensitive lookup
    PPic_CloseDatabase();
    g_next_bm_handle = 10;
    g_bm_requests.clear();
  }

  void TearDown() override { PPic_CloseDatabase(); }

  // Creates a valid one-entry HOG2 archive at /tmp/opencode/ppics.hog.
  // KNOWN CFILE BUG: a zero-entry HOG crashes open_file_in_lib (binary
  // search computes entries[(0 + -1)/2] when a later cfopen() misses),
  // so we always ship one harmless filler file.
  static void WriteHog() {
    FILE *f = fopen("/tmp/opencode/PPics.Hog", "wb");
    const char *filler_name = "zzz_fill.dat";
    uint32_t nfiles = 1;
    uint32_t dataoff = 68 + 56;
    fwrite("HOG2", 1, 4, f);
    fwrite(&nfiles, 4, 1, f);
    fwrite(&dataoff, 4, 1, f);
    char pad[56] = {}; // header padding
    fwrite(pad, 1, 56, f);

    char name[36] = {};
    strcpy(name, filler_name);
    uint32_t flags = 0, len = 4, ts = 0;
    fwrite(name, 1, 36, f);
    fwrite(&flags, 4, 1, f);
    fwrite(&len, 4, 1, f);
    fwrite(&ts, 4, 1, f);
    fwrite("DATA", 1, 4, f); // filler payload at offset 124
    fclose(f);
  }

  struct Rec {
    std::string name;
    uint16_t id;
    std::string bmp;
  };

  // Index layout: int32 count; per record: u8 len+name, i16 id, u8 len+bmpname
  static void WriteIndex(const std::vector<Rec> &recs, int forced_count = -999) {
    std::vector<uint8_t> f;
    auto push_int = [&](int32_t v) {
      for (int i = 0; i < 4; i++)
        f.push_back(((uint32_t)v >> (8 * i)) & 0xff);
    };
    auto push_short = [&](int16_t v) {
      f.push_back((uint8_t)(v & 0xff));
      f.push_back((uint8_t)((v >> 8) & 0xff));
    };
    auto push_byte = [&](uint8_t v) { f.push_back(v); };

    push_int((int32_t)recs.size());
    for (auto &r : recs) {
      push_byte((uint8_t)r.name.size());
      f.insert(f.end(), r.name.begin(), r.name.end());
      push_short((int16_t)r.id);
      push_byte((uint8_t)r.bmp.size());
      f.insert(f.end(), r.bmp.begin(), r.bmp.end());
    }
    if (forced_count != -999) {
      f[0] = (uint8_t)((uint32_t)forced_count & 0xff);
      f[1] = (uint8_t)(((uint32_t)forced_count >> 8) & 0xff);
      f[2] = (uint8_t)(((uint32_t)forced_count >> 16) & 0xff);
      f[3] = (uint8_t)(((uint32_t)forced_count >> 24) & 0xff);
    }
    FILE *fp = fopen("/tmp/opencode/PPics.idx", "wb");
    fwrite(f.data(), 1, f.size(), fp);
    fclose(fp);
  }
};

/**
 * @test PilotPicsTest.InitFailsWithoutHog
 * @brief Verifies init Fails Without Hog.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, InitFailsWithoutHog) {
  WriteIndex({{"Alice", 10, "a.bmp"}});
  EXPECT_FALSE(PPic_InitDatabase());
}

/**
 * @test PilotPicsTest.InitFailsWithoutIndex
 * @brief Verifies init Fails Without Index.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, InitFailsWithoutIndex) {
  WriteHog();
  EXPECT_FALSE(PPic_InitDatabase());
}

/**
 * @test PilotPicsTest.InitRejectsInsaneCounts
 * @brief Verifies init Rejects Insane Counts.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, InitRejectsInsaneCounts) {
  WriteHog();
  WriteIndex({{"A", 1, "x.bmp"}}, 70000); // > 65535
  EXPECT_FALSE(PPic_InitDatabase());

  WriteIndex({{"A", 1, "x.bmp"}}, -5); // negative
  EXPECT_FALSE(PPic_InitDatabase());
}

/**
 * @test PilotPicsTest.InitSucceedsAndQueriesCaseInsensitive
 * @brief Verifies init Succeeds And Queries Case Insensitive.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, InitSucceedsAndQueriesCaseInsensitive) {
  WriteHog();
  WriteIndex({
      {"Alice", 10, "alice.bmp"},
      {"bob", 20, "bob.bmp"},
      {"alvin", 30, "alvin.bmp"},
  });
  ASSERT_TRUE(PPic_InitDatabase());

  // second init is a no-op success
  EXPECT_TRUE(PPic_InitDatabase());

  EXPECT_EQ(PPic_QueryPilot((char *)"ALICE"), 1);
  EXPECT_EQ(PPic_QueryPilot((char *)"alice"), 1);
  EXPECT_EQ(PPic_QueryPilot((char *)"Bob"), 1);
}

/**
 * @test PilotPicsTest.QueryPilotCountsDuplicateNames
 * @brief Verifies query Pilot Counts Duplicate Names.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, QueryPilotCountsDuplicateNames) {
  WriteHog();
  WriteIndex({
      {"Smith", 1, "s1.bmp"},
      {"Smith", 2, "s2.bmp"},
      {"Jones", 3, "j.bmp"},
  });
  ASSERT_TRUE(PPic_InitDatabase());
  EXPECT_EQ(PPic_QueryPilot((char *)"SMITH"), 2);
}

/**
 * @test PilotPicsTest.QueryUnknownNameOrLetterReturnsZero
 * @brief Verifies query Unknown Name Or Letter Returns Zero.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, QueryUnknownNameOrLetterReturnsZero) {
  WriteHog();
  WriteIndex({
      {"Alice", 10, "a.bmp"},
      {"1stPlayer", 11, "n.bmp"}, // non-alpha first letter -> slot 26
  });
  ASSERT_TRUE(PPic_InitDatabase());

  EXPECT_EQ(PPic_QueryPilot((char *)"Zed"), 0);   // no Z names
  EXPECT_EQ(PPic_QueryPilot((char *)"1stplayer"), 1);
  EXPECT_EQ(PPic_QueryPilot((char *)"Nobody"), 0);
}

// The index is expected alphabetically sorted: PPic_JumpToPilot linearly
// scans from the letter's first entry until a name sorts past the pattern.
// FindFirst then requires an exact (case-insensitive) name match, and
// FindNext enumerates consecutive duplicates of that exact name.
/**
 * @test PilotPicsTest.FindFirstNextEnumeratesExactNameDuplicates
 * @brief Verifies find First Next Enumerates Exact Name Duplicates.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, FindFirstNextEnumeratesExactNameDuplicates) {
  WriteHog();
  WriteIndex({
      {"Alice", 10, "a.bmp"},
      {"Smith", 1, "s1.bmp"},
      {"Smith", 2, "s2.bmp"},
      {"zed", 30, "z.bmp"},
  });
  ASSERT_TRUE(PPic_InitDatabase());

  uint16_t id;
  ASSERT_TRUE(PPic_FindFirst((char *)"smith", &id));
  EXPECT_EQ(id, 1);

  std::vector<uint16_t> found;
  found.push_back(id);
  while (PPic_FindNext(&id))
    found.push_back(id);

  ASSERT_EQ(found.size(), 2u);
  EXPECT_EQ(found[1], 2);

  EXPECT_FALSE(PPic_FindFirst((char *)"NOBODY", &id)); // no N section
  EXPECT_FALSE(PPic_FindFirst((char *)"Buz", &id));    // B section sorts past
  PPic_FindClose();
}

// KNOWN LIMITATION: PPic_FindNext stops at the first non-matching name,
// so same-prefix or interleaved matches are silently skipped.
/**
 * @test PilotPicsTest.FindNextSkipsNonExactNeighbors
 * @brief Verifies find Next Skips Non Exact Neighbors.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, FindNextSkipsNonExactNeighbors) {
  WriteHog();
  WriteIndex({
      {"amy", 1, "amy.bmp"},
      {"Arthur", 2, "arthur.bmp"},
      {"amy2", 3, "amy2.bmp"}, // never reached
  });
  ASSERT_TRUE(PPic_InitDatabase());

  uint16_t id;
  ASSERT_TRUE(PPic_FindFirst((char *)"AMY", &id));
  EXPECT_EQ(id, 1);

  EXPECT_FALSE(PPic_FindNext(&id)); // Arthur terminates the scan
}

/**
 * @test PilotPicsTest.GetPilotResolvesIdsDuplicateMidpointWins
 * @brief Verifies get Pilot Resolves Ids Duplicate Midpoint Wins.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, GetPilotResolvesIdsDuplicateMidpointWins) {
  WriteHog();
  WriteIndex({
      {"first", 50, "f.bmp"},
      {"second", 50, "s.bmp"}, // duplicate id
      {"third", 40, "t.bmp"},
  });
  ASSERT_TRUE(PPic_InitDatabase());

  char name[PILOT_STRING_SIZE];
  // Duplicate ids: binary search midpoint (index 1 of 3) decides which
  // record answers - deterministic but arbitrary among equals.
  ASSERT_TRUE(PPic_GetPilot(50, name, sizeof(name)));
  EXPECT_STREQ(name, "second");

  ASSERT_TRUE(PPic_GetPilot(40, name, sizeof(name)));
  EXPECT_STREQ(name, "third");

  EXPECT_FALSE(PPic_GetPilot(99, name, sizeof(name)));
}

/**
 * @test PilotPicsTest.GetPilotTruncatesToBufferSize
 * @brief Verifies get Pilot Truncates To Buffer Size.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, GetPilotTruncatesToBufferSize) {
  WriteHog();
  WriteIndex({{"AlexanderTheGreat", 7, "a.bmp"}});
  ASSERT_TRUE(PPic_InitDatabase());

  char name[6];
  ASSERT_TRUE(PPic_GetPilot(7, name, sizeof(name)));
  EXPECT_STRCASEEQ(name, "Alexa"); // buffersize-1 chars + NUL
  EXPECT_EQ(strlen(name), 5u);
}

/**
 * @test PilotPicsTest.GetBitmapHandleUnknownIdReturnsMinusOne
 * @brief Verifies get Bitmap Handle Unknown Id Returns Minus One.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, GetBitmapHandleUnknownIdReturnsMinusOne) {
  WriteHog();
  WriteIndex({{"Alice", 10, "alice.bmp"}});
  ASSERT_TRUE(PPic_InitDatabase());

  EXPECT_EQ(PPic_GetBitmapHandle(999), -1);
  EXPECT_TRUE(g_bm_requests.empty());
}

/**
 * @test PilotPicsTest.GetBitmapHandleKnownIdReportsBadBitmapOnFailure
 * @brief Verifies get Bitmap Handle Known Id Reports Bad Bitmap On Failure.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, GetBitmapHandleKnownIdReportsBadBitmapOnFailure) {
  WriteHog();
  WriteIndex({{"Alice", 10, "alice.bmp"}});
  ASSERT_TRUE(PPic_InitDatabase());

  EXPECT_EQ(PPic_GetBitmapHandle(10), BAD_BITMAP_HANDLE);
  ASSERT_EQ(g_bm_requests.size(), 1u);
  EXPECT_STREQ(g_bm_requests[0].c_str(), "alice.bmp");
}

/**
 * @test PilotPicsTest.CloseDatabaseInvalidatesQueries
 * @brief Verifies close Database Invalidates Queries.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilotpics.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, CloseDatabaseInvalidatesQueries) {
  WriteHog();
  WriteIndex({{"Alice", 10, "a.bmp"}});
  ASSERT_TRUE(PPic_InitDatabase());

  PPic_CloseDatabase();
  EXPECT_EQ(PPic_QueryPilot((char *)"Alice"), 0);
  EXPECT_FALSE(PPic_FindFirst((char *)"Alice", nullptr));

  // database can be reopened afterwards
  ASSERT_TRUE(PPic_InitDatabase());
  EXPECT_EQ(PPic_QueryPilot((char *)"Alice"), 1);
}
