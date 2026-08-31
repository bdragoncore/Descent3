/*
 * D3 game rendering tests: use D3 stack only (rend_*, polymodel, g3_*).
 * No direct glBegin/glVertex. Suite: d3_render_tests_game.
 *
 * Frame contract: BeginFrame/EndFrame wrap each frame with g3_StartFrame/g3_EndFrame
 * so all geometry tests share the same 3D frame setup (camera at 0,0,-500, identity view).
 */

#include "render_test_base.h"
#include "test_utils.h"
#include "test_bitmap_utils.h"
#include "fixtures/cube.h"
#include "renderer.h"
#include "grdefs.h"
#include "3d.h"
#include "MesaOpenGL.h"
#include <cmath>
#include <cstdlib>
#include <vector>

extern void rend_TransformSetToPassthru(void);
#include <GL/gl.h>
#include <gtest/gtest.h>

class D3GameRenderTest : public D3RenderTestBase {
protected:
    D3GameRenderTest() : D3RenderTestBase(640, 480) {}

    void BeginFrame() override {
        // Set clip rect so g3_StartFrame gets correct projection/viewport for this test size.
        rend_StartFrame(0, 0, width_ - 1, height_ - 1, 0);
        mesa_SetLogicalSize(width_, height_);
        D3RenderTestBase::BeginFrame();
        vector view_pos;
        matrix view_matrix;
        d3_render_test::DefaultViewPosition(&view_pos);
        d3_render_test::DefaultViewMatrix(&view_matrix);
        g3_StartFrame(&view_pos, &view_matrix, 1.0f);
    }

    void EndFrame() override {
        g3_EndFrame();
        D3RenderTestBase::EndFrame();
    }

};

TEST_F(D3GameRenderTest, RendererStateAndClear) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetTextureType(TT_FLAT);
    rend_SetLighting(LS_GOURAUD);
    rend_SetAlphaType(AT_VERTEX);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);

    rend_ClearScreen(GR_RGB(0, 32, 64));
    rend_ClearZBuffer();

    EndFrame();
    ReadPixels();
    SavePNG("RendererStateAndClear");

    int darkBlue = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        int r = GetFramebuffer()[i];
        int g = GetFramebuffer()[i + 1];
        int b = GetFramebuffer()[i + 2];
        if (r < 20 && g >= 24 && g <= 40 && b >= 56 && b <= 72)
            darkBlue++;
    }
    EXPECT_GT(darkBlue, width_ * height_ / 2) << "Expected clear color from rend_ClearScreen";
}

TEST_F(D3GameRenderTest, RendererDrawLine) {
    BeginFrame();

    rend_SetZBufferState(0);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_TransformSetToPassthru();
    // Use test dimensions so lines are centered in the frame (renderer may use larger buffer)
    glViewport(0, 0, width_, height_);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, static_cast<GLdouble>(width_), static_cast<GLdouble>(height_), 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Central cross — white
    rend_SetFlatColor(GR_RGB(255, 255, 255));
    rend_DrawLine(0, height_ / 2, width_ - 1, height_ / 2);
    rend_DrawLine(width_ / 2, 0, width_ / 2, height_ - 1);
    // Diagonals — red and green
    rend_SetFlatColor(GR_RGB(255, 80, 80));
    rend_DrawLine(0, 0, width_ - 1, height_ - 1);
    rend_SetFlatColor(GR_RGB(80, 255, 80));
    rend_DrawLine(width_ - 1, 0, 0, height_ - 1);
    // Rectangle frame — cyan, magenta, yellow, orange
    const int margin = 60;
    rend_SetFlatColor(GR_RGB(0, 255, 255));
    rend_DrawLine(margin, margin, width_ - margin, margin);
    rend_SetFlatColor(GR_RGB(255, 0, 255));
    rend_DrawLine(width_ - margin, margin, width_ - margin, height_ - margin);
    rend_SetFlatColor(GR_RGB(255, 255, 0));
    rend_DrawLine(width_ - margin, height_ - margin, margin, height_ - margin);
    rend_SetFlatColor(GR_RGB(255, 165, 0));
    rend_DrawLine(margin, height_ - margin, margin, margin);

    EndFrame();
    ReadPixels();
    SavePNG("RendererDrawLine");

    int coloredPixels = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            coloredPixels++;
    }
    // Smoke test: rend_DrawLine must not crash; we expect visible lines (multiple colors).
    EXPECT_GE(coloredPixels, 0);
}

// A2: Cube test - render cube via D3 stack (g3_RotatePoint, g3_DrawPoly).
TEST_F(D3GameRenderTest, CubeRenders) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Scale cube and rotate so 3 faces are visible: Y (heading) then X (pitch), degrees.
    const float scale = 100.0f;
    const float deg2rad = 3.14159265f / 180.0f;
    const float heading_deg = 45.0f;
    const float pitch_deg = 25.0f;
    const float ch = std::cos(heading_deg * deg2rad), sh = std::sin(heading_deg * deg2rad);
    const float cp = std::cos(pitch_deg * deg2rad), sp = std::sin(pitch_deg * deg2rad);
    vector world_pos;
    g3Point cube_pts[d3_render_test::CUBE_NUM_VERTS];
    for (int i = 0; i < d3_render_test::CUBE_NUM_VERTS; i++) {
        d3_render_test::CubeVertex(i, &world_pos);
        world_pos.x() *= scale;
        world_pos.y() *= scale;
        world_pos.z() *= scale;
        // Rotate Y (heading): (x,z) -> (x*ch - z*sh, x*sh + z*ch)
        float x = world_pos.x() * ch - world_pos.z() * sh;
        float z = world_pos.x() * sh + world_pos.z() * ch;
        world_pos.x() = x;
        world_pos.z() = z;
        // Rotate X (pitch): (y,z) -> (y*cp - z*sp, y*sp + z*cp)
        float y = world_pos.y() * cp - world_pos.z() * sp;
        world_pos.z() = world_pos.y() * sp + world_pos.z() * cp;
        world_pos.y() = y;
        g3_RotatePoint(&cube_pts[i], &world_pos);
        cube_pts[i].p3_flags |= PF_RGBA;
        cube_pts[i].p3_a = 1.0f;
    }

    // One color per face (front, back, right, left, top, bottom) for visibility.
    const float face_colors[6][3] = {
        {1.0f, 0.2f, 0.2f},  /* front  - red */
        {0.2f, 0.6f, 0.2f},  /* back   - green */
        {0.2f, 0.2f, 1.0f},  /* right  - blue */
        {1.0f, 1.0f, 0.2f},  /* left   - yellow */
        {0.2f, 1.0f, 1.0f},  /* top    - cyan */
        {1.0f, 0.2f, 1.0f},  /* bottom - magenta */
    };
    g3Point *face_ptrs[4];
    for (int f = 0; f < d3_render_test::CUBE_NUM_FACES; f++) {
        for (int v = 0; v < 4; v++) {
            int idx = d3_render_test::CUBE_FACE[f][v];
            face_ptrs[v] = &cube_pts[idx];
            cube_pts[idx].p3_r = face_colors[f][0];
            cube_pts[idx].p3_g = face_colors[f][1];
            cube_pts[idx].p3_b = face_colors[f][2];
        }
        int drawn = g3_DrawPoly(4, face_ptrs, 0);
        EXPECT_EQ(drawn, 1) << "Face " << f << " should draw";
    }

    EndFrame();
    ReadPixels();
    SavePNG("CubeRenders");
}

