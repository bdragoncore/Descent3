/*
 * D3TextFixture - HUD text rendering tests (requires d3.hog).
 * Linked into d3_render_tests executable.
 */

#include "render_test_base.h"
#include <GL/gl.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <string>

class D3TextFixture : public D3RenderTestBase {
public:
    D3TextFixture() : D3RenderTestBase(640, 480) {}

protected:
    void SetUp() override {
        D3RenderTestBase::SetUp();
        if (!IsInitialized()) {
            GTEST_SKIP() << "Renderer init failed: " << GetError();
            return;
        }

        const char *d3path = getenv("D3_PATH");
        std::string base;
        if (d3path && d3path[0]) {
            base = d3path;
        } else {
            const char *home = getenv("HOME");
            if (home)
                base = std::string(home) + "/Descent3";
        }

        if (base.empty()) {
            GTEST_SKIP() << "d3.hog not found — set D3_PATH to your Descent 3 install";
            return;
        }

        AddSearchPath(base.c_str());

        if (!LoadFont("hihud.fnt")) {
            GTEST_SKIP() << "Could not load hihud.fnt: " << GetError();
            return;
        }

        SetFontScale(1.0f);
    }

    int CountDominantChannel(int chan) const {
        int count = 0;
        for (int i = 0; i < width_ * height_ * 4; i += 4) {
            uint8_t r = framebuffer_[i], g = framebuffer_[i + 1], b = framebuffer_[i + 2];
            uint8_t val = (chan == 0) ? r : (chan == 1) ? g : b;
            uint8_t o1 = (chan == 0) ? g : (chan == 1) ? r : r;
            uint8_t o2 = (chan == 0) ? b : (chan == 1) ? b : g;
            if (val > 100 && val > o1 + 30 && val > o2 + 30)
                count++;
        }
        return count;
    }
};

TEST_F(D3TextFixture, RenderHudGreen) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SetTextColorGreen();
    RenderText(100, 100, "GREEN HUD TEXT");

    EndFrame();
    ReadPixels();
    SavePNG("RenderHudGreen");

    int green = CountDominantChannel(1);
    (void)green;
}

TEST_F(D3TextFixture, RenderHudColors) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SetTextColorRed();
    RenderText(50, 50, "RED");

    SetTextColorGreen();
    RenderText(50, 100, "GREEN");

    SetTextColorBlue();
    RenderText(50, 150, "BLUE");

    SetTextColorYellow();
    RenderText(50, 200, "YELLOW");

    SetTextColorWhite();
    RenderText(50, 250, "WHITE");

    EndFrame();
    ReadPixels();
    SavePNG("RenderHudColors");

    int coloured = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i] > 50 || framebuffer_[i + 1] > 50 || framebuffer_[i + 2] > 50)
            coloured++;
    }
    (void)coloured;
}

TEST_F(D3TextFixture, RenderHudTextBounds) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SetTextColorWhite();
    RenderText(0, 0, "TOP-LEFT");
    RenderText(width_ - 80, 0, "TOP-RIGHT");
    RenderText(0, height_ - 20, "BOTTOM-LEFT");
    RenderText(width_ - 100, height_ - 20, "BOTTOM-RIGHT");

    EndFrame();
    ReadPixels();
    SavePNG("RenderHudTextBounds");

    int nonblack = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (framebuffer_[i] > 30 || framebuffer_[i + 1] > 30 || framebuffer_[i + 2] > 30)
            nonblack++;
    }
    (void)nonblack;
}
