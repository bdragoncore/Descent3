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

#include <cstring>
#include <filesystem>

#include "application.h"
#include "args.h"
#include "bitmap.h"
#include "d3movie.h"
#include "ddio.h"
#include "game.h"
#include "gamefont.h"
#include "grtext.h"
#include "log.h"
#include "mem.h"
#include "mvelib.h"
#include "renderer.h"
#include "theora_movie.h"

namespace {
MovieFrameCallback_fp Movie_callback = nullptr;
uint16_t CurrentPalette[256];
int Movie_bm_handle = -1;
uint32_t Movie_current_framenum = 0;
bool Movie_looping = false;
bool Movie_vid_set = false;

// Unified handle for movie sequences started with mve_SequenceStart().
// The MVE decoder and the Ogg Theora decoder (for .ogv cutscenes) share
// the same sequence API, so we tag which decoder a handle belongs to.
struct MovieSequence {
  enum class Type { MVE, THEORA } type;
  MVESTREAM *mve;
  TheoraMovie *theora;
};
} // namespace

static void *CallbackAlloc(uint32_t size);
static void CallbackFree(void *p);
static uint32_t CallbackFileRead(void *stream, void *pBuffer, uint32_t bufferCount);
static void InitializePalette();
static void CallbackSetPalette(const uint8_t *pBuffer, uint32_t start, uint32_t count);
static void CallbackShowFrame(uint8_t *buf, uint32_t bufw, uint32_t bufh, uint32_t sx,
                              uint32_t sy, uint32_t w, uint32_t h, uint32_t dstx, uint32_t dsty,
                              uint32_t hicolor);
static std::filesystem::path LocateMovieFile(const std::filesystem::path &name);
static void RenderTheoraFrame(TheoraMovie *tm, bool flip);

#ifndef NO_MOVIES
static bool mve_InitSound();
static void mve_CloseSound();
#endif

int mve_Init() {
#ifndef NO_MOVIES
  return MVELIB_NOERROR;
#else
  return MVELIB_INIT_ERROR;
#endif
}

// callback called per frame of playback of movies.
void mve_SetCallback(MovieFrameCallback_fp callBack) {
#ifndef NO_MOVIES
  Movie_callback = callBack;
#endif
}

// used to tell movie library how to render movies.
void mve_SetRenderProperties(int16_t x, int16_t y, int16_t w, int16_t h, renderer_type type, bool hicolor) {}