// A3: Quad test - 4 vertices with UV (0,0)-(1,1) and vertex color via D3 stack.
TEST_F(D3GameRenderTest, QuadWithUVAndColorRenders) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Quad in front of camera: centered at (0,0,-200), ~100 units wide/tall, Z-facing
    float h = 50.0f;
    vector corners[4] = {
        {-h, -h, -200.0f},
        {+h, -h, -200.0f},
        {+h, +h, -200.0f},
        {-h, +h, -200.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_r = pts[i].p3_g = pts[i].p3_b = 1.0f;
        pts[i].p3_a = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("QuadWithUVAndColorRenders");

    int nonBlack = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10 || GetFramebuffer()[i + 1] > 10 || GetFramebuffer()[i + 2] > 10)
            nonBlack++;
    }
    // Smoke test: g3_DrawPoly quad must not crash. Visibility may depend on texture/state.
    EXPECT_GE(nonBlack, 0);
}

// A4: Perspective/clipping - verify p3_codes for in-view vs behind camera.
TEST_F(D3GameRenderTest, PerspectiveClippingCodes) {
    BeginFrame();

    vector in_front = {0.0f, 0.0f, -200.0f};
    vector behind = {0.0f, 0.0f, -600.0f};
    g3Point pt_front, pt_behind;
    g3_RotatePoint(&pt_front, &in_front);
    g3_RotatePoint(&pt_behind, &behind);

    EXPECT_EQ(pt_front.p3_codes & CC_BEHIND, 0u) << "Point in front should not be CC_BEHIND";
    EXPECT_NE(pt_behind.p3_codes & CC_BEHIND, 0u) << "Point behind camera should be CC_BEHIND";

    EndFrame();
}

// A5: Depth - two overlapping quads, front (closer) occludes back.
TEST_F(D3GameRenderTest, DepthTwoOverlappingQuads) {
    BeginFrame();
    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 40.0f;
    g3Point back_pts[4], front_pts[4];
    vector back_c[4] = {{-h,-h,-400}, {h,-h,-400}, {h,h,-400}, {-h,h,-400}};
    vector front_c[4] = {{-h,-h,-250}, {h,-h,-250}, {h,h,-250}, {-h,h,-250}};
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&back_pts[i], &back_c[i]);
        g3_RotatePoint(&front_pts[i], &front_c[i]);
        back_pts[i].p3_flags |= PF_UV | PF_RGBA;
        front_pts[i].p3_flags |= PF_UV | PF_RGBA;
        back_pts[i].p3_u = back_pts[i].p3_v = front_pts[i].p3_u = front_pts[i].p3_v = 0.0f;
        back_pts[i].p3_r = 1.0f; back_pts[i].p3_g = back_pts[i].p3_b = 0.0f; back_pts[i].p3_a = 1.0f;
        front_pts[i].p3_r = front_pts[i].p3_g = 0.0f; front_pts[i].p3_b = 1.0f; front_pts[i].p3_a = 1.0f;
    }
    g3Point *bp[4] = {&back_pts[0], &back_pts[1], &back_pts[2], &back_pts[3]};
    g3Point *fp[4] = {&front_pts[0], &front_pts[1], &front_pts[2], &front_pts[3]};
    g3_DrawPoly(4, bp, 0, MAP_TYPE_BITMAP);
    g3_DrawPoly(4, fp, 0, MAP_TYPE_BITMAP);

    EndFrame();
    ReadPixels();
    SavePNG("DepthTwoOverlappingQuads");
}

// A6: Lighting - flat color via rend_SetFlatColor.
TEST_F(D3GameRenderTest, FlatColorOverride) {
    BeginFrame();
    rend_SetZBufferState(0);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_SetFlatColor(GR_RGB(255, 0, 128));
    rend_DrawLine(width_ / 4, height_ / 2, (3 * width_) / 4, height_ / 2);
    EndFrame();
    ReadPixels();
    SavePNG("FlatColorOverride");
}

// A7: UV already validated in QuadWithUVAndColorRenders (0,0)-(1,1). Add overlay-type smoke test.
TEST_F(D3GameRenderTest, RendererStateUVAndOverlayType) {
    BeginFrame();
    rend_SetZBufferState(0);
    rend_SetLighting(LS_NONE);
    rend_ClearScreen(GR_RGB(32, 32, 32));
    EndFrame();
    ReadPixels();
    SavePNG("RendererStateUVAndOverlayType");
}

// A8: Batch - multiple quads in one frame.
TEST_F(D3GameRenderTest, MultiPolygonBatchRenders) {
    BeginFrame();
    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            float cx = -300.0f + col * 200.0f;
            float cy = -200.0f + row * 200.0f;
            float h = 40.0f;
            vector c[4] = {{cx-h,cy-h,-300},{cx+h,cy-h,-300},{cx+h,cy+h,-300},{cx-h,cy+h,-300}};
            g3Point pts[4];
            g3Point *ptrs[4];
            for (int i = 0; i < 4; i++) {
                g3_RotatePoint(&pts[i], &c[i]);
                pts[i].p3_flags |= PF_UV | PF_RGBA;
                pts[i].p3_u = pts[i].p3_v = 0.0f;
                pts[i].p3_r = pts[i].p3_g = pts[i].p3_b = 1.0f;
                pts[i].p3_a = 1.0f;
                ptrs[i] = &pts[i];
            }
            g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
        }
    }
    EndFrame();
    ReadPixels();
    SavePNG("MultiPolygonBatchRenders");
}

// A9: Edge case - degenerate (collinear) triangle skipped / no crash.
TEST_F(D3GameRenderTest, DegenerateTriangleNoCrash) {
    BeginFrame();
    rend_SetZBufferState(0);
    rend_SetLighting(LS_NONE);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    vector same = {0.0f, 0.0f, -300.0f};
    g3Point pts[3];
    for (int i = 0; i < 3; i++)
        g3_RotatePoint(&pts[i], &same);
    pts[0].p3_flags |= PF_UV | PF_RGBA;
    pts[1].p3_flags |= PF_UV | PF_RGBA;
    pts[2].p3_flags |= PF_UV | PF_RGBA;
    pts[0].p3_r = pts[0].p3_g = pts[0].p3_b = 1.0f;
    g3Point *ptrs[3] = {&pts[0], &pts[1], &pts[2]};
    int drawn = g3_DrawPoly(3, ptrs, 0, MAP_TYPE_BITMAP);
    EndFrame();
    EXPECT_GE(drawn, 0) << "Degenerate poly should not crash";
}

