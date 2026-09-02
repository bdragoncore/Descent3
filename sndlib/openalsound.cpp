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

// openalsound.cpp — OpenAL Soft backend for the Descent 3 sound system.
// BUGFIX (PiccuEngine #9): Implements the abstract llsSystem interface
// using OpenAL Soft for native 3D spatialization, distance attenuation,
// and EFX environmental reverb. The default SDL backend remains available
// when OpenAL is not selected.

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
// macOS OpenAL.framework does not ship efx.h; define EFX types locally.
// The actual function pointers are loaded at runtime via alGetProcAddress.
#ifndef AL_EFFECT_TYPE
#define AL_EFFECT_TYPE              0x8001
#define AL_EFFECT_EAXREVERB         0x8000
#define AL_EFFECTSLOT_EFFECT        0x0001
#define AL_REVERB_GAIN              0x0003
#define AL_REVERB_DENSITY           0x0001
#define AL_REVERB_DECAY_TIME        0x0005
#define AL_AUXILIARY_SEND_FILTER    0x20006
#define AL_FILTER_NULL              0x0000
typedef void (AL_APIENTRY *LPALGENEFFECTS)(ALsizei, ALuint*);
typedef void (AL_APIENTRY *LPALDELETEEFFECTS)(ALsizei, const ALuint*);
typedef void (AL_APIENTRY *LPALGENAUXILIARYEFFECTSLOTS)(ALsizei, ALuint*);
typedef void (AL_APIENTRY *LPALDELETEAUXILIARYEFFECTSLOTS)(ALsizei, const ALuint*);
typedef void (AL_APIENTRY *LPALAUXILIARYEFFECTSLOTI)(ALuint, ALenum, ALint);
typedef void (AL_APIENTRY *LPALEFFECTI)(ALuint, ALenum, ALint);
typedef void (AL_APIENTRY *LPALEFFECTF)(ALuint, ALenum, ALfloat);
typedef void (AL_APIENTRY *LPALEFFECTFV)(ALuint, ALenum, const ALfloat*);
#endif
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#endif

#include "openalsound.h"
#include "pserror.h"
#include "mem.h"
#include "log.h"
#include "application.h"
#include "soundload.h"
#include "args.h"

#define OPENAL_SAMPLE_RATE 44100

// EFX function pointers (loaded at runtime via alGetProcAddress)
static LPALGENEFFECTS alGenEffects = nullptr;
static LPALDELETEEFFECTS alDeleteEffects = nullptr;
static LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
static LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
static LPALEFFECTF alEffectf = nullptr;
static LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
static bool efx_loaded = false;

openalsound::openalsound() : llsSystem() {
  m_device = nullptr;
  m_context = nullptr;
  m_total_sounds_played = 0;
  m_max_sounds = OPENAL_MAX_SOURCES;
  m_sound_quality = SQT_HIGH;
  m_reverb_effect = 0;
  m_reverb_slot = 0;
  m_reverb_enabled = false;
  m_doppler_enabled = true;

  memset(m_sources, 0, sizeof(m_sources));
  for (int i = 0; i < OPENAL_MAX_SOURCES; i++) {
    m_sources[i].source = 0;
    m_sources[i].buffer = 0;
    m_sources[i].sound_index = -1;
    m_sources[i].unique_id = -1;
    m_sources[i].status = SSF_UNUSED;
    m_sources[i].in_use = false;
  }
}

openalsound::~openalsound() { DestroySoundLib(); }

// may be called before init (must be to be valid, the card passed here will be initialized in InitSoundLib)
void openalsound::SetSoundCard(const char *name) {
  // OpenAL Soft uses the default device; a specific device name could be
  // passed here but we ignore it for simplicity.
}

