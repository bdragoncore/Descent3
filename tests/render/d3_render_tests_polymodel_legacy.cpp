/*
 * D3PolymodelRenderLegacy - ported from d3_polymodel_render_tests.cpp.
 * Linked into d3_render_tests executable.
 */

#include "render_test_base.h"
#include "renderer.h"
#include <GL/gl.h>
#include <gtest/gtest.h>
#include <cmath>

class D3PolymodelRenderLegacy : public D3RenderTestBase {
protected:
    D3PolymodelRenderLegacy() : D3RenderTestBase(256, 256) {}

    int countColorPixels(int r, int g, int b, int tolerance = 50) const {
        int count = 0;
        for (int i = 0; i < width_ * height_ * 4; i += 4) {
            if (std::abs((int)framebuffer_[i] - r) < tolerance &&
                std::abs((int)framebuffer_[i + 1] - g) < tolerance &&
                std::abs((int)framebuffer_[i + 2] - b) < tolerance)
                count++;
        }
        return count;
    }
};

TEST_F(D3PolymodelRenderLegacy, OpenGLContext) {
    BeginFrame();
    const char *glVendor = (const char *)glGetString(GL_VENDOR);
    const char *glRenderer = (const char *)glGetString(GL_RENDERER);
    EXPECT_TRUE(glVendor != nullptr && glRenderer != nullptr) << "GL context created";
    EndFrame();
    ReadPixels();
    SavePNG("PolymodelLegacy_OpenGLContext");
}

TEST_F(D3PolymodelRenderLegacy, RenderWithLighting) {
    BeginFrame();
    rend_SetZBufferState(1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float lightPos[] = {2.0f, 2.0f, 2.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    float lightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -2.0, 2.0, 0.1, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

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

    int litPixels = countColorPixels(200, 200, 200, 60);
    EXPECT_GT(litPixels, 2000);

    SavePNG("PolymodelLegacy_RenderWithLighting");
}

TEST_F(D3PolymodelRenderLegacy, RenderTexturedQuad) {
    BeginFrame();

    unsigned int texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    unsigned char texData[32 * 32 * 4];
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            int idx = (y * 32 + x) * 4;
            texData[idx] = (x < 16) ? 255 : 0;
            texData[idx + 1] = (y < 16) ? 255 : 0;
            texData[idx + 2] = 128;
            texData[idx + 3] = 255;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, 0.1, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.8f, -0.8f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0.8f, -0.8f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.8f, 0.8f, -1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.8f, 0.8f, -1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glFlush();
    glFinish();

    ReadPixels();

    int nonBlack = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i] > 10 || framebuffer_[i + 1] > 10 || framebuffer_[i + 2] > 10)
            nonBlack++;
    }
    EXPECT_GT(nonBlack, 5000);

    glDeleteTextures(1, &texId);

    SavePNG("PolymodelLegacy_RenderTexturedQuad");
}
