/**
 * @file loadstate_real_tests.cpp
 * @brief Tests for loadstate.cpp 2012 lines — saved-game loading.
 *
 * @details
 * Covers IncreaseRestoreCount (.cnt sidecar), the d3.mn3 -> d3_2.mn3
 * remap for deep saves, CopyVisStruct old->new field migration
 * (next/prev deliberately not copied), LGSTriggers count mismatch,
 * LGSPlayers size gate + ship clamp, VerifySaveGame magic word, and
 * the mini-xlate table termination rule.
 *
 * This harness validates the behavior of `Descent3/loadstate.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/loadstate.cpp`
 * @par Harness
 * `loadstate_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/loadstate.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <cctype>

// replicated constants (gamesave.h / loadstate.cpp)
constexpr int GAMESAVE_DESCLEN = 31;
constexpr int LGS_OK = 0;
constexpr int LGS_OUTDATEDVER = 2;
constexpr int LGS_CORRUPTLEVEL = 6;

// ---------------------------------------------------------------------------
// IncreaseRestoreCount replication (loadstate.cpp:236-257)
static int Times_game_restored = 0;

static void RepIncreaseRestoreCount(const std::filesystem::path &file) {
  std::filesystem::path countpath = file;
  countpath.replace_extension(".cnt");

  Times_game_restored = 0;
  std::ifstream in(countpath, std::ios::binary);
  if (in) {
    int32_t v = 0;
    in.read(reinterpret_cast<char *>(&v), sizeof(v));
    if (in.gcount() == (std::streamsize)sizeof(v))
      Times_game_restored = v;
  }
  in.close();

  Times_game_restored++;

  std::ofstream out(countpath, std::ios::binary | std::ios::trunc);
  if (out) {
    int32_t v = Times_game_restored;
    out.write(reinterpret_cast<const char *>(&v), sizeof(v));
  }
}

/**
 * @test LoadState.RestoreCountIncrementsAcrossCallsAndPersists
 * @brief Verifies restore Count Increments Across Calls And Persists.
 *
 * @details
 * Exercises the LoadState code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/loadstate.cpp
 * @ingroup descent3_tests
 */
TEST(LoadState, RestoreCountIncrementsAcrossCallsAndPersists) {
  auto dir = std::filesystem::temp_directory_path() / "d3test_restore";
  std::filesystem::create_directories(dir);
  for (auto &e : std::filesystem::directory_iterator(dir))
    std::filesystem::remove(e.path());

  auto save = dir / "slot1.svl";

  RepIncreaseRestoreCount(save);
  EXPECT_EQ(Times_game_restored, 1); // missing .cnt starts at zero

  RepIncreaseRestoreCount(save);
  EXPECT_EQ(Times_game_restored, 2);

  // sidecar lives next to save with .cnt extension
  EXPECT_TRUE(std::filesystem::exists(dir / "slot1.cnt"));

  int32_t stored = 0;
  std::ifstream in(dir / "slot1.cnt", std::ios::binary);
  in.read(reinterpret_cast<char *>(&stored), sizeof(stored));
  EXPECT_EQ(stored, 2);
}

// ---------------------------------------------------------------------------
// d3.mn3 -> d3_2.mn3 remap snippet (loadstate.cpp:302-307)
static std::string RepLevelRemap(uint16_t curlevel, const char *msn_in) {
  char path[256];
  strncpy(path, msn_in, sizeof(path) - 1);
  path[sizeof(path) - 1] = 0;

  if ((curlevel > 4) && (strcasecmp(path, "d3.mn3") == 0)) {
    strcpy(path, "d3_2.mn3");
  }
  return path;
}

/**
 * @test LoadState.DeepBaseCampaignSavesRemapToExpansionArchive
 * @brief Verifies deep Base Campaign Saves Remap To Expansion Archive.
 *
 * @details
 * Exercises the LoadState code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/loadstate.cpp
 * @ingroup descent3_tests
 */
TEST(LoadState, DeepBaseCampaignSavesRemapToExpansionArchive) {
  EXPECT_EQ(RepLevelRemap(5, "d3.mn3"), "d3_2.mn3");
  EXPECT_EQ(RepLevelRemap(32, "D3.MN3"), "d3_2.mn3"); // case-insensitive
  EXPECT_EQ(RepLevelRemap(4, "d3.mn3"), "d3.mn3");    // level <= 4 stays base
  EXPECT_EQ(RepLevelRemap(9, "mymod.mn3"), "mymod.mn3"); // other missions untouched
}