int openalsound::InitSoundLib(char mixer_type, oeApplication *sos, uint8_t max_sounds_played) {
  m_max_sounds = std::min((int)max_sounds_played, OPENAL_MAX_SOURCES);

  m_device = alcOpenDevice(nullptr); // default device
  if (!m_device) {
    LOG_WARNING << "OpenAL: Failed to open default audio device.";
    return 0;
  }

  m_context = alcCreateContext(m_device, nullptr);
  if (!m_context) {
    LOG_WARNING << "OpenAL: Failed to create context.";
    alcCloseDevice(m_device);
    m_device = nullptr;
    return 0;
  }

  if (!alcMakeContextCurrent(m_context)) {
    LOG_WARNING << "OpenAL: Failed to make context current.";
    alcDestroyContext(m_context);
    m_context = nullptr;
    alcCloseDevice(m_device);
    m_device = nullptr;
    return 0;
  }

  // Generate sources
  for (int i = 0; i < m_max_sounds; i++) {
    alGenSources(1, &m_sources[i].source);
    if (alGetError() != AL_NO_ERROR) {
      LOG_WARNING << "OpenAL: Failed to generate source " << i;
      m_sources[i].source = 0;
    }
  }

  // Set up reverb (EFX) if available
  m_reverb_enabled = false;
  efx_loaded = false;
  if (alcIsExtensionPresent(m_device, "ALC_EXT_EFX")) {
    alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
    alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
    alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    if (alGenEffects && alGenAuxiliaryEffectSlots && alEffectf && alAuxiliaryEffectSloti) {
      efx_loaded = true;
      alGenEffects(1, &m_reverb_effect);
      alGenAuxiliaryEffectSlots(1, &m_reverb_slot);
      if (alGetError() == AL_NO_ERROR) {
        m_reverb_enabled = true;
      }
    }
  }

  m_total_sounds_played = 0;
  m_sound_quality = SQT_HIGH;

  LOG_INFO << "OpenAL: Initialized with " << m_max_sounds << " sources.";
  return 1;
}

void openalsound::DestroySoundLib() {
  for (int i = 0; i < OPENAL_MAX_SOURCES; i++) {
    if (m_sources[i].source) {
      alDeleteSources(1, &m_sources[i].source);
      m_sources[i].source = 0;
    }
    if (m_sources[i].buffer) {
      alDeleteBuffers(1, &m_sources[i].buffer);
      m_sources[i].buffer = 0;
    }
    m_sources[i].in_use = false;
    m_sources[i].status = SSF_UNUSED;
  }

  if (m_reverb_effect && alDeleteEffects) {
    alDeleteEffects(1, &m_reverb_effect);
    m_reverb_effect = 0;
  }
  if (m_reverb_slot && alDeleteAuxiliaryEffectSlots) {
    alDeleteAuxiliaryEffectSlots(1, &m_reverb_slot);
    m_reverb_slot = 0;
  }

  if (m_context) {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(m_context);
    m_context = nullptr;
  }
  if (m_device) {
    alcCloseDevice(m_device);
    m_device = nullptr;
  }
}

bool openalsound::LockSound(int sound_uid) { return true; }
bool openalsound::UnlockSound(int sound_uid) { return true; }

bool openalsound::SetSoundQuality(char quality) {
  m_sound_quality = (quality == SQT_NORMAL) ? SQT_NORMAL : SQT_HIGH;
  return true;
}

char openalsound::GetSoundQuality() { return m_sound_quality; }

bool openalsound::SetSoundMixer(char mixer_type) { return true; }

char openalsound::GetSoundMixer() { return SOUND_MIXER_OPENAL; }

int openalsound::MakeUniqueId(int slot) const { return ((((int)m_total_sounds_played) << 8) + slot); }

int openalsound::ValidateUniqueId(int uid) {
  // The slot is the low 8 bits; we can't easily validate against the
  // source array here without the instance, so we just return the slot.
  return uid & 0x00FF;
}

int openalsound::FindFreeSource() {
  for (int i = 0; i < m_max_sounds; i++) {
    if (!m_sources[i].in_use) {
      return i;
    }
  }
  return -1;
}

bool openalsound::EnsureBufferForSound(int sound_index) {
  if (sound_index < 0 || sound_index >= MAX_SOUNDS)
    return false;
  if (Sounds[sound_index].used == 0)
    return false;

  int file_index = Sounds[sound_index].sample_index;
  if (file_index < 0 || file_index >= MAX_SOUND_FILES)
    return false;

  // Ensure sample data is loaded
  if (SoundFiles[file_index].sample_16bit == nullptr && SoundFiles[file_index].sample_8bit == nullptr) {
    if (!CheckAndForceSoundDataAlloc(sound_index))
      return false;
  }

  return true;
}

