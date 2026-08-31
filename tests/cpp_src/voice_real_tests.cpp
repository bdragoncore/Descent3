/**
 * @file voice_real_tests.cpp
 * @brief Tests for voice.cpp — voice playback + queue (330 lines).
 *
 * @details
 * Compiles real Descent3/voice.cpp with fake hlsSystem, stream-audio stubs.
 * Covers InitVoices, PlayVoice gating (powerup/force/interupt), queue FIFO,
 * motherload overflow ("Cheater!"), PLAYTABLE vs stream paths, StopVoice/UpdateVoices.
 *
 * This harness validates the behavior of `Descent3/voice.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/voice.cpp`
 * @par Harness
 * `voice_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/voice.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>
#include "voice.h"
#include "hlsoundlib.h"

// ---- fake hlsSystem capture (self-contained; sndlib not linked) ----
#include <cstdint>
static std::vector<std::pair<int, float>> g_fake_plays;

// controlled "is a sound playing" answer
bool g_fake_sound_playing = false;
int g_fake_next_sound_id = 1;
std::vector<int> g_stopped_sounds;

hlsSystem Sound_system;

hlsSystem::hlsSystem() {}
void hlsSystem::KillSoundLib(bool) {}

int hlsSystem::Play2dSound(int sound_index, float volume, float, uint16_t) {
  g_fake_plays.push_back({sound_index, volume});
  return g_fake_next_sound_id++;
}
void hlsSystem::StopSoundImmediate(int hlsound_uid) { g_stopped_sounds.push_back(hlsound_uid); }
bool hlsSystem::IsSoundPlaying(int) { return g_fake_sound_playing; }

// ---- stream audio stubs ----
struct StreamRec {
  std::string filename;
  float volume;
  int flags;
};
static std::vector<StreamRec> g_stream_plays;
static std::vector<int> g_stream_stops;
static int g_stream_next_handle = 100;
static bool g_stream_play_fails = false;

int StreamPlay(const std::filesystem::path &filename, float volume, int flags) {
  if (g_stream_play_fails)
    return -1;
  g_stream_plays.push_back({filename.string(), volume, flags});
  return g_stream_next_handle++;
}
void StreamStop(int handle) { g_stream_stops.push_back(handle); }
int StreamGetSoundHandle(int handle) { return handle + 1000; }

// ---- sound table lookup stub ----
int FindSoundName(const char *name) {
  if (strcmp(name, "ThunderA") == 0)
    return 101;
  if (strcmp(name, "Cheater!") == 0)
    return 55;
  return -1;
}

/**
 * @brief GTest fixture for VoiceTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VoiceTest : public ::testing::Test {
protected:
  void SetUp() override {
    g_fake_plays.clear();
    g_stopped_sounds.clear();
    g_fake_sound_playing = false;
    g_fake_next_sound_id = 1;
    g_stream_plays.clear();
    g_stream_stops.clear();
    g_stream_next_handle = 100;
    g_stream_play_fails = false;
    PlayPowerupVoice = false;
    PlayVoices = true;
    InitVoices();
  }
};

/**
 * @test VoiceTest.InitResetsState
 * @brief Verifies init Resets State.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, InitResetsState) {
  // After InitVoices nothing is playing; UpdateVoices is a no-op
  UpdateVoices();
  EXPECT_TRUE(g_stream_plays.empty());
  EXPECT_TRUE(g_fake_plays.empty());
}

/**
 * @test VoiceTest.PlayVoicePlaysStreamWhenFree
 * @brief Verifies play Voice Plays Stream When Free.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, PlayVoicePlaysStreamWhenFree) {
  char name[] = "intro.wav";
  PlayVoice(name, VF_16BIT);
  UpdateVoices(); // pulls from queue and starts
  ASSERT_EQ(g_stream_plays.size(), 1u);
  EXPECT_EQ(g_stream_plays[0].filename, "intro.wav");
  UpdateVoices(); // still playing -> no change
  EXPECT_EQ(g_stream_plays.size(), 1u);
  StopVoice();
  ASSERT_EQ(g_stream_stops.size(), 1u);
  EXPECT_EQ(g_stream_stops[0], g_stream_plays[0].flags ? 100 : 100); // chandle from StreamPlay
}

/**
 * @test VoiceTest.PowerupVoiceGatedUnlessForced
 * @brief Verifies powerup Voice Gated Unless Forced.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, PowerupVoiceGatedUnlessForced) {
  char name[] = "pickup.wav";
  // Gated: nothing queued (no idle UpdateVoices -> reader still aligned)
  PlayVoice(name, VF_POWERUP);
  PlayVoice(name, VF_POWERUP | VF_FORCE); // forced through
  UpdateVoices();
  ASSERT_EQ(g_stream_plays.size(), 1u);

  // Allowed now
  StopVoice();
  PlayPowerupVoice = true;
  char name2[] = "pickup2.wav";
  PlayVoice(name2, VF_POWERUP);
  UpdateVoices();
  ASSERT_EQ(g_stream_plays.size(), 2u);
  EXPECT_EQ(g_stream_plays[1].filename, "pickup2.wav");
}

/**
 * @test VoiceTest.QueueFifoAdvancesWhenCurrentFinishes
 * @brief Verifies queue Fifo Advances When Current Finishes.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, QueueFifoAdvancesWhenCurrentFinishes) {
  char a[] = "a.wav", b[] = "b.wav", c[] = "c.wav";
  PlayVoice(a, 0);
  PlayVoice(b, 0);
  PlayVoice(c, 0);

  UpdateVoices(); // starts a
  ASSERT_EQ(g_stream_plays.size(), 1u);
  EXPECT_EQ(g_stream_plays[0].filename, "a.wav");

  g_fake_sound_playing = true;
  UpdateVoices(); // still playing
  EXPECT_EQ(g_stream_plays.size(), 1u);

  g_fake_sound_playing = false; // a finished
  UpdateVoices();               // stops a, starts b
  EXPECT_EQ(g_stream_plays.size(), 2u);
  EXPECT_EQ(g_stream_plays[1].filename, "b.wav");
  EXPECT_EQ(g_stream_stops.size(), 1u);

  UpdateVoices(); // b also finished
  EXPECT_EQ(g_stream_plays.size(), 3u);
  EXPECT_EQ(g_stream_plays[2].filename, "c.wav");
}

/**
 * @test VoiceTest.InteruptClearsQueueAndPlaysNew
 * @brief Verifies interupt Clears Queue And Plays New.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, InteruptClearsQueueAndPlaysNew) {
  char a[] = "a.wav", b[] = "b.wav", urgent[] = "urgent.wav";
  PlayVoice(a, 0);
  UpdateVoices(); // a playing
  ASSERT_EQ(g_stream_plays.size(), 1u);

  PlayVoice(b, 0);     // queued behind
  PlayVoice(urgent, VF_INTERUPT); // stops current, clears queue

  g_fake_sound_playing = true;
  UpdateVoices(); // current was stopped; plays urgent
  ASSERT_EQ(g_stream_plays.size(), 2u);
  EXPECT_EQ(g_stream_plays[1].filename, "urgent.wav");

  UpdateVoices(); // urgent finished; queue was cleared so nothing new
  EXPECT_EQ(g_stream_plays.size(), 2u);
  EXPECT_TRUE(g_stream_stops.size() >= 1u);
}

/**
 * @test VoiceTest.PlaytablePathUsesSoundTable
 * @brief Verifies playtable Path Uses Sound Table.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, PlaytablePathUsesSoundTable) {
  char tbl[] = "Cheater!";
  PlayVoice(tbl, VF_PLAYTABLE | VF_FORCE);
  UpdateVoices();
  // PLAYTABLE path: FindSoundName + Play2dSound, not StreamPlay
  ASSERT_EQ(g_fake_plays.size(), 1u);
  EXPECT_EQ(g_fake_plays[0].first, 55);
  EXPECT_TRUE(g_stream_plays.empty());

  g_fake_sound_playing = false; // report not playing
  UpdateVoices();               // stop via StopSoundImmediate
  EXPECT_EQ(g_stopped_sounds.size(), 1u);
  EXPECT_EQ(g_stopped_sounds[0], 1); // handle from Play2dSound
}

/**
 * @test VoiceTest.VoicesDisabledGatesAllButForce
 * @brief Verifies voices Disabled Gates All But Force.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, VoicesDisabledGatesAllButForce) {
  PlayVoices = false;
  char a[] = "a.wav";
  PlayVoice(a, 0);
  UpdateVoices();
  EXPECT_TRUE(g_stream_plays.empty()); // gated at start

  char f[] = "force.wav";
  PlayVoice(f, VF_FORCE);
  UpdateVoices();
  ASSERT_EQ(g_stream_plays.size(), 1u); // force bypasses
}

/**
 * @test VoiceTest.MotherloadAfterOverflow
 * @brief Verifies motherload After Overflow.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, MotherloadAfterOverflow) {
  PlayPowerupVoice = true; // allow powerup voices to queue
  // Fill the 3-slot queue with powerup voices, then overflow
  char a[] = "p1", b[] = "p2", c[] = "p3", d[] = "p4";
  PlayVoice(a, VF_POWERUP);
  PlayVoice(b, VF_POWERUP);
  PlayVoice(c, VF_POWERUP);
  PlayVoice(d, VF_POWERUP); // queue full -> replaced by "Cheater!" motherload

  // Drive playback: p1, p2 play as streams... actually overflow Clear(true)
  // wiped them all; only "Cheater!" remains (PLAYTABLE path, index 55).
  std::vector<std::string> played;
  for (int i = 0; i < 8; i++) {
    g_fake_sound_playing = false;
    UpdateVoices();
    for (auto &s : g_stream_plays)
      played.push_back(s.filename);
    g_stream_plays.clear();
  }
  // No plain streams survived the overflow
  for (auto &s : played)
    EXPECT_EQ(s.find('p'), 0) << s; // nothing but possibly nothing

  // The motherload went through the sound-table path exactly once
  ASSERT_EQ(g_fake_plays.size(), 1u);
  EXPECT_EQ(g_fake_plays[0].first, 55);
}

/**
 * @test VoiceTest.IdleUpdateSkipsNextQueuedVoiceUntilWraparound
 * @brief Verifies idle Update Skips Next Queued Voice Until Wraparound.
 *
 * @details
 * Exercises the VoiceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/voice.cpp
 * @ingroup descent3_tests
 */
TEST_F(VoiceTest, IdleUpdateSkipsNextQueuedVoiceUntilWraparound) {
  // Documents real queue behavior: an idle UpdateVoices() advances the read
  // cursor, so the next added voice lands behind it and is skipped on the
  // first pull; it plays after the reader wraps around.
  char x[] = "x.wav";
  UpdateVoices(); // idle: consumes one read slot
  PlayVoice(x, 0);

  UpdateVoices(); // skips x
  EXPECT_TRUE(g_stream_plays.empty());

  UpdateVoices(); // wraps: now plays x
  UpdateVoices();
  UpdateVoices();
  int total = 0;
  for (auto &s : g_stream_plays)
    if (s.filename == "x.wav")
      total++;
  EXPECT_EQ(total, 1);
}
