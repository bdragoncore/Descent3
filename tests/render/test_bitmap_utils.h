#ifndef TESTS_RENDER_TEST_BITMAP_UTILS_H
#define TESTS_RENDER_TEST_BITMAP_UTILS_H

#include "bitmap.h"
#include "grdefs.h"
#include <cmath>

namespace d3_render_test {

// Pack 8-bit RGB into D3 16-bit 5-5-5-1 format with opaque flag set
inline uint16_t RgbTo16(uint8_t r, uint8_t g, uint8_t b) {
  return GR_RGB16(r, g, b) | OPAQUE_FLAG;
}

// Write a 16-bit pixel to a bitmap via bm_data pointer
inline void BmSetPixel(int bm, int x, int y, uint16_t color) {
  uint16_t *data = bm_data(bm, 0);
  if (data)
    data[y * bm_rowsize(bm, 0) / sizeof(uint16_t) + x] = color;
}

inline uint16_t BmReadPixel(int bm, int x, int y) {
  uint16_t *data = bm_data(bm, 0);
  if (data)
    return data[y * bm_rowsize(bm, 0) / sizeof(uint16_t) + x];
  return 0;
}

// Create a checkerboard bitmap
inline int CreateCheckerboardBitmap(int w, int h, int cell_size,
                                     ddgr_color color1, ddgr_color color2) {
  int bm = bm_AllocBitmap(w, h, 0);
  if (bm < 0) return -1;

  uint16_t c1 = RgbTo16(GR_COLOR_RED(color1), GR_COLOR_GREEN(color1), GR_COLOR_BLUE(color1));
  uint16_t c2 = RgbTo16(GR_COLOR_RED(color2), GR_COLOR_GREEN(color2), GR_COLOR_BLUE(color2));

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int cell_x = (x / cell_size) % 2;
      int cell_y = (y / cell_size) % 2;
      BmSetPixel(bm, x, y, (cell_x == cell_y) ? c1 : c2);
    }
  }
  GameBitmaps[bm].flags |= BF_CHANGED;
  return bm;
}

// Create a radial gradient bitmap
inline int CreateRadialBitmap(int w, int h,
                                ddgr_color center_color, ddgr_color edge_color) {
  int bm = bm_AllocBitmap(w, h, 0);
  if (bm < 0) return -1;

  float cx = w / 2.0f, cy = h / 2.0f;
  float max_dist = std::sqrt(cx * cx + cy * cy);

  uint8_t cr = GR_COLOR_RED(center_color), cg = GR_COLOR_GREEN(center_color), cb = GR_COLOR_BLUE(center_color);
  uint8_t er = GR_COLOR_RED(edge_color), eg = GR_COLOR_GREEN(edge_color), eb = GR_COLOR_BLUE(edge_color);

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      float dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
      float t = (dist > max_dist) ? 1.0f : (dist / max_dist);
      uint8_t r = (uint8_t)(cr + (int)(er - cr) * t);
      uint8_t g = (uint8_t)(cg + (int)(eg - cg) * t);
      uint8_t b = (uint8_t)(cb + (int)(eb - cb) * t);
      BmSetPixel(bm, x, y, RgbTo16(r, g, b));
    }
  }
  GameBitmaps[bm].flags |= BF_CHANGED;
  return bm;
}

// Create a vertical gradient bitmap
inline int CreateVerticalGradientBitmap(int w, int h,
                                          ddgr_color top_color, ddgr_color bottom_color) {
  int bm = bm_AllocBitmap(w, h, 0);
  if (bm < 0) return -1;

  uint8_t tr = GR_COLOR_RED(top_color), tg = GR_COLOR_GREEN(top_color), tb = GR_COLOR_BLUE(top_color);
  uint8_t br = GR_COLOR_RED(bottom_color), bg = GR_COLOR_GREEN(bottom_color), bb = GR_COLOR_BLUE(bottom_color);

  for (int y = 0; y < h; y++) {
    float t = (float)y / (h - 1);
    uint8_t r = (uint8_t)(tr + (int)(br - tr) * t);
    uint8_t g = (uint8_t)(tg + (int)(bg - tg) * t);
    uint8_t b = (uint8_t)(tb + (int)(bb - tb) * t);
    uint16_t c = RgbTo16(r, g, b);
    for (int x = 0; x < w; x++) {
      BmSetPixel(bm, x, y, c);
    }
  }
  GameBitmaps[bm].flags |= BF_CHANGED;
  return bm;
}

// Create a 4-quadrant colored bitmap
inline int CreateQuadrantBitmap(int w, int h,
                                  ddgr_color tl, ddgr_color tr,
                                  ddgr_color bl, ddgr_color br) {
  int bm = bm_AllocBitmap(w, h, 0);
  if (bm < 0) return -1;

  uint16_t c_tl = RgbTo16(GR_COLOR_RED(tl), GR_COLOR_GREEN(tl), GR_COLOR_BLUE(tl));
  uint16_t c_tr = RgbTo16(GR_COLOR_RED(tr), GR_COLOR_GREEN(tr), GR_COLOR_BLUE(tr));
  uint16_t c_bl = RgbTo16(GR_COLOR_RED(bl), GR_COLOR_GREEN(bl), GR_COLOR_BLUE(bl));
  uint16_t c_br = RgbTo16(GR_COLOR_RED(br), GR_COLOR_GREEN(br), GR_COLOR_BLUE(br));

  int mid_x = w / 2, mid_y = h / 2;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint16_t color;
      if (y < mid_y) color = (x < mid_x) ? c_tl : c_tr;
      else           color = (x < mid_x) ? c_bl : c_br;
      BmSetPixel(bm, x, y, color);
    }
  }
  GameBitmaps[bm].flags |= BF_CHANGED;
  return bm;
}

}  // namespace d3_render_test

#endif
