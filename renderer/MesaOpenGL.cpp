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

/*
 * MESA OPENGL SOFTWARE RENDERER
 * =============================
 *
 * PURPOSE:
 * This is a Mesa-specific backend renderer designed exclusively for unit testing.
 * It provides headless OpenGL rendering using Mesa's software rasterizer (llvmpipe)
 * without requiring a GPU or display server.
 *
 * ARCHITECTURE:
 * - Attaches to an already-current EGL context (created by OffscreenGL::Init())
 * - Does NOT create its own EGL context — the test harness owns it
 * - Provides identical API to HardwareOpenGL for test compatibility
 *
 * DEPENDENCIES:
 * - EGL (EGL_PLATFORM_SURFACELESS_MESA)
 * - OpenGL (core profile)
 * - Mesa llvmpipe driver
 *
 * ENVIRONMENT VARIABLES:
 * - GALLIUM_DRIVER=llvmpipe: Forces software rasterizer
 * - LIBGL_ALWAYS_SOFTWARE=1: Ensures no hardware acceleration
 * - MESA_GL_VERSION_OVERRIDE=3.3: Sets GL version
 * - MESA_GLSL_VERSION_OVERRIDE=330: Sets GLSL version
 *
 * USAGE:
 * Only used by d3_text_render_tests and other rendering unit tests.
 * Production builds do not link against this renderer.
 *
 * FRAMEBUFFER ACCESS:
 * - mesa_GetFramebuffer(): Returns raw framebuffer data
 * - mesa_GetFramebufferWidth/Height(): Returns dimensions
 * - rend_Screenshot(): Creates NewBitmap from framebuffer
 */

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "byteswap.h"
#include "pserror.h"
#include "mono.h"
#include "3d.h"
#include "renderer.h"
#include "application.h"
#include "bitmap.h"
#include "lightmap.h"
#include "log.h"
#include "rend_opengl.h"
#include "grdefs.h"
#include "mem.h"
#include "config.h"
#include "rtperformance.h"
#include "HardwareInternal.h"
#include "HardwareOpenGL.h"
#include "args.h"
#include "NewBitmap.h"

extern int gpu_Overlay_map;
int Bump_map = 0;
int Bumpmap_ready = 0;
extern uint8_t gpu_Overlay_type;
float Z_bias = 0.0f;
uint8_t Renderer_close_flag = 0;
extern uint8_t Renderer_initted;
renderer_type Renderer_type = RENDERER_MESA;

#ifndef GL_UNSIGNED_SHORT_5_5_5_1
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#endif

#ifndef GL_UNSIGNED_SHORT_4_4_4_4
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#endif

#define CHECK_ERROR(x)

#define GET_WRAP_STATE(x) ((x) >> 4)
#define GET_FILTER_STATE(x) ((x) & 0x0f)

#define SET_WRAP_STATE(x, s)                                                                                           \
  {                                                                                                                    \
    (x) &= 0x0F;                                                                                                       \
    (x) |= ((s) << 4);                                                                                                 \
  }
#define SET_FILTER_STATE(x, s)                                                                                         \
  {                                                                                                                    \
    (x) &= 0xF0;                                                                                                       \
    (x) |= (s);                                                                                                        \
  }

static int OpenGL_polys_drawn = 0;
static int OpenGL_verts_processed = 0;
static int OpenGL_uploads = 0;
static int OpenGL_sets_this_frame[10];
static int OpenGL_packed_pixels = 0;
static std::vector<GLuint> textures_;
static int OpenGL_cache_initted = 0;
static int OpenGL_last_bound[2];

extern int gpu_last_frame_polys_drawn;
extern int gpu_last_frame_verts_processed;
extern int gpu_last_uploaded;

extern float gpu_Alpha_factor;
extern float gpu_Alpha_multiplier;

uint16_t *OpenGL_bitmap_remap = nullptr;
uint16_t *OpenGL_lightmap_remap = nullptr;
uint8_t *OpenGL_bitmap_states = nullptr;
uint8_t *OpenGL_lightmap_states = nullptr;

uint32_t *opengl_Upload_data = nullptr;
uint32_t *opengl_Translate_table = nullptr;
uint32_t *opengl_4444_translate_table = nullptr;

uint16_t *opengl_packed_Upload_data = nullptr;
uint16_t *opengl_packed_Translate_table = nullptr;
uint16_t *opengl_packed_4444_translate_table = nullptr;

extern rendering_state gpu_state;
extern renderer_preferred_state gpu_preferred_state;

void opengl_GetInformation();

bool OpenGL_multitexture_state = false;
int Already_loaded = 0;
bool opengl_Blending_on = false;

namespace {
class MesaOpenGL final : public HardwareOpenGL {
public:
  bool SetupContext(int width, int height) override {
    EGLDisplay dpy = eglGetCurrentDisplay();
    EGLContext ctx = eglGetCurrentContext();
    if (dpy == EGL_NO_DISPLAY || ctx == EGL_NO_CONTEXT) {
      LOG_ERROR << "Mesa: No current EGL context — call OffscreenGL::Init() first";
      return false;
    }

    LOG_INFO << "Mesa: Attaching to existing EGL context";

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DITHER);
    glShadeModel(GL_SMOOTH);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glViewport(0, 0, width, height);

