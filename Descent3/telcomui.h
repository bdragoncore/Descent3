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

#ifndef TELCOMUI_H
#define TELCOMUI_H

#include "grdefs.h"

// BUGFIX #685: the TelCom UI (briefing screens, goals, cargo, automap, ship
// select) is authored for a 640x480 screen. All TelCom layout is kept in art
// coordinates internally; these helpers convert to and from the current window
// at the draw and input boundaries so the UI fills high-resolution displays
// the same way the main menu art does.

// art-space -> window-space
inline int TelcomScaledX(int x, int window_w) { return x * window_w / FIXED_SCREEN_WIDTH; }
inline int TelcomScaledY(int y, int window_h) { return y * window_h / FIXED_SCREEN_HEIGHT; }

// window-space -> art-space (mouse input)
inline int TelcomUnscaledX(int x, int window_w) { return x * FIXED_SCREEN_WIDTH / window_w; }
inline int TelcomUnscaledY(int y, int window_h) { return y * FIXED_SCREEN_HEIGHT / window_h; }

#endif // TELCOMUI_H