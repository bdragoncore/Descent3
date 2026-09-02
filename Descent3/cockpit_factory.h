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

// cockpit_factory.h — Creates the active ICockpit implementation
// based on the user's config setting.

#ifndef COCKPIT_FACTORY_H
#define COCKPIT_FACTORY_H

#include "ICockpit.h"

// Creates and sets the active cockpit implementation based on mode.
// Call this once at startup (after LoadGameSettings) and whenever
// the user changes the cockpit mode setting.
ICockpit *CreateCockpit(int mode);

#endif