// plays a movie using the current screen.
int mve_PlayMovie(const std::filesystem::path &pMovieName, oeApplication *pApp) {
#ifndef NO_MOVIES
  // first, find that movie.
  std::filesystem::path real_name = LocateMovieFile(pMovieName);
  if (real_name.empty()) {
    LOG_ERROR << "MOVIE: File " << pMovieName << " not found, skipping playback";
    return MVELIB_FILE_ERROR;
  }

  std::string ext = real_name.extension().string();
  bool is_theora = (stricmp(ext.c_str(), ".ogv") == 0);

  if (is_theora) {
    // BUGFIX #685: The cutscenes are Ogg Theora videos. The MVE decoder
    // cannot read them, so decode with libtheora and render each frame.
    TheoraMovie *tm = TheoraOpen(real_name.u8string().c_str(), false);
    if (tm == nullptr) {
      LOG_ERROR << "MOVIE: Failed to open Theora movie " << pMovieName;
      return MVELIB_INIT_ERROR;
    }

    Movie_bm_handle = -1;
    Movie_current_framenum = 0;

    bool aborted = false;
    int result;
    while ((result = TheoraDecodeFrame(tm)) == 0) {
      // let the OS do its thing
      pApp->defer();

      // check for bail
      int key = ddio_KeyInKey();
      if (key == KEY_ESC) {
        aborted = true;
        break;
      }

      RenderTheoraFrame(tm, true);
    }

    // free our bitmap
    if (Movie_bm_handle != -1) {
      bm_FreeBitmap(Movie_bm_handle);
      Movie_bm_handle = -1;
    }

    TheoraClose(tm);

    // determine the return code
    int err = MVELIB_NOERROR;
    if (aborted) {
      err = MVELIB_PLAYBACK_ABORTED;
    } else if (result != 1) {
      err = MVELIB_PLAYBACK_ERROR;
    }

    // BUGFIX (PiccuEngine #12): Restore renderer state after movie.
    if (Movie_vid_set) {
      rend_SetPreferredState(&Render_preferred_state);
      Movie_vid_set = false;
    }

    return err;
  }

  // open movie file.
  FILE *hFile = fopen((const char*)real_name.u8string().c_str(), "rb");
  if (hFile == nullptr) {
    LOG_ERROR << "MOVIE: Unable to open " << pMovieName;
    return MVELIB_FILE_ERROR;
  }

  // setup
  MVE_sfCallbacks(CallbackShowFrame);
  MVE_memCallbacks(CallbackAlloc, CallbackFree);
  MVE_ioCallbacks(CallbackFileRead);
  MVE_palCallbacks(CallbackSetPalette);
  InitializePalette();
  Movie_bm_handle = -1;

  if (!mve_InitSound()) {
    LOG_ERROR << "Failed to initialize sound";
    fclose(hFile);
    return MVELIB_INIT_ERROR;
  }

  MVESTREAM *mve = MVE_rmPrepMovie(hFile, -1, -1, 0);
  if (mve == nullptr) {
    LOG_ERROR.printf("Failed to prepMovie %s", pMovieName.u8string().c_str());
    fclose(hFile);
    mve_CloseSound();
    return MVELIB_INIT_ERROR;
  }

  bool aborted = false;
  Movie_current_framenum = 0;
  int result;
  while ((result = MVE_rmStepMovie(mve)) == 0) {
    // let the OS do its thing
    pApp->defer();

    // check for bail
    int key = ddio_KeyInKey();
    if (key == KEY_ESC) {
      aborted = true;
      break;
    }
  }

  // free our bitmap
  if (Movie_bm_handle != -1) {
    bm_FreeBitmap(Movie_bm_handle);
    Movie_bm_handle = -1;
  }

  // close our file handle
  fclose(hFile);

  // determine the return code
  int err = MVELIB_NOERROR;
  if (aborted) {
    err = MVELIB_PLAYBACK_ABORTED;
  } else if (result != MVE_ERR_EOF) {
    err = MVELIB_PLAYBACK_ERROR;
  }

  // cleanup and shutdown
  MVE_rmEndMovie(mve);

  // BUGFIX (PiccuEngine #12): Restore renderer state after movie.
  // If we changed the renderer resolution for the movie, restore
  // the original preferred state so the game renders correctly.
  if (Movie_vid_set) {
    rend_SetPreferredState(&Render_preferred_state);
    Movie_vid_set = false;
  }

  // reset sound
  mve_CloseSound();

  // return out
  return err;
#else
  return MVELIB_INIT_ERROR;
#endif
}

void *CallbackAlloc(uint32_t size) { return mem_malloc(size); }

void CallbackFree(void *p) { mem_free(p); }

uint32_t CallbackFileRead(void *stream, void *pBuffer, uint32_t bufferCount) {
  uint32_t numRead = fread(pBuffer, 1, bufferCount, (FILE *)stream);
  return (numRead == bufferCount) ? 1 : 0;
}

void InitializePalette() {
  for (unsigned short & i : CurrentPalette) {
    i = OPAQUE_FLAG | GR_RGB16(0, 0, 0);
  }
}

void CallbackSetPalette(const uint8_t *pBuffer, uint32_t start, uint32_t count) {
#ifndef NO_MOVIES
  pBuffer += start * 3;

  for (uint32_t i = 0; i < count; ++i) {
    uint32_t r = pBuffer[0] << 2;
    uint32_t g = pBuffer[1] << 2;
    uint32_t b = pBuffer[2] << 2;
    pBuffer += 3;

    CurrentPalette[start + i] = OPAQUE_FLAG | GR_RGB16(r, g, b);
  }
#endif
}

int NextPow2(int n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  return n;
}

#ifndef NO_MOVIES
void BlitToMovieBitmap(uint8_t *buf, uint32_t bufw, uint32_t bufh, uint32_t hicolor,
                       bool usePow2Texture, int &texW, int &texH) {
  // get some sizes
  int drawWidth = bufw;
  int drawHeight = bufh;

  // BUGFIX (PiccuEngine #12): Remove pow2 texture restriction.
  // Modern GPUs don't need power-of-two textures. Using the actual
  // video dimensions eliminates unnecessary padding and stretching.
  texW = drawWidth;
  texH = drawHeight;

  if (Movie_bm_handle == -1) {
    // Allocate our bitmap
    Movie_bm_handle = bm_AllocBitmap(texW, texH, 0);
  }

  uint16_t *pPixelData = (uint16_t *)bm_data(Movie_bm_handle, 0);
  GameBitmaps[Movie_bm_handle].flags |= BF_CHANGED;
  if (hicolor) {
    uint16_t *wBuf = (uint16_t *)buf;
    for (int y = 0; y < drawHeight; ++y) {
      for (int x = 0; x < drawWidth; ++x) {
        uint16_t col16 = *wBuf++;
        // Convert to RGB555
        pPixelData[x] = col16 | OPAQUE_FLAG;
      }

      pPixelData += texW;
    }
  } else {
    for (int y = 0; y < drawHeight; ++y) {
      for (int x = 0; x < drawWidth; ++x) {
        uint8_t palIndex = *buf++;
        pPixelData[x] = CurrentPalette[palIndex];
      }

      pPixelData += texW;
    }
  }
}

