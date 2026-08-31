/*
 * Stub implementations for D3 dependencies
 * Compiled as C to ensure proper symbol names (no C++ mangling)
 */

#include <stdio.h>
#include <stdarg.h>

void AssertionFailed(const char *expr, const char *file, int line) {
  fprintf(stderr, "AssertionFailed: %s (%s:%d)\n", expr, file, line);
}

int Int3MessageBox(const char *msg, int flags) {
  fprintf(stderr, "Int3MessageBox: %s (flags=%d)\n", msg, flags);
  return 0;
}

void Debug_break(void) { /* no-op for tests */ }

// Random number stub
int ps_rand(void) { return 0; }

// Paging stubs
int paged_in_count = 0;
int paged_in_num = 0;

void DebugBreak_callback_stop(void) { /* no-op */ }

void DebugBreak_callback_resume(void) { /* no-op */ }

int FindArg(const char *which) { return 0; }

void ddio_SplitPath(const char *srcPath, char *path, char *filename, char *ext) {
  if (path)
    *path = '\0';
  if (filename)
    *filename = '\0';
  if (ext)
    *ext = '\0';

  if (!srcPath || !*srcPath)
    return;

  const char *lastSlash = __builtin_strrchr(srcPath, '/');
  const char *lastDot = __builtin_strrchr(srcPath, '.');

  if (filename) {
    if (lastSlash) {
      __builtin_strncpy(filename, lastSlash + 1, 255);
      filename[255] = '\0';
    } else {
      __builtin_strncpy(filename, srcPath, 255);
      filename[255] = '\0';
    }
    if (lastDot && lastDot > (lastSlash ? lastSlash : srcPath)) {
      __builtin_strncpy(ext, lastDot + 1, 255);
      ext[255] = '\0';
    }
  }

  if (path) {
    if (lastSlash) {
      size_t len = lastSlash - srcPath;
      __builtin_strncpy(path, srcPath, len < 256 ? len : 255);
      path[len < 256 ? len : 255] = '\0';
    } else {
      *path = '\0';
    }
  }
}

// Error function stub
void Error(const char *fmt, ...) {
  fprintf(stderr, "Error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

// stricmp stub (case-insensitive compare)
int stricmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? (*s1 + 32) : *s1;
    char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? (*s2 + 32) : *s2;
    if (c1 != c2)
      return c1 - c2;
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

// CFile error stub (normally in cfile.cpp but needed for real cfile tests)
void ThrowCFileError(int type, void *file, const char *msg) {
  (void)type;
  (void)file;
  fprintf(stderr, "CFile Error: %s\n", msg ? msg : "unknown");
}
