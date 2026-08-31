/*
 * D3 Render Tests - Main entry point; connects all render test suites to the gtest runner.
 *
 * Architecture:
 *   OffscreenGL   — owns the EGL pbuffer context (offscreen_gl.h)
 *   MesaOpenGL    — D3 renderer backend that attaches to the current context
 *   D3RenderTestBase — GTest base class wiring everything together
 *
 * Suites:
 *   d3_render_tests_game  — D3 game rendering (rend_*, polymodel; no direct GL)
 *
 * All rendered PNGs are written to build/tests/render_output.
 * Env vars are hardcoded for headless rendering.
 */

#include <cstdlib>
#include <cstdio>

// Hardcode env vars for headless Mesa llvmpipe rendering
static void setupEnvironment() {
    setenv("GALLIUM_DRIVER", "llvmpipe", 1);
    setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
    setenv("MESA_GL_VERSION_OVERRIDE", "3.3", 1);
    setenv("MESA_GLSL_VERSION_OVERRIDE", "330", 1);
    setenv("SDL_VIDEODRIVER", "offscreen", 1);
}

#include "render_test_base.h"
#include "offscreen_gl.h"
#include <gtest/gtest.h>

int main(int argc, char **argv) {
    setupEnvironment();

    printf("Starting D3 Render Tests (consolidated)...\n");
    fflush(stdout);

    ::testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();

    OffscreenGL::Shutdown();

    return result;
}