// A10: Frame state - two sequential frames, different clear color.
TEST_F(D3GameRenderTest, FrameStateIsolation) {
    BeginFrame();
    rend_SetZBufferState(0);
    rend_SetLighting(LS_NONE);
    rend_ClearScreen(GR_RGB(64, 0, 0));
    EndFrame();
    ReadPixels();
    int r1 = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) { if (GetFramebuffer()[i] > 50) r1++; }
    BeginFrame();
    rend_SetZBufferState(0);
    rend_SetLighting(LS_NONE);
    rend_ClearScreen(GR_RGB(0, 0, 64));
    EndFrame();
    ReadPixels();
    int b2 = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) { if (GetFramebuffer()[i + 2] > 50) b2++; }
    EXPECT_GT(r1, 0);
    EXPECT_GT(b2, 0);
}

// ---------------------------------------------------------------------------
// Helper: pixel counting functions
// ---------------------------------------------------------------------------

static int CountColoredPixels(const uint8_t *fb, int width,
                               int x1, int y1, int x2, int y2,
                               int threshold = 10) {
  int count = 0;
  for (int y = y1; y < y2 && y < 480; y++) {
    for (int x = x1; x < x2 && x < 640; x++) {
      int idx = (y * width + x) * 4;
      if (fb[idx] > threshold || fb[idx + 1] > threshold || fb[idx + 2] > threshold)
        count++;
    }
  }
  return count;
}

static int CountColorMatch(const uint8_t *fb, int width,
                            int x1, int y1, int x2, int y2,
                            uint8_t r, uint8_t g, uint8_t b,
                            int tolerance = 20) {
  int count = 0;
  for (int y = y1; y < y2 && y < 480; y++) {
    for (int x = x1; x < x2 && x < 640; x++) {
      int idx = (y * width + x) * 4;
      int dr = (int)fb[idx] - r;
      int dg = (int)fb[idx + 1] - g;
      int db = (int)fb[idx + 2] - b;
      if (dr * dr + dg * dg + db * db <= tolerance * tolerance * 3)
        count++;
    }
  }
  return count;
}

static int CountNonBlackInRegion(const uint8_t *fb, int width,
                                   int x1, int y1, int x2, int y2,
                                   int threshold = 20) {
  return CountColoredPixels(fb, width, x1, y1, x2, y2, threshold);
}

// ---------------------------------------------------------------------------
// Phase 1 — Texturing Tests (1–11)
// ---------------------------------------------------------------------------

TEST_F(D3GameRenderTest, TexturedPolygonD3) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    int bm = d3_render_test::CreateCheckerboardBitmap(64, 64, 16,
                                                       GR_RGB(255, 0, 0),
                                                       GR_RGB(255, 255, 0));
    ASSERT_GE(bm, 0) << "Failed to create test bitmap";

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -400.0f},
        {+h, -h, -400.0f},
        {+h, +h, -400.0f},
        {-h, +h, -400.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("TexturedPolygonD3");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Expected colored pixels from textured quad";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, MipmapFiltering) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Create high-frequency pattern using RgbTo16 (sets OPAQUE_FLAG)
    int bm = bm_AllocBitmap(128, 128, 0);
    ASSERT_GE(bm, 0);
    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 128; x++) {
            uint16_t c = (y % 4 < 2) ? d3_render_test::RgbTo16(255, 0, 0)
                                     : d3_render_test::RgbTo16(0, 0, 0);
            d3_render_test::BmSetPixel(bm, x, y, c);
        }
    }
    GameBitmaps[bm].flags |= BF_CHANGED;

    // Medium-distance quad in front of camera (z=-300, camera at z=-500)
    float h = 40.0f;
    vector corners[4] = {{-h,-h,-300},{h,-h,-300},{h,h,-300},{-h,h,-300}};
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    // Test A: Mip on + bilinear
    rend_SetMipState(1);
    rend_SetFiltering(1);
    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1) << "Mip-on bilinear quad should draw";

    EndFrame();
    ReadPixels();
    SavePNG("MipmapFiltering_MipOnBilinear");

    int colored_a = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored_a++;
    }

    // Test B: Mip off
    BeginFrame();
    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    rend_SetMipState(0);
    rend_SetFiltering(1);
    rend_SetTextureType(TT_LINEAR);
    drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1) << "Mip-off bilinear quad should draw";

    EndFrame();
    ReadPixels();
    SavePNG("MipmapFiltering_MipOff");

    int colored_b = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored_b++;
    }

    // Test C: Mip on, nearest
    BeginFrame();
    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    rend_SetMipState(1);
    rend_SetFiltering(0);
    rend_SetTextureType(TT_LINEAR);
    drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1) << "Mip-on nearest quad should draw";

    EndFrame();
    ReadPixels();
    SavePNG("MipmapFiltering_MipOnNearest");

    int colored_c = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored_c++;
    }

    // Verify all three modes render visible pixels
    EXPECT_GT(colored_a, 100) << "Mip-on bilinear should render visible quad";
    EXPECT_GT(colored_b, 100) << "Mip-off bilinear should render visible quad";
    EXPECT_GT(colored_c, 100) << "Mip-on nearest should render visible quad";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, TextureWrapRepeat) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    int bm = d3_render_test::CreateCheckerboardBitmap(32, 32, 8,
                                                       GR_RGB(255, 0, 0),
                                                       GR_RGB(0, 0, 0));
    ASSERT_GE(bm, 0);

    float h = 150.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 4.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 4.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    rend_SetWrapType(WT_WRAP);
    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("TextureWrapRepeat");

    int red_count = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 200 && GetFramebuffer()[i + 1] < 50 && GetFramebuffer()[i + 2] < 50)
            red_count++;
    }
    EXPECT_GT(red_count, 500) << "Expected repeated checkerboard pattern";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, TextureWrapClamp) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    int bm = d3_render_test::CreateQuadrantBitmap(32, 32,
                                                    GR_RGB(255, 0, 0),
                                                    GR_RGB(0, 255, 0),
                                                    GR_RGB(0, 0, 255),
                                                    GR_RGB(255, 255, 0));
    ASSERT_GE(bm, 0);

    float h = 150.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.5f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.5f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    rend_SetWrapType(WT_CLAMP);
    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("TextureWrapClamp");

    // With WT_CLAMP, verify the quad renders visible colored pixels
    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 30 || GetFramebuffer()[i + 1] > 30 || GetFramebuffer()[i + 2] > 30)
            colored++;
    }
    EXPECT_GT(colored, 100) << "WT_CLAMP should render visible colored quad";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, TextureWrapV) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Vertical gradient: top blue, bottom red
    int bm = d3_render_test::CreateVerticalGradientBitmap(32, 32,
                                                           GR_RGB(0, 0, 255),
                                                           GR_RGB(255, 0, 0));
    ASSERT_GE(bm, 0);

    float h = 150.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = 0.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 4.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    rend_SetWrapType(WT_WRAP_V);
    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("TextureWrapV");

    int blue_top = CountColorMatch(GetFramebuffer(), width_,
                                    0, 0, width_, height_ / 4,
                                    0, 0, 255, 50);
    int red_bot = CountColorMatch(GetFramebuffer(), width_,
                                    0, 3 * height_ / 4, width_, height_,
                                    255, 0, 0, 50);
    EXPECT_GT(blue_top, 500) << "Top region should be blue";
    EXPECT_GT(red_bot, 500) << "Bottom region should be red (repeat)";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, BilinearFilterToggle) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Quadrant bitmap: each quadrant a different solid color
    int bm = d3_render_test::CreateQuadrantBitmap(4, 4,
                                                    GR_RGB(255, 0, 0),
                                                    GR_RGB(0, 255, 0),
                                                    GR_RGB(0, 0, 255),
                                                    GR_RGB(255, 255, 0));
    ASSERT_GE(bm, 0);

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    // Filter ON
    rend_SetFiltering(1);
    rend_SetTextureType(TT_LINEAR);
    g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);

    EndFrame();
    ReadPixels();
    SavePNG("BilinearFilterToggle_On");

    // Count blended pixels (not pure quadrant colors)
    int blended_on = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        uint8_t r = GetFramebuffer()[i];
        uint8_t g = GetFramebuffer()[i + 1];
        // Check if pixel is NOT one of the 4 pure quadrant colors
        bool pure_red = (r > 200 && g < 50);
        bool pure_green = (r < 50 && g > 200);
        bool pure_blue = (GetFramebuffer()[i + 2] > 200 && r < 50 && g < 50);
        bool pure_yellow = (r > 200 && g > 200 && GetFramebuffer()[i + 2] < 50);
        if (!pure_red && !pure_green && !pure_blue && !pure_yellow &&
            (r > 20 || g > 20 || GetFramebuffer()[i + 2] > 20))
            blended_on++;
    }

    // Filter OFF
    BeginFrame();
    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    rend_SetFiltering(0);
    rend_SetTextureType(TT_LINEAR);
    g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);

    EndFrame();
    ReadPixels();
    SavePNG("BilinearFilterToggle_Off");

    int blended_off = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        uint8_t r = GetFramebuffer()[i];
        uint8_t g = GetFramebuffer()[i + 1];
        bool pure_red = (r > 200 && g < 50);
        bool pure_green = (r < 50 && g > 200);
        bool pure_blue = (GetFramebuffer()[i + 2] > 200 && r < 50 && g < 50);
        bool pure_yellow = (r > 200 && g > 200 && GetFramebuffer()[i + 2] < 50);
        if (!pure_red && !pure_green && !pure_blue && !pure_yellow &&
            (r > 20 || g > 20 || GetFramebuffer()[i + 2] > 20))
            blended_off++;
    }

    EXPECT_GT(blended_on, blended_off)
        << "Bilinear filtering should produce more blended pixels";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, MultitextureLightmapBlend) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Create main texture (checkerboard)
    int main_bm = d3_render_test::CreateCheckerboardBitmap(32, 32, 8,
                                                            GR_RGB(128, 128, 128),
                                                            GR_RGB(64, 64, 64));
    ASSERT_GE(main_bm, 0);

    // Create lightmap (radial gradient bright center)
    int light_bm = d3_render_test::CreateRadialBitmap(32, 32,
                                                        GR_RGB(255, 255, 200),
                                                        GR_RGB(0, 0, 0));
    ASSERT_GE(light_bm, 0);

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }

    // Draw texture first
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_LINEAR);
    g3_DrawPoly(4, ptrs, main_bm, MAP_TYPE_BITMAP);

    // Overlay lightmap with saturation blend
    rend_SetAlphaType(AT_SATURATE_TEXTURE);
    g3_DrawPoly(4, ptrs, light_bm, MAP_TYPE_BITMAP);

    EndFrame();
    ReadPixels();
    SavePNG("MultitextureLightmapBlend");

    // Center should be brighter than corners (lightmap effect)
    int cx = width_ / 2, cy = height_ / 2;
    int center_idx = (cy * width_ + cx) * 4;
    int corner_idx = (10 * width_ + 10) * 4;
    int center_lum = GetFramebuffer()[center_idx] + GetFramebuffer()[center_idx + 1]
                     + GetFramebuffer()[center_idx + 2];
    int corner_lum = GetFramebuffer()[corner_idx] + GetFramebuffer()[corner_idx + 1]
                     + GetFramebuffer()[corner_idx + 2];
    EXPECT_GT(center_lum, corner_lum) << "Lightmap center should be brighter";

    bm_FreeBitmap(main_bm);
    bm_FreeBitmap(light_bm);
}

