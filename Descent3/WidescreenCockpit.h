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

// WidescreenCockpit.h — Aspect-ratio-corrected cockpit for widescreen displays
// Inherits from LegacyCockpit and overrides Render() to apply a horizontal
// vertex scale that counteracts the stretching caused by non-4:3 projection.

#ifndef WIDESCREENCOCKPIT_H
#define WIDESCREENCOCKPIT_H

#include "LegacyCockpit.h"

class WidescreenCockpit : public LegacyCockpit {
public:
  WidescreenCockpit();
  ~WidescreenCockpit() override;

  void Render() override;

private:
  float ComputeHorizontalScale() const;
};

#endif