void CallbackShowFrame(uint8_t *buf, uint32_t bufw, uint32_t bufh, uint32_t sx, uint32_t sy,
                       uint32_t w, uint32_t h, uint32_t dstx, uint32_t dsty, uint32_t hicolor) {
  // prepare our bitmap
  int texW, texH;
  BlitToMovieBitmap(buf, bufw, bufh, hicolor, true, texW, texH);

  // calculate UVs from texture
  unsigned int drawWidth = bufw;
  unsigned int drawHeight = bufh;
  float u = float(drawWidth - 1) / float(texW - 1);
  float v = float(drawHeight - 1) / float(texH - 1);

  // BUGFIX (PiccuEngine #12): Use actual video dimensions instead of
  // hardcoded 640x480. On the first frame, set the renderer to match
  // the video resolution so the movie fills the screen properly.
  if (!Movie_vid_set) {
    Render_preferred_state.width = drawWidth;
    Render_preferred_state.height = drawHeight;
    Movie_vid_set = true;
    rend_SetPreferredState(&Render_preferred_state);
  }

  StartFrame(0, 0, drawWidth, drawHeight, false);

  rend_ClearScreen(GR_BLACK);
  rend_SetAlphaType(AT_CONSTANT);
  rend_SetAlphaValue(255);
  rend_SetLighting(LS_NONE);
  rend_SetColorModel(CM_MONO);
  rend_SetOverlayType(OT_NONE);
  rend_SetWrapType(WT_CLAMP);
  rend_SetFiltering(0);
  rend_SetZBufferState(0);
  // BUGFIX (PiccuEngine #12): Draw at (0,0) to fill the screen.
  rend_DrawScaledBitmap(0, 0, drawWidth, drawHeight, Movie_bm_handle, 0.0f, 0.0f, u, v);
  rend_SetFiltering(1);
  rend_SetZBufferState(1);

  // call our callback
  if (Movie_callback != nullptr) {
    Movie_callback(dstx, dsty, Movie_current_framenum);
  }
  ++Movie_current_framenum;

  EndFrame();

  rend_Flip();
}

// This callback is same as CallbackShowFrame() but don't flip renderer at end, so there no flickering
void CallbackShowFrameNoFlip(unsigned char *buf, unsigned int bufw, unsigned int bufh, unsigned int sx, unsigned int sy,
                       unsigned int w, unsigned int h, unsigned int dstx, unsigned int dsty, unsigned int hicolor) {
  // prepare our bitmap
  int texW, texH;
  BlitToMovieBitmap(buf, bufw, bufh, hicolor, true, texW, texH);

  // calculate UVs from texture
  unsigned int drawWidth = bufw;
  unsigned int drawHeight = bufh;
  float u = float(drawWidth - 1) / float(texW - 1);
  float v = float(drawHeight - 1) / float(texH - 1);

  // BUGFIX (PiccuEngine #12): Use actual video dimensions instead of
  // hardcoded 640x480.
  if (!Movie_vid_set) {
    Render_preferred_state.width = drawWidth;
    Render_preferred_state.height = drawHeight;
    Movie_vid_set = true;
    rend_SetPreferredState(&Render_preferred_state);
  }

  StartFrame(0, 0, drawWidth, drawHeight, false);

  rend_ClearScreen(GR_BLACK);
  rend_SetAlphaType(AT_CONSTANT);
  rend_SetAlphaValue(255);
  rend_SetLighting(LS_NONE);
  rend_SetColorModel(CM_MONO);
  rend_SetOverlayType(OT_NONE);
  rend_SetWrapType(WT_CLAMP);
  rend_SetFiltering(0);
  rend_SetZBufferState(0);
  // BUGFIX (PiccuEngine #12): Draw at (0,0) to fill the screen.
  rend_DrawScaledBitmap(0, 0, drawWidth, drawHeight, Movie_bm_handle, 0.0f, 0.0f, u, v);
  rend_SetFiltering(1);
  rend_SetZBufferState(1);

  // call our callback
  if (Movie_callback != nullptr) {
    Movie_callback(dstx, dsty, Movie_current_framenum);
  }
  ++Movie_current_framenum;

  EndFrame();
}