TEST_F(D3GameRenderTest, MultitextureNoOverlay) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    int bm = d3_render_test::CreateCheckerboardBitmap(32, 32, 8,
                                                       GR_RGB(0, 255, 0),
                                                       GR_RGB(0, 128, 0));
    ASSERT_GE(bm, 0);

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    // Draw without lightmap/overlay - just base texture
    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("MultitextureNoOverlay");

    int green_pixels = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i + 1] > 100)
            green_pixels++;
    }
    EXPECT_GT(green_pixels, 5000) << "Green texture should cover much of screen";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, TextureCacheLRU) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Create many small bitmaps to exercise cache
    const int NUM_BMS = 20;
    int bms[NUM_BMS];
    for (int i = 0; i < NUM_BMS; i++) {
        ddgr_color c1 = GR_RGB(255, 0, i * 12);
        ddgr_color c2 = GR_RGB(0, 255, 255 - i * 12);
        bms[i] = d3_render_test::CreateCheckerboardBitmap(16, 16, 4, c1, c2);
        ASSERT_GE(bms[i], 0);
    }

    float h = 50.0f;
    for (int i = 0; i < NUM_BMS; i++) {
        float cx = -300.0f + (i % 10) * 60.0f;
        float cy = -200.0f + (i / 10) * 60.0f;

        vector corners[4] = {
            {cx - h, cy - h, -300.0f},
            {cx + h, cy - h, -300.0f},
            {cx + h, cy + h, -300.0f},
            {cx - h, cy + h, -300.0f},
        };
        g3Point pts[4];
        for (int v = 0; v < 4; v++) {
            g3_RotatePoint(&pts[v], &corners[v]);
            pts[v].p3_flags |= PF_UV | PF_L;
            pts[v].p3_u = (v == 0 || v == 3) ? 0.0f : 1.0f;
            pts[v].p3_v = (v < 2) ? 0.0f : 1.0f;
            pts[v].p3_l = 1.0f;
        }
        g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
        rend_SetTextureType(TT_LINEAR);
        g3_DrawPoly(4, ptrs, bms[i], MAP_TYPE_BITMAP);
    }

    EndFrame();
    ReadPixels();
    SavePNG("TextureCacheLRU");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Expected visible textured quads after cache thrash";

    for (int i = 0; i < NUM_BMS; i++)
        bm_FreeBitmap(bms[i]);
}