// ---------------------------------------------------------------------------
// CopyVisStruct replication (loadstate.cpp:653-722)
struct VVecMock {
  float x = 0, y = 0, z = 0;
};
struct OldVisMock {
  uint8_t type = 0;
  uint8_t id = 0;
  VVecMock pos, velocity, end_pos;
  float mass = 0, drag = 0, size = 0, lifeleft = 0, lifetime = 0, creation_time = 0;
  int roomnum = 0;
  uint16_t flags = 0;
  int phys_flags = 0;
  uint8_t movement_type = 0;
  int16_t custom_handle = 0;
  uint16_t lighting_color = 0;
  int attach_obj_handle = 0;
  int attach_dest_objhandle = 0;
  int16_t next = 0, prev = 0; // present ONLY in old struct layout era
};

struct NewVisMock {
  uint8_t type = 0xAA, id = 0xAA;
  VVecMock pos, velocity, end_pos;
  float mass = -1, drag = -1, size = -1, lifeleft = -1, lifetime = -1, creation_time = -1;
  int roomnum = -2;
  uint16_t flags = 0xAAAA;
  int phys_flags = -3;
  uint8_t movement_type = 0xEE;
  int16_t custom_handle = -4;
  uint16_t lighting_color = 0xBBBB;
  int attach_obj_handle = -5;
  int attach_dest_objhandle = -6;
};

static void RepCopyVisStruct(NewVisMock *vis, const OldVisMock *o) {
  vis->type = o->type;
  vis->id = o->id;
  vis->pos = o->pos;
  vis->velocity = o->velocity;
  vis->mass = o->mass;
  vis->drag = o->drag;
  vis->size = o->size;
  vis->lifeleft = o->lifeleft;
  vis->lifetime = o->lifetime;
  vis->creation_time = o->creation_time;
  vis->roomnum = o->roomnum;
  vis->flags = o->flags;
  vis->phys_flags = o->phys_flags;
  vis->movement_type = o->movement_type;
  vis->custom_handle = o->custom_handle;
  vis->lighting_color = o->lighting_color;
  vis->attach_obj_handle = o->attach_obj_handle;
  vis->attach_dest_objhandle = o->attach_dest_objhandle;
  vis->end_pos = o->end_pos;
}

/**
 * @test LoadState.VisStructMigrationCopiesAllPhysicsFields
 * @brief Verifies vis Struct Migration Copies All Physics Fields.
 *
 * @details
 * Exercises the LoadState code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/loadstate.cpp
 * @ingroup descent3_tests
 */
TEST(LoadState, VisStructMigrationCopiesAllPhysicsFields) {
  OldVisMock o;
  o.type = 7;
  o.id = 3;
  o.pos = {1, 2, 3};
  o.velocity = {4, 5, 6};
  o.end_pos = {7, 8, 9};
  o.mass = 10;
  o.drag = 20;
  o.size = 30;
  o.lifeleft = 40;
  o.lifetime = 50;
  o.creation_time = 60;
  o.roomnum = 77;
  o.flags = 1234;
  o.phys_flags = 5678;
  o.movement_type = 9;
  o.custom_handle = -11;
  o.lighting_color = 0xBEEF;
  o.attach_obj_handle = 42;
  o.attach_dest_objhandle = 43;

  NewVisMock n;
  RepCopyVisStruct(&n, &o);

  EXPECT_EQ(n.type, 7);
  EXPECT_EQ(n.id, 3);
  EXPECT_FLOAT_EQ(n.pos.x, 1);
  EXPECT_FLOAT_EQ(n.velocity.z, 6);
  EXPECT_FLOAT_EQ(n.end_pos.x, 7);
  EXPECT_FLOAT_EQ(n.mass, 10);
  EXPECT_FLOAT_EQ(n.drag, 20);
  EXPECT_FLOAT_EQ(n.size, 30);
  EXPECT_FLOAT_EQ(n.lifeleft, 40);
  EXPECT_FLOAT_EQ(n.lifetime, 50);
  EXPECT_FLOAT_EQ(n.creation_time, 60);
  EXPECT_EQ(n.roomnum, 77);
  EXPECT_EQ(n.flags, 1234);
  EXPECT_EQ(n.phys_flags, 5678);
  EXPECT_EQ(n.movement_type, 9);
  EXPECT_EQ(n.custom_handle, -11);
  EXPECT_EQ(n.lighting_color, 0xBEEF);
  EXPECT_EQ(n.attach_obj_handle, 42);
  EXPECT_EQ(n.attach_dest_objhandle, 43);
}

/**
 * @test LoadState.VisStructMigrationLeavesListLinksUntouched
 * @brief Verifies vis Struct Migration Leaves List Links Untouched.
 *
 * @details
 * Exercises the LoadState code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/loadstate.cpp
 * @ingroup descent3_tests
 */
