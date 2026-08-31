/*
 * D3MesaRendererLegacy - ported from d3_mesa_renderer_tests.cpp.
 * Linked into d3_render_tests executable.
 */

#include "render_test_base.h"
#include <GL/gl.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

class D3MesaRendererLegacy : public D3RenderTestBase {
public:
    D3MesaRendererLegacy() : D3RenderTestBase(640, 480) {}

protected:
    void SetUp() override {
        D3RenderTestBase::SetUp();
        if (!IsInitialized()) {
            GTEST_SKIP() << "Renderer init failed: " << GetError();
            return;
        }
        const char *d3path = getenv("D3_PATH");
        std::string base;
        if (d3path && d3path[0])
            base = d3path;
        else {
            const char *home = getenv("HOME");
            if (home)
                base = std::string(home) + "/Descent3";
        }
        if (!base.empty())
            AddSearchPath(base.c_str());
        (void)LoadFont("hihud.fnt");
    }

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

TEST_F(D3MesaRendererLegacy, OpenGLContextCreation) {
    BeginFrame();
    const char *glVendor = (const char *)glGetString(GL_VENDOR);
    const char *glRenderer = (const char *)glGetString(GL_RENDERER);
    EXPECT_TRUE(glVendor != nullptr && glRenderer != nullptr) << "GL context created";
    EndFrame();
    ReadPixels();
    SavePNG("MesaRendererLegacy_OpenGLContextCreation");
}

TEST_F(D3MesaRendererLegacy, DrawRedQuad) {
    BeginFrame();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GetWidth(), 0, GetHeight(), -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    SetTextColorWhite();
    RenderText(10, GetHeight() - 20, "DrawRedQuad");

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(GetWidth() * 0.25f, GetHeight() * 0.25f, 0.0f);
    glVertex3f(GetWidth() * 0.75f, GetHeight() * 0.25f, 0.0f);
    glVertex3f(GetWidth() * 0.75f, GetHeight() * 0.75f, 0.0f);
    glVertex3f(GetWidth() * 0.25f, GetHeight() * 0.75f, 0.0f);
    glEnd();

    glFlush();
    glFinish();

    ReadPixels();

    int redPixels = countColorPixels(255, 0, 0, 30);
    EXPECT_GT(redPixels, 5000) << "Expected many red pixels";

    SavePNG("MesaRendererLegacy_DrawRedQuad");
}

TEST_F(D3MesaRendererLegacy, DrawBlueGreenQuad) {
    BeginFrame();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GetWidth(), 0, GetHeight(), -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    SetTextColorWhite();
    RenderText(10, GetHeight() - 20, "DrawBlueGreenQuad");

    glColor3f(0.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(GetWidth() * 0.1f, GetHeight() * 0.1f, 0.0f);
    glVertex3f(GetWidth() * 0.5f, GetHeight() * 0.1f, 0.0f);
    glVertex3f(GetWidth() * 0.5f, GetHeight() * 0.5f, 0.0f);
    glVertex3f(GetWidth() * 0.1f, GetHeight() * 0.5f, 0.0f);
    glEnd();

    glFlush();
    glFinish();

    ReadPixels();

    int cyanPixels = countColorPixels(0, 255, 255, 30);
    EXPECT_GT(cyanPixels, 5000) << "Expected many cyan pixels";

    SavePNG("MesaRendererLegacy_DrawBlueGreenQuad");
}

TEST_F(D3MesaRendererLegacy, FramebufferConsistency) {
    BeginFrame();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GetWidth(), 0, GetHeight(), -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(GetWidth() * 0.25f, GetHeight() * 0.25f, 0.0f);
    glVertex3f(GetWidth() * 0.75f, GetHeight() * 0.25f, 0.0f);
    glVertex3f(GetWidth() * 0.75f, GetHeight() * 0.75f, 0.0f);
    glVertex3f(GetWidth() * 0.25f, GetHeight() * 0.75f, 0.0f);
    glEnd();

    glFlush();
    glFinish();
    ReadPixels();

    std::vector<uint8_t> fb1(width_ * height_ * 4);
    std::memcpy(fb1.data(), framebuffer_, fb1.size());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(GetWidth() * 0.25f, GetHeight() * 0.25f, 0.0f);
    glVertex3f(GetWidth() * 0.75f, GetHeight() * 0.25f, 0.0f);
    glVertex3f(GetWidth() * 0.75f, GetHeight() * 0.75f, 0.0f);
    glVertex3f(GetWidth() * 0.25f, GetHeight() * 0.75f, 0.0f);
    glEnd();

    glFlush();
    glFinish();
    ReadPixels();

    int diff = 0;
    for (size_t i = 0; i < fb1.size(); i++) {
        if (fb1[i] != framebuffer_[i])
            diff++;
    }
    EXPECT_EQ(diff, 0) << "Framebuffer should be consistent across renders";

    SavePNG("MesaRendererLegacy_FramebufferConsistency");
}

TEST_F(D3MesaRendererLegacy, RenderCube) {
    BeginFrame();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GetWidth(), 0, GetHeight(), -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    SetTextColorWhite();
    RenderText(10, GetHeight() - 20, "RenderCube");

    glColor3f(0.7f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(GetWidth() * 0.3f, GetHeight() * 0.3f, 0.0f);
    glVertex3f(GetWidth() * 0.7f, GetHeight() * 0.3f, 0.0f);
    glVertex3f(GetWidth() * 0.7f, GetHeight() * 0.5f, 0.0f);
    glVertex3f(GetWidth() * 0.3f, GetHeight() * 0.5f, 0.0f);
    glEnd();

    glColor3f(0.2f, 0.7f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(GetWidth() * 0.3f, GetHeight() * 0.5f, 0.0f);
    glVertex3f(GetWidth() * 0.7f, GetHeight() * 0.5f, 0.0f);
    glVertex3f(GetWidth() * 0.7f, GetHeight() * 0.7f, 0.0f);
    glVertex3f(GetWidth() * 0.3f, GetHeight() * 0.7f, 0.0f);
    glEnd();

    glColor3f(0.2f, 0.2f, 0.7f);
    glBegin(GL_QUADS);
    glVertex3f(GetWidth() * 0.3f, GetHeight() * 0.7f, 0.0f);
    glVertex3f(GetWidth() * 0.7f, GetHeight() * 0.7f, 0.0f);
    glVertex3f(GetWidth() * 0.7f, GetHeight() * 0.9f, 0.0f);
    glVertex3f(GetWidth() * 0.3f, GetHeight() * 0.9f, 0.0f);
    glEnd();

    glFlush();
    glFinish();

    ReadPixels();

    int coloredPixels = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if ((framebuffer_[i] > 50 && framebuffer_[i + 1] < 100 && framebuffer_[i + 2] < 100) ||
            (framebuffer_[i] < 100 && framebuffer_[i + 1] > 50 && framebuffer_[i + 2] < 100) ||
            (framebuffer_[i] < 100 && framebuffer_[i + 1] < 100 && framebuffer_[i + 2] > 50))
            coloredPixels++;
    }
    EXPECT_GT(coloredPixels, 3000) << "Expected colored cube faces";

    SavePNG("MesaRendererLegacy_RenderCube");
}

TEST_F(D3MesaRendererLegacy, DrawGreenText) {
    if (font_handle_ < 0)
        GTEST_SKIP() << "Need hihud.fnt (d3.hog) for text";
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GetWidth(), 0, GetHeight(), -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    SetTextColorWhite();
    RenderText(10, GetHeight() - 20, "DrawGreenText");
    SetTextColorGreen();
    RenderText(10, GetHeight() - 40, "D3 MESA TEST");

    glFlush();
    glFinish();

    ReadPixels();

    int greenPixels = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i] < 50 && framebuffer_[i + 1] > 200 && framebuffer_[i + 2] < 50)
            greenPixels++;
    }
    EXPECT_GT(greenPixels, 100) << "Expected green text pixels";

    SavePNG("MesaRendererLegacy_DrawGreenText");
}