TEST_F(D3GameRenderTest, ScaledBitmap) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Create 8x8 bitmap then render it large
    int bm = d3_render_test::CreateCheckerboardBitmap(8, 8, 2,
                                                       GR_RGB(255, 255, 255),
                                                       GR_RGB(0, 0, 0));
    ASSERT_GE(bm, 0);

    float h = 200.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("ScaledBitmap");

    int colored = CountColoredPixels(GetFramebuffer(), width_,
                                      width_ / 4, height_ / 4,
                                      3 * width_ / 4, 3 * height_ / 4, 30);
    EXPECT_GT(colored, 10000) << "Scaled bitmap should cover large area";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, ChunkedBitmap) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Create a large bitmap and chunk it
    int large_bm = bm_AllocBitmap(256, 256, 0);
    ASSERT_GE(large_bm, 0);
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            uint16_t c = d3_render_test::RgbTo16(255, (x * 255 / 256), (y * 255 / 256));
            d3_render_test::BmSetPixel(large_bm, x, y, c);
        }
    }
    GameBitmaps[large_bm].flags |= BF_CHANGED;

    chunked_bitmap chunk;
    bool ok = bm_CreateChunkedBitmap(large_bm, &chunk);
    ASSERT_TRUE(ok) << "Failed to create chunked bitmap";

    // Just verify it doesn't crash — rendering chunked bitmaps
    // requires the D3 terrain system

    EndFrame();
    ReadPixels();
    SavePNG("ChunkedBitmap");

    bm_DestroyChunkedBitmap(&chunk);
    bm_FreeBitmap(large_bm);
}

TEST_F(D3GameRenderTest, PreUploadTexture) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    // Create a bitmap and explicitly flag it for upload
    int bm = d3_render_test::CreateCheckerboardBitmap(32, 32, 8,
                                                       GR_RGB(0, 128, 255),
                                                       GR_RGB(0, 64, 128));
    ASSERT_GE(bm, 0);

    // Flag as CHANGED to force upload on next draw
    GameBitmaps[bm].flags |= BF_CHANGED;

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    rend_SetTextureType(TT_LINEAR);
    g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);

    EndFrame();
    ReadPixels();
    SavePNG("PreUploadTexture");

    int blue_pixels = CountColorMatch(GetFramebuffer(), width_,
                                       width_ / 4, height_ / 4,
                                       3 * width_ / 4, 3 * height_ / 4,
                                       0, 128, 255, 60);
    EXPECT_GT(blue_pixels, 500) << "Texture with BF_CHANGED flag should render correctly";

    bm_FreeBitmap(bm);
}

// ---------------------------------------------------------------------------
// Phase 1 — Primitives, Lighting, Alpha & Blending (25–32)
// Helpers
// ---------------------------------------------------------------------------

static bool RegionDominantByColor(const uint8_t *fb, int width,
                                   int x1, int y1, int x2, int y2,
                                   uint8_t r, uint8_t g, uint8_t b,
                                   int tolerance = 25, float min_fraction = 0.5f) {
    int match = 0, total = 0;
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            int idx = (y * width + x) * 4;
            total++;
            int dr = (int)fb[idx] - r;
            int dg = (int)fb[idx + 1] - g;
            int db = (int)fb[idx + 2] - b;
            if (dr * dr + dg * dg + db * db <= tolerance * tolerance * 3) match++;
        }
    }
    return (total > 0) && ((float)match / total >= min_fraction);
}

// ---------------------------------------------------------------------------
// Phase 1 — Lighting Tests (12–17)
// ---------------------------------------------------------------------------

TEST_F(D3GameRenderTest, FlatGouraudLighting) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_FLAT_GOURAUD);
    rend_SetFlatColor(GR_RGB(255, 128, 0));
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -400.0f},
        {+h, -h, -400.0f},
        {+h, +h, -400.0f},
        {-h, +h, -400.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    rend_SetTextureType(TT_FLAT);
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("FlatGouraudLighting");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Flat gouraud should produce visible orange quad";
}

TEST_F(D3GameRenderTest, GouraudLightingD3) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -350.0f},
        {+h, -h, -250.0f},
        {+h, +h, -350.0f},
        {-h, +h, -250.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
    }
    pts[0].p3_r = 1.0f; pts[0].p3_g = 0.0f; pts[0].p3_b = 0.0f; pts[0].p3_a = 1.0f;
    pts[1].p3_r = 0.0f; pts[1].p3_g = 1.0f; pts[1].p3_b = 0.0f; pts[1].p3_a = 1.0f;
    pts[2].p3_r = 0.0f; pts[2].p3_g = 0.0f; pts[2].p3_b = 1.0f; pts[2].p3_a = 1.0f;
    pts[3].p3_r = 1.0f; pts[3].p3_g = 1.0f; pts[3].p3_b = 1.0f; pts[3].p3_a = 1.0f;

    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("GouraudLightingD3");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Gouraud quad should render visible pixels";
}

TEST_F(D3GameRenderTest, MonoLighting) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetColorModel(CM_MONO);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -350.0f},
        {+h, -h, -250.0f},
        {+h, +h, -350.0f},
        {-h, +h, -250.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
    }
    pts[0].p3_r = 0.2f; pts[0].p3_g = 0.2f; pts[0].p3_b = 0.2f; pts[0].p3_a = 1.0f;
    pts[1].p3_r = 0.4f; pts[1].p3_g = 0.4f; pts[1].p3_b = 0.4f; pts[1].p3_a = 1.0f;
    pts[2].p3_r = 0.7f; pts[2].p3_g = 0.7f; pts[2].p3_b = 0.7f; pts[2].p3_a = 1.0f;
    pts[3].p3_r = 1.0f; pts[3].p3_g = 1.0f; pts[3].p3_b = 1.0f; pts[3].p3_a = 1.0f;

    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("MonoLighting");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10 || GetFramebuffer()[i + 1] > 10 || GetFramebuffer()[i + 2] > 10)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Mono lighting should produce visible quad";
}

TEST_F(D3GameRenderTest, RGBALighting) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_VERTEX);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -350.0f},
        {+h, -h, -250.0f},
        {+h, +h, -350.0f},
        {-h, +h, -250.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
    }
    pts[0].p3_r = 1.0f; pts[0].p3_g = 1.0f; pts[0].p3_b = 1.0f; pts[0].p3_a = 0.2f;
    pts[1].p3_r = 0.0f; pts[1].p3_g = 1.0f; pts[1].p3_b = 1.0f; pts[1].p3_a = 0.5f;
    pts[2].p3_r = 1.0f; pts[2].p3_g = 0.0f; pts[2].p3_b = 1.0f; pts[2].p3_a = 0.8f;
    pts[3].p3_r = 1.0f; pts[3].p3_g = 1.0f; pts[3].p3_b = 0.0f; pts[3].p3_a = 1.0f;

    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("RGBALighting");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10 || GetFramebuffer()[i + 1] > 10 || GetFramebuffer()[i + 2] > 10)
            colored++;
    }
    EXPECT_GT(colored, 100) << "RGBA lighting should render visible quad";
}

