/*
 * D3 Render Tests - Main entry point; connects all render test suites to the gtest runner.
 *
 * Architecture:
 *   OffscreenGL   — owns the EGL pbuffer context (offscreen_gl.h)
 *   MesaOpenGL    — D3 renderer backend that attaches to the current context
 *   D3RenderTestBase — GTest base class wiring everything together
 *
 * Suites (one executable per suite):
 *   d3_render_tests_egl   — EGL tests (context, clear, depth, SavePNG)
 *   d3_render_tests_gl    — Standard GL tests (raw OpenGL: quads, cube, perspective)
 *   d3_render_tests_game  — D3 game rendering (rend_*, polymodel; no direct GL)
 *   d3_render_tests_hud   — D3 game rendering (HUD, text; requires d3.hog)
 *
 * All rendered PNGs are written to build/tests/render_output.
 * Env vars are hardcoded for headless rendering.
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>

// Hardcode env vars for headless Mesa llvmpipe rendering
static void setupEnvironment() {
    setenv("GALLIUM_DRIVER", "llvmpipe", 1);
    setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
    setenv("MESA_GL_VERSION_OVERRIDE", "3.3", 1);
    setenv("MESA_GLSL_VERSION_OVERRIDE", "330", 1);
    setenv("SDL_VIDEODRIVER", "offscreen", 1);
}

// Remove --skip-trace from argv so gtest doesn't see an unknown flag.
// All four render test executables (egl, gl, game, hud) use this main; rebuild
// all of them after changing this so --skip-trace is honored everywhere.
// Returns true if the flag was present.
static bool stripSkipTraceFlag(int* argc, char** argv) {
    int n = *argc;
    for (int i = 1; i < n; i++) {
        if (strcmp(argv[i], "--skip-trace") == 0) {
            for (int j = i; j < n - 1; j++)
                argv[j] = argv[j + 1];
            argv[n - 1] = nullptr;
            (*argc)--;
            return true;
        }
    }
    return false;
}

#include "render_test_base.h"
#include "offscreen_gl.h"
#include "tracer/render_listener.h"
#include <gtest/gtest.h>

int main(int argc, char **argv) {
    setupEnvironment();

    bool skip_trace = stripSkipTraceFlag(&argc, argv);

    printf("Starting D3 Render Tests (consolidated)...\n");
    if (skip_trace)
        printf("Tracing disabled (--skip-trace).\n");
    fflush(stdout);

    ::testing::InitGoogleTest(&argc, argv);

    if (!skip_trace) {
        ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
        listeners.Append(new RenderTraceListener());
    }

    int result = RUN_ALL_TESTS();

    OffscreenGL::Shutdown();

    return result;
}
