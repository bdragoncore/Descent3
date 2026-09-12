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

#ifndef THEORA_MOVIE_H
#define THEORA_MOVIE_H

#include <cstdio>
#include <cstdint>

#ifdef HAVE_THEORA
#include <ogg/ogg.h>
#include <theora/theoradec.h>

// Ogg Theora movie decoder used to play the .ogv cutscenes and the
// animated main menu background (mainmenu.ogv) that the original D3D
// renderer played but the OpenGL path skipped.
struct TheoraMovie {
  FILE *file;
  char filename[512];
  ogg_sync_state oy;
  ogg_page og;
  ogg_stream_state vo;
  th_info ti;
  th_comment tc;
  th_setup_info *ts;
  th_dec_ctx *td;
  int video_stream;
  bool stream_initialized;
  bool eof;
  bool looping;
  th_ycbcr_buffer ycbcr;
  ogg_int64_t granulepos;
  int width;
  int height;
  bool headers_done;
};

// Opens an .ogv file and initializes the Theora decoder.
// Returns NULL on failure.
TheoraMovie *TheoraOpen(const char *filename, bool looping);

// Decodes the next video frame. Returns 0 on success, 1 on EOF/error.
int TheoraDecodeFrame(TheoraMovie *tm);

// Converts the last decoded frame to RGB555 and writes it into the
// provided pixel buffer (width*height entries).
void TheoraToRGB555(const TheoraMovie *tm, uint16_t *pixels, int pitch);

// Closes the decoder and frees all resources.
void TheoraClose(TheoraMovie *tm);

#else // !HAVE_THEORA

// Stub when libtheora is unavailable (CI runners, minimal platforms).
// All entry points report failure so callers fall back gracefully.
// width/height are present because d3movie reads them for blit sizing.
struct TheoraMovie {
  int width;
  int height;
};

static inline TheoraMovie *TheoraOpen(const char *, bool) { return nullptr; }
static inline int TheoraDecodeFrame(TheoraMovie *) { return 1; }
static inline void TheoraToRGB555(const TheoraMovie *, uint16_t *, int) {}
static inline void TheoraClose(TheoraMovie *) {}

#endif // HAVE_THEORA

#endif