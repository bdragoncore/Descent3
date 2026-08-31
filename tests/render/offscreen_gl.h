/*
 * OffscreenGL - EGL Pbuffer Context Wrapper for Headless Rendering
 *
 * Creates an EGL pbuffer surface using Mesa's surfaceless platform
 * with llvmpipe software rasterizer. No display server required.
 *
 * Usage:
 *   OffscreenGL::Init(640, 480);   // before any GL or rend_Init calls
 *   // ... render ...
 *   OffscreenGL::ReadPixels(buf);   // RGBA, top-left origin
 *   OffscreenGL::Shutdown();
 */

#ifndef OFFSCREEN_GL_H
#define OFFSCREEN_GL_H

#include <cstdint>

#include <EGL/egl.h>
#include <EGL/eglext.h>

class OffscreenGL {
public:
  /// Create EGL display + pbuffer surface + GL 3.3 compat context.
  /// Sets llvmpipe env vars, makes context current.
  /// Returns true on success.
  static bool Init(int width, int height);

  /// Tear down EGL resources (context, surface, display).
  static void Shutdown();

  /// Read the colour buffer into @p dst (width*height*4 bytes, RGBA).
  /// The image is flipped so that row 0 is the TOP (PNG convention).
  static void ReadPixels(uint8_t *dst);

  static int  GetWidth()   { return width_;  }
  static int  GetHeight()  { return height_; }
  static bool IsReady()    { return ready_;  }

  static EGLDisplay GetDisplay() { return display_; }
  static EGLSurface GetSurface() { return surface_; }
  static EGLContext GetContext() { return context_; }

private:
  static EGLDisplay display_;
  static EGLSurface surface_;
  static EGLContext  context_;
  static int  width_;
  static int  height_;
  static bool ready_;
};

#endif /* OFFSCREEN_GL_H */
