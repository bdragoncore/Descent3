/*
 * D3 Render Test Base - C++ Base Class for GL Rendering Tests
 * Provides reusable infrastructure for D3 rendering tests including grtext
 *
 * Uses OffscreenGL for EGL pbuffer context creation (Mesa llvmpipe).
 * MesaOpenGL renderer attaches to the already-current context.
 * No display server (X11/Wayland) required.
 */

#ifndef RENDER_TEST_BASE_H
#define RENDER_TEST_BASE_H

#include "core_init.h"
#include "offscreen_gl.h"

#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <string>
#include <memory>

#include <gtest/gtest.h>

#ifdef __linux__
#include <GL/gl.h>
#endif

// Default render output directory — overridden by RENDER_OUTPUT_DIR define
#ifndef RENDER_OUTPUT_DIR
#define RENDER_OUTPUT_DIR "build/tests/render_output"
#endif

class D3RenderTestBase : public ::testing::Test {
public:
  D3RenderTestBase(int width = 640, int height = 480, const char *output_dir = RENDER_OUTPUT_DIR);
  virtual ~D3RenderTestBase();

  virtual void SetUp();
  virtual void TearDown();
  virtual void RenderFrame() {}

  uint8_t *GetFramebuffer() { return framebuffer_; }
  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }

  // Font/text rendering functions
  bool LoadFont(const char *fontname);
  void UnloadFont();
  void RenderText(int x, int y, const char *text);
  void RenderTextf(int x, int y, const char *fmt, ...);
  void SetTextColor(uint8_t r, uint8_t g, uint8_t b);
  void SetTextColorGreen() { SetTextColor(0, 255, 0); }
  void SetTextColorRed() { SetTextColor(255, 0, 0); }
  void SetTextColorYellow() { SetTextColor(255, 255, 0); }
  void SetTextColorBlue() { SetTextColor(0, 0, 255); }
  void SetTextColorWhite() { SetTextColor(255, 255, 255); }
  void SetTextAlpha(uint8_t alpha);
  void SetFontScale(float scale);

  // Frame and rendering functions (virtual so game suite can wrap with g3_StartFrame/g3_EndFrame)
  virtual void BeginFrame();
  virtual void EndFrame();
  bool SavePNG(const char *filename);
  void ReadPixels();
  bool AddSearchPath(const char *path);
  const char *GetError() const { return last_error_.c_str(); }
  bool IsInitialized() const { return initialized_; }
  const std::string &GetOutputDir() const { return output_dir_; }

  // Render test name overlay using grtext
  void RenderTestNameOverlay();

protected:
  int width_;
  int height_;
  uint8_t *framebuffer_;
  int font_handle_;

private:
  std::string output_dir_;
  std::string last_error_;
  bool initialized_;
  bool renderer_initialized_;
  bool core_initialized_;
  bool egl_initialized_;
  bool font_loaded_;
  uint8_t text_color_[3];
  uint8_t text_alpha_;
  float font_scale_;
};

#endif
