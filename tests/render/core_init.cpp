/*
 * D3 Render Test Base - Core Initialization Implementation
 * Provides base class functionality for GL/rendering tests
 */

#include "core_init.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>

#include "mem.h"
#include "grtext.h"
#include "cfile.h"
#include "bitmap.h"

static char g_error_msg[256] = {0};
static bool g_initialized = false;
static int g_init_flags = 0;
static int g_font_handle = -1;

const char *D3Core_GetError(void) { return g_error_msg; }

bool D3Core_IsInitialized(void) { return g_initialized; }

int D3Core_AddSearchPath(const char *path) {
  if (!path) {
    strcpy(g_error_msg, "D3Core_AddSearchPath: null path");
    return 0;
  }

  // Add as both a base directory (for cf_LocatePath / cf_OpenLibrary) and search path
  cf_AddBaseDirectory(path);
  bool result = cf_SetSearchPath(path);
  if (!result) {
    snprintf(g_error_msg, sizeof(g_error_msg), "Failed to add search path: %s", path);
    return 0;
  }

  // Open any HOG files found in the directory so cfopen can find files inside them
  std::filesystem::path dir(path);
  for (auto const &entry : std::filesystem::directory_iterator(dir)) {
    if (entry.is_regular_file()) {
      std::string ext = entry.path().extension().string();
      for (auto &c : ext)
        c = tolower(c);
      if (ext == ".hog") {
        std::string fname = entry.path().filename().string();
        cf_OpenLibrary(fname.c_str());
      }
    }
  }

  return 1;
}

int D3Core_LoadFont(const char *fontname) {
  if (!fontname) {
    strcpy(g_error_msg, "D3Core_LoadFont: null fontname");
    return -1;
  }

  if (g_font_handle >= 0) {
    grfont_Free(g_font_handle);
    g_font_handle = -1;
  }

  g_font_handle = grfont_Load(fontname);
  if (g_font_handle < 0) {
    snprintf(g_error_msg, sizeof(g_error_msg), "Failed to load font: %s", fontname);
    return -1;
  }

  grtext_SetFont(g_font_handle);
  return g_font_handle;
}

void D3Core_UnloadFont(int font_handle) {
  if (font_handle >= 0) {
    grfont_Free(font_handle);
    if (font_handle == g_font_handle) {
      g_font_handle = -1;
    }
  }
}

bool D3Core_Initialize(D3InitFlags flags) {
  g_error_msg[0] = '\0';

  // Only initialize subsystems that haven't been initialized yet
  int new_flags = flags & ~g_init_flags;

  if (new_flags & D3_INIT_MEM) {
    mem_Init();
  }

  if (new_flags & D3_INIT_BITMAP) {
    bm_InitBitmaps();
  }

  if (new_flags & D3_INIT_GRTEXT) {
    grfont_Reset(); // Must be called before grfont_Load — sets Font_init = true
    grtext_Init();
  }

  g_init_flags |= flags;
  g_initialized = true;
  return true;
}

void D3Core_Shutdown(void) {
  if (!g_initialized) {
    return;
  }

  if (g_font_handle >= 0) {
    grfont_Free(g_font_handle);
    g_font_handle = -1;
  }

  grtext_Reset();

  g_initialized = false;
  g_init_flags = 0;
}