// Locates a movie file in the movies/ directory. Tries the given name
// first, then falls back to the .ogv extension so the Ogg Theora
// cutscenes (mainmenu.ogv, intro.ogv, ...) are found even when the
// caller asks for the legacy .mve name.
static std::filesystem::path LocateMovieFile(const std::filesystem::path &name) {
  std::filesystem::path real_name = cf_LocatePath(std::filesystem::path("movies") / name);
  if (!real_name.empty()) {
    return real_name;
  }
  std::filesystem::path ogv_name = name;
  ogv_name.replace_extension(".ogv");
  return cf_LocatePath(std::filesystem::path("movies") / ogv_name);
}

// Renders a decoded Theora frame to the screen, mirroring what
// CallbackShowFrame() does for MVE frames. If flip is true the back
// buffer is presented, otherwise the caller is expected to flip.
void RenderTheoraFrame(TheoraMovie *tm, bool flip) {
  int drawWidth = tm->width;
  int drawHeight = tm->height;

  if (Movie_bm_handle == -1) {
    // BUGFIX #685: The OpenGL renderer (opengl_MakeBitmapCurrent) rejects
    // non-square textures.  Allocate a square bitmap large enough to hold
    // the frame and use UV coordinates to display only the video portion.
    int texSize = (drawWidth > drawHeight) ? drawWidth : drawHeight;
    Movie_bm_handle = bm_AllocBitmap(texSize, texSize, 0);
    // Clear the extra padding rows/cols so they don't bleed.
    uint16_t *pPixelData = (uint16_t *)bm_data(Movie_bm_handle, 0);
    std::memset(pPixelData, 0, texSize * texSize * sizeof(uint16_t));
  }

  int texW = bm_w(Movie_bm_handle, 0);
  int texH = bm_h(Movie_bm_handle, 0);

  uint16_t *pPixelData = (uint16_t *)bm_data(Movie_bm_handle, 0);
  GameBitmaps[Movie_bm_handle].flags |= BF_CHANGED;

  // Copy the decoded frame row-by-row into the top-left of the square bitmap.
  TheoraToRGB555(tm, pPixelData, texW);

  float u = float(drawWidth - 1) / float(texW - 1);
  float v = float(drawHeight - 1) / float(texH - 1);

  // BUGFIX #685: Draw the video scaled to the current display resolution
  // instead of forcing the renderer to the native video size.
  int screenW = Max_window_w;
  int screenH = Max_window_h;

  StartFrame(0, 0, screenW, screenH, false);

  rend_ClearScreen(GR_BLACK);
  rend_SetAlphaType(AT_CONSTANT);
  rend_SetAlphaValue(255);
  rend_SetLighting(LS_NONE);
  rend_SetColorModel(CM_MONO);
  rend_SetOverlayType(OT_NONE);
  rend_SetWrapType(WT_CLAMP);
  rend_SetFiltering(0);
  rend_SetZBufferState(0);
  rend_DrawScaledBitmap(0, 0, screenW, screenH, Movie_bm_handle, 0.0f, 0.0f, u, v);
  rend_SetFiltering(1);
  rend_SetZBufferState(1);

  if (Movie_callback != nullptr) {
    Movie_callback(0, 0, Movie_current_framenum);
  }
  ++Movie_current_framenum;

  EndFrame();

  if (flip) {
    rend_Flip();
  }
}
#endif