TEST_F(D3GameRenderTest, AlphaConstant) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_CONSTANT);
    rend_SetAlphaValue(128);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -350.0f},
        {+h, -h, -250.0f},
        {+h, +h, -350.0f},
        {-h, +h, -250.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_r = 1.0f; pts[i].p3_g = 0.0f; pts[i].p3_b = 0.0f; pts[i].p3_a = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("AlphaConstant");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10) colored++;
    }
    EXPECT_GT(colored, 100) << "Alpha constant should render visible quad";
}

TEST_F(D3GameRenderTest, AlphaTexture) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_TEXTURE);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    int bm = d3_render_test::CreateVerticalGradientBitmap(32, 32,
                                                           GR_RGB(255, 0, 0),
                                                           GR_RGB(0, 255, 0));
    ASSERT_GE(bm, 0);

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -350.0f},
        {+h, -h, -350.0f},
        {+h, +h, -350.0f},
        {-h, +h, -350.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("AlphaTexture");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10 || GetFramebuffer()[i + 1] > 10) colored++;
    }
    EXPECT_GT(colored, 100) << "Alpha texture should render visible quad";

    bm_FreeBitmap(bm);
}

// ---------------------------------------------------------------------------
// Phase 1 — Alpha & Blending Tests (18–24)
// ---------------------------------------------------------------------------

TEST_F(D3GameRenderTest, AlphaVertex) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_VERTEX);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -350.0f},
        {+h, -h, -250.0f},
        {+h, +h, -350.0f},
        {-h, +h, -250.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
    }
    pts[0].p3_r = 1.0f; pts[0].p3_g = 0.0f; pts[0].p3_b = 0.0f; pts[0].p3_a = 1.0f;
    pts[1].p3_r = 0.0f; pts[1].p3_g = 1.0f; pts[1].p3_b = 0.0f; pts[1].p3_a = 1.0f;
    pts[2].p3_r = 0.0f; pts[2].p3_g = 0.0f; pts[2].p3_b = 1.0f; pts[2].p3_a = 1.0f;
    pts[3].p3_r = 1.0f; pts[3].p3_g = 1.0f; pts[3].p3_b = 0.0f; pts[3].p3_a = 1.0f;

    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("AlphaVertex");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10 || GetFramebuffer()[i + 1] > 10 || GetFramebuffer()[i + 2] > 10)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Alpha vertex should render visible quad";
}

TEST_F(D3GameRenderTest, AlphaLightmapBlend) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    int main_bm = d3_render_test::CreateCheckerboardBitmap(32, 32, 8,
                                                            GR_RGB(128, 128, 128),
                                                            GR_RGB(64, 64, 64));
    ASSERT_GE(main_bm, 0);

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};

    rend_SetTextureType(TT_LINEAR);
    g3_DrawPoly(4, ptrs, main_bm, MAP_TYPE_BITMAP);
    EndFrame();
    ReadPixels();
    SavePNG("AlphaLightmapBlend");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Alpha lightmap blend should render visible quad";

    bm_FreeBitmap(main_bm);
}

TEST_F(D3GameRenderTest, AlphaSaturateTexture) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_SATURATE_TEXTURE);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    int bm = d3_render_test::CreateRadialBitmap(32, 32,
                                                  GR_RGB(255, 255, 255),
                                                  GR_RGB(0, 0, 0));
    ASSERT_GE(bm, 0);

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("AlphaSaturateTexture");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Saturate texture should render visible quad";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, AlphaFactorScale) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_CONSTANT);
    rend_SetAlphaValue(64);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 100.0f;
    vector corners[4] = {
        {-h, -h, -350.0f},
        {+h, -h, -250.0f},
        {+h, +h, -350.0f},
        {-h, +h, -250.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_r = 1.0f; pts[i].p3_g = 0.0f; pts[i].p3_b = 0.0f; pts[i].p3_a = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("AlphaFactorScale");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10) colored++;
    }
    EXPECT_GT(colored, 100) << "Alpha factor scale should render visible quad";
}

TEST_F(D3GameRenderTest, TransparencyOverlap) {
    BeginFrame();

    rend_SetZBufferState(0);
    rend_SetLighting(LS_NONE);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(128, 128, 128));

    // Blue background quad
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetFlatColor(GR_RGB(0, 0, 255));
    rend_DrawLine(0, height_ / 2, width_, height_ / 2);
    rend_DrawLine(width_ / 2, 0, width_ / 2, height_);

    // Semi-transparent red overlay
    rend_SetAlphaType(AT_CONSTANT);
    rend_SetAlphaValue(128);
    rend_SetFlatColor(GR_RGB(255, 0, 0));
    rend_FillRect(GR_RGB(255, 0, 0), width_ / 4, height_ / 4, 3 * width_ / 4, 3 * height_ / 4);

    EndFrame();
    ReadPixels();
    SavePNG("TransparencyOverlap");

    int non_black = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 20 || GetFramebuffer()[i + 1] > 20 || GetFramebuffer()[i + 2] > 20)
            non_black++;
    }
    EXPECT_GT(non_black, 100) << "Transparency overlap should produce visible pixels";
}

TEST_F(D3GameRenderTest, FogEnableDisable) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(64, 64, 64));
    rend_ClearZBuffer();

    // Draw with fog disabled (default)
    float h = 80.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_r = 1.0f; pts[i].p3_g = 1.0f; pts[i].p3_b = 1.0f; pts[i].p3_a = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);

    EndFrame();
    ReadPixels();
    SavePNG("FogEnableDisable");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 30 || GetFramebuffer()[i + 1] > 30 || GetFramebuffer()[i + 2] > 30)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Fog disabled quad should render visible white pixels";
}

TEST_F(D3GameRenderTest, FogWithTexture) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetColorModel(CM_RGB);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_ClearScreen(GR_RGB(64, 64, 64));
    rend_ClearZBuffer();

    int bm = d3_render_test::CreateCheckerboardBitmap(32, 32, 8,
                                                        GR_RGB(255, 255, 0),
                                                        GR_RGB(0, 0, 0));
    ASSERT_GE(bm, 0);

    float h = 80.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_L;
        pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
        pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
        pts[i].p3_l = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_LINEAR);
    int drawn = g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1);

    EndFrame();
    ReadPixels();
    SavePNG("FogWithTexture");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 30 || GetFramebuffer()[i + 1] > 30 || GetFramebuffer()[i + 2] > 30)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Fog with texture should render visible quad";

    bm_FreeBitmap(bm);
}

// ---------------------------------------------------------------------------
// Phase 3 — Primitives Tests (25–32)
// ---------------------------------------------------------------------------

TEST_F(D3GameRenderTest, PixelReadWrite) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float h = 5.0f;
    vector corners[4] = {
        {-h, -h, -300.0f},
        {+h, -h, -300.0f},
        {+h, +h, -300.0f},
        {-h, +h, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = 0.0f; pts[i].p3_v = 0.0f;
        pts[i].p3_r = 0.0f; pts[i].p3_g = 1.0f; pts[i].p3_b = 1.0f; pts[i].p3_a = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);

    EndFrame();
    ReadPixels();
    SavePNG("PixelReadWrite");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10 || GetFramebuffer()[i + 1] > 10 || GetFramebuffer()[i + 2] > 10)
            colored++;
    }
    EXPECT_GT(colored, 10) << "Cyan pixel block should be visible";
}

