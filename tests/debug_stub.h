/*
 * Debug stub for renderer tests - provides no-op implementations
 * of debug functions to avoid linking issues
 */
#ifndef DEBUG_STUB_H
#define DEBUG_STUB_H

#include <cstdio>

#define mprintf(x, ...) fprintf(stderr, __VA_ARGS__)

#define ASSERT(x) ((void)0)
#define Int3() ((void)0)
#define DEBUG_BREAK() ((void)0)

#define ASSERTION_FAILED_MSG(msg) do { fprintf(stderr, "ASSERTION FAILED: %s\n", msg); } while(0)

#endif
