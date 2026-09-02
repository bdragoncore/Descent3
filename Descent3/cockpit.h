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

// cockpit.h — Cockpit system public API
// Free functions that delegate to the active ICockpit implementation.
// All callers use these functions; the underlying implementation is
// selected at runtime via CreateCockpit().

#ifndef COCKPIT_H
#define COCKPIT_H

#include "ICockpit.h"

// renders the cockpit.
void RenderCockpit();

// initialization of cockpit information.
void InitCockpit(int ship_index);

// Forces freeing of cockpit information
void FreeCockpit();

// check if cockpit exists
bool IsValidCockpit();

// forces opening of cockpit on hud
void OpenCockpit();

// forces closing of cockpit on hud.
void CloseCockpit();

// resizes cockpit.
void ResizeCockpit();

// forces quick open of cockpit
void QuickOpenCockpit();

// forces quick closing of cockpit
void QuickCloseCockpit();

// returns the submodel number of the monitor requested
bsp_info *CockpitGetMonitorSubmodel(uint16_t monitor_flag);

// returns the polymodel for the hud
poly_model *CockpitGetPolyModel();

// cockpit effects.
void StartCockpitShake(float mag, vector *vec);

// Tell whether the cockpit is animating
int CockpitState();

#endif
