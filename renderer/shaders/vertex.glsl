#version 150 core

/*
 * Descent 3
 * Copyright (C) 2024 Descent Developers
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

in vec3 in_pos;
in vec4 in_color;
in vec2 in_uv0;
in vec2 in_uv1;

out vec4 vertex_color;
out vec2 vertex_uv0;
out vec2 vertex_uv1;
out vec4 vertex_modelview_pos;
out float vertex_age;

uniform mat4 u_modelview;
uniform mat4 u_projection;
uniform float u_z_bias;
uniform float u_age; // Plasma impact effect age (0 = freshly created, ~1 = fully faded)

void main()
{
    vertex_modelview_pos = u_modelview * vec4(in_pos, 1);
    gl_Position = u_projection * vertex_modelview_pos;
    // BUGFIX #685: Z-bias applied per-vertex in the vertex shader (matching the
    // legacy D3D/OpenGL software-transform renderer) instead of being baked
    // into the model-view matrix. Baking Z_bias into the matrix shifted
    // view-space Z, which after the perspective divide shifted screen-space
    // X/Y — causing scorch marks/particles to appear offset from walls.
    // Here we modify ONLY clip-space z (not w), scaled by clip.w so that the
    // perspective divide yields the correct biased screen-space depth while
    // leaving X/Y untouched.
    if (u_z_bias != 0.0) {
        float biased_w = 1.0 / (vertex_modelview_pos.z + u_z_bias);
        float biased_z = max(0.0, 1.0 - biased_w);
        gl_Position.z = biased_z * gl_Position.w;
    }
    vertex_color = in_color;
    vertex_uv0 = in_uv0;
    vertex_uv1 = in_uv1;
    vertex_age = u_age;
}