    LOG_INFO.printf("Mesa: Renderer attached at %d x %d", width, height);
    opengl_GetInformation();
    return true;
  }

  void DestroyContext(bool just_resizing) override {}

  void PresentFrame() const override { glFlush(); }

  std::unique_ptr<NewBitmap> Screenshot(int width, int height) const override {
    auto result = std::make_unique<NewBitmap>(width, height, PixelDataFormat::RGBA32, true);
    if (!result || result->getData() == nullptr) {
      return nullptr;
    }

    auto *tmp = new uint8_t[width * height * 4];
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp);

    uint8_t *dst = static_cast<uint8_t *>(result->getData());
    for (int y = 0; y < height; y++) {
      memcpy(dst + y * width * 4, tmp + (height - 1 - y) * width * 4, width * 4);
    }
    delete[] tmp;
    return result;
  }

  void SetFullScreen(bool fullscreen) override { LOG_DEBUG.printf("Fullscreen not supported in Mesa soft device"); }

  bool InitWindowMode() override { return true; }
};

MesaOpenGL g_mesa_backend;
} // namespace

// NOTE: EGL context is owned by OffscreenGL (tests/render/offscreen_gl.h).
// MesaOpenGL attaches to the already-current context; it does NOT
// create or destroy EGL resources.

bool opengl_CheckExtension(std::string_view extName) {
  // Check OpenGL extensions using standard GL functions
  GLint numExtensions;
  glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
  for (GLint i = 0; i < numExtensions; i++) {
    if (extName == reinterpret_cast<char const *>(glGetStringi(GL_EXTENSIONS, i))) {
      return true;
    }
  }
  return false;
}

void opengl_GetInformation() {
  LOG_INFO.printf("Mesa OpenGL Vendor: %s", glGetString(GL_VENDOR));
  LOG_INFO.printf("Mesa OpenGL Renderer: %s", glGetString(GL_RENDERER));
  LOG_INFO.printf("Mesa OpenGL Version: %s", glGetString(GL_VERSION));
}

int opengl_MakeTextureObject(int tn) {
  GLuint num;
  glGenTextures(1, &num);
  textures_.push_back(num);

  glActiveTexture(GL_TEXTURE0_ARB + tn);
  glBindTexture(GL_TEXTURE_2D, num);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  CHECK_ERROR(2)
  return num;
}

int opengl_InitCache() {
  OpenGL_bitmap_remap = mem_rmalloc<uint16_t>(MAX_BITMAPS);
  ASSERT(OpenGL_bitmap_remap);
  OpenGL_lightmap_remap = mem_rmalloc<uint16_t>(MAX_LIGHTMAPS);
  ASSERT(OpenGL_lightmap_remap);

  OpenGL_bitmap_states = mem_rmalloc<uint8_t>(MAX_BITMAPS);
  ASSERT(OpenGL_bitmap_states);
  OpenGL_lightmap_states = mem_rmalloc<uint8_t>(MAX_LIGHTMAPS);
  ASSERT(OpenGL_lightmap_states);

  int i;
  for (i = 0; i < MAX_BITMAPS; i++) {
    OpenGL_bitmap_remap[i] = 65535;
    OpenGL_bitmap_states[i] = 255;
    GameBitmaps[i].flags |= BF_CHANGED | BF_BRAND_NEW;
  }

  for (i = 0; i < MAX_LIGHTMAPS; i++) {
    OpenGL_lightmap_remap[i] = 65535;
    OpenGL_lightmap_states[i] = 255;
    GameLightmaps[i].flags |= LF_CHANGED | LF_BRAND_NEW;
  }

  CHECK_ERROR(3)
  OpenGL_cache_initted = 1;
  return 1;
}

void opengl_SetDefaults() {
  LOG_INFO << "Setting states";

  gpu_state.cur_color = 0x00FFFFFF;
  gpu_state.cur_bilinear_state = -1;
  gpu_state.cur_zbuffer_state = -1;
  gpu_state.cur_texture_quality = -1;
  gpu_state.cur_light_state = LS_GOURAUD;
  gpu_state.cur_color_model = CM_MONO;
  gpu_state.cur_bilinear_state = -1;
  gpu_state.cur_alpha_type = AT_TEXTURE;

  glEnable(GL_BLEND);
  glEnable(GL_DITHER);
  opengl_Blending_on = true;

  rend_SetAlphaType(AT_ALWAYS);
  rend_SetAlphaValue(255);
  rend_SetFiltering(1);
  rend_SetLighting(LS_NONE);
  rend_SetTextureType(TT_FLAT);
  rend_SetColorModel(CM_RGB);
  rend_SetZBufferState(1);
  rend_SetGammaValue(gpu_preferred_state.gamma);
  OpenGL_last_bound[0] = 9999999;
  OpenGL_last_bound[1] = 9999999;
  OpenGL_multitexture_state = false;

  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, gpu_state.screen_width, gpu_state.screen_height);
  glDisable(GL_SCISSOR_TEST);

  // Skip texture binding for Mesa - bitmap system may not be fully initialized
  // gpu_BindTexture(BAD_BITMAP_HANDLE, MAP_TYPE_BITMAP, 0);
  // gpu_BindTexture(BAD_BITMAP_HANDLE, MAP_TYPE_BITMAP, 1);

  glActiveTexture(GL_TEXTURE0_ARB + 1);
  glDisable(GL_TEXTURE_2D); // Disable texturing on unit 1
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glEnable(GL_DITHER);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  glActiveTexture(GL_TEXTURE0_ARB + 0);
  glDisable(GL_TEXTURE_2D); // Disable texturing on unit 0 (for flat color rendering)
}

