/*
 * D3MesaRenderLegacy - ported from d3_mesa_render_tests.cpp.
 * Linked into d3_render_tests executable.
 */

#include "render_test_base.h"
#include "renderer.h"
#include <GL/gl.h>
#include <gtest/gtest.h>

class D3MesaRenderLegacy : public D3RenderTestBase {
protected:
    D3MesaRenderLegacy() : D3RenderTestBase(256, 256) {}

    void setupOrtho2D(float left, float right, float bottom, float top) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(left, right, bottom, top, 0.1f, 10.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
};

TEST_F(D3MesaRenderLegacy, InitAndRenderCube) {
    BeginFrame();
    rend_SetZBufferState(1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -2.0, 2.0, 0.1, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_TRIANGLES);
    glNormal3f(0, 0, 1);
    glVertex3f(-1.0f, -1.0f, -2.0f);
    glVertex3f(1.0f, -1.0f, -2.0f);
    glVertex3f(0.0f, 1.0f, -2.0f);
    glEnd();

    glFlush();
    glFinish();

    ReadPixels();
    SavePNG("MesaLegacy_InitAndRenderCube");
}

TEST_F(D3MesaRenderLegacy, RenderWithGrtext) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GetWidth(), 0, GetHeight(), -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(10, GetHeight() - 30);
    glVertex2f(GetWidth() - 10, GetHeight() - 30);
    glVertex2f(GetWidth() - 10, GetHeight() - 10);
    glVertex2f(10, GetHeight() - 10);
    glEnd();

    glFlush();
    glFinish();

    ReadPixels();

    int greenPixels = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i] < 50 && framebuffer_[i + 1] > 200 && framebuffer_[i + 2] < 50)
            greenPixels++;
    }
    EXPECT_GT(greenPixels, 100) << "Expected some green pixels";

    SavePNG("MesaLegacy_RenderWithGrtext");
}
