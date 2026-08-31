/* Minimal stubs for D3 platform dependencies */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __linux__
#include <GL/gl.h>
#endif

/* Renderer types - must match D3 headers */
typedef unsigned int ddgr_color;
typedef unsigned char ubyte;
typedef signed char sbyte;

typedef enum {
  TT_FLAT, TT_LINEAR, TT_PERSPECTIVE, TT_LINEAR_SPECIAL, TT_PERSPECTIVE_SPECIAL,
} texture_type;

typedef enum {
  LS_NONE, LS_GOURAUD, LS_PHONG, LS_FLAT_GOURAUD
} light_state;

typedef enum {
  CM_MONO, CM_RGB,
} color_model;

/* Error handling - C linkage */
extern "C" {

void (*DebugBreak_callback_stop)(void) = NULL;
void (*DebugBreak_callback_resume)(void) = NULL;

void AssertionFailed(const char* expstr, const char* file, int line) {
    fprintf(stderr, "Assertion failed: %s at %s:%d\n", expstr, file, line);
}

void Int3MessageBox(const char* file, int line) {
    fprintf(stderr, "Int3 at %s:%d\n", file, line);
}

void Error(const char* fmt, ...) {
    va_list args;
    fprintf(stderr, "Error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/* ddebug stubs - both C and C++ linkage needed */
extern "C" int FindArg(const char* arg);
int FindArg(const char* arg) { 
    (void)arg; 
    return 0; 
}

char** GameArgs = NULL;

/* Bitmap stubs */
int ps_rand(void) { return rand(); }
int paged_in_count = 0;
int paged_in_num = 0;

} // extern "C"

/* ddio_internal stubs - C++ linkage */
struct ddio_init_info;
int ddio_InternalKeyInit(ddio_init_info* info) { 
    (void)info; 
    return 1; 
}

void ddio_InternalKeyClose() {}
void ddio_InternalResetKey(unsigned char k) { (void)k; }
float ddio_InternalKeyDownTime(unsigned char k) { (void)k; return 0.0f; }

/* ddio file path stubs - C++ linkage */
void ddio_SplitPath(const char* src, char* path, char* filename, char* ext) {
    const char* last_slash = strrchr(src, '/');
    const char* last_dot = strrchr(src, '.');
    
    if (path) path[0] = '\0';
    if (filename) filename[0] = '\0';
    if (ext) ext[0] = '\0';
    if (!src) return;
    
    if (last_slash) {
        if (path) {
            size_t len = last_slash - src + 1;
            strncpy(path, src, len);
            path[len] = '\0';
        }
        src = last_slash + 1;
    }
    
    if (last_dot && last_dot > src) {
        if (filename) {
            size_t len = last_dot - src;
            strncpy(filename, src, len);
            filename[len] = '\0';
        }
        if (ext) strcpy(ext, last_dot);
    } else {
        if (filename) strcpy(filename, src);
    }
}

void ddio_MakePath(char* dest, const char* dir, const char* filename, ...) {
    if (!dest) return;
    dest[0] = '\0';
    if (dir) strcat(dest, dir);
    if (filename) strcat(dest, filename);
}

int ddio_GetFileLength(FILE* fp) {
    if (!fp) return 0;
    long pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, pos, SEEK_SET);
    return (int)len;
}

void ddio_GetFullPath(char* dest, const char* src) {
    if (dest && src) strcpy(dest, src);
}

void ddio_CopyFileTime(char* dest, const char* src) { 
    (void)dest; (void)src; 
}

int ddio_FileDiff(const char* a, const char* b) { 
    (void)a; (void)b; 
    return 0; 
}

/* Glob stubs - C++ linkage */
int PSGlobHasPattern(char* str) { 
    (void)str; 
    return 0; 
}

int PSGlobMatch(char* pattern, char* str, int foo, int bar) { 
    (void)pattern; (void)str; (void)foo; (void)bar; 
    return 0; 
}

/* Renderer stubs - C++ linkage */
void rend_SetFlatColor(ddgr_color col) { 
#ifdef __linux__
    int r = (col >> 16) & 0xFF;
    int g = (col >> 8) & 0xFF;
    int b = col & 0xFF;
    glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
#endif
    (void)col;
}

void rend_SetAlphaValue(ubyte val) { (void)val; }
void rend_SetFiltering(sbyte state) { (void)state; }
void rend_SetZBufferState(sbyte state) { 
#ifdef __linux__
    if (state) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
#endif
    (void)state;
}

void rend_SetTextureType(texture_type type) { (void)type; }
void rend_SetOverlayType(ubyte type) { (void)type; }
void rend_SetLighting(light_state state) { (void)state; }
void rend_SetColorModel(color_model model) { (void)model; }
void rend_SetAlphaType(sbyte type) { (void)type; }

void rend_DrawScaledBitmap(int x1, int y1, int x2, int y2, int bm, 
                           float u0, float v0, float u1, float v1, 
                           int color, float* alphas) {
    (void)x1; (void)y1; (void)x2; (void)y2; (void)bm;
    (void)u0; (void)v0; (void)u1; (void)v1;
    (void)color; (void)alphas;
}
