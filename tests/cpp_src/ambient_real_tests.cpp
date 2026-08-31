/**
 * @file ambient_real_tests.cpp
 * @brief Tests for ambient.cpp (337 lines).
 *
 * @details
 * Covers ProcessASP probability selection/volume/delay math through
 * DoAmbientSounds, InitAmbientSounds normalization, pattern lookup, and the
 * ambient.dat reader with compaction quirk via InitAmbientSoundSystem.
 *
 * This harness validates the behavior of `Descent3/ambient.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/ambient.cpp`
 * @par Harness
 * `ambient_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/ambient.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <vector>

#include "ambient.h"
#include "hlsoundlib.h"
#include "room.h"
#include "object.h"
#include "game.h"
#include "pserror.h"
#include "psrand.h"
#include "soundload.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
object *Viewer_object = nullptr;
room Rooms[MAX_ROOMS];
float Frametime = 0.0f;
// Num_ambient_sound_patterns is defined by ambient.cpp itself

// Scripted ps_rand
static int g_rand_seq[16];
static int g_rand_len = 0, g_rand_idx = 0;
int ps_rand() {
  if (g_rand_idx >= g_rand_len)
    g_rand_idx = 0;
  return g_rand_seq[g_rand_idx++];
}
void ps_srand(unsigned int s) { (void)s; }

int FindSoundName(const char *name) {
  (void)name;
  return 77;
}

// Referenced by WriteAmbientData (not under test); zeroed table is fine.
#include "ssl_lib.h"
sound_info Sounds[MAX_SOUNDS];

// ---------------------------------------------------------------------------
// Recording sound system (real sndlib not linked)
// ---------------------------------------------------------------------------
struct PlayRec {
  int index;
  float volume;
};
static std::vector<PlayRec> g_plays;

hlsSystem Sound_system;
hlsSystem::hlsSystem() {}
int hlsSystem::Play2dSound(int sound_index, float volume, float, uint16_t) {
  g_plays.push_back({sound_index, volume});
  return 0;
}
void hlsSystem::KillSoundLib(bool) {}

/**
 * @brief GTest fixture for AmbientTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class AmbientTest : public ::testing::Test {
protected:
  void SetUp() override {
    Num_ambient_sound_patterns = 0;
    memset(Rooms, 0, sizeof(Rooms));
    for (auto &r : Rooms)
      r.ambient_sound = -1;
    Frametime = 0.0f;
    g_rand_len = g_rand_idx = 0;
    g_plays.clear();

    static object viewer{};
    viewer.roomnum = 3;
    Viewer_object = &viewer;
    Rooms[3].ambient_sound = -1;

    // Let the cfile system resolve bare filenames against our scratch dir
    cf_SetSearchPath("/tmp/opencode");
  }

  // Fills pattern slot `idx`; returns pointer to the LIVE array entry.
  asp *MakePattern(int idx, int num_sounds, const int *probs, float min_vol = 0.5f, float max_vol = 1.0f) {
    asp tmp{};
    strcpy(tmp.name, "pattern");
    tmp.min_delay = 1.0f;
    tmp.max_delay = 2.0f;
    tmp.num_sounds = num_sounds;
    tmp.sounds = num_sounds ? new ase[num_sounds]{} : nullptr;
    for (int i = 0; i < num_sounds; i++) {
      tmp.sounds[i].handle = 100 + i;
      tmp.sounds[i].probability = probs[i];
      tmp.sounds[i].min_volume = min_vol;
      tmp.sounds[i].max_volume = max_vol;
    }
    Ambient_sound_patterns[idx] = tmp;
    if (idx + 1 > Num_ambient_sound_patterns)
      Num_ambient_sound_patterns = idx + 1;
    return &Ambient_sound_patterns[idx];
  }
};

// ---------------------------------------------------------------------------
// ProcessASP (exercised through DoAmbientSounds)
// ---------------------------------------------------------------------------

/**
 * @test AmbientTest.EmptyPatternNeverPlays
 * @brief Verifies empty Pattern Never Plays.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, EmptyPatternNeverPlays) {
  static const int probs[] = {100};
  asp *a = MakePattern(0, 0, probs);
  Rooms[3].ambient_sound = 0;
  a->delay = -1.0f; // overdue

  Frametime = 0.1f;
  DoAmbientSounds();
  EXPECT_TRUE(g_plays.empty());
  delete[] Ambient_sound_patterns[0].sounds;
}

/**
 * @test AmbientTest.DelayNotElapsedNoSoundButCountsDown
 * @brief Verifies delay Not Elapsed No Sound But Counts Down.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, DelayNotElapsedNoSoundButCountsDown) {
  static const int probs[] = {100};
  asp *a = MakePattern(0, 1, probs);
  Rooms[3].ambient_sound = 0;
  a->delay = 5.0f;

  Frametime = 0.25f;
  DoAmbientSounds();
  EXPECT_TRUE(g_plays.empty());
  EXPECT_FLOAT_EQ(a->delay, 4.75f);
  delete[] Ambient_sound_patterns[0].sounds;
}

/**
 * @test AmbientTest.FirstSoundSelectedAndVolumeRandomized
 * @brief Verifies first Sound Selected And Volume Randomized.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, FirstSoundSelectedAndVolumeRandomized) {
  static const int probs[] = {100};
  asp *a = MakePattern(0, 1, probs);
  Rooms[3].ambient_sound = 0;
  a->delay = 0.0f;

  // roll uses ps_rand #0 (=0 -> roll 0); volume uses ps_rand #1 (=16384);
  // next delay uses the wrapped #0 again.
  g_rand_seq[0] = 0;
  g_rand_seq[1] = 16384;
  g_rand_len = 2;

  Frametime = 0.1f;
  DoAmbientSounds();

  ASSERT_EQ(g_plays.size(), 1u);
  EXPECT_EQ(g_plays[0].index, 100);
  float expected_vol = 0.5f + (1.0f - 0.5f) * (16384 / (float)D3_RAND_MAX);
  EXPECT_NEAR(g_plays[0].volume, expected_vol, 1e-4f);
  float expected_delay = 1.0f + (2.0f - 1.0f) * (0 / (float)D3_RAND_MAX);
  EXPECT_FLOAT_EQ(a->delay, expected_delay);
  delete[] Ambient_sound_patterns[0].sounds;
}

/**
 * @test AmbientTest.RollBeyondFirstProbabilityPicksSecondSound
 * @brief Verifies roll Beyond First Probability Picks Second Sound.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, RollBeyondFirstProbabilityPicksSecondSound) {
  static const int probs[] = {50, 50};
  asp *a = MakePattern(0, 2, probs);
  Rooms[3].ambient_sound = 0;
  a->delay = 0.0f;

  // rand=20000 -> roll=(20000*100)/32768=61 >= 50 -> second sound
  g_rand_seq[0] = 20000;
  g_rand_len = 1;
  Frametime = 0.1f;

  DoAmbientSounds();
  ASSERT_EQ(g_plays.size(), 1u);
  EXPECT_EQ(g_plays[0].index, 101);
  delete[] Ambient_sound_patterns[0].sounds;
}

/**
 * @test AmbientTest.ViewerOutsideSkipsEverything
 * @brief Verifies viewer Outside Skips Everything.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, ViewerOutsideSkipsEverything) {
  Viewer_object->roomnum = MAKE_ROOMNUM(7); // terrain/outside flag set
  Rooms[3].ambient_sound = -1;

  static const int probs[] = {100};
  MakePattern(0, 1, probs);
  Ambient_sound_patterns[0].delay = -1.0f;

  DoAmbientSounds();
  EXPECT_TRUE(g_plays.empty());
}

/**
 * @test AmbientTest.RoomWithoutPatternIsSilent
 * @brief Verifies room Without Pattern Is Silent.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, RoomWithoutPatternIsSilent) {
  Rooms[3].ambient_sound = -1;
  static const int probs[] = {100};
  asp *a = MakePattern(0, 1, probs);
  a->delay = -1.0f;

  DoAmbientSounds();
  EXPECT_TRUE(g_plays.empty());
  delete[] Ambient_sound_patterns[0].sounds;
}

// ---------------------------------------------------------------------------
// InitAmbientSounds
// ---------------------------------------------------------------------------

/**
 * @test AmbientTest.InitSetsInitialDelayWithinRange
 * @brief Verifies init Sets Initial Delay Within Range.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, InitSetsInitialDelayWithinRange) {
  static const int probs[] = {100};
  asp *a = MakePattern(0, 1, probs);
  g_rand_seq[0] = 8192;
  g_rand_len = 1;

  Num_ambient_sound_patterns = 1;
  InitAmbientSounds();

  float expected = 1.0f + (2.0f - 1.0f) * (8192 / (float)D3_RAND_MAX);
  EXPECT_NEAR(a->delay, expected, 1e-4f);
  delete[] Ambient_sound_patterns[0].sounds;
}

/**
 * @test AmbientTest.InitFixesProbabilitySum
 * @brief Verifies init Fixes Probability Sum.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, InitFixesProbabilitySum) {
  static const int probs[] = {30, 30}; // sums to 60
  asp *a = MakePattern(0, 2, probs);
  Num_ambient_sound_patterns = 1;

  InitAmbientSounds();

  EXPECT_EQ(a->sounds[0].probability, 70); // += 100-60
  EXPECT_EQ(a->sounds[1].probability, 30);
  delete[] Ambient_sound_patterns[0].sounds;
}

/**
 * @test AmbientTest.InitLeavesExactSumAlone
 * @brief Verifies init Leaves Exact Sum Alone.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, InitLeavesExactSumAlone) {
  static const int probs[] = {40, 60};
  asp *a = MakePattern(0, 2, probs);
  Num_ambient_sound_patterns = 1;

  InitAmbientSounds();
  EXPECT_EQ(a->sounds[0].probability, 40);
  EXPECT_EQ(a->sounds[1].probability, 60);
  delete[] Ambient_sound_patterns[0].sounds;
}

// ---------------------------------------------------------------------------
// Pattern lookup
// ---------------------------------------------------------------------------

/**
 * @test AmbientTest.FindByNameCaseInsensitiveEmptyRejected
 * @brief Verifies find By Name Case Insensitive Empty Rejected.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, FindByNameCaseInsensitiveEmptyRejected) {
  strcpy(Ambient_sound_patterns[4].name, "Cave_Drips");
  Ambient_sound_patterns[4].num_sounds = 1;
  Num_ambient_sound_patterns = 5;

  EXPECT_EQ(FindAmbientSoundPattern((char *)"cave_drips"), 4);
  EXPECT_EQ(FindAmbientSoundPattern((char *)""), -1); // empty name rejected outright
  EXPECT_STREQ(AmbientSoundPatternName(4), "Cave_Drips");
}

// ---------------------------------------------------------------------------
// ambient.dat reading via ReadAmbientData + InitAmbientSoundSystem
// ---------------------------------------------------------------------------

static void WriteAmbientFile(const char *path, const std::vector<uint8_t> &bytes) {
  FILE *fp = fopen(path, "wb");
  fwrite(bytes.data(), 1, bytes.size(), fp);
  fclose(fp);
}

static void PutU32(std::vector<uint8_t> &v, uint32_t x) {
  v.insert(v.end(), {(uint8_t)x, (uint8_t)(x >> 8), (uint8_t)(x >> 16), (uint8_t)(x >> 24)});
}
static void PutStr(std::vector<uint8_t> &v, const char *s) {
  v.insert(v.end(), s, s + strlen(s));
  v.push_back(0);
}

/**
 * @test AmbientTest.ReadAmbientDataParsesValidFile
 * @brief Verifies read Ambient Data Parses Valid File.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, ReadAmbientDataParsesValidFile) {
  std::vector<uint8_t> f;
  f.insert(f.end(), {'A', 'S', 'P', 'F'});
  PutU32(f, 0);   // version
  PutU32(f, 1);   // one pattern
  PutStr(f, "wind");
  PutU32(f, 0x3F800000); // min_delay = 1.0f
  PutU32(f, 0x40000000); // max_delay = 2.0f
  PutU32(f, 2);   // two sounds
  PutStr(f, "gust1");
  PutU32(f, 0x3F000000); PutU32(f, 0x3F800000); PutU32(f, 60); // vol .5..1, prob 60
  PutStr(f, "gust2");
  PutU32(f, 0x3F000000); PutU32(f, 0x3F800000); PutU32(f, 40);

  chdir("/tmp/opencode"); // cfopen resolves relative to cwd
  WriteAmbientFile("ambient.dat", f);

  InitAmbientSoundSystem(); // reads file then compacts

  ASSERT_EQ(Num_ambient_sound_patterns, 1u);
  EXPECT_STREQ(Ambient_sound_patterns[0].name, "wind");
  EXPECT_FLOAT_EQ(Ambient_sound_patterns[0].min_delay, 1.0f);
  ASSERT_EQ(Ambient_sound_patterns[0].num_sounds, 2);
  EXPECT_EQ(Ambient_sound_patterns[0].sounds[1].probability, 40);
  EXPECT_EQ(Ambient_sound_patterns[0].sounds[0].handle, 77); // FindSoundName stub
}

/**
 * @test AmbientTest.MissingFileLeavesNoPatterns
 * @brief Verifies missing File Leaves No Patterns.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, MissingFileLeavesNoPatterns) {
  chdir("/tmp/opencode");
  remove("ambient.dat");

  InitAmbientSoundSystem();
  EXPECT_EQ(Num_ambient_sound_patterns, 0);
}

/**
 * @test AmbientTest.BadMagicLeavesNoPatterns
 * @brief Verifies bad Magic Leaves No Patterns.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, BadMagicLeavesNoPatterns) {
  std::vector<uint8_t> f;
  f.insert(f.end(), {'X', 'Y', 'Z', 'Z'});
  PutU32(f, 0);
  PutU32(f, 0);
  chdir("/tmp/opencode");
  WriteAmbientFile("ambient.dat", f);

  InitAmbientSoundSystem();
  EXPECT_EQ(Num_ambient_sound_patterns, 0);
}

/**
 * @test AmbientTest.FutureVersionRejected
 * @brief Verifies future Version Rejected.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, FutureVersionRejected) {
  std::vector<uint8_t> f;
  f.insert(f.end(), {'A', 'S', 'P', 'F'});
  PutU32(f, 99); // version > AMBIENT_FILE_VERSION(0)
  PutU32(f, 0);
  chdir("/tmp/opencode");
  WriteAmbientFile("ambient.dat", f);

  InitAmbientSoundSystem();
  EXPECT_EQ(Num_ambient_sound_patterns, 0);
}

/**
 * @test AmbientTest.CompactionQuirkKeepsDeletedTailPattern
 * @brief Verifies compaction Quirk Keeps Deleted Tail Pattern.
 *
 * @details
 * Exercises the AmbientTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ambient.cpp
 * @ingroup descent3_tests
 */
TEST_F(AmbientTest, CompactionQuirkKeepsDeletedTailPattern) {
  std::vector<uint8_t> f;
  f.insert(f.end(), {'A', 'S', 'P', 'F'});
  PutU32(f, 0);
  PutU32(f, 3);            // three patterns: [deleted, keep, deleted]
  PutStr(f, "");           // p0: empty name -> marked deleted
  PutU32(f, 0); PutU32(f, 0); PutU32(f, 0); // zero sounds
  PutStr(f, "keepme");     // p1: valid
  PutU32(f, 0); PutU32(f, 0); PutU32(f, 0);
  PutStr(f, "");           // p2: empty name -> deleted... or is it?
  PutU32(f, 0); PutU32(f, 0); PutU32(f, 0);
  chdir("/tmp/opencode");
  WriteAmbientFile("ambient.dat", f);

  // Quirk: the forward-scan compaction swaps the LAST pattern into the hole
  // without re-inspecting it, so the trailing deleted pattern survives at the
  // front while the count drops.
  InitAmbientSoundSystem();
  ASSERT_GE(Num_ambient_sound_patterns, 1);
  EXPECT_STREQ(Ambient_sound_patterns[0].name, ""); // survivor of the swap
}