extern renderer_preferred_state Render_preferred_state;

int opengl_Setup(oeApplication *app, const int *width, const int *height) {
  g_mesa_backend.SetParentApplication(app);
  if (!g_mesa_backend.SetupContext(*width, *height)) {
    return 0;
  }
  Already_loaded = 1;
  return 1;
}

int opengl_Init(oeApplication *app, renderer_preferred_state *pref_state) {
  int width, height;
  int retval = 1;
  int i;

  LOG_INFO << "Setting up Mesa soft device mode!";
  LOG_INFO << "Mesa renderer initialized successfully";

  if (pref_state) {
    gpu_preferred_state = *pref_state;
  }

  g_mesa_backend.SetParentApplication(app);

  width = gpu_preferred_state.width;
  height = gpu_preferred_state.height;

  if (!opengl_Setup(app, &width, &height)) {
    opengl_Close();
    return 0;
  }

  memset(&gpu_state, 0, sizeof(rendering_state));
  gpu_state.screen_width = width;
  gpu_state.screen_height = height;

  OpenGL_packed_pixels = opengl_CheckExtension("GL_EXT_packed_pixels");

  // Initialize the texture cache — bitmap system must be initialized first
  opengl_InitCache();

  if (OpenGL_packed_pixels) {
    opengl_packed_Upload_data = mem_rmalloc<uint16_t>(2048 * 2048);
    opengl_packed_Translate_table = mem_rmalloc<uint16_t>(65536);
    opengl_packed_4444_translate_table = mem_rmalloc<uint16_t>(65536);

    ASSERT(opengl_packed_Upload_data);
    ASSERT(opengl_packed_Translate_table);
    ASSERT(opengl_packed_4444_translate_table);

    LOG_INFO << "Building packed OpenGL translate table...";

    for (i = 0; i < 65536; i++) {
      int r = (i >> 10) & 0x1f;
      int g = (i >> 5) & 0x1f;
      int b = i & 0x1f;

      uint16_t pix;
      if (!(i & OPAQUE_FLAG)) {
        pix = 0;
      } else {
        pix = (r << 11) | (g << 6) | (b << 1) | 1;
      }
      opengl_packed_Translate_table[i] = INTEL_INT(pix);

      int a = (i >> 12) & 0xf;
      r = (i >> 8) & 0xf;
      g = (i >> 4) & 0xf;
      b = i & 0xf;
      pix = (r << 12) | (g << 8) | (b << 4) | a;
      opengl_packed_4444_translate_table[i] = INTEL_INT(pix);
    }
  } else {
    opengl_Upload_data = mem_rmalloc<uint32_t>(2048 * 2048);
    opengl_Translate_table = mem_rmalloc<uint32_t>(65536);
    opengl_4444_translate_table = mem_rmalloc<uint32_t>(65536);

    ASSERT(opengl_Upload_data);
    ASSERT(opengl_Translate_table);
    ASSERT(opengl_4444_translate_table);

    LOG_INFO << "Building OpenGL translate table...";

    for (i = 0; i < 65536; i++) {
      uint32_t pix;
      int r = (i >> 10) & 0x1f;
      int g = (i >> 5) & 0x1f;
      int b = i & 0x1f;

      float fr = (float)r / 31.0f;
      float fg = (float)g / 31.0f;
      float fb = (float)b / 31.0f;

      r = 255 * fr;
      g = 255 * fg;
      b = 255 * fb;

      if (!(i & OPAQUE_FLAG)) {
        pix = 0;
      } else {
        pix = (255 << 24) | (b << 16) | (g << 8) | (r);
      }
      opengl_Translate_table[i] = INTEL_INT(pix);

      int a = (i >> 12) & 0xf;
      r = (i >> 8) & 0xf;
      g = (i >> 4) & 0xf;
      b = i & 0xf;

      float fa = (float)a / 15.0f;
      fr = (float)r / 15.0f;
      fg = (float)g / 15.0f;
      fb = (float)b / 15.0f;

      a = 255 * fa;
      r = 255 * fr;
      g = 255 * fg;
      b = 255 * fb;

      pix = (a << 24) | (b << 16) | (g << 8) | (r);
      opengl_4444_translate_table[i] = INTEL_INT(pix);
    }
  }

  opengl_SetDefaults();
  g3_ForceTransformRefresh();

  CHECK_ERROR(4)
  gpu_state.initted = 1;

  LOG_INFO.printf("Mesa soft device initialization at %d x %d was successful.", width, height);

  return retval;
}

