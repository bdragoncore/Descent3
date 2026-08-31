/*
 * EGL tests: context creation, surface, clear, depth, SavePNG.
 * Suite: d3_render_tests_egl.
 */

#include "render_test_base.h"
#include <GL/gl.h>
#include <gtest/gtest.h>

class D3EGLContextTest : public D3RenderTestBase {
public:
    D3EGLContextTest() : D3RenderTestBase(800, 600) {}

protected:
    void RenderFrame() override {
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};

TEST_F(D3EGLContextTest, EglContextCreation) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_EQ(glGetError(), (GLenum)GL_NO_ERROR) << "GL should be working";

    const char *glVendor = (const char *)glGetString(GL_VENDOR);
    const char *glRenderer = (const char *)glGetString(GL_RENDERER);
    EXPECT_TRUE(glVendor != nullptr && glVendor[0] != '\0') << "GL_VENDOR should be non-null";
    EXPECT_TRUE(glRenderer != nullptr && glRenderer[0] != '\0') << "GL_RENDERER should be non-null";
}

TEST_F(D3EGLContextTest, BasicClear) {
    BeginFrame();
    RenderFrame();
    EndFrame();
    ReadPixels();

    int matching = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i] >= 20 && framebuffer_[i] <= 30 && framebuffer_[i + 1] >= 46 && framebuffer_[i + 1] <= 56 &&
            framebuffer_[i + 2] >= 71 && framebuffer_[i + 2] <= 81) {
            matching++;
        }
    }
    double pct = (double)matching / (width_ * height_) * 100.0;
    EXPECT_GT(pct, 95.0) << "Expected clear colour, got " << pct << "%";
}

TEST_F(D3EGLContextTest, RedClear) {
    BeginFrame();
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EndFrame();
    ReadPixels();

    int red = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i] > 200 && framebuffer_[i + 1] < 50 && framebuffer_[i + 2] < 50)
            red++;
    }
    EXPECT_GT((double)red / (width_ * height_) * 100.0, 95.0);
}

TEST_F(D3EGLContextTest, MultipleClearColor) {
    BeginFrame();
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EndFrame();
    ReadPixels();

    int green = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i + 1] > 200 && framebuffer_[i] < 50 && framebuffer_[i + 2] < 50)
            green++;
    }
    EXPECT_GT((double)green / (width_ * height_) * 100.0, 95.0);

    BeginFrame();
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EndFrame();
    ReadPixels();

    int blue = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i + 2] > 200 && framebuffer_[i] < 50 && framebuffer_[i + 1] < 50)
            blue++;
    }
    EXPECT_GT((double)blue / (width_ * height_) * 100.0, 95.0);
}

TEST_F(D3EGLContextTest, DepthTestEnabled) {
    BeginFrame();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GLboolean dt;
    glGetBooleanv(GL_DEPTH_TEST, &dt);
    EXPECT_EQ(dt, GL_TRUE);

    glDisable(GL_DEPTH_TEST);
    EndFrame();
    ReadPixels();
}

TEST_F(D3EGLContextTest, SavePng) {
    BeginFrame();
    glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EndFrame();
    ReadPixels();

    bool ok = SavePNG("EGLClear");
    EXPECT_TRUE(ok) << GetError();
}