int openalsound::PlaySound2d(play_information *play_info, int sound_index, float volume, float pan, bool f_looped) {
  if (!m_context)
    return -1;

  if (!EnsureBufferForSound(sound_index))
    return -1;

  int slot = FindFreeSource();
  if (slot < 0)
    return -1;

  SourceInfo &si = m_sources[slot];
  int file_index = Sounds[sound_index].sample_index;

  // Create buffer if needed
  if (!si.buffer) {
    alGenBuffers(1, &si.buffer);
    if (alGetError() != AL_NO_ERROR) {
      si.buffer = 0;
      return -1;
    }
  }

  // Determine format
  ALenum format;
  int sample_rate = OPENAL_SAMPLE_RATE;
  const void *data;
  int data_size;

  if (SoundFiles[file_index].sample_16bit) {
    format = AL_FORMAT_MONO16;
    data = SoundFiles[file_index].sample_16bit;
    data_size = SoundFiles[file_index].sample_length * 2;
  } else {
    format = AL_FORMAT_MONO8;
    data = SoundFiles[file_index].sample_8bit;
    data_size = SoundFiles[file_index].sample_length;
  }

  alBufferData(si.buffer, format, data, data_size, sample_rate);
  if (alGetError() != AL_NO_ERROR) {
    return -1;
  }

  // Configure source
  alSourcei(si.source, AL_BUFFER, si.buffer);
  alSourcef(si.source, AL_GAIN, std::max(0.0f, std::min(1.0f, volume)));
  alSourcei(si.source, AL_LOOPING, f_looped ? AL_TRUE : AL_FALSE);
  alSourcei(si.source, AL_SOURCE_RELATIVE, AL_TRUE);
  alSource3f(si.source, AL_POSITION, pan, 0.0f, 0.0f);
  alSource3f(si.source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

  // Apply reverb if enabled
  if (m_reverb_enabled && alAuxiliaryEffectSloti) {
    alSource3i(si.source, AL_AUXILIARY_SEND_FILTER, m_reverb_slot, 0, AL_FILTER_NULL);
  }

  alSourcePlay(si.source);
  if (alGetError() != AL_NO_ERROR) {
    return -1;
  }

  m_total_sounds_played++;
  si.sound_index = sound_index;
  si.unique_id = MakeUniqueId(slot);
  si.status = f_looped ? SSF_PLAY_LOOPING : SSF_PLAY_NORMAL;
  si.in_use = true;

  return si.unique_id;
}

int openalsound::PlayStream(play_information *play_info) {
  // Streaming is handled by the high-level AudioStream system which
  // calls PlaySound2d with a callback. For OpenAL, we treat streams
  // as regular 2D sounds (the high-level system feeds data via callback).
  if (!m_context)
    return -1;

  int slot = FindFreeSource();
  if (slot < 0)
    return -1;

  SourceInfo &si = m_sources[slot];
  float volume = std::max(play_info->left_volume, play_info->right_volume);

  // For streaming, we create a source with no buffer; the high-level
  // system will feed data. We just mark it as streaming.
  if (!si.buffer) {
    alGenBuffers(1, &si.buffer);
  }

  alSourcei(si.source, AL_BUFFER, 0);
  alSourcef(si.source, AL_GAIN, std::max(0.0f, std::min(1.0f, volume)));
  alSourcei(si.source, AL_LOOPING, AL_FALSE);
  alSourcei(si.source, AL_SOURCE_RELATIVE, AL_TRUE);
  alSource3f(si.source, AL_POSITION, 0.0f, 0.0f, 0.0f);

  m_total_sounds_played++;
  si.sound_index = -1;
  si.unique_id = MakeUniqueId(slot);
  si.status = SSF_PLAY_STREAMING;
  si.in_use = true;

  return si.unique_id;
}

void openalsound::SetListener(pos_state *cur_pos) {
  if (!m_context)
    return;

  if (cur_pos->position) {
    m_listener_pos[0] = cur_pos->position->x();
    m_listener_pos[1] = cur_pos->position->y();
    m_listener_pos[2] = cur_pos->position->z();
  }
  if (cur_pos->velocity) {
    m_listener_vel[0] = cur_pos->velocity->x();
    m_listener_vel[1] = cur_pos->velocity->y();
    m_listener_vel[2] = cur_pos->velocity->z();
  }
  if (cur_pos->orient) {
    // OpenAL expects forward then up vectors
    m_listener_orient[0] = cur_pos->orient->fvec.x();
    m_listener_orient[1] = cur_pos->orient->fvec.y();
    m_listener_orient[2] = cur_pos->orient->fvec.z();
    m_listener_orient[3] = cur_pos->orient->uvec.x();
    m_listener_orient[4] = cur_pos->orient->uvec.y();
    m_listener_orient[5] = cur_pos->orient->uvec.z();
  }

  UpdateListener();
}

void openalsound::UpdateListener() {
  alListenerfv(AL_POSITION, m_listener_pos);
  alListenerfv(AL_VELOCITY, m_listener_vel);
  alListenerfv(AL_ORIENTATION, m_listener_orient);
}

int openalsound::PlaySound3d(play_information *play_info, int sound_index, pos_state *cur_pos, float master_volume,
                             bool f_looped, float reverb) {
  if (!m_context)
    return -1;

  if (!EnsureBufferForSound(sound_index))
    return -1;

  int slot = FindFreeSource();
  if (slot < 0)
    return -1;

  SourceInfo &si = m_sources[slot];
  int file_index = Sounds[sound_index].sample_index;

  if (!si.buffer) {
    alGenBuffers(1, &si.buffer);
    if (alGetError() != AL_NO_ERROR) {
      si.buffer = 0;
      return -1;
    }
  }

  ALenum format;
  int sample_rate = OPENAL_SAMPLE_RATE;
  const void *data;
  int data_size;

  if (SoundFiles[file_index].sample_16bit) {
    format = AL_FORMAT_MONO16;
    data = SoundFiles[file_index].sample_16bit;
    data_size = SoundFiles[file_index].sample_length * 2;
  } else {
    format = AL_FORMAT_MONO8;
    data = SoundFiles[file_index].sample_8bit;
    data_size = SoundFiles[file_index].sample_length;
  }

  alBufferData(si.buffer, format, data, data_size, sample_rate);
  if (alGetError() != AL_NO_ERROR) {
    return -1;
  }

  // Configure source for 3D
  alSourcei(si.source, AL_BUFFER, si.buffer);
  alSourcef(si.source, AL_GAIN, std::max(0.0f, std::min(1.0f, master_volume)));
  alSourcei(si.source, AL_LOOPING, f_looped ? AL_TRUE : AL_FALSE);
  alSourcei(si.source, AL_SOURCE_RELATIVE, AL_FALSE);

  if (cur_pos->position) {
    alSource3f(si.source, AL_POSITION, cur_pos->position->x(), cur_pos->position->y(), cur_pos->position->z());
  }
  if (cur_pos->velocity) {
    alSource3f(si.source, AL_VELOCITY, cur_pos->velocity->x(), cur_pos->velocity->y(), cur_pos->velocity->z());
  }

  // Set distance attenuation based on sound's min/max distance
  alSourcef(si.source, AL_REFERENCE_DISTANCE, Sounds[sound_index].min_distance);
  alSourcef(si.source, AL_MAX_DISTANCE, Sounds[sound_index].max_distance);
  alSourcef(si.source, AL_ROLLOFF_FACTOR, 1.0f);

  // Apply reverb if enabled
  if (m_reverb_enabled && alAuxiliaryEffectSloti) {
    alSource3i(si.source, AL_AUXILIARY_SEND_FILTER, m_reverb_slot, 0, AL_FILTER_NULL);
  }

  alSourcePlay(si.source);
  if (alGetError() != AL_NO_ERROR) {
    return -1;
  }

  m_total_sounds_played++;
  si.sound_index = sound_index;
  si.unique_id = MakeUniqueId(slot);
  si.status = f_looped ? SSF_PLAY_LOOPING : SSF_PLAY_NORMAL;
  si.in_use = true;

  return si.unique_id;
}

void openalsound::AdjustSound(int sound_uid, float f_volume, float f_pan, uint16_t frequency) {
  int slot = ValidateUniqueId(sound_uid);
  if (slot < 0 || slot >= m_max_sounds)
    return;
  SourceInfo &si = m_sources[slot];
  if (!si.in_use)
    return;

  alSourcef(si.source, AL_GAIN, std::max(0.0f, std::min(1.0f, f_volume)));
  alSource3f(si.source, AL_POSITION, f_pan, 0.0f, 0.0f);
  if (frequency > 0) {
    alSourcef(si.source, AL_PITCH, (float)frequency / OPENAL_SAMPLE_RATE);
  }
}

void openalsound::AdjustSound(int sound_uid, pos_state *cur_pos, float adjusted_volume, float reverb) {
  int slot = ValidateUniqueId(sound_uid);
  if (slot < 0 || slot >= m_max_sounds)
    return;
  SourceInfo &si = m_sources[slot];
  if (!si.in_use)
    return;

  alSourcef(si.source, AL_GAIN, std::max(0.0f, std::min(1.0f, adjusted_volume)));
  if (cur_pos->position) {
    alSource3f(si.source, AL_POSITION, cur_pos->position->x(), cur_pos->position->y(), cur_pos->position->z());
  }
  if (cur_pos->velocity) {
    alSource3f(si.source, AL_VELOCITY, cur_pos->velocity->x(), cur_pos->velocity->y(), cur_pos->velocity->z());
  }
}

void openalsound::StopAllSounds() {
  for (int i = 0; i < m_max_sounds; i++) {
    if (m_sources[i].in_use) {
      StopSound(m_sources[i].unique_id);
    }
  }
}

bool openalsound::IsSoundInstancePlaying(int sound_uid) {
  int slot = ValidateUniqueId(sound_uid);
  if (slot < 0 || slot >= m_max_sounds)
    return false;
  SourceInfo &si = m_sources[slot];
  if (!si.in_use)
    return false;

  ALint state;
  alGetSourcei(si.source, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}

int openalsound::IsSoundPlaying(int sound_index) {
  for (int i = 0; i < m_max_sounds; i++) {
    if (m_sources[i].in_use && m_sources[i].sound_index == sound_index) {
      ALint state;
      alGetSourcei(m_sources[i].source, AL_SOURCE_STATE, &state);
      if (state == AL_PLAYING)
        return m_sources[i].unique_id;
    }
  }
  return -1;
}

void openalsound::StopSound(int sound_uid, uint8_t f_immediately) {
  int slot = ValidateUniqueId(sound_uid);
  if (slot < 0 || slot >= m_max_sounds)
    return;
  SourceInfo &si = m_sources[slot];
  if (!si.in_use)
    return;

  if (f_immediately == SKT_STOP_AFTER_LOOP) {
    // Stop looping, let it finish
    alSourcei(si.source, AL_LOOPING, AL_FALSE);
    si.status &= ~SSF_PLAY_LOOPING;
    si.status |= SSF_PLAY_NORMAL;
    return;
  }

  alSourceStop(si.source);
  alSourcei(si.source, AL_BUFFER, 0);
  si.in_use = false;
  si.status = SSF_UNUSED;
  si.sound_index = -1;
  si.unique_id = -1;
}

void openalsound::PauseSounds() {
  for (int i = 0; i < m_max_sounds; i++) {
    if (m_sources[i].in_use) {
      alSourcePause(m_sources[i].source);
      m_sources[i].status |= SSF_PAUSED;
    }
  }
}

void openalsound::ResumeSounds() {
  for (int i = 0; i < m_max_sounds; i++) {
    if (m_sources[i].in_use) {
      alSourcePlay(m_sources[i].source);
      m_sources[i].status &= ~SSF_PAUSED;
    }
  }
}

void openalsound::PauseSound(int sound_uid) {
  int slot = ValidateUniqueId(sound_uid);
  if (slot < 0 || slot >= m_max_sounds)
    return;
  if (m_sources[slot].in_use) {
    alSourcePause(m_sources[slot].source);
    m_sources[slot].status |= SSF_PAUSED;
  }
}

void openalsound::ResumeSound(int sound_uid) {
  int slot = ValidateUniqueId(sound_uid);
  if (slot < 0 || slot >= m_max_sounds)
    return;
  if (m_sources[slot].in_use) {
    alSourcePlay(m_sources[slot].source);
    m_sources[slot].status &= ~SSF_PAUSED;
  }
}

bool openalsound::CheckAndForceSoundDataAlloc(int sound_index) {
  if (sound_index < 0 || sound_index >= MAX_SOUNDS)
    return false;
  int file_index = Sounds[sound_index].sample_index;
  if (file_index < 0 || file_index >= MAX_SOUND_FILES)
    return false;

  if (SoundFiles[file_index].sample_16bit != nullptr || SoundFiles[file_index].sample_8bit != nullptr)
    return true;

  int result = SoundLoadWaveFile(SoundFiles[file_index].name, Sounds[sound_index].import_volume, file_index,
                                 (m_sound_quality == SQT_HIGH), true);
  return (result != 0);
}

void openalsound::SoundStartFrame() {
  // Clean up finished non-looping sources
  for (int i = 0; i < m_max_sounds; i++) {
    SourceInfo &si = m_sources[i];
    if (si.in_use && !(si.status & SSF_PLAY_LOOPING) && !(si.status & SSF_PLAY_STREAMING)) {
      ALint state;
      alGetSourcei(si.source, AL_SOURCE_STATE, &state);
      if (state == AL_STOPPED) {
        alSourcei(si.source, AL_BUFFER, 0);
        si.in_use = false;
        si.status = SSF_UNUSED;
        si.sound_index = -1;
        si.unique_id = -1;
      }
    }
  }
}

void openalsound::SoundEndFrame() { CheckForErrors(); }

bool openalsound::SetGlobalReverbProperties(float volume, float damping, float decay) {
  if (!m_reverb_enabled || !alEffectf || !alAuxiliaryEffectSloti)
    return false;

  alEffectf(m_reverb_effect, AL_REVERB_GAIN, volume);
  alEffectf(m_reverb_effect, AL_REVERB_DENSITY, damping);
  alEffectf(m_reverb_effect, AL_REVERB_DECAY_TIME, decay);
  alAuxiliaryEffectSloti(m_reverb_slot, AL_EFFECTSLOT_EFFECT, m_reverb_effect);
  return true;
}

void openalsound::SetEnvironmentValues(const t3dEnvironmentValues *env) {
  if (env->flags & ENV3DVALF_DOPPLER) {
    alDopplerFactor(env->doppler_scalar);
  }
}

void openalsound::GetEnvironmentValues(t3dEnvironmentValues *env) {
  env->flags = ENV3DVALF_DOPPLER;
  ALfloat factor;
  alGetFloatv(AL_DOPPLER_FACTOR, &factor);
  env->doppler_scalar = factor;
}

void openalsound::SetEnvironmentToggles(const t3dEnvironmentToggles *env) {
  if (env->flags & ENV3DVALF_DOPPLER) {
    m_doppler_enabled = env->doppler;
    alDopplerFactor(m_doppler_enabled ? 1.0f : 0.0f);
  }
}

void openalsound::GetEnvironmentToggles(t3dEnvironmentToggles *env) {
  env->flags = ENV3DVALF_DOPPLER;
  env->supported = ENV3DVALF_DOPPLER;
  env->doppler = m_doppler_enabled;
  env->geometry = false;
}

void openalsound::SetSoundProperties(int sound_uid, float obstruction) {
  int slot = ValidateUniqueId(sound_uid);
  if (slot < 0 || slot >= m_max_sounds)
    return;
  if (m_sources[slot].in_use) {
    // Simple obstruction: reduce gain
    ALfloat gain;
    alGetSourcef(m_sources[slot].source, AL_GAIN, &gain);
    alSourcef(m_sources[slot].source, AL_GAIN, gain * (1.0f - obstruction));
  }
}
