/*
 * D3 Render Test Base - C++ Base Class Implementation
 * Provides reusable infrastructure for D3 rendering tests using grtext
 *
 * Uses OffscreenGL for EGL pbuffer context (Mesa llvmpipe).
 * MesaOpenGL renderer attaches to the already-current context.
 */

#include "render_test_base.h"

#include <cstdio>
#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <iomanip>

// stb_image_write for PNG output (implementation compiled here once)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "md5/md5.h"
#include "grtext.h"
#include "renderer.h"
#include "MesaOpenGL.h"

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

D3RenderTestBase::D3RenderTestBase(int width, int height, const char *output_dir)
    : width_(width), height_(height), font_handle_(-1), output_dir_(output_dir), initialized_(false),
      renderer_initialized_(false), core_initialized_(false), egl_initialized_(false), font_loaded_(false),
      text_alpha_(255), font_scale_(1.0f) {
  text_color_[0] = text_color_[1] = text_color_[2] = 255;
  framebuffer_ = new uint8_t[width * height * 4]();
}

D3RenderTestBase::~D3RenderTestBase() {
  TearDown();
  delete[] framebuffer_;
}

// ---------------------------------------------------------------------------
// SetUp / TearDown  (follows mandatory init order from testing plan)
// ---------------------------------------------------------------------------

// Static flags — subsystems are initialized once and persist across all tests.
// This avoids double-init issues with bm_InitBitmaps, opengl_InitCache, etc.
static bool s_subsystems_initialized = false;

// Maximum pbuffer size so any test's viewport (width_, height_) fits. Prevents
// viewport larger than surface when the first run test has smaller dimensions.
static constexpr int kMaxRenderTestWidth = 800;
static constexpr int kMaxRenderTestHeight = 600;

void D3RenderTestBase::SetUp() {
  if (initialized_)
    return;

  if (!s_subsystems_initialized) {
    // 1. OffscreenGL — EGL pbuffer + llvmpipe context current (fixed max size
    //    so later tests with larger viewports are not clipped)
    if (!OffscreenGL::IsReady()) {
      if (!OffscreenGL::Init(kMaxRenderTestWidth, kMaxRenderTestHeight)) {
        last_error_ = "OffscreenGL::Init failed";
        return;
      }
    }

    // 2. MEM + BITMAP — must be initialized before rend_Init so the texture
    //    cache (opengl_InitCache) can access GameBitmaps safely.
    if (!D3Core_Initialize((D3InitFlags)(D3_INIT_MEM | D3_INIT_BITMAP))) {
      last_error_ = "D3Core_Initialize(MEM|BITMAP) failed: " + std::string(D3Core_GetError());
      return;
    }

    // 3. rend_Init — attaches to existing EGL context (use same max size as pbuffer)
    renderer_preferred_state pref;
    memset(&pref, 0, sizeof(pref));
    pref.width = kMaxRenderTestWidth;
    pref.height = kMaxRenderTestHeight;
    pref.bit_depth = 32;
    pref.filtering = 1;

    int result = rend_Init(RENDERER_MESA, nullptr, &pref);
    if (!result) {
      last_error_ = "rend_Init failed";
      return;
    }

    // 4. grtext — requires BITMAP (already done) + renderer (just done)
    if (!D3Core_Initialize((D3InitFlags)(D3_INIT_GRTEXT))) {
      last_error_ = "D3Core_Initialize(GRTEXT) failed: " + std::string(D3Core_GetError());
      return;
    }

    // 5. Load default font for text rendering overlays
    // Try to find lohud.fnt in common locations
    AddSearchPath((std::string(getenv("HOME")) + "/Descent3").c_str());
    AddSearchPath("/usr/share/descent3");
    AddSearchPath("."); // Try current directory as fallback
    // Note: Font loading will be attempted on first text render if not already loaded

    s_subsystems_initialized = true;
  }

  egl_initialized_ = true;
  renderer_initialized_ = true;
  core_initialized_ = true;

  // Ensure output directory exists
  std::filesystem::create_directories(output_dir_);

  initialized_ = true;
}

void D3RenderTestBase::TearDown() {
  if (!initialized_)
    return;

  if (font_handle_ >= 0) {
    D3Core_UnloadFont(font_handle_);
    font_handle_ = -1;
    font_loaded_ = false;
  }

  // Do NOT shut down subsystems per-test — they persist across all tests.
  // OffscreenGL, renderer, bitmap, and grtext are initialized once.
  // Cleanup happens in main() via OffscreenGL::Shutdown().

  initialized_ = false;
}

// ---------------------------------------------------------------------------
// Font / text
// ---------------------------------------------------------------------------

bool D3RenderTestBase::LoadFont(const char *fontname) {
  if (!fontname) {
    last_error_ = "LoadFont: null fontname";
    return false;
  }
  int handle = D3Core_LoadFont(fontname);
  if (handle < 0) {
    last_error_ = std::string("Failed to load font: ") + fontname;
    return false;
  }
  font_handle_ = handle;
  font_loaded_ = true;
  return true;
}

void D3RenderTestBase::UnloadFont() {
  if (font_handle_ >= 0) {
    D3Core_UnloadFont(font_handle_);
    font_handle_ = -1;
    font_loaded_ = false;
  }
}

void D3RenderTestBase::RenderText(int x, int y, const char *text) {
  if (!text)
    return;
  grtext_Puts(x, y, const_cast<char *>(text));
}

