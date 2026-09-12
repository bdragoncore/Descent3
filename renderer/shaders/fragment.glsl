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

in vec4 vertex_color;
in vec2 vertex_uv0;
in vec2 vertex_uv1;
in vec4 vertex_modelview_pos;
in float vertex_age;

out vec4 out_color;

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;
uniform int u_texture_enable;
uniform bool u_fog_enable;
uniform vec4 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;
uniform float u_gamma; // Gamma correction
uniform float u_plasma_glow; // Plasma impact glow strength (0 = off, 1 = full)
uniform vec4 u_plasma_color; // Plasma impact core color

float branchless_invert_or_zero(in float value) {
    // sign() returns 1 if val > 0, -1 if val < 0, and 0 if val == 0
    float sign_squared = sign(value) * sign(value);
    // so this will either be 1/value, or 0/-1
    return sign_squared / (value + sign_squared - 1.0);
}

void main()
{
    out_color = vertex_color
        // take advantage of the fact that we're multiplying to make vec4(1) represent a "disabled"
        // texture sample. a real sample will always have component values <= 1, bool is defined
        // to cast to either 0 or 1, so taking the max() of the sample and an _inverted_ enable
        // signal lets this ignore a texture w/o branching. we use a bitfield to save bandwidth.
        * max(texture(u_texture0, vertex_uv0), vec4(float(!bool((u_texture_enable >> 0) & 1))))
        * max(texture(u_texture1, vertex_uv1), vec4(float(!bool((u_texture_enable >> 1) & 1))));

    // Plasma impact glow effect: when enabled, blend a procedural glow over
    // the base texture. The glow fades in/out based on vertex_age (0 = born, 1 = dead).
    // A radial pulse travels from center to edge for the "electric" look.
    if (u_plasma_glow > 0.0) {
        float dist_from_center = length(vertex_uv0 - vec2(0.5));
        // Fade the glow as the effect ages
        float life_alpha = 1.0 - vertex_age;
        life_alpha = smoothstep(0.0, 0.2, life_alpha);
        // Radial pulse from center to edge
        float pulse = smoothstep(0.3 - u_plasma_glow * 0.2, 0.5, dist_from_center);
        // Glow color blends from core (cyan-green) to outer (yellow-white)
        vec3 glow_col = mix(u_plasma_color.rgb * 0.6, vec3(1.0, 1.0, 0.7), pulse);
        vec3 glow = glow_col * life_alpha * u_plasma_glow * (1.0 - dist_from_center * 0.5);
        // Additive blend the glow
        out_color.rgb += glow * 0.5;
    }

    float fog_factor = clamp((u_fog_end - length(vertex_modelview_pos)) * branchless_invert_or_zero(u_fog_end - u_fog_start), 0, 1);
    // out_color is unchanged when fog_factor is 1 (ie, fog distance == u_fog_start). thus, to disable,
    // fog_factor must also be 1. invert u_fog_enable (so that it is 1 when disabled) and take the max.
    fog_factor = max(fog_factor, float(!u_fog_enable));
    out_color = out_color * fog_factor + (1 - fog_factor) * u_fog_color;
    out_color.rgb = pow(out_color.rgb, vec3(1.0/u_gamma));
}