void opengl_Close(const bool just_resizing) {
  CHECK_ERROR(5)

  if (!textures_.empty()) {
    glDeleteTextures(textures_.size(), textures_.data());
    textures_.clear();
  }

  g_mesa_backend.DestroyContext(just_resizing);

  if (OpenGL_packed_pixels) {
    if (opengl_packed_Upload_data)
      mem_free(opengl_packed_Upload_data);
    if (opengl_packed_Translate_table)
      mem_free(opengl_packed_Translate_table);
    if (opengl_packed_4444_translate_table)
      mem_free(opengl_packed_4444_translate_table);
    opengl_packed_Upload_data = nullptr;
    opengl_packed_Translate_table = nullptr;
    opengl_packed_4444_translate_table = nullptr;
  } else {
    if (opengl_Upload_data)
      mem_free(opengl_Upload_data);
    if (opengl_Translate_table)
      mem_free(opengl_Translate_table);
    if (opengl_4444_translate_table)
      mem_free(opengl_4444_translate_table);
    opengl_Upload_data = nullptr;
    opengl_Translate_table = nullptr;
    opengl_4444_translate_table = nullptr;
  }

  if (OpenGL_cache_initted) {
    mem_free(OpenGL_lightmap_remap);
    mem_free(OpenGL_bitmap_remap);
    mem_free(OpenGL_lightmap_states);
    mem_free(OpenGL_bitmap_states);
    OpenGL_cache_initted = 0;
  }

  gpu_state.initted = 0;
}

void opengl_TranslateBitmapToOpenGL(int texnum, int bm_handle, int map_type, int replace, int tn) {
  uint16_t *bm_ptr;
  int w, h, size;

  glActiveTexture(GL_TEXTURE0_ARB + tn);

  if (map_type == MAP_TYPE_LIGHTMAP) {
    if (GameLightmaps[bm_handle].flags & LF_BRAND_NEW)
      replace = 0;
    bm_ptr = lm_data(bm_handle);
    GameLightmaps[bm_handle].flags &= ~(LF_CHANGED | LF_BRAND_NEW);
    w = lm_w(bm_handle);
    h = lm_h(bm_handle);
    size = GameLightmaps[bm_handle].square_res;
  } else {
    if (GameBitmaps[bm_handle].flags & BF_BRAND_NEW)
      replace = 0;
    bm_ptr = bm_data(bm_handle, 0);
    GameBitmaps[bm_handle].flags &= ~(BF_CHANGED | BF_BRAND_NEW);
    w = bm_w(bm_handle, 0);
    h = bm_h(bm_handle, 0);
    size = w;
  }

  if (OpenGL_last_bound[tn] != texnum) {
    glBindTexture(GL_TEXTURE_2D, texnum);
    OpenGL_sets_this_frame[0]++;
    OpenGL_last_bound[tn] = texnum;
  }

  if (OpenGL_packed_pixels) {
    if (map_type == MAP_TYPE_LIGHTMAP) {
      uint16_t *left_data = (uint16_t *)opengl_packed_Upload_data;
      int bm_left = 0;
      for (int i = 0; i < h; i++, left_data += size, bm_left += w) {
        uint16_t *dest_data = left_data;
        for (int t = 0; t < w; t++) {
          *dest_data++ = opengl_packed_Translate_table[bm_ptr[bm_left + t]];
        }
      }
      if (replace) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
                        opengl_packed_Upload_data);
      } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, size, size, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
                     opengl_packed_Upload_data);
      }
    } else {
      // Bitmap upload (packed pixels)
      int limit = bm_mipped(bm_handle) ? NUM_MIP_LEVELS + 3 : 1;
      for (int m = 0; m < limit; m++) {
        if (m < NUM_MIP_LEVELS) {
          bm_ptr = bm_data(bm_handle, m);
          w = bm_w(bm_handle, m);
          h = bm_h(bm_handle, m);
        } else {
          bm_ptr = bm_data(bm_handle, NUM_MIP_LEVELS - 1);
          w = bm_w(bm_handle, NUM_MIP_LEVELS - 1);
          h = bm_h(bm_handle, NUM_MIP_LEVELS - 1);
          w >>= m - (NUM_MIP_LEVELS - 1);
          h >>= m - (NUM_MIP_LEVELS - 1);
          if ((w < 1) || (h < 1))
            continue;
        }

        if (bm_format(bm_handle) == BITMAP_FORMAT_4444) {
          if (bm_mipped(bm_handle)) {
            for (int i = 0; i < w * h; i++)
              opengl_packed_Upload_data[i] = 0xf | (opengl_packed_4444_translate_table[bm_ptr[i]]);
          } else {
            for (int i = 0; i < w * h; i++)
              opengl_packed_Upload_data[i] = opengl_packed_4444_translate_table[bm_ptr[i]];
          }
          if (replace) {
            glTexSubImage2D(GL_TEXTURE_2D, m, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,
                            opengl_packed_Upload_data);
          } else {
            glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA4, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,
                         opengl_packed_Upload_data);
          }
        } else {
          for (int i = 0; i < w * h; i++) {
            opengl_packed_Upload_data[i] = opengl_packed_Translate_table[bm_ptr[i]];
          }
          if (replace) {
            glTexSubImage2D(GL_TEXTURE_2D, m, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
                            opengl_packed_Upload_data);
          } else {
            glTexImage2D(GL_TEXTURE_2D, m, GL_RGB5_A1, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
                         opengl_packed_Upload_data);
          }
        }
      }
    }
  } else {
    if (map_type == MAP_TYPE_LIGHTMAP) {
      uint32_t *left_data = (uint32_t *)opengl_Upload_data;
      int bm_left = 0;
      for (int i = 0; i < h; i++, left_data += size, bm_left += w) {
        uint32_t *dest_data = left_data;
        for (int t = 0; t < w; t++) {
          *dest_data++ = opengl_Translate_table[bm_ptr[bm_left + t]];
        }
      }
      if (size > 0) {
        if (replace) {
          glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, opengl_Upload_data);
        } else {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, opengl_Upload_data);
        }
      }
    } else {
      // Bitmap upload (non-packed pixels)
      int limit = bm_mipped(bm_handle) ? NUM_MIP_LEVELS + 3 : 1;
      for (int m = 0; m < limit; m++) {
        bm_ptr = bm_data(bm_handle, m);
        w = bm_w(bm_handle, m);
        h = bm_h(bm_handle, m);

        if (bm_format(bm_handle) == BITMAP_FORMAT_4444) {
          if (bm_mipped(bm_handle)) {
            for (int i = 0; i < w * h; i++)
              opengl_Upload_data[i] = INTEL_INT((255 << 24)) | opengl_4444_translate_table[bm_ptr[i]];
          } else {
            for (int i = 0; i < w * h; i++)
              opengl_Upload_data[i] = opengl_4444_translate_table[bm_ptr[i]];
          }
        } else {
          for (int i = 0; i < w * h; i++)
            opengl_Upload_data[i] = opengl_Translate_table[bm_ptr[i]];
        }

        if ((w > 0) && (h > 0)) {
          if (replace) {
            glTexSubImage2D(GL_TEXTURE_2D, m, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, opengl_Upload_data);
          } else {
            glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, opengl_Upload_data);
          }
        }
      }
    }
  }

  if (map_type == MAP_TYPE_LIGHTMAP) {
    GameLightmaps[bm_handle].flags &= ~LF_LIMITS;
  }

  CHECK_ERROR(6)
  OpenGL_uploads++;
}

