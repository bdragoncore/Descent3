/*
 * Descent 3
 * Copyright (C) 2024 Parallax Software, 2026 InsanityBringer (PiccuEngine)
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

// cockpit_factory.cpp — Creates the active ICockpit implementation

#include "cockpit_factory.h"
#include "LegacyCockpit.h"
#include "WidescreenCockpit.h"
#include "log.h"

static ICockpit *s_active_cockpit = nullptr;
static int s_cockpit_mode = COCKPIT_MODE_WIDESCREEN;

ICockpit *GetCockpit() { return s_active_cockpit; }

void SetCockpitMode(int mode) { s_cockpit_mode = mode; }

int GetCockpitMode() { return s_cockpit_mode; }

ICockpit *CreateCockpit(int mode) {
  // Free existing cockpit if any
  if (s_active_cockpit) {
    s_active_cockpit->Free();
    delete s_active_cockpit;
    s_active_cockpit = nullptr;
  }

  s_cockpit_mode = mode;

  switch (mode) {
  case COCKPIT_MODE_LEGACY:
    LOG_INFO << "Creating legacy (4:3) cockpit.";
    s_active_cockpit = new LegacyCockpit();
    break;
  case COCKPIT_MODE_WIDESCREEN:
  default:
    LOG_INFO << "Creating widescreen cockpit.";
    s_active_cockpit = new WidescreenCockpit();
    break;
  }

  return s_active_cockpit;
}