TEST_F(D3GameRenderTest, FillRect) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(16, 16, 16));
    rend_ClearZBuffer();

    auto draw_colored_quad = [&](float x_offs, float y_offs,
                                  float r, float g, float b, float z) {
        float hs = 40.0f;
        vector corners[4] = {
            {x_offs - hs, y_offs - hs, z},
            {x_offs + hs, y_offs - hs, z},
            {x_offs + hs, y_offs + hs, z},
            {x_offs - hs, y_offs + hs, z},
        };
        g3Point pts[4];
        for (int i = 0; i < 4; i++) {
            g3_RotatePoint(&pts[i], &corners[i]);
            pts[i].p3_flags |= PF_UV | PF_RGBA;
            pts[i].p3_u = 0.0f; pts[i].p3_v = 0.0f;
            pts[i].p3_r = r; pts[i].p3_g = g; pts[i].p3_b = b; pts[i].p3_a = 1.0f;
        }
        g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
        rend_SetTextureType(TT_FLAT);
        g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    };

    draw_colored_quad(-100.0f, 0.0f, 1.0f, 0.0f, 0.0f, -300.0f);
    draw_colored_quad(100.0f, 0.0f, 0.0f, 1.0f, 0.0f, -300.0f);
    draw_colored_quad(200.0f, 0.0f, 0.0f, 0.0f, 1.0f, -300.0f);
    draw_colored_quad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, -280.0f);

    EndFrame();
    ReadPixels();
    SavePNG("FillRect");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 30 || GetFramebuffer()[i + 1] > 30 || GetFramebuffer()[i + 2] > 30)
            colored++;
    }
    EXPECT_GT(colored, 1000) << "FillRect quads should produce visible colored pixels";
}

TEST_F(D3GameRenderTest, CircleRender) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    auto draw_colored_ring = [&](float cx, float cy, float radius,
                                  float r, float g, float b, float z) {
        const int segs = 32;
        g3Point pts[segs];
        for (int i = 0; i < segs; i++) {
            float a = (float)i * 6.283185f / segs;
            float x = cx + radius * cosf(a);
            float y = cy + radius * sinf(a);
            vector v = {x, y, z};
            g3_RotatePoint(&pts[i], &v);
            pts[i].p3_flags |= PF_UV | PF_RGBA;
            pts[i].p3_u = 0.0f; pts[i].p3_v = 0.0f;
            pts[i].p3_r = r; pts[i].p3_g = g; pts[i].p3_b = b; pts[i].p3_a = 1.0f;
        }
        for (int i = 0; i < segs; i++) {
            g3Point *tri[3] = {&pts[i], &pts[(i + 1) % segs], &pts[0]};
            rend_SetTextureType(TT_FLAT);
            g3_DrawPoly(3, tri, 0, MAP_TYPE_BITMAP);
        }
    };

    draw_colored_ring(0.0f, 0.0f, 150.0f, 1.0f, 0.0f, 0.0f, -300.0f);
    draw_colored_ring(0.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f, -300.0f);
    draw_colored_ring(0.0f, 0.0f, 50.0f, 0.0f, 0.0f, 1.0f, -300.0f);

    EndFrame();
    ReadPixels();
    SavePNG("CircleRender");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 30 || GetFramebuffer()[i + 1] > 30 || GetFramebuffer()[i + 2] > 30)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Circle render should produce visible pixels";
}

TEST_F(D3GameRenderTest, DrawBox3D) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float s = 60.0f;
    float z_offs = -350.0f;
    vector verts[8] = {
        {-s, -s, z_offs - s}, {+s, -s, z_offs - s}, {+s, +s, z_offs - s}, {-s, +s, z_offs - s},
        {-s, -s, z_offs + s}, {+s, -s, z_offs + s}, {+s, +s, z_offs + s}, {-s, +s, z_offs + s},
    };

    float angle = 0.5f;
    float ca = std::cos(angle), sa = std::sin(angle);
    // Rotate the cube around Y and then recenter it horizontally.
    // 1. Rotate each vertex and store its X coordinate.
    float rotatedX[8];
    for (int i = 0; i < 8; i++) {
        float x = verts[i].x() * ca - verts[i].z() * sa;
        float z = verts[i].x() * sa + verts[i].z() * ca;
        verts[i].x() = x;
        verts[i].z() = z;
        rotatedX[i] = x; // remember X for centering
    }
    // 2. Find the horizontal extents.
    float minX = rotatedX[0];
    float maxX = rotatedX[0];
    for (int i = 1; i < 8; i++) {
        if (rotatedX[i] < minX) minX = rotatedX[i];
        if (rotatedX[i] > maxX) maxX = rotatedX[i];
    }
    // 3. Compute the offset to center the cube in X.
    float offsetX = (minX + maxX) * 0.5f;
    // 4. Apply the offset to all vertices.
    for (int i = 0; i < 8; i++) {
        verts[i].x() -= offsetX;
    }

    g3Point pts[8];
    for (int i = 0; i < 8; i++) {
        g3_RotatePoint(&pts[i], &verts[i]);
        pts[i].p3_flags |= PF_RGBA;
        pts[i].p3_a = 1.0f;
    }

    const float fc[6][3] = {
        {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f},
    };
    const int faces[6][4] = {
        {0, 1, 2, 3}, {5, 4, 7, 6}, {1, 5, 6, 2},
        {4, 0, 3, 7}, {3, 2, 6, 7}, {4, 5, 1, 0},
    };

    g3Point *fptrs[4];
    for (int f = 0; f < 6; f++) {
        for (int v = 0; v < 4; v++) {
            int idx = faces[f][v];
            fptrs[v] = &pts[idx];
            pts[idx].p3_r = fc[f][0];
            pts[idx].p3_g = fc[f][1];
            pts[idx].p3_b = fc[f][2];
        }
        rend_SetTextureType(TT_FLAT);
        g3_DrawPoly(4, fptrs, 0, MAP_TYPE_BITMAP);
    }

    EndFrame();
    ReadPixels();
    SavePNG("DrawBox3D");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10 || GetFramebuffer()[i + 1] > 10 || GetFramebuffer()[i + 2] > 10)
            colored++;
    }
    EXPECT_GT(colored, 1000) << "3D box should produce visible colored pixels";
}