extern bool Force_one_texture;

int opengl_MakeBitmapCurrent(int handle, int map_type, int tn) {
  int w, h, texnum;

  if (map_type == MAP_TYPE_LIGHTMAP) {
    w = GameLightmaps[handle].square_res;
    h = GameLightmaps[handle].square_res;
  } else {
    if (Force_one_texture)
      handle = 0;
    w = bm_w(handle, 0);
    h = bm_h(handle, 0);
  }

  if (w != h) {
    LOG_WARNING << "Can't use non-square textures with OpenGL!";
    return 0;
  }

  if (map_type == MAP_TYPE_LIGHTMAP) {
    if (OpenGL_lightmap_remap[handle] == 65535) {
      texnum = opengl_MakeTextureObject(tn);
      SET_WRAP_STATE(OpenGL_lightmap_states[handle], 1);
      SET_FILTER_STATE(OpenGL_lightmap_states[handle], 0);
      OpenGL_lightmap_remap[handle] = texnum;
      opengl_TranslateBitmapToOpenGL(texnum, handle, map_type, 0, tn);
    } else {
      texnum = OpenGL_lightmap_remap[handle];
      if (GameLightmaps[handle].flags & LF_CHANGED)
        opengl_TranslateBitmapToOpenGL(texnum, handle, map_type, 1, tn);
    }
  } else {
    if (OpenGL_bitmap_remap[handle] == 65535) {
      texnum = opengl_MakeTextureObject(tn);
      SET_WRAP_STATE(OpenGL_bitmap_states[handle], 1);
      SET_FILTER_STATE(OpenGL_bitmap_states[handle], 0);
      OpenGL_bitmap_remap[handle] = texnum;
      opengl_TranslateBitmapToOpenGL(texnum, handle, map_type, 0, tn);
    } else {
      texnum = OpenGL_bitmap_remap[handle];
      if (GameBitmaps[handle].flags & BF_CHANGED) {
        opengl_TranslateBitmapToOpenGL(texnum, handle, map_type, 1, tn);
      }
    }
  }

  if (OpenGL_last_bound[tn] != texnum) {
    glActiveTexture(GL_TEXTURE0_ARB + tn);
    glBindTexture(GL_TEXTURE_2D, texnum);
    OpenGL_last_bound[tn] = texnum;
    OpenGL_sets_this_frame[0]++;
  }

  CHECK_ERROR(7)
  return 1;
}

