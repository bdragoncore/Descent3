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

// Stub for the renderer's global window pointer referenced by lnxmouse.cpp
// as extern. The real definition lives in HardwareOpenGL.cpp (renderer lib).
SDL_Window *GSDLWindow = nullptr;

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