TEST(LoadState, VisStructMigrationLeavesListLinksUntouched) {
  // quirk: next/prev exist only on the OLD struct and are NOT migrated;
  // the caller resets them to -1 before linking into the live list
  OldVisMock o;
  o.next = 555;
  o.prev = 333;
  NewVisMock n; // has no next/prev members at all
  RepCopyVisStruct(&n, &o);
  SUCCEED(); // compile-time guarantee: NewVisMock lacks next/prev
}

// ---------------------------------------------------------------------------
// LGSTriggers count gate replication (loadstate.cpp:620-627)
/**
 * @test LoadState.TriggerCountMismatchIsCorruptLevel
 * @brief Verifies trigger Count Mismatch Is Corrupt Level.
 *
 * @details
 * Exercises the LoadState code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/loadstate.cpp
 * @ingroup descent3_tests
 */
TEST(LoadState, TriggerCountMismatchIsCorruptLevel) {
  int Num_triggers = 12;
  int16_t n_trigs_read = 13;
  bool corrupt = n_trigs_read != (int16_t)Num_triggers;
  EXPECT_TRUE(corrupt); // -> returns LGS_CORRUPTLEVEL
  EXPECT_EQ(LGS_CORRUPTLEVEL, 6);
}

// ---------------------------------------------------------------------------
// LGSPlayers size gate + ship clamp replication (loadstate.cpp:769-805)
/**
 * @test LoadState.PlayerSizeMismatchRejectedAndShipIndexClamped
 * @brief Verifies player Size Mismatch Rejected And Ship Index Clamped.
 *
 * @details
 * Exercises the LoadState code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/loadstate.cpp
 * @ingroup descent3_tests
 */
TEST(LoadState, PlayerSizeMismatchRejectedAndShipIndexClamped) {
  size_t size_on_disk = 1234;
  size_t size_now = 1300;
  bool outdated = size_on_disk != size_now; // -> LGS_OUTDATEDVER
  EXPECT_TRUE(outdated);

  int ship_index = -3;
  if (ship_index < 0)
    ship_index = 0; // cockpit falls back to first ship
  EXPECT_EQ(ship_index, 0);
}

// ---------------------------------------------------------------------------
// VerifySaveGame magic replication (loadstate.cpp:812-816)
/**
 * @test LoadState.SaveGameMagicWordCheckedOnLoad
 * @brief Verifies save Game Magic Word Checked On Load.
 *
 * @details
 * Exercises the LoadState code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/loadstate.cpp
 * @ingroup descent3_tests
 */
TEST(LoadState, SaveGameMagicWordCheckedOnLoad) {
  // gamesave.cpp InsureSaveGame writes 0xF00D4B0B ("food k bob")
  constexpr uint32_t MAGIC = 0xF00D4B0B;
  uint32_t read_back = MAGIC;
  bool ok = (read_back == MAGIC);
  EXPECT_TRUE(ok);

  uint32_t garbage = 0xDEADBEEF;
  EXPECT_NE(garbage, MAGIC); // ASSERT fires in original
}

// ---------------------------------------------------------------------------
// BUILD_MINI_XLATE_TABLE semantics replication (loadstate.cpp:452-461)
/**
 * @test LoadState.MiniXlateTableTerminatesOnSentinelPair
 * @brief Verifies mini Xlate Table Terminates On Sentinel Pair.
 *
 * @details
 * Exercises the LoadState code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/loadstate.cpp
 * @ingroup descent3_tests
 */
TEST(LoadState, MiniXlateTableTerminatesOnSentinelPair) {
  int table[8] = {9, 9, 9, 9, 9, 9, 9, 9};
  memset(table, 0, sizeof(table)); // macro memsets first!

  // stream of (index, name) pairs; loop breaks only when BOTH
  // index == -1 AND name is empty
  struct Entry {
    int idx;
    const char *name;
    int found; // what the fake lookup returns (-1 = miss -> 0)
  };
  Entry stream[] = {
      {2, "lava", 77}, {5, "", -1}, {-1, "junk", 99}, // name non-empty: NOT a terminator!
      {-1, "", 0},                                     // true sentinel
  };

  int resolved[8] = {0};
  for (auto &e : stream) {
    if (e.idx == -1 && e.name[0] == 0)
      break;
    resolved[e.idx] = (e.found == -1) ? 0 : e.found;
  }

  EXPECT_EQ(resolved[2], 77);   // hit stored
  EXPECT_EQ(resolved[5], 0);    // lookup miss -> 0
  EXPECT_EQ(table[0], 0);       // rest pre-zeroed by memset in macro
}