void opengl_MakeWrapTypeCurrent(int handle, int map_type, int tn) {
  int uwrap;
  wrap_type dest_wrap;

  if (tn == 1)
    dest_wrap = WT_CLAMP;
  else
    dest_wrap = gpu_state.cur_wrap_type;

  if (map_type == MAP_TYPE_LIGHTMAP)
    uwrap = GET_WRAP_STATE(OpenGL_lightmap_states[handle]);
  else
    uwrap = GET_WRAP_STATE(OpenGL_bitmap_states[handle]);

  if (uwrap == dest_wrap)
    return;

  glActiveTexture(GL_TEXTURE0_ARB + tn);
  OpenGL_sets_this_frame[1]++;

  if (gpu_state.cur_wrap_type == WT_CLAMP) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  } else if (gpu_state.cur_wrap_type == WT_WRAP_V) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  if (map_type == MAP_TYPE_LIGHTMAP) {
    SET_WRAP_STATE(OpenGL_lightmap_states[handle], dest_wrap);
  } else {
    SET_WRAP_STATE(OpenGL_bitmap_states[handle], dest_wrap);
  }

  CHECK_ERROR(8)
}

void opengl_MakeFilterTypeCurrent(int handle, int map_type, int tn) {
  int magf;
  int8_t dest_state;

  if (map_type == MAP_TYPE_LIGHTMAP) {
    magf = GET_FILTER_STATE(OpenGL_lightmap_states[handle]);
    dest_state = 1;
  } else {
    magf = GET_FILTER_STATE(OpenGL_bitmap_states[handle]);
    dest_state = gpu_preferred_state.filtering;
    if (!gpu_state.cur_bilinear_state)
      dest_state = 0;
  }

  if (magf == dest_state)
    return;

  glActiveTexture(GL_TEXTURE0_ARB + tn);
  OpenGL_sets_this_frame[2]++;

  if (dest_state) {
    if (map_type == MAP_TYPE_BITMAP && bm_mipped(handle)) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
  } else {
    if (map_type == MAP_TYPE_BITMAP && bm_mipped(handle)) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
  }

  if (map_type == MAP_TYPE_LIGHTMAP) {
    SET_FILTER_STATE(OpenGL_lightmap_states[handle], dest_state);
  } else {
    SET_FILTER_STATE(OpenGL_bitmap_states[handle], dest_state);
  }

  CHECK_ERROR(9)
}

void gpu_SetMultitextureBlendMode(bool state) {
  if (OpenGL_multitexture_state == state)
    return;
  OpenGL_multitexture_state = state;
}

// Transform functions — use fixed-function GL for Mesa compatibility profile
void rend_TransformSetToPassthru() {
  int width = gpu_state.screen_width;
  int height = gpu_state.screen_height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, height, 0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
}

void rend_TransformSetViewport(int lx, int ty, int width, int height) {
  glViewport(lx, gpu_state.screen_height - (ty + height - 1), width, height);
}

void rend_TransformSetProjection(float trans[4][4]) {
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(&trans[0][0]);
  glMatrixMode(GL_MODELVIEW);
}

void rend_TransformSetModelView(float trans[4][4]) {
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(&trans[0][0]);
}

void rend_SetGammaValue(float val) {
  gpu_preferred_state.gamma = val;
  LOG_DEBUG.printf("Setting gamma to %f", val);
}

void rend_SetFullScreen(bool fullscreen) { g_mesa_backend.SetFullScreen(fullscreen); }

bool rend_InitWindowMode() { return g_mesa_backend.InitWindowMode(); }

void opengl_ResetCache() {
  if (OpenGL_cache_initted) {
    mem_free(OpenGL_lightmap_remap);
    mem_free(OpenGL_bitmap_remap);
    mem_free(OpenGL_lightmap_states);
    mem_free(OpenGL_bitmap_states);
    OpenGL_cache_initted = 0;
  }
  opengl_InitCache();
}

uint8_t opengl_Framebuffer_ready = 0;
chunked_bitmap opengl_Chunked_bitmap;

void opengl_ChangeChunkedBitmap(int bm_handle, chunked_bitmap *chunk) {}

void rend_SetMipState(int8_t mipstate) {}

// Mesa-specific renderer initialization function
// This function is only used when the Mesa renderer is explicitly linked
int mesa_rend_Init(renderer_type state, oeApplication *app, renderer_preferred_state *pref_state) {
  int retval = 0;
#ifndef DEDICATED_ONLY
  rend_SetRendererType(state);
  if (!Renderer_initted) {
    if (!Renderer_close_flag) {
      atexit(rend_Close);
      Renderer_close_flag = 1;
    }
    Renderer_initted = 1;
  }

  LOG_DEBUG.printf("Renderer init is set to %d", Renderer_initted);

#ifndef OEM_V3
  // Only initialize Mesa renderer if the state is RENDERER_MESA
  if (state == RENDERER_MESA) {
    LOG_INFO << "Initializing Mesa renderer";
    retval = opengl_Init(app, pref_state);
  } else {
    LOG_WARNING.printf("Mesa renderer called with state %d, expected RENDERER_MESA (%d)", state, RENDERER_MESA);
    retval = 0;
  }
#endif

#endif
  return retval;
}

