/*
 * Descent 3
 * Copyright (C) 2024 Descent Developers
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

#include "movie_sound.h"

namespace D3 {

MovieSoundDevice::MovieSoundDevice(int sample_rate, uint16_t sample_size, uint8_t channels, bool is_compressed,
                                   uint32_t device_id, float volume) {
  SDL_AudioSpec spec{};
  spec.freq = sample_rate;
  spec.format = (sample_size == 2) ? SDL_AUDIO_S16LE : SDL_AUDIO_U8;
  spec.channels = channels;

  // BUGFIX #487: Use the game's audio device (passed in) instead of always
  // opening the SDL default. This unifies MVE cutscene audio with the game's
  // sound device so cutscenes respect the game's volume and -nosound settings.
  SDL_AudioDeviceID dev = (device_id != 0) ? device_id : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
  this->stream = SDL_OpenAudioDeviceStream(dev, &spec, nullptr, nullptr);
  this->m_is_compressed = is_compressed;
  this->m_sample_size = sample_size;

  // BUGFIX #487: Apply the game's master volume to the MVE audio stream.
  if (this->stream != nullptr && volume != 1.0f) {
    SDL_SetAudioStreamGain(this->stream, volume);
  }
};

MovieSoundDevice::~MovieSoundDevice() {
  if (this->stream != nullptr) {
    // BUGFIX #487: Destroy the stream (unbinding it from the device) rather
    // than closing the device, so a shared game audio device stays open.
    SDL_DestroyAudioStream(this->stream);
  }
}

void MovieSoundDevice::FillBuffer(char *buffer, int len) const { SDL_PutAudioStreamData(this->stream, buffer, len); };

void MovieSoundDevice::Play() { SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(this->stream)); }

void MovieSoundDevice::Stop() { SDL_PauseAudioDevice(SDL_GetAudioStreamDevice(this->stream)); }

void MovieSoundDevice::Lock() {}

void MovieSoundDevice::Unlock() {}

} // namespace D3
