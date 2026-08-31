/**
 * @file fake_hlsystem.cpp
 * @brief Fake hlsSystem member definitions for weather tests.
 *
 * @details
 * The real sndlib/hlsoundlib.cpp is NOT linked; we supply the minimal set of
 * member functions referenced by Descent3/weather.cpp (Play2dSound) and by
 * the inline destructor (KillSoundLib), plus the constructor.
 *
 * This harness validates the behavior of `sndlib/hlsoundlib.h (stub)`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `sndlib/hlsoundlib.h (stub)`
 * @par Harness
 * `fake_hlsystem.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see sndlib/hlsoundlib.h (stub)
 */

#include "hlsoundlib.h"
#include <vector>

struct FakePlayRec {
  int index;
  float volume;
};
std::vector<FakePlayRec> g_fake_plays;

// The global sound system instance (normally defined in sndlib/hlsoundlib.cpp)
hlsSystem Sound_system;

hlsSystem::hlsSystem() {
  m_ll_sound_ptr = nullptr;
  m_master_volume = 1.0f;
  m_sounds_played = 0;
  m_pause_new = false;
  m_cur_environment = 0;
  n_lls_sounds = 0;
}

void hlsSystem::KillSoundLib(bool) {}

int hlsSystem::Play2dSound(int sound_index, float volume, float, uint16_t) {
  g_fake_plays.push_back({sound_index, volume});
  return 0;
}

int hlsSystem::Play2dSound(int, int, float, float, unsigned short) { return 0; }

int hlsSystem::Play3dSound(int, pos_state *, float, int, float) { return 0; }
int hlsSystem::Play3dSound(int, object *, float, int, float) { return 0; }
int hlsSystem::Play3dSound(int, int, pos_state *, float, int, float) { return 0; }
void hlsSystem::StopSoundImmediate(int) {}
int hlsSystem::Play3dSound(int, int, object *, float, int, float) { return 0; }