void D3RenderTestBase::RenderTextf(int x, int y, const char *fmt, ...) {
  if (!fmt)
    return;
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  grtext_Puts(x, y, buffer);
}

void D3RenderTestBase::SetTextColor(uint8_t r, uint8_t g, uint8_t b) {
  ddgr_color col = (r << 16) | (g << 8) | b;
  grtext_SetColor(col);
}

void D3RenderTestBase::SetTextAlpha(uint8_t alpha) { grtext_SetAlpha(alpha); }
void D3RenderTestBase::SetFontScale(float scale) { grtext_SetFontScale(scale); }

// ---------------------------------------------------------------------------
// Frame helpers
// ---------------------------------------------------------------------------

void D3RenderTestBase::BeginFrame() {
  memset(framebuffer_, 0, width_ * height_ * 4);
  glViewport(0, 0, width_, height_);
  glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  // Set grtext clipping parameters to the full viewport so text isn't clipped
  grtext_SetParameters(0, 0, width_, height_, 4);
}

void D3RenderTestBase::EndFrame() { grtext_Flush(); }

void D3RenderTestBase::ReadPixels() {
  // Read pixels at this test's resolution (may differ from pbuffer size)
  glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer_);
  // Y-flip so row 0 is top (PNG convention)
  int stride = width_ * 4;
  uint8_t *tmp = new uint8_t[stride];
  for (int y = 0; y < height_ / 2; y++) {
    uint8_t *top = framebuffer_ + y * stride;
    uint8_t *bot = framebuffer_ + (height_ - 1 - y) * stride;
    memcpy(tmp, top, stride);
    memcpy(top, bot, stride);
    memcpy(bot, tmp, stride);
  }
  delete[] tmp;
}

// ---------------------------------------------------------------------------
// PNG output via stb_image_write
// ---------------------------------------------------------------------------

bool D3RenderTestBase::SavePNG(const char *filename) {
  if (!filename) {
    last_error_ = "SavePNG: null filename";
    return false;
  }

  // Get current test info from Google Test
  const ::testing::TestInfo *test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_prefix = "Test";
  if (test_info) {
    // Include suite name and test name in filename for unique identification
    test_prefix += test_info->test_suite_name();
    test_prefix += "_";
    test_prefix += test_info->name();
    test_prefix += "_";
  }

  // Build the final filename: "Test{Suite}_{Test}_{filename}.png"
  std::string name = test_prefix;
  name += filename;
  if (name.length() < 4 || name.substr(name.length() - 4) != ".png") {
    name += ".png";
  }

  std::string fullpath = output_dir_ + "/" + name;

  // Ensure output directory exists
  std::filesystem::create_directories(output_dir_);

  int ok = stbi_write_png(fullpath.c_str(), width_, height_, 4, framebuffer_, width_ * 4);
  if (!ok) {
    last_error_ = std::string("stbi_write_png failed: ") + fullpath;
    return false;
  }

  // Compute MD5 hash of the framebuffer data
  MD5 hasher;
  hasher.update(framebuffer_, width_ * height_ * 4);
  auto digest = hasher.digest();

  // Write hash to .md5 file
  std::string md5path = fullpath.substr(0, fullpath.length() - 4) + ".md5";
  std::ofstream md5file(md5path);
  if (!md5file) {
    last_error_ = std::string("Failed to write MD5 file: ") + md5path;
    return false;
  }
  for (int i = 0; i < 16; i++) {
    md5file << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(digest[i]);
  }
  md5file << "\n";

  return true;
}

// ---------------------------------------------------------------------------
// Misc
// ---------------------------------------------------------------------------

bool D3RenderTestBase::AddSearchPath(const char *path) { return D3Core_AddSearchPath(path) != 0; }

// ---------------------------------------------------------------------------
// Test name overlay
// ---------------------------------------------------------------------------

void D3RenderTestBase::RenderTestNameOverlay() {
  // Get current test name from gtest
  const ::testing::TestInfo *test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  if (!test_info)
    return;

  // Load font if not already loaded (base class handles this internally)
  if (font_handle_ < 0) {
    if (!LoadFont("hihud.fnt")) {
      // Font not available, skip overlay
      return;
    }
  }

  const char *test_name = test_info->name();
  const char *suite_name = test_info->test_suite_name();

  // Save current text state
  uint8_t saved_color[3] = {text_color_[0], text_color_[1], text_color_[2]};
  uint8_t saved_alpha = text_alpha_;
  float saved_scale = font_scale_;

  // Save current GL matrices and state
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_TEST); // Disable depth test for 2D text

  // Setup ortho projection for 2D text rendering
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width_, height_, 0, -1, 1); // Flip Y so (0,0) is top-left
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set overlay text style - bright cyan for visibility
  SetTextColor(0, 255, 255);
  SetTextAlpha(255);
  SetFontScale(1.0f);

  // Build test name string
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%s.%s", suite_name, test_name);

  // Render at top-left corner with small margin
  RenderText(10, 10, buffer);
  EndFrame();

  // Restore GL matrices and state
  if (depth_test)
    glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Restore saved text state
  SetTextColor(saved_color[0], saved_color[1], saved_color[2]);
  SetTextAlpha(saved_alpha);
  SetFontScale(saved_scale);
}
