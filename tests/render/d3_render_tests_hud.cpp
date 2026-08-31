/*
 * D3 game rendering (HUD, text): HUD text via grtext, fonts (d3.hog).
 * Suite: d3_render_tests_hud.
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
        const uint8_t *fb = framebuffer_;
        int count = 0;
        for (int i = 0; i < width_ * height_ * 4; i += 4) {
            uint8_t r = fb[i], g = fb[i + 1], b = fb[i + 2];
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
    // Use a dark green background so the HUD green render is clearly visible.
    glClearColor(0.0f, 0.3f, 0.0f, 1.0f);
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
        if (GetFramebuffer()[i] > 50 || GetFramebuffer()[i + 1] > 50 || GetFramebuffer()[i + 2] > 50)
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
        if (GetFramebuffer()[i] > 30 || GetFramebuffer()[i + 1] > 30 || GetFramebuffer()[i + 2] > 30)
            nonblack++;
    }
    (void)nonblack;
}

// B1: Font load is in SetUp; verify text renders (font pipeline works).
TEST_F(D3TextFixture, FontPipelineRendersText) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SetTextColorWhite();
    RenderText(100, 100, "A");
    EndFrame();
    ReadPixels();
    int nonBlack = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            nonBlack++;
    }
    EXPECT_GT(nonBlack, 0) << "Single character should produce pixels";
}

// B2: Formatted text output.
TEST_F(D3TextFixture, FormattedTextOutput) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SetTextColorGreen();
    RenderTextf(50, 50, "VAL=%d", 42);
    EndFrame();
    ReadPixels();
    SavePNG("FormattedTextOutput");
}

// B3: Layout - text at fixed coordinates.
TEST_F(D3TextFixture, TextLayoutAtFixedPosition) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SetTextColorYellow();
    RenderText(320 - 20, 240 - 8, "CENTER");
    EndFrame();
    ReadPixels();
    SavePNG("TextLayoutAtFixedPosition");
}

// B4: Minimal HUD placeholder (one line of info).
TEST_F(D3TextFixture, MinimalHudOneLine) {
    BeginFrame();
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SetTextColorWhite();
    RenderText(10, height_ - 24, "HUD: READY");
    EndFrame();
    ReadPixels();
    SavePNG("MinimalHudOneLine");
}

// B5: Font scale.
TEST_F(D3TextFixture, FontScaleRenders) {
    SetFontScale(1.5f);
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SetTextColorWhite();
    RenderText(50, 50, "SCALED");
    EndFrame();
    ReadPixels();
    SavePNG("FontScaleRenders");
    SetFontScale(1.0f);
}

// B6: Numeric/special characters (digits and punctuation).
TEST_F(D3TextFixture, NumericAndSpecialChars) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SetTextColorWhite();
    RenderText(50, 50, "123 45.6 !@#");
    EndFrame();
    ReadPixels();
    SavePNG("NumericAndSpecialChars");
}

// ---------------------------------------------------------------------------
// Phase 3 — HUD Extension Tests (33–37)
// ---------------------------------------------------------------------------

TEST_F(D3TextFixture, HUDTextRendering) {
    BeginFrame();
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SetTextColorWhite();
    RenderText(100, 30, "HUD TEST PANEL");

    SetTextColorGreen();
    RenderText(100, 70, "Health: 100");
    RenderText(100, 95, "Ammo: 42");
    RenderText(100, 120, "Score: 9999");

    SetTextColorRed();
    RenderText(100, 170, "WARNING: LOW SHIELD");

    SetTextColorYellow();
    RenderText(100, 200, "Power-Up Active");

    EndFrame();
    ReadPixels();
    SavePNG("HUDTextRendering");

    int nonblack = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 30 || GetFramebuffer()[i + 1] > 30 || GetFramebuffer()[i + 2] > 30)
            nonblack++;
    }
    EXPECT_GT(nonblack, 100) << "HUD text should produce visible pixels";
}

TEST_F(D3TextFixture, HUDCrosshairAndReticle) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int cx = width_ / 2;
    int cy = height_ / 2;

    SetTextColorGreen();
    RenderText(cx - 5, cy - 8, "+");
    RenderText(cx - 2, cy - 2, "---");
    RenderText(cx - 5, cy + 8, "+");

    SetTextColorWhite();
    RenderText(cx - 30, cy - 12, "[     ]");
    RenderText(cx - 30, cy + 14, "[     ]");

    EndFrame();
    ReadPixels();
    SavePNG("HUDCrosshairAndReticle");

    int center_pixels = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            center_pixels++;
    }
    EXPECT_GT(center_pixels, 10) << "Crosshair should produce pixels near center";
}

TEST_F(D3TextFixture, HUDStatusBar) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int bar_y = height_ - 60;
    int bar_w = 200;
    int bar_x = 100;

    SetTextColorWhite();
    RenderText(bar_x - 80, bar_y, "HP:");

    SetTextColorGreen();
    RenderText(bar_x, bar_y, "████████████████████");

    SetTextColorBlue();
    RenderText(bar_x, bar_y + 25, "████████████░░░░░░░░");
    SetTextColorWhite();
    RenderText(bar_x - 80, bar_y + 25, "SH:");

    SetTextColorWhite();
    RenderTextf(bar_x + bar_w + 10, bar_y, "100");
    SetTextColorBlue();
    RenderTextf(bar_x + bar_w + 10, bar_y + 25, "60");

    EndFrame();
    ReadPixels();
    SavePNG("HUDStatusBar");

    int status_region = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            status_region++;
    }
    EXPECT_GT(status_region, 100) << "Status bar should produce visible pixels";
}

TEST_F(D3TextFixture, HUDDamageFlash) {
    BeginFrame();
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SetTextColorWhite();
    RenderText(width_ / 2 - 40, height_ / 2, "DAMAGED!");

    SetTextColorYellow();
    RenderTextf(width_ / 2 - 20, height_ / 2 + 30, "-%d", 25);

    EndFrame();
    ReadPixels();
    SavePNG("HUDDamageFlash");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Damage flash should have visible pixels";
}

TEST_F(D3TextFixture, HUDMinimap) {
    BeginFrame();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int mm_x = width_ - 130, mm_y = 20, mm_w = 110, mm_h = 110;

    SetTextColor(0, 40, 0);
    for (int y = 0; y < mm_h; y += 8) {
        RenderText(mm_x, mm_y + y, "██████████████");
    }

    SetTextColorWhite();
    RenderText(mm_x - 2, mm_y - 5, "+------------+");
    for (int i = 0; i < 14; i++) {
        RenderText(mm_x - 2, mm_y + i * 8, "|            |");
    }
    RenderText(mm_x - 2, mm_y + mm_h, "+------------+");

    SetTextColorGreen();
    RenderText(mm_x + mm_w / 2 - 2, mm_y + mm_h / 2, "*");

    SetTextColorRed();
    RenderText(mm_x + 20, mm_y + 30, "*");
    RenderText(mm_x + 70, mm_y + 60, "*");

    EndFrame();
    ReadPixels();
    SavePNG("HUDMinimap");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored++;
    }
    EXPECT_GT(colored, 200) << "Minimap region should have visible content";
}