// Define rend_Init as a macro to redirect to mesa_rend_Init when Mesa renderer is enabled
// This allows the Mesa renderer to override the HardwareOpenGL renderer
#ifdef RENDERER_MESA_ENABLED
int rend_Init(renderer_type state, oeApplication *app, renderer_preferred_state *pref_state) {
  return mesa_rend_Init(state, app, pref_state);
}
#endif

void rend_Close() {
  LOG_DEBUG.printf("CLOSE: Renderer init is set to %d", Renderer_initted);
  if (!Renderer_initted)
    return;

  opengl_Close();
  Renderer_initted = 0;
}

void gpu_BindTexture(int handle, int map_type, int slot) {
  opengl_MakeBitmapCurrent(handle, map_type, slot);
  opengl_MakeWrapTypeCurrent(handle, map_type, slot);
  opengl_MakeFilterTypeCurrent(handle, map_type, slot);
}

void gpu_RenderPolygon(PosColorUVVertex *vData, uint32_t nv) {
  bool was_blend = (glIsEnabled(GL_BLEND) != GL_FALSE);
  if (gpu_state.cur_texture_quality > 0) {
    glEnable(GL_TEXTURE_2D);
  } else {
    glDisable(GL_TEXTURE_2D);
    // Flat path: avoid GL_DST_COLOR multiplicative blend (clears to black → all black)
    if (was_blend) glDisable(GL_BLEND);
  }
  glBegin(GL_TRIANGLE_FAN);
  for (uint32_t i = 0; i < nv; i++) {
    glColor4f(vData[i].color.r, vData[i].color.g, vData[i].color.b, vData[i].color.a);
    glTexCoord2f(vData[i].uv.s, vData[i].uv.t);
    glVertex3f(vData[i].pos.x(), vData[i].pos.y(), vData[i].pos.z());
  }
  glEnd();

  // Restore blend state if we disabled it above
  if (was_blend) glEnable(GL_BLEND);

  OpenGL_polys_drawn++;
  OpenGL_verts_processed += nv;
}

void gpu_RenderPolygonUV2(PosColorUV2Vertex *vData, uint32_t nv) {
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLE_FAN);
  for (uint32_t i = 0; i < nv; i++) {
    glColor4f(vData[i].color.r, vData[i].color.g, vData[i].color.b, vData[i].color.a);
    glTexCoord2f(vData[i].uv0.s, vData[i].uv0.t);
    glVertex3f(vData[i].pos.x(), vData[i].pos.y(), vData[i].pos.z());
  }
  glEnd();

  OpenGL_polys_drawn++;
  OpenGL_verts_processed += nv;
  CHECK_ERROR(10)
}

void rend_SetFlatColor(ddgr_color color) { gpu_state.cur_color = color; }

void rend_SetFogState(int8_t state) {}
void rend_SetFogBorders(float nearz, float farz) {}

void rend_SetRendererType(renderer_type state) {
  Renderer_type = state;
  LOG_DEBUG.printf("RendererType is set to %d.", state);
}

void rend_SetLighting(light_state state) {
  if (state == gpu_state.cur_light_state)
    return;
  glActiveTexture(GL_TEXTURE0_ARB + 0);
  OpenGL_sets_this_frame[4]++;
  gpu_state.cur_light_state = state;
  CHECK_ERROR(13)
}

void rend_SetColorModel(color_model state) { gpu_state.cur_color_model = state; }

void rend_SetTextureType(texture_type state) {
  if (state == TT_FLAT) {
    gpu_state.cur_texture_quality = 0;
  } else {
    gpu_state.cur_texture_quality = 2;
  }
  if (state == gpu_state.cur_texture_type)
    return;
  glActiveTexture(GL_TEXTURE0_ARB + 0);
  OpenGL_sets_this_frame[3]++;
  gpu_state.cur_texture_type = state;
  CHECK_ERROR(12)
}

void rend_StartFrame(int x1, int y1, int x2, int y2, int clear_flags) {
  if (clear_flags & RF_CLEAR_ZBUFFER) {
    glClear(GL_DEPTH_BUFFER_BIT);
  }
  gpu_state.clip_x1 = x1;
  gpu_state.clip_y1 = y1;
  gpu_state.clip_x2 = x2;
  gpu_state.clip_y2 = y2;
}

