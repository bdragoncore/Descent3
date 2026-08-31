/*
 * MesaOpenGL Header
 * Header for Mesa OpenGL software renderer
 *
 * NOTE: EGL context is owned by OffscreenGL (tests/render/offscreen_gl.h).
 * MesaOpenGL attaches to the already-current context via rend_Init().
 */

#pragma once

#include "renderer.h"

// Mesa OpenGL specific functions
void mesa_GetInformation();
int mesa_GetFramebufferWidth();
int mesa_GetFramebufferHeight();
