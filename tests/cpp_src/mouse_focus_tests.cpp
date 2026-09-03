/*
 * Descent 3
 * Copyright (C) 2024 Parallax Software
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file mouse_focus_tests.cpp
 * @brief Tests for BUGFIX #676: Mouse focus not captured at runtime.
 *
 * These tests verify the ddio mouse grab state management layer.
 * The full fix also requires SDL_SetWindowRelativeMouseMode calls
 * in lnxmouse.cpp and SDL_EVENT_WINDOW_FOCUS_GAINED handling in
 * sdlmain.cpp, which require a running SDL context to test.
 */

#include <gtest/gtest.h>
#include <SDL3/SDL.h>

#include "ddio.h"
#include "ddio_lnx.h"
#include "lnxapp.h"

// Stub for the renderer's global window pointer referenced by lnxmouse.cpp
// as extern. The real definition lives in HardwareOpenGL.cpp (renderer lib).
SDL_Window *GSDLWindow = nullptr;

// sdlMouseMotionFilter is defined in ddio/lnxmouse.cpp (part of the ddio lib).
bool sdlMouseMotionFilter(SDL_Event const *event);

// Minimal stubs for oeLnxApplication (linux/lnxapp.cpp) so the test can
// construct an app object for Lnx_app_obj without linking the whole linux
// library. flags=0 avoids the console/atexit paths in the real ctor.
oeLnxApplication::oeLnxApplication(unsigned flags) {
  m_Flags = flags;
  m_AppActive = true;
  m_X = m_Y = m_W = m_H = 0;
}
oeLnxApplication::oeLnxApplication(tLnxAppInfo *appinfo) {
  m_Flags = appinfo->flags;
  m_X = appinfo->wnd_x;
  m_Y = appinfo->wnd_y;
  m_W = appinfo->wnd_w;
  m_H = appinfo->wnd_h;
  m_AppActive = true;
}
oeLnxApplication::~oeLnxApplication() {}
void oeLnxApplication::init() {}
void oeLnxApplication::get_info(void *info) {
  tLnxAppInfo *appinfo = (tLnxAppInfo *)info;
  appinfo->flags = m_Flags;
  appinfo->wnd_x = m_X;
  appinfo->wnd_y = m_Y;
  appinfo->wnd_w = m_W;
  appinfo->wnd_h = m_H;
}
unsigned oeLnxApplication::defer() { return 0; }
void oeLnxApplication::set_defer_handler(void (*)(bool)) {}
void oeLnxApplication::delay(float) {}
int oeLnxApplication::flags() const { return m_Flags; }
void oeLnxApplication::set_sizepos(int x, int y, int w, int h) {
  m_X = x;
  m_Y = y;
  m_W = w;
  m_H = h;
}
const char *oeLnxApplication::get_window_name() { return "Descent 3"; }
void oeLnxApplication::clear_window() {}

class MouseFocusTest : public ::testing::Test {
protected:
  void SetUp() override {
    ddio_MouseSetGrab(true);
  }
};

TEST_F(MouseFocusTest, DefaultGrabStateIsTrue) {
  EXPECT_TRUE(ddio_MouseGetGrab());
}

TEST_F(MouseFocusTest, SetGrabFalse) {
  ddio_MouseSetGrab(false);
  EXPECT_FALSE(ddio_MouseGetGrab());
}

TEST_F(MouseFocusTest, SetGrabTrue) {
  ddio_MouseSetGrab(false);
  ddio_MouseSetGrab(true);
  EXPECT_TRUE(ddio_MouseGetGrab());
}

TEST_F(MouseFocusTest, SetGrabIsIdempotent) {
  ddio_MouseSetGrab(true);
  EXPECT_TRUE(ddio_MouseGetGrab());
  ddio_MouseSetGrab(true);
  EXPECT_TRUE(ddio_MouseGetGrab());
}

// Verify that ddio_MouseGetGrab and ddio_MouseSetGrab work as a pair,
// which is the state management contract that the SDL integration depends on.
TEST_F(MouseFocusTest, GrabToggleRoundTrip) {
  ddio_MouseSetGrab(false);
  EXPECT_FALSE(ddio_MouseGetGrab());
  ddio_MouseSetGrab(true);
  EXPECT_TRUE(ddio_MouseGetGrab());
  ddio_MouseSetGrab(false);
  EXPECT_FALSE(ddio_MouseGetGrab());
}

// BUGFIX #676: When the mouse is not grabbed (e.g. -nomousegrab), SDL
// reports xrel/yrel as 0, so the virtual position never updates and the
// cursor appears stuck. The motion filter must map the absolute window
// position into the virtual coordinate space instead.
TEST_F(MouseFocusTest, UngrabbedMotionUsesAbsolutePosition) {
  oeLnxApplication app(0u);
  app.set_sizepos(0, 0, 3440, 1440);
  Lnx_app_obj = &app;

  ddio_MouseInit();
  ddio_MouseSetVCoords(3440 * 20, 1440 * 20);
  ddio_MouseSetGrab(false);

  SDL_Event ev{};
  ev.type = SDL_EVENT_MOUSE_MOTION;
  ev.motion.x = 100;
  ev.motion.y = 200;
  ev.motion.xrel = 0;
  ev.motion.yrel = 0;
  sdlMouseMotionFilter(&ev);

  int x = -1, y = -1;
  ddio_MouseGetState(&x, &y, nullptr, nullptr, nullptr, nullptr);
  EXPECT_EQ(x, 100 * 20);
  EXPECT_EQ(y, 200 * 20);
}

// BUGFIX #676: The absolute position must be clamped to the virtual
// coordinate limits, not just the window size.
TEST_F(MouseFocusTest, UngrabbedMotionClampsToVirtualCoords) {
  oeLnxApplication app(0u);
  app.set_sizepos(0, 0, 3440, 1440);
  Lnx_app_obj = &app;

  ddio_MouseInit();
  ddio_MouseSetVCoords(3440 * 20, 1440 * 20);
  ddio_MouseSetGrab(false);

  SDL_Event ev{};
  ev.type = SDL_EVENT_MOUSE_MOTION;
  ev.motion.x = 5000; // beyond the window width
  ev.motion.y = -100; // before the window top
  ev.motion.xrel = 0;
  ev.motion.yrel = 0;
  sdlMouseMotionFilter(&ev);

  int x = -1, y = -1;
  ddio_MouseGetState(&x, &y, nullptr, nullptr, nullptr, nullptr);
  EXPECT_EQ(x, 3440 * 20 - 1);
  EXPECT_EQ(y, 0);
}

// The grabbed path must keep accumulating relative deltas (unchanged
// behavior) so in-game mouse look is unaffected.
TEST_F(MouseFocusTest, GrabbedMotionAccumulatesRelativeDeltas) {
  oeLnxApplication app(0u);
  app.set_sizepos(0, 0, 3440, 1440);
  Lnx_app_obj = &app;

  ddio_MouseInit();
  ddio_MouseSetVCoords(3440 * 20, 1440 * 20);
  ddio_MouseSetGrab(true);

  SDL_Event ev{};
  ev.type = SDL_EVENT_MOUSE_MOTION;
  ev.motion.x = 0;
  ev.motion.y = 0;
  ev.motion.xrel = 100;
  ev.motion.yrel = 50;
  sdlMouseMotionFilter(&ev);

  int x = -1, y = -1;
  ddio_MouseGetState(&x, &y, nullptr, nullptr, nullptr, nullptr);
  EXPECT_EQ(x, 1720 + 100); // initial center + delta
  EXPECT_EQ(y, 720 + 50);
}
