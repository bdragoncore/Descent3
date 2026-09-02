/*
 * Descent 3
 * Copyright (C) 2024 Parallax Software, 2026 InsanityBringer (PiccuEngine)
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

// openalsound.h — OpenAL Soft backend for the Descent 3 sound system.
// BUGFIX (PiccuEngine #9): Implements llsSystem via OpenAL Soft, giving
// the user a choice between the existing SDL/software mixer and a native
// OpenAL backend with hardware-accelerated 3D sound and EFX reverb.

#ifndef OPENALSOUND_H
#define OPENALSOUND_H

#include "ssl_lib.h"
#include <AL/al.h>
#include <AL/alc.h>

#define OPENAL_MAX_SOURCES 256

class openalsound : public llsSystem {
public:
  openalsound();
  ~openalsound() override;

  void SetSoundCard(const char *name) override;
  int InitSoundLib(char mixer_type, oeApplication *sos, uint8_t max_sounds_played) override;
  void DestroySoundLib() override;

  bool LockSound(int sound_uid) override;
  bool UnlockSound(int sound_uid) override;

  bool SetSoundQuality(char quality) override;
  char GetSoundQuality() override;
  bool SetSoundMixer(char mixer_type) override;
  char GetSoundMixer() override;

  int PlaySound2d(play_information *play_info, int sound_index, float volume, float pan, bool f_looped) override;
  int PlayStream(play_information *play_info) override;

  void SetListener(pos_state *cur_pos) override;
  int PlaySound3d(play_information *play_info, int sound_index, pos_state *cur_pos, float master_volume,
                  bool f_looped, float reverb = 0.5f) override;
  void AdjustSound(int sound_uid, float f_volume, float f_pan, uint16_t frequency) override;
  void AdjustSound(int sound_uid, pos_state *cur_pos, float adjusted_volume, float reverb = 0.5f) override;

  void StopAllSounds() override;

  bool IsSoundInstancePlaying(int sound_uid) override;
  int IsSoundPlaying(int sound_index) override;

  void StopSound(int sound_uid, uint8_t f_immediately = SKT_STOP_IMMEDIATELY) override;

  void PauseSounds() override;
  void ResumeSounds() override;
  void PauseSound(int sound_uid) override;
  void ResumeSound(int sound_uid) override;

  bool CheckAndForceSoundDataAlloc(int sound_file_index) override;

  void SoundStartFrame() override;
  void SoundEndFrame() override;

  bool SetGlobalReverbProperties(float volume, float damping, float decay) override;

  void SetEnvironmentValues(const t3dEnvironmentValues *env) override;
  void GetEnvironmentValues(t3dEnvironmentValues *env) override;
  void SetEnvironmentToggles(const t3dEnvironmentToggles *env) override;
  void GetEnvironmentToggles(t3dEnvironmentToggles *env) override;

  bool SoundPropertySupport() const override { return true; }
  void SetSoundProperties(int sound_uid, float obstruction) override;

private:
  struct SourceInfo {
    ALuint source;
    ALuint buffer;
    int sound_index;
    int unique_id;
    uint8_t status; // SSF_* flags
    bool in_use;
  };

  void UpdateListener();
  bool EnsureBufferForSound(int sound_index);
  void FreeSource(SourceInfo &si);
  int FindFreeSource();
  int MakeUniqueId(int slot) const;
  static int ValidateUniqueId(int uid);

  ALCdevice *m_device;
  ALCcontext *m_context;
  SourceInfo m_sources[OPENAL_MAX_SOURCES];
  int m_total_sounds_played;
  int m_max_sounds;
  char m_sound_quality;

  // Listener state
  ALfloat m_listener_pos[3];
  ALfloat m_listener_vel[3];
  ALfloat m_listener_orient[6]; // forward + up

  // Reverb
  ALuint m_reverb_effect;
  ALuint m_reverb_slot;
  bool m_reverb_enabled;

  // Environment
  bool m_doppler_enabled;
};

#endif