void rend_Flip() {
#ifndef RELEASE
  int i;
  RTP_INCRVALUE(texture_uploads, OpenGL_uploads);
  RTP_INCRVALUE(polys_drawn, OpenGL_polys_drawn);
  LOG_VERBOSE.printf("Uploads=%d    Polys=%d   Verts=%d", OpenGL_uploads, OpenGL_polys_drawn, OpenGL_verts_processed);
  LOG_VERBOSE.printf("Sets= 0:%d   1:%d   2:%d   3:%d", OpenGL_sets_this_frame[0], OpenGL_sets_this_frame[1],
                     OpenGL_sets_this_frame[2], OpenGL_sets_this_frame[3]);
  for (i = 0; i < 10; i++) {
    OpenGL_sets_this_frame[i] = 0;
  }
#endif

  gpu_last_frame_polys_drawn = OpenGL_polys_drawn;
  gpu_last_frame_verts_processed = OpenGL_verts_processed;
  gpu_last_uploaded = OpenGL_uploads;

  OpenGL_uploads = 0;
  OpenGL_polys_drawn = 0;
  OpenGL_verts_processed = 0;

  g_mesa_backend.PresentFrame();
}

void rend_EndFrame() {}

void rend_SetZBufferState(int8_t state) {
  if (state == gpu_state.cur_zbuffer_state)
    return;
  OpenGL_sets_this_frame[5]++;
  gpu_state.cur_zbuffer_state = state;
  if (state) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  CHECK_ERROR(14)
}

void rend_ClearScreen(ddgr_color color) {
  int r = (color >> 16 & 0xFF);
  int g = (color >> 8 & 0xFF);
  int b = (color & 0xFF);
  glClearColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rend_ClearZBuffer() { glClear(GL_DEPTH_BUFFER_BIT); }

void rend_ResetCache() {
  LOG_DEBUG << "Resetting texture cache!";
  opengl_ResetCache();
}

void rend_FillRect(ddgr_color color, int x1, int y1, int x2, int y2) {}

void rend_SetPixel(ddgr_color color, int x, int y) {}

ddgr_color rend_GetPixel(int x, int y) {
  uint8_t pixel[4] = {0};
  // GL origin is bottom-left, flip Y
  glReadPixels(x, gpu_state.screen_height - 1 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  return (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];
}

void rend_DrawLine(int x1, int y1, int x2, int y2) {
  // Pixel space: match rend_TransformSetToPassthru() ortho (0,w x 0,h, top=0)
  const float r = (float)((gpu_state.cur_color >> 16) & 0xFF) / 255.0f;
  const float g = (float)((gpu_state.cur_color >> 8) & 0xFF) / 255.0f;
  const float b = (float)(gpu_state.cur_color & 0xFF) / 255.0f;
  const float x1f = (float)(x1 + gpu_state.clip_x1);
  const float y1f = (float)(y1 + gpu_state.clip_y1);
  const float x2f = (float)(x2 + gpu_state.clip_x1);
  const float y2f = (float)(y2 + gpu_state.clip_y1);

  glDisable(GL_TEXTURE_2D);
  glColor4f(r, g, b, 1.0f);
  glLineWidth(2.0f);
  glBegin(GL_LINES);
  glVertex2f(x1f, y1f);
  glVertex2f(x2f, y2f);
  glEnd();
  glLineWidth(1.0f);
}

void rend_SetFogColor(ddgr_color color) {}

void rend_SetAlphaType(int8_t atype) {
  if (atype == gpu_state.cur_alpha_type)
    return;
  glActiveTexture(GL_TEXTURE0_ARB + 0);
  OpenGL_sets_this_frame[6]++;

  if (atype == AT_ALWAYS) {
    if (opengl_Blending_on) {
      glDisable(GL_BLEND);
      opengl_Blending_on = false;
    }
  } else {
    if (!opengl_Blending_on) {
      glEnable(GL_BLEND);
      opengl_Blending_on = true;
    }
  }

  switch (atype) {
  case AT_ALWAYS:
  case AT_TEXTURE:
    rend_SetAlphaValue(255);
    glBlendFunc(GL_ONE, GL_ZERO);
    break;
  case AT_CONSTANT:
  case AT_CONSTANT_TEXTURE:
  case AT_VERTEX:
  case AT_CONSTANT_TEXTURE_VERTEX:
  case AT_CONSTANT_VERTEX:
  case AT_TEXTURE_VERTEX:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  case AT_LIGHTMAP_BLEND:
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    break;
  case AT_SATURATE_TEXTURE:
  case AT_LIGHTMAP_BLEND_SATURATE:
  case AT_SATURATE_VERTEX:
  case AT_SATURATE_CONSTANT_VERTEX:
  case AT_SATURATE_TEXTURE_VERTEX:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    break;
  default:
    break;
  }
  gpu_state.cur_alpha_type = atype;
  gpu_Alpha_multiplier = rend_GetAlphaMultiplier();
  CHECK_ERROR(15)
}

void rend_DrawSpecialLine(g3Point *p0, g3Point *p1) {}

std::unique_ptr<NewBitmap> rend_Screenshot() {
  return g_mesa_backend.Screenshot(gpu_state.screen_width, gpu_state.screen_height);
}

int mesa_GetFramebufferWidth() { return gpu_state.screen_width; }

int mesa_GetFramebufferHeight() { return gpu_state.screen_height; }

void mesa_SetLogicalSize(int w, int h) {
  gpu_state.screen_width = w;
  gpu_state.screen_height = h;
}
