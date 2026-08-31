/*
 * OffscreenGL - EGL Pbuffer Context Wrapper Implementation
 *
 * Creates a headless OpenGL 3.3 compatibility context using Mesa llvmpipe.
 * The context is made current so that subsequent GL / rend_Init calls
 * attach to it automatically.
 */

#include "offscreen_gl.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

// Static member definitions
EGLDisplay OffscreenGL::display_ = EGL_NO_DISPLAY;
EGLSurface OffscreenGL::surface_ = EGL_NO_SURFACE;
EGLContext OffscreenGL::context_ = EGL_NO_CONTEXT;
int OffscreenGL::width_ = 0;
int OffscreenGL::height_ = 0;
bool OffscreenGL::ready_ = false;

bool OffscreenGL::Init(int width, int height) {
  if (ready_) {
    fprintf(stderr, "OffscreenGL: already initialised (%dx%d)\n", width_, height_);
    return true;
  }

  // Force Mesa software rendering
  setenv("GALLIUM_DRIVER", "llvmpipe", 1);
  setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
  setenv("MESA_GL_VERSION_OVERRIDE", "3.3", 1);
  setenv("MESA_GLSL_VERSION_OVERRIDE", "330", 1);

  // --- EGL display via surfaceless platform ---
  auto eglGetPlatformDisplayEXT =
      reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));

  if (eglGetPlatformDisplayEXT) {
    display_ = eglGetPlatformDisplayEXT(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
  }
  if (display_ == EGL_NO_DISPLAY) {
    // Fallback
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  }
  if (display_ == EGL_NO_DISPLAY) {
    fprintf(stderr, "OffscreenGL: no EGL display\n");
    return false;
  }

  EGLint major, minor;
  if (!eglInitialize(display_, &major, &minor)) {
    fprintf(stderr, "OffscreenGL: eglInitialize failed\n");
    return false;
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    fprintf(stderr, "OffscreenGL: eglBindAPI(EGL_OPENGL_API) failed\n");
    return false;
  }

  // --- Choose config ---
  const EGLint cfg_attr[] = {EGL_SURFACE_TYPE,
                             EGL_PBUFFER_BIT,
                             EGL_RENDERABLE_TYPE,
                             EGL_OPENGL_BIT,
                             EGL_RED_SIZE,
                             8,
                             EGL_GREEN_SIZE,
                             8,
                             EGL_BLUE_SIZE,
                             8,
                             EGL_ALPHA_SIZE,
                             8,
                             EGL_DEPTH_SIZE,
                             16,
                             EGL_NONE};
  EGLConfig config;
  EGLint num_configs = 0;
  if (!eglChooseConfig(display_, cfg_attr, &config, 1, &num_configs) || num_configs == 0) {
    fprintf(stderr, "OffscreenGL: eglChooseConfig failed\n");
    return false;
  }

  // --- Pbuffer surface ---
  const EGLint pb_attr[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
  surface_ = eglCreatePbufferSurface(display_, config, pb_attr);
  if (surface_ == EGL_NO_SURFACE) {
    fprintf(stderr, "OffscreenGL: eglCreatePbufferSurface failed\n");
    return false;
  }

  // --- GL 3.3 compatibility context ---
  const EGLint ctx_attr[] = {EGL_CONTEXT_MAJOR_VERSION,
                             3,
                             EGL_CONTEXT_MINOR_VERSION,
                             3,
                             EGL_CONTEXT_OPENGL_PROFILE_MASK,
                             EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
                             EGL_NONE};
  context_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, ctx_attr);
  if (context_ == EGL_NO_CONTEXT) {
    fprintf(stderr, "OffscreenGL: eglCreateContext failed\n");
    return false;
  }

  if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
    fprintf(stderr, "OffscreenGL: eglMakeCurrent failed\n");
    return false;
  }

  width_ = width;
  height_ = height;
  ready_ = true;

  fprintf(stdout, "OffscreenGL: llvmpipe ready (%dx%d EGL %d.%d)\n", width, height, major, minor);
  fprintf(stdout, "  GL Vendor:   %s\n", glGetString(GL_VENDOR));
  fprintf(stdout, "  GL Renderer: %s\n", glGetString(GL_RENDERER));
  fprintf(stdout, "  GL Version:  %s\n", glGetString(GL_VERSION));
  fflush(stdout);

  return true;
}

void OffscreenGL::Shutdown() {
  if (!ready_)
    return;

  eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

  if (context_ != EGL_NO_CONTEXT) {
    eglDestroyContext(display_, context_);
    context_ = EGL_NO_CONTEXT;
  }
  if (surface_ != EGL_NO_SURFACE) {
    eglDestroySurface(display_, surface_);
    surface_ = EGL_NO_SURFACE;
  }
  if (display_ != EGL_NO_DISPLAY) {
    eglTerminate(display_);
    display_ = EGL_NO_DISPLAY;
  }

  width_ = 0;
  height_ = 0;
  ready_ = false;
}

void OffscreenGL::ReadPixels(uint8_t *dst) {
  if (!ready_ || !dst)
    return;

  // Read from GL framebuffer (Y=0 at bottom)
  glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, dst);

  // Flip vertically so row 0 = top (PNG convention)
  const int row_bytes = width_ * 4;
  auto *tmp = new uint8_t[row_bytes];
  for (int y = 0; y < height_ / 2; ++y) {
    uint8_t *top = dst + y * row_bytes;
    uint8_t *bottom = dst + (height_ - 1 - y) * row_bytes;
    memcpy(tmp, top, row_bytes);
    memcpy(top, bottom, row_bytes);
    memcpy(bottom, tmp, row_bytes);
  }
  delete[] tmp;
}
