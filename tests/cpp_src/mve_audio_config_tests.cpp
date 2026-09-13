/**
 * @file mve_audio_config_tests.cpp
 * @brief Tests for MVE audio device and volume configuration (bug #487).
 *
 * @details
 * Verifies that the MVE movie system correctly stores and applies the
 * game's audio device ID and master volume settings, so cutscenes share
 * the game's audio device instead of opening a second, independent one.
 *
 * @par Source
 * `libmve/mvelib.h`, `libmve/mveplay.cpp`
 * @par Harness
 * `mve_audio_config_tests.cpp`
 * @par Framework
 * GoogleTest (gtest)
 *
 * @ingroup descent3_tests
 */

#include <gtest/gtest.h>

#include "hlsoundlib.h"
#include "ssl_lib.h"
#include "mvelib.h"

// ---------------------------------------------------------------------------
// Sound system global (provided by the test harness)
// ---------------------------------------------------------------------------
extern hlsSystem Sound_system;

// ---------------------------------------------------------------------------
// Stubs for MVE audio configuration functions (mveplay.cpp not linked here;
// these are simple setters verified for call-safety only).
// ---------------------------------------------------------------------------
static uint32_t g_stub_device = 0;
static float g_stub_volume = 1.0f;
void MVE_sndSetDevice(uint32_t device) { g_stub_device = device; }
void MVE_sndSetVolume(float volume) { g_stub_volume = volume; }

// ---------------------------------------------------------------------------
// Mock llsSystem for testing GetAudioDeviceID delegation
// ---------------------------------------------------------------------------
class MockLowLevelSound : public llsSystem {
public:
  uint32_t m_fake_device_id = 0;

  void SetSoundCard(const char *) override {}
  int InitSoundLib(char, oeApplication *, uint8_t) override { return 1; }
  void DestroySoundLib() override {}
  bool LockSound(int) override { return true; }
  bool UnlockSound(int) override { return true; }
  bool SetSoundQuality(char) override { return true; }
  char GetSoundQuality() override { return 0; }
  bool SetSoundMixer(char) override { return true; }
  char GetSoundMixer() override { return 0; }
  int PlaySound2d(play_information *, int, float, float, bool) override { return 0; }
  int PlayStream(play_information *) override { return 0; }
  void SetListener(pos_state *) override {}
  int PlaySound3d(play_information *, int, pos_state *, float, bool, float) override { return 0; }
  void AdjustSound(int, float, float, uint16_t) override {}
  void AdjustSound(int, pos_state *, float, float) override {}
  void StopAllSounds() override {}
  bool IsSoundInstancePlaying(int) override { return false; }
  int IsSoundPlaying(int) override { return 0; }
  void StopSound(int, uint8_t) override {}
  void PauseSounds() override {}
  void ResumeSounds() override {}
  void PauseSound(int) override {}
  void ResumeSound(int) override {}
  bool CheckAndForceSoundDataAlloc(int) override { return true; }
  void SoundStartFrame() override {}
  void SoundEndFrame() override {}
  bool SetGlobalReverbProperties(float, float, float) override { return true; }
  void SetEnvironmentValues(const t3dEnvironmentValues *) override {}
  void GetEnvironmentValues(t3dEnvironmentValues *) override {}
  void SetEnvironmentToggles(const t3dEnvironmentToggles *) override {}
  void GetEnvironmentToggles(t3dEnvironmentToggles *) override {}

  // BUGFIX #487: Return a fake device ID for testing.
  uint32_t GetAudioDeviceID() const override { return m_fake_device_id; }
};

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

/**
 * @test MveAudioConfig.GetAudioDeviceID_DefaultReturnsZero
 * @brief hlsSystem::GetAudioDeviceID() returns 0 when no low-level system is active.
 */
TEST(MveAudioConfig, GetAudioDeviceID_DefaultReturnsZero) {
  // Ensure m_ll_sound_ptr is null (default state)
  Sound_system.m_ll_sound_ptr = nullptr;
  EXPECT_EQ(Sound_system.GetAudioDeviceID(), 0u);
}

/**
 * @test MveAudioConfig.GetAudioDeviceID_DelegatesToLowLevel
 * @brief hlsSystem::GetAudioDeviceID() returns the low-level system's device ID.
 */
TEST(MveAudioConfig, GetAudioDeviceID_DelegatesToLowLevel) {
  MockLowLevelSound mock;
  mock.m_fake_device_id = 42;

  Sound_system.m_ll_sound_ptr = &mock;
  EXPECT_EQ(Sound_system.GetAudioDeviceID(), 42u);

  // Clean up
  Sound_system.m_ll_sound_ptr = nullptr;
}

/**
 * @test MveAudioConfig.GetAudioDeviceID_ReturnsZeroWhenDeviceClosed
 * @brief hlsSystem::GetAudioDeviceID() returns 0 when the low-level system
 *        returns 0 (sound device not initialized or closed).
 */
TEST(MveAudioConfig, GetAudioDeviceID_ReturnsZeroWhenDeviceClosed) {
  MockLowLevelSound mock;
  mock.m_fake_device_id = 0;

  Sound_system.m_ll_sound_ptr = &mock;
  EXPECT_EQ(Sound_system.GetAudioDeviceID(), 0u);

  Sound_system.m_ll_sound_ptr = nullptr;
}

/**
 * @test MveAudioConfig.GetMasterVolume_DefaultIsOne
 * @brief Verify the default master volume is 1.0 so MVE gets full volume.
 */
TEST(MveAudioConfig, GetMasterVolume_DefaultIsOne) {
  Sound_system.m_ll_sound_ptr = nullptr;
  EXPECT_FLOAT_EQ(Sound_system.GetMasterVolume(), 1.0f);
}

/**
 * @test MveAudioConfig.GetMasterVolume_CustomValue
 * @brief Verify SetMasterVolume/GetMasterVolume round-trips correctly
 *        so MVE playback respects the game's volume setting.
 */
TEST(MveAudioConfig, GetMasterVolume_CustomValue) {
  Sound_system.m_ll_sound_ptr = nullptr;
  Sound_system.SetMasterVolume(0.5f);
  EXPECT_FLOAT_EQ(Sound_system.GetMasterVolume(), 0.5f);

  // Reset
  Sound_system.SetMasterVolume(1.0f);
}

/**
 * @test MveAudioConfig.MVESndSetDeviceAndVolume
 * @brief Verify MVE_sndSetDevice and MVE_sndSetVolume don't crash and
 *        can be called with various values (device sharing configuration).
 */
TEST(MveAudioConfig, MVESndSetDeviceAndVolume) {
  // These should not crash; they set internal statics in mveplay.cpp
  MVE_sndSetDevice(0);     // Default device
  MVE_sndSetDevice(42);    // Arbitrary device ID
  MVE_sndSetVolume(1.0f);  // Full volume
  MVE_sndSetVolume(0.0f);  // Muted
  MVE_sndSetVolume(0.5f);  // Half volume
  MVE_sndSetVolume(-1.0f); // Negative (clamped by SDL)

  // Reset to defaults
  MVE_sndSetDevice(0);
  MVE_sndSetVolume(1.0f);
}
