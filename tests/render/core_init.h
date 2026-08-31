/*
 * D3 Render Test Base - Core Initialization Header
 * Provides C API for initializing D3 subsystems needed for rendering tests
 */

#ifndef CORE_INIT_H
#define CORE_INIT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Core init flags */
typedef enum {
  D3_INIT_MEM = 1 << 0,    /* Memory manager */
  D3_INIT_ERROR = 1 << 1,  /* Error system */
  D3_INIT_TIMER = 1 << 2,  /* Timer */
  D3_INIT_DDIO = 1 << 3,   /* I/O abstraction */
  D3_INIT_CFILE = 1 << 4,  /* File system */
  D3_INIT_BITMAP = 1 << 5, /* Texture system */
  D3_INIT_3D = 1 << 6,     /* 3D system (matrices) */
  D3_INIT_RENDER = 1 << 7, /* Renderer */
  D3_INIT_GRTEXT = 1 << 8, /* Text rendering */

  D3_INIT_MINIMAL = D3_INIT_MEM | D3_INIT_ERROR | D3_INIT_DDIO | D3_INIT_CFILE,
  D3_INIT_RENDERING = D3_INIT_MINIMAL | D3_INIT_3D | D3_INIT_BITMAP | D3_INIT_RENDER | D3_INIT_GRTEXT,
  D3_INIT_ALL = 0xFFFFFFFF
} D3InitFlags;

/* Initialize D3 core subsystems
 * Call with flags OR'd together (e.g., D3_INIT_RENDERING)
 * Returns true on success, false on failure
 */
bool D3Core_Initialize(D3InitFlags flags);

/* Shutdown D3 core subsystems (reverse order of init) */
void D3Core_Shutdown(void);

/* Get last error message */
const char *D3Core_GetError(void);

/* Check if core is initialized */
bool D3Core_IsInitialized(void);

/* Add search path for HOG files */
int D3Core_AddSearchPath(const char *path);

/* Load font from HOG file
 * Returns font handle >= 0 on success, -1 on failure
 */
int D3Core_LoadFont(const char *fontname);

/* Unload font */
void D3Core_UnloadFont(int font_handle);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INIT_H */