intptr_t mve_SequenceStart(const char *mvename, void *fhandle, oeApplication *app, bool looping) {
#ifndef NO_MOVIES
  // first, find that movie..
  std::filesystem::path real_name = LocateMovieFile(mvename);
  if (real_name.empty()) {
    LOG_WARNING.printf("MOVIE: Unable to find %s", mvename);
    return 0;
  }

  std::string ext = real_name.extension().string();
  bool is_theora = (stricmp(ext.c_str(), ".ogv") == 0);

  MovieSequence *seq = new MovieSequence;

  if (is_theora) {
    // BUGFIX #685: The main menu background (mainmenu.ogv) is an Ogg
    // Theora video. The MVE decoder cannot read it, so decode it with
    // libtheora and feed the frames into the movie bitmap pipeline.
    TheoraMovie *tm = TheoraOpen(real_name.u8string().c_str(), looping);
    if (tm == nullptr) {
      LOG_ERROR.printf("Failed to open Theora movie %s", real_name.u8string().c_str());
      delete seq;
      return 0;
    }
    seq->type = MovieSequence::Type::THEORA;
    seq->theora = tm;
    seq->mve = nullptr;
    Movie_bm_handle = -1;
    Movie_looping = looping;
    rend_SetFrameBufferCopyState(true);
    return (intptr_t)seq;
  }

  // MVE path
  FILE *file = fopen((const char*)real_name.u8string().c_str(), "rb");
  if (file == nullptr) {
    LOG_WARNING.printf("MOVIE: Unable to open %s", (const char*)real_name.u8string().c_str());
    delete seq;
    return 0;
  }
  // Preserve the original contract: the caller's filehandle (passed as
  // &filehandle) receives the opened FILE*.
  fhandle = file;

  // setup
  MVE_memCallbacks(CallbackAlloc, CallbackFree);
  MVE_ioCallbacks(CallbackFileRead);
  MVE_sfCallbacks(CallbackShowFrameNoFlip);
  MVE_palCallbacks(CallbackSetPalette);
  InitializePalette();
  Movie_bm_handle = -1;
  Movie_looping = looping;

  // let the render know we will be copying bitmaps to framebuffer (or something)
  rend_SetFrameBufferCopyState(true);

  MVESTREAM *mve = MVE_rmPrepMovie(file, -1, -1, 0);
  if (mve == nullptr) {
    LOG_ERROR.printf("Failed to PrepMovie %s", mvename);
    fclose(file);
    delete seq;
    return MVELIB_INIT_ERROR;
  }

  seq->type = MovieSequence::Type::MVE;
  seq->mve = mve;
  seq->theora = nullptr;
  return (intptr_t)seq;
#else
  return nullptr;
#endif
}

intptr_t mve_SequenceFrame(intptr_t handle, void *fhandle, bool sequence, int *bm_handle) {
#ifndef NO_MOVIES
  if (bm_handle) {
    *bm_handle = -1;
  }

  if (handle == -1) {
    return -1;
  }

  MovieSequence *seq = (MovieSequence *)handle;

  if (seq->type == MovieSequence::Type::THEORA) {
    TheoraMovie *tm = seq->theora;
    if (TheoraDecodeFrame(tm) != 0) {
      return -1;
    }

    // BUGFIX #685: Render the frame directly into the back buffer at the
    // current screen resolution.  We signal bm_handle == -1 so that the
    // caller (FrameMovie) skips its rend_CopyBitmapToFramebuffer call,
    // which would otherwise paste the bitmap at native video size.
    RenderTheoraFrame(tm, false);

    if (bm_handle) {
      *bm_handle = -1;
    }
    return handle;
  }

  static unsigned sw = 0, sh = 0, hicolor = 0;
  int err = 0;

reread_frame:

  // get the next frame of data
  uint8_t *pBuffer = nullptr;
  err = MVE_rmStepMovie(seq->mve);

  if (err == 0) {
    // blit to bitmap
    int texW, texH;
    BlitToMovieBitmap(pBuffer, sw, sh, hicolor, false, texW, texH);

    if (bm_handle) {
      *bm_handle = Movie_bm_handle;
    }

    return handle;
  }

  if (Movie_looping && err == MVE_ERR_EOF) {
    mve_reset(seq->mve);
    sequence = true;
    goto reread_frame;
  }

  return -1;
#else
  return -1;
#endif
}

bool mve_SequenceClose(intptr_t hMovie, void *hFile) {
#ifndef NO_MOVIES
  if (hMovie == -1)
    return false;

  MovieSequence *seq = (MovieSequence *)hMovie;
  if (seq->type == MovieSequence::Type::THEORA) {
    TheoraClose(seq->theora);
  } else {
    MVE_rmEndMovie(seq->mve);
  }

  // free our bitmap
  if (Movie_bm_handle != -1) {
    bm_FreeBitmap(Movie_bm_handle);
    Movie_bm_handle = -1;
  }

  // We're no longer needing this
  rend_SetFrameBufferCopyState(false);

  delete seq;

  return true;
#else
  return false;
#endif
}

void mve_Puts(int16_t x, int16_t y, ddgr_color col, const char *txt) {
  grtext_SetFont(BRIEFING_FONT);
  grtext_SetColor(col);
  grtext_SetAlpha(255);
  grtext_SetFlags(GRTEXTFLAG_SHADOW);
  grtext_CenteredPrintf(0, y, txt);
  grtext_Flush();
}

void mve_ClearRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  // Note: I can not figure out how to clear, and then write over it with text. It always covers my text!
  // rend_FillRect( GR_BLACK, x1, y1, x2, y2 );
}

#ifndef NO_MOVIES
bool mve_InitSound() {
  MVE_sndInit(FindArg("-nosound") == 0);

  return true;
}

void mve_CloseSound() {
  // TODO: close the driver out
}

#endif
