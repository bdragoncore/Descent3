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

/*
 * FXAA 3.11 post-process pass.
 * ====================================
 * Runs as a full-screen pass after the fog pass and before the final window
 * blit.  This is the classic FXAA 3.11 (console-style) algorithm: it samples
 * the four diagonal neighbors plus the center, derives an edge direction from
 * the diagonal luma differences, reduces that direction by the local contrast
 * (so flat areas are untouched), and blends four samples along the edge.
 *
 * The final luma-range check is what prevents ghosting: if the blended result
 * falls outside the local luma min/max, the shader falls back to the closer
 * two-sample average instead of over-blending into a "double image".
 *
 * The source texture is bound to unit 0 (u_scene); u_rcp_frame carries the
 * reciprocal framebuffer size so the shader can step in texel units.
 */

in vec2 v_uv;

out vec4 out_color;

uniform sampler2D u_scene;
uniform vec2 u_rcp_frame;

#define FXAA_REDUCE_MIN (1.0 / 128.0)
#define FXAA_REDUCE_MUL (1.0 / 8.0)
#define FXAA_SPAN_MAX 8.0

void main() {
    vec2 v_rgbM = v_uv;
    vec2 v_rgbNW = v_uv + vec2(-1.0, -1.0) * u_rcp_frame;
    vec2 v_rgbNE = v_uv + vec2(1.0, -1.0) * u_rcp_frame;
    vec2 v_rgbSW = v_uv + vec2(-1.0, 1.0) * u_rcp_frame;
    vec2 v_rgbSE = v_uv + vec2(1.0, 1.0) * u_rcp_frame;

    vec3 rgbNW = texture(u_scene, v_rgbNW).xyz;
    vec3 rgbNE = texture(u_scene, v_rgbNE).xyz;
    vec3 rgbSW = texture(u_scene, v_rgbSW).xyz;
    vec3 rgbSE = texture(u_scene, v_rgbSE).xyz;
    vec4 texColor = texture(u_scene, v_rgbM);
    vec3 rgbM = texColor.xyz;
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
              max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
              dir * rcpDirMin)) * u_rcp_frame;

    vec3 rgbA = 0.5 * (
        texture(u_scene, v_rgbM + dir * (1.0 / 3.0 - 0.5)).xyz +
        texture(u_scene, v_rgbM + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(u_scene, v_rgbM + dir * -0.5).xyz +
        texture(u_scene, v_rgbM + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        out_color = vec4(rgbA, texColor.a);
    else
        out_color = vec4(rgbB, texColor.a);
}