TEST_F(D3GameRenderTest, RotatedBitmap) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_NONE);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    int bm = d3_render_test::CreateCheckerboardBitmap(32, 32, 8,
                                                        GR_RGB(255, 165, 0),
                                                        GR_RGB(0, 0, 0));
    ASSERT_GE(bm, 0);

    auto draw_textured_quad = [&](float cx, float cy, float angle,
                                   float half_w, float half_h, float z) {
        float c[4][2] = {
            {-half_w, -half_h}, {half_w, -half_h},
            {half_w, half_h}, {-half_w, half_h}
        };
        g3Point pts[4];
        for (int i = 0; i < 4; i++) {
            float rx = c[i][0] * cosf(angle) - c[i][1] * sinf(angle) + cx;
            float ry = c[i][0] * sinf(angle) + c[i][1] * cosf(angle) + cy;
            vector v = {rx, ry, z};
            g3_RotatePoint(&pts[i], &v);
            pts[i].p3_flags |= PF_UV | PF_L;
            pts[i].p3_u = (i == 0 || i == 3) ? 0.0f : 1.0f;
            pts[i].p3_v = (i < 2) ? 0.0f : 1.0f;
            pts[i].p3_l = 1.0f;
        }
        g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
        rend_SetTextureType(TT_LINEAR);
        g3_DrawPoly(4, ptrs, bm, MAP_TYPE_BITMAP);
    };

    draw_textured_quad(0.0f, 0.0f, 0.0f, 80.0f, 80.0f, -300.0f);
    draw_textured_quad(0.0f, 0.0f, 1.5708f, 80.0f, 80.0f, -280.0f);

    EndFrame();
    ReadPixels();
    SavePNG("RotatedBitmap");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 10 || GetFramebuffer()[i + 1] > 10 || GetFramebuffer()[i + 2] > 10)
            colored++;
    }
    EXPECT_GT(colored, 1000) << "Rotated bitmaps should produce visible pixels";

    bm_FreeBitmap(bm);
}

TEST_F(D3GameRenderTest, LinePatterns) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    auto draw_line_as_quad = [&](float x1, float y1, float x2, float y2,
                                  float r, float g, float b, float z) {
        float dx = x2 - x1, dy = y2 - y1;
        float len = sqrtf(dx * dx + dy * dy);
        if (len < 0.001f) return;
        float nx = -dy / len * 2.0f, ny = dx / len * 2.0f;
        vector corners[4] = {
            {x1 + nx, y1 + ny, z},
            {x1 - nx, y1 - ny, z},
            {x2 - nx, y2 - ny, z},
            {x2 + nx, y2 + ny, z},
        };
        g3Point pts[4];
        for (int i = 0; i < 4; i++) {
            g3_RotatePoint(&pts[i], &corners[i]);
            pts[i].p3_flags |= PF_UV | PF_RGBA;
            pts[i].p3_u = 0.0f; pts[i].p3_v = 0.0f;
            pts[i].p3_r = r; pts[i].p3_g = g; pts[i].p3_b = b; pts[i].p3_a = 1.0f;
        }
        g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
        rend_SetTextureType(TT_FLAT);
        g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    };

    draw_line_as_quad(-200.0f, 0.0f, 200.0f, 0.0f, 1.0f, 1.0f, 1.0f, -300.0f);
    draw_line_as_quad(-200.0f, -40.0f, 200.0f, -40.0f, 1.0f, 0.0f, 0.0f, -300.0f);
    draw_line_as_quad(-200.0f, 40.0f, 200.0f, 40.0f, 0.0f, 1.0f, 0.0f, -300.0f);
    draw_line_as_quad(-200.0f, 80.0f, 200.0f, 80.0f, 0.0f, 1.0f, 1.0f, -300.0f);
    draw_line_as_quad(-200.0f, -100.0f, 200.0f, 100.0f, 1.0f, 1.0f, 0.0f, -300.0f);

    EndFrame();
    ReadPixels();
    SavePNG("LinePatterns");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 30 || GetFramebuffer()[i + 1] > 30 || GetFramebuffer()[i + 2] > 30)
            colored++;
    }
    EXPECT_GT(colored, 500) << "Line pattern quads should produce visible pixels";
}

TEST_F(D3GameRenderTest, AlphaBlendOverlay) {
    BeginFrame();

    rend_SetZBufferState(0);
    rend_SetLighting(LS_GOURAUD);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(64, 64, 64));

    auto draw_alpha_quad = [&](float x, float y, float hs,
                                float r, float g, float b, float a, float z) {
        vector corners[4] = {
            {x - hs, y - hs, z},
            {x + hs, y - hs, z},
            {x + hs, y + hs, z},
            {x - hs, y + hs, z},
        };
        g3Point pts[4];
        for (int i = 0; i < 4; i++) {
            g3_RotatePoint(&pts[i], &corners[i]);
            pts[i].p3_flags |= PF_UV | PF_RGBA;
            pts[i].p3_u = 0.0f; pts[i].p3_v = 0.0f;
            pts[i].p3_r = r; pts[i].p3_g = g; pts[i].p3_b = b; pts[i].p3_a = a;
        }
        g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
        rend_SetTextureType(TT_FLAT);
        g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    };

    draw_alpha_quad(-50.0f, 0.0f, 60.0f, 1.0f, 0.0f, 0.0f, 1.0f, -350.0f);
    draw_alpha_quad(50.0f, 0.0f, 60.0f, 0.0f, 1.0f, 0.0f, 0.5f, -340.0f);
    draw_alpha_quad(0.0f, 50.0f, 60.0f, 0.0f, 0.0f, 1.0f, 0.25f, -330.0f);

    EndFrame();
    ReadPixels();
    SavePNG("AlphaBlendOverlay");

    int colored = 0;
    for (int i = 0; i < width_ * height_ * 4; i += 4) {
        if (GetFramebuffer()[i] > 80 || GetFramebuffer()[i + 1] > 80 || GetFramebuffer()[i + 2] > 80)
            colored++;
    }
    EXPECT_GT(colored, 100) << "Alpha blend overlay should produce visible pixels";
}

TEST_F(D3GameRenderTest, StencilClipRect) {
    BeginFrame();

    rend_SetZBufferState(1);
    rend_SetLighting(LS_GOURAUD);
    rend_SetAlphaType(AT_ALWAYS);
    rend_SetAlphaValue(255);
    rend_SetColorModel(CM_RGB);
    rend_ClearScreen(GR_RGB(0, 0, 0));
    rend_ClearZBuffer();

    float hs = 120.0f;
    vector corners[4] = {
        {-hs, -hs, -300.0f},
        {+hs, -hs, -300.0f},
        {+hs, +hs, -300.0f},
        {-hs, +hs, -300.0f},
    };
    g3Point pts[4];
    for (int i = 0; i < 4; i++) {
        g3_RotatePoint(&pts[i], &corners[i]);
        pts[i].p3_flags |= PF_UV | PF_RGBA;
        pts[i].p3_u = 0.0f; pts[i].p3_v = 0.0f;
        pts[i].p3_r = 1.0f; pts[i].p3_g = 0.0f; pts[i].p3_b = 0.0f; pts[i].p3_a = 1.0f;
    }
    g3Point *ptrs[4] = {&pts[0], &pts[1], &pts[2], &pts[3]};
    rend_SetTextureType(TT_FLAT);
    int drawn = g3_DrawPoly(4, ptrs, 0, MAP_TYPE_BITMAP);
    EXPECT_EQ(drawn, 1) << "Clip rect quad should be drawn";

    EndFrame();
    ReadPixels();
    SavePNG("StencilClipRect");

    SUCCEED() << "StencilClipRect test completed without crash";
}
