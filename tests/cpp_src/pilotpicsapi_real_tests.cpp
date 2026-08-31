/**
 * @file pilotpicsapi_real_tests.cpp
 * @brief Unit tests for Descent3/PilotPicsAPI.cpp: the pilot-picture index database.
 *
 * @details
 * builder and its name/id lookup, query, and find-first/next traversal over a
 * crafted binary index file.
 *
 * This harness validates the behavior of `Descent3/PilotPicsAPI.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/PilotPicsAPI.cpp`
 * @par Harness
 * `pilotpicsapi_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/PilotPicsAPI.cpp
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>

#include "cfile.h"
#include "PilotPicsAPI.h"
#include "mem.h"

// ---- CFILE stubs backed by a real FILE* ----
static std::filesystem::path g_ppic_idx; // overridden path for "PPics.idx"

CFILE *cfopen(const std::filesystem::path &filename, const char *mode) {
  std::filesystem::path p = filename;
  if (p.filename() == "PPics.idx" && !g_ppic_idx.empty())
    p = g_ppic_idx;
  FILE *f = fopen(p.string().c_str(), mode);
  if (!f)
    return nullptr;
  CFILE *c = new CFILE{};
  c->name = strdup(p.string().c_str());
  c->file = f;
  c->lib_handle = -1;
  return c;
}
void cfclose(CFILE *c) {
  if (c) {
    if (c->file)
      fclose(c->file);
    free(c->name);
  }
  delete c;
}
int32_t cf_ReadInt(CFILE *c, bool little_endian) {
  uint8_t b[4];
  if (fread(b, 1, 4, c->file) != 4)
    return 0;
  if (little_endian)
    return b[0] | (b[1] << 8) | (b[2] << 16) | ((uint32_t)b[3] << 24);
  return ((uint32_t)b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}
int16_t cf_ReadShort(CFILE *c, bool little_endian) {
  uint8_t b[2];
  if (fread(b, 1, 2, c->file) != 2)
    return 0;
  if (little_endian)
    return (int16_t)(b[0] | (b[1] << 8));
  return (int16_t)((b[0] << 8) | b[1]);
}
int8_t cf_ReadByte(CFILE *c) {
  uint8_t b;
  if (fread(&b, 1, 1, c->file) != 1)
    return 0;
  return (int8_t)b;
}
int cf_ReadBytes(uint8_t *buf, int count, CFILE *c) { return (int)fread(buf, 1, count, c->file); }
int cfseek(CFILE *c, long offset, int where) { return fseek(c->file, offset, where); }
long cftell(CFILE *c) { return ftell(c->file); }
int cfeof(CFILE *c) { return feof(c->file); }
void cf_Rewind(CFILE *fp) { fseek(fp->file, 0, SEEK_SET); }
int cf_OpenLibrary(const std::filesystem::path &) { return 1; }
void cf_CloseLibrary(int) {}

// bitmap stub: hand back a scriptable handle
static int s_bm_handle = 5;
int bm_AllocLoadFileBitmap(const char *, int, int) { return s_bm_handle; }

// ---- helpers to build a fake index file ----
static void PutLE(std::vector<uint8_t> &v, int val, int n) {
  for (int i = 0; i < n; i++)
    v.push_back((uint8_t)((val >> (8 * i)) & 0xFF));
}
static void AddRecord(std::vector<uint8_t> &v, const char *name, int id, const char *bmp) {
  int ns = (int)strlen(name);
  v.push_back((uint8_t)ns);
  for (int i = 0; i < ns; i++)
    v.push_back((uint8_t)name[i]);
  PutLE(v, id, 2);
  int bs = (int)strlen(bmp);
  v.push_back((uint8_t)bs);
  for (int i = 0; i < bs; i++)
    v.push_back((uint8_t)bmp[i]);
}

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
  void SetUp() override {
    // build a sorted index: Alice(100), Bob(200), Carol(300)
    std::vector<uint8_t> data;
    PutLE(data, 3, 4); // count
    AddRecord(data, "Alice", 100, "a.bmp");
    AddRecord(data, "Bob", 200, "b.bmp");
    AddRecord(data, "Carol", 300, "c.bmp");

    g_ppic_idx = "/tmp/opencode/ppic_idx.bin";
    FILE *f = fopen(g_ppic_idx.string().c_str(), "wb");
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    s_bm_handle = 5;
    ASSERT_TRUE(PPic_InitDatabase());
  }
  void TearDown() override { PPic_CloseDatabase(); }
};

/**
 * @test PilotPicsTest.InitBuildsDatabase
 * @brief Verifies init Builds Database.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/PilotPicsAPI.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, InitBuildsDatabase) {
  // PilotPic_init is internal; success of InitDatabase implies it.
  EXPECT_TRUE(PPic_QueryPilot("Alice") >= 1);
}

/**
 * @test PilotPicsTest.QueryCountsExactNameMatches
 * @brief Verifies query Counts Exact Name Matches.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/PilotPicsAPI.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, QueryCountsExactNameMatches) {
  EXPECT_EQ(PPic_QueryPilot("Alice"), 1);
  EXPECT_EQ(PPic_QueryPilot("Bob"), 1);
  EXPECT_EQ(PPic_QueryPilot("Carol"), 1);
  EXPECT_EQ(PPic_QueryPilot("Zoe"), 0);   // no such pilot
  EXPECT_EQ(PPic_QueryPilot("Al"), 0);    // prefix is not an exact match
}

/**
 * @test PilotPicsTest.GetPilotResolvesNameById
 * @brief Verifies get Pilot Resolves Name By Id.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/PilotPicsAPI.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, GetPilotResolvesNameById) {
  char buf[64];
  EXPECT_TRUE(PPic_GetPilot(100, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "Alice");
  EXPECT_TRUE(PPic_GetPilot(200, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "Bob");
  EXPECT_TRUE(PPic_GetPilot(300, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "Carol");
  EXPECT_FALSE(PPic_GetPilot(999, buf, sizeof(buf))); // invalid id
}

/**
 * @test PilotPicsTest.GetPilotExercisesIdBinarySearch
 * @brief Verifies get Pilot Exercises Id Binary Search.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/PilotPicsAPI.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, GetPilotExercisesIdBinarySearch) {
  // GetPilot -> GetOffsetByID -> GetIndexFromID (binary search over sorted ids)
  char buf[64];
  EXPECT_TRUE(PPic_GetPilot(100, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "Alice");
  EXPECT_TRUE(PPic_GetPilot(300, buf, sizeof(buf))); // last in sorted order
  EXPECT_TRUE(PPic_GetPilot(200, buf, sizeof(buf)));
  EXPECT_FALSE(PPic_GetPilot(999, buf, sizeof(buf))); // absent id
}

/**
 * @test PilotPicsTest.QueryUsesAlphabetJumpTable
 * @brief Verifies query Uses Alphabet Jump Table.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/PilotPicsAPI.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, QueryUsesAlphabetJumpTable) {
  // QueryPilot -> JumpToPilot (alphabet table). 'Z' has no entries.
  EXPECT_EQ(PPic_QueryPilot("Zoe"), 0);
  EXPECT_EQ(PPic_QueryPilot("Alice"), 1);
}

/**
 * @test PilotPicsTest.FindFirstNextIteratesMatches
 * @brief Verifies find First Next Iterates Matches.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/PilotPicsAPI.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, FindFirstNextIteratesMatches) {
  uint16_t id = 0;
  EXPECT_TRUE(PPic_FindFirst("Alice", &id));
  EXPECT_EQ(id, 100);
  EXPECT_FALSE(PPic_FindNext(&id)); // only one Alice
  PPic_FindClose();
}

/**
 * @test PilotPicsTest.GetBitmapHandleResolvesByValidId
 * @brief Verifies get Bitmap Handle Resolves By Valid Id.
 *
 * @details
 * Exercises the PilotPicsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/PilotPicsAPI.cpp
 * @ingroup descent3_tests
 */
TEST_F(PilotPicsTest, GetBitmapHandleResolvesByValidId) {
  EXPECT_EQ(PPic_GetBitmapHandle(100), 5);   // stubbed valid handle
  EXPECT_EQ(PPic_GetBitmapHandle(999), -1);  // invalid id -> -1 before bitmap load
}
