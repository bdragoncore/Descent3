/*
* Descent 3
* Copyright (C) 2024 Parallax Software
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

#include "theora_movie.h"

#include <cstring>

#include "log.h"

// BUGFIX #685: The main menu background (mainmenu.ogv) is an Ogg Theora
// video that the original D3D renderer played but the OpenGL path skipped.
// The MVE decoder cannot read .ogv files, so this module decodes them with
// libtheora and feeds the frames into the existing movie bitmap pipeline.

// Pulls the next packet from the Theora stream, reading more pages from the
// file as needed. Returns true on success, false on EOF/error.
static bool TheoraNextPacket(TheoraMovie *tm, ogg_packet *op) {
  while (true) {
    int ret = ogg_stream_packetout(&tm->vo, op);
    if (ret > 0) {
      return true;
    }
    if (ret < 0) {
      LOG_ERROR << "Theora: stream error";
      return false;
    }

    // No packet available; read more data from the file.
    char *buffer = ogg_sync_buffer(&tm->oy, 4096);
    int bytes = (int)fread(buffer, 1, 4096, tm->file);
    ogg_sync_wrote(&tm->oy, bytes);

    int page_ret = ogg_sync_pageout(&tm->oy, &tm->og);
    if (page_ret == 1) {
      ogg_stream_pagein(&tm->vo, &tm->og);
    } else if (page_ret == 0) {
      if (bytes < 4096) {
        return false; // EOF
      }
    } else {
      LOG_ERROR << "Theora: sync error";
      return false;
    }
  }
}

// Reads the comment and setup headers (the identification header was already
// parsed in TheoraInitDecoder). The first data packet is left in the stream
// queue for TheoraDecodeFrame to consume.
static bool TheoraReadHeaders(TheoraMovie *tm) {
  // The Theora stream has three headers: identification, comment, setup.
  // th_decode_headerin() returns 3, 2, 1 for those (non-zero = success).
  int header_count = 1; // identification header already parsed

  while (header_count < 3) {
    ogg_packet op;
    if (!TheoraNextPacket(tm, &op)) {
      LOG_ERROR << "Theora: unexpected EOF while reading headers";
      return false;
    }
    if (th_decode_headerin(&tm->ti, &tm->tc, &tm->ts, &op) <= 0) {
      LOG_ERROR << "Theora: bad header packet";
      return false;
    }
    ++header_count;
  }

  return true;
}

// Initializes the Ogg/Theora decoder state for an already-open file.
// Returns false on failure (caller must clean up).
static bool TheoraInitDecoder(TheoraMovie *tm) {
  ogg_sync_init(&tm->oy);
  th_comment_init(&tm->tc);
  th_info_init(&tm->ti);
  tm->video_stream = -1;
  tm->stream_initialized = false;

  // Find the Theora BOS (beginning of stream) page.
  while (!tm->stream_initialized) {
    char *buffer = ogg_sync_buffer(&tm->oy, 4096);
    int bytes = (int)fread(buffer, 1, 4096, tm->file);
    ogg_sync_wrote(&tm->oy, bytes);

    if (ogg_sync_pageout(&tm->oy, &tm->og) != 1) {
      if (bytes < 4096) {
        LOG_ERROR << "Theora: no Theora stream found";
        return false;
      }
      continue;
    }

    if (ogg_page_bos(&tm->og)) {
      ogg_stream_state test;
      ogg_stream_init(&test, ogg_page_serialno(&tm->og));
      ogg_stream_pagein(&test, &tm->og);

      ogg_packet op;
      if (ogg_stream_packetout(&test, &op) > 0 && op.bytes >= 7 &&
          memcmp(op.packet, "\200theora", 7) == 0) {
        tm->video_stream = ogg_page_serialno(&tm->og);
        ogg_stream_init(&tm->vo, tm->video_stream);
        tm->stream_initialized = true;
        ogg_stream_pagein(&tm->vo, &tm->og);

        // Consume the identification header from vo's queue and parse it.
        // (The BOS page holds all three headers plus the first data packet;
        // the remaining packets stay queued for TheoraReadHeaders and the
        // decode loop.)
        ogg_packet ident;
        if (ogg_stream_packetout(&tm->vo, &ident) <= 0 ||
            th_decode_headerin(&tm->ti, &tm->tc, &tm->ts, &ident) != 3) {
          LOG_ERROR << "Theora: failed to parse identification header";
          ogg_stream_clear(&test);
          return false;
        }
      }
      ogg_stream_clear(&test);
    }
  }

  if (!TheoraReadHeaders(tm)) {
    return false;
  }

  tm->td = th_decode_alloc(&tm->ti, tm->ts);
  th_setup_free(tm->ts);
  tm->ts = nullptr;

  if (tm->td == nullptr) {
    LOG_ERROR << "Theora: failed to allocate decoder";
    return false;
  }

  tm->width = tm->ti.frame_width;
  tm->height = tm->ti.frame_height;

  return true;
}

// Frees all decoder state but keeps the TheoraMovie struct itself so it
// can be re-initialized (used for looping).
static void TheoraResetDecoder(TheoraMovie *tm) {
  if (tm->td != nullptr) {
    th_decode_free(tm->td);
    tm->td = nullptr;
  }
  if (tm->ts != nullptr) {
    th_setup_free(tm->ts);
    tm->ts = nullptr;
  }
  th_comment_clear(&tm->tc);
  th_info_clear(&tm->ti);
  if (tm->stream_initialized) {
    ogg_stream_clear(&tm->vo);
    tm->stream_initialized = false;
  }
  ogg_sync_clear(&tm->oy);
  if (tm->file != nullptr) {
    fclose(tm->file);
    tm->file = nullptr;
  }
}

TheoraMovie *TheoraOpen(const char *filename, bool looping) {
  TheoraMovie *tm = new TheoraMovie;
  memset(tm, 0, sizeof(*tm));
  tm->looping = looping;
  strncpy(tm->filename, filename, sizeof(tm->filename) - 1);
  tm->filename[sizeof(tm->filename) - 1] = 0;

  tm->file = fopen(filename, "rb");
  if (tm->file == nullptr) {
    LOG_ERROR << "Theora: unable to open " << filename;
    delete tm;
    return nullptr;
  }

  if (!TheoraInitDecoder(tm)) {
    TheoraClose(tm);
    return nullptr;
  }

  return tm;
}

int TheoraDecodeFrame(TheoraMovie *tm) {
  if (tm->eof) {
    return 1;
  }

  while (true) {
    ogg_packet op;
    if (!TheoraNextPacket(tm, &op)) {
      // EOF reached.
      if (tm->looping) {
        // Reopen the file and re-initialize the decoder to loop.
        TheoraResetDecoder(tm);
        tm->file = fopen(tm->filename, "rb");
        if (tm->file != nullptr && TheoraInitDecoder(tm)) {
          continue;
        }
      }
      tm->eof = true;
      return 1;
    }

    if (th_decode_packetin(tm->td, &op, &tm->granulepos) >= 0) {
      th_decode_ycbcr_out(tm->td, tm->ycbcr);
      return 0;
    }
  }
}

void TheoraToRGB555(const TheoraMovie *tm, uint16_t *pixels, int pitch) {
  const int width = tm->width;
  const int height = tm->height;

  const uint8_t *y_plane = tm->ycbcr[0].data;
  const uint8_t *cb_plane = tm->ycbcr[1].data;
  const uint8_t *cr_plane = tm->ycbcr[2].data;
  const int y_stride = tm->ycbcr[0].stride;
  const int cb_stride = tm->ycbcr[1].stride;
  const int cr_stride = tm->ycbcr[2].stride;

  for (int y = 0; y < height; ++y) {
    uint16_t *row = pixels + (size_t)y * pitch;
    for (int x = 0; x < width; ++x) {
      int Y = y_plane[y * y_stride + x];
      int Cb = cb_plane[(y >> 1) * cb_stride + (x >> 1)] - 128;
      int Cr = cr_plane[(y >> 1) * cr_stride + (x >> 1)] - 128;

      int R = Y + ((91881 * Cr) >> 16);
      int G = Y - ((22554 * Cb + 46802 * Cr) >> 16);
      int B = Y + ((116130 * Cb) >> 16);

      R = R < 0 ? 0 : (R > 255 ? 255 : R);
      G = G < 0 ? 0 : (G > 255 ? 255 : G);
      B = B < 0 ? 0 : (B > 255 ? 255 : B);

      row[x] = 0x8000 | ((uint16_t)(R >> 3) << 10) | ((uint16_t)(G >> 3) << 5) | (uint16_t)(B >> 3);
    }
  }
}

void TheoraClose(TheoraMovie *tm) {
  if (tm == nullptr) {
    return;
  }

  TheoraResetDecoder(tm);
  delete tm;
}