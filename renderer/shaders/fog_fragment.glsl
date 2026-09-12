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
 * VOLUMETRIC FOG PASS (Phase 2)
 * =============================
 * Ray-marches a procedural 3D density field per-pixel and composites the
 * accumulated in-scattering over the rendered scene.  Runs as a full-screen
 * pass after the scene is resolved to a texture.
 *
 * Phase 2 adds light interaction: the in-scattering term is driven by the
 * level's actual sun (direction + color, set via rend_SetSunLight) instead
 * of a hardcoded direction, and screen-space god rays (light shafts) are
 * sampled from the bright areas of the resolved scene along the line toward
 * the sun's screen position.
 *
 * Depth convention: the renderer uses a near=0 / far=infinity projection
 * (proj[2][2]=1, proj[2][3]=1, proj[3][2]=-1), so the depth buffer value is
 * depth = 1/(2*d) where d is the view-space distance.  The ray direction for
 * a pixel is derived directly from the projection scale factors (u_proj00,
 * u_proj11) and the NDC position, avoiding an inverse-projection matrix.
 */

in vec2 v_uv;

out vec4 out_color;

uniform sampler2D u_scene_color;
uniform sampler2D u_scene_depth;
uniform vec3 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;
uniform float u_fog_density;
uniform float u_noise_scale;
uniform float u_noise_freq;
uniform int u_steps;
uniform float u_proj00;
uniform float u_proj11;
uniform vec3 u_sun_dir;       // world-space direction toward the sun
uniform vec3 u_sun_color;     // sun RGB intensity (HDR allowed)
uniform vec2 u_sun_screen;    // sun position in scene UV coords (god rays)
uniform int u_god_rays;       // 0/1 enable screen-space light shafts
uniform int u_god_ray_samples;
uniform mat4 u_inv_view;
uniform int u_fog_enable;

// Hash-based value noise (3D).  Deterministic, no texture lookups.
float hash(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.zyx + 31.32);
    return fract((p.x + p.y) * p.z);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(
        mix(mix(hash(i + vec3(0, 0, 0)), hash(i + vec3(1, 0, 0)), f.x),
            mix(hash(i + vec3(0, 1, 0)), hash(i + vec3(1, 1, 0)), f.x), f.y),
        mix(mix(hash(i + vec3(0, 0, 1)), hash(i + vec3(1, 0, 1)), f.x),
            mix(hash(i + vec3(0, 1, 1)), hash(i + vec3(1, 1, 1)), f.x), f.y),
        f.z);
}

// Fractal Brownian motion: 3 octaves of value noise for a cloudy density field.
float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 3; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// Screen-space god rays (light shafts): sample the resolved scene along the
// line from the sun's screen position to this pixel, accumulating bright
// pixels (light sources) as shafts.  Samples are weighted toward the sun so
// shafts fade with distance from it.
vec3 god_rays(vec2 uv) {
    vec2 dir = uv - u_sun_screen;
    vec2 step = dir / float(u_god_ray_samples);
    vec3 accum = vec3(0.0);
    float weight = 0.0;
    vec2 p = u_sun_screen;
    for (int i = 0; i < 16; i++) {
        if (i >= u_god_ray_samples)
            break;
        p += step;
        vec3 c = texture(u_scene_color, p).rgb;
        float lum = dot(c, vec3(0.299, 0.587, 0.114));
        float bright = smoothstep(0.6, 1.0, lum);
        float falloff = 1.0 - float(i) / float(u_god_ray_samples);
        accum += c * bright * falloff;
        weight += falloff;
    }
    return accum / max(weight, 1e-4);
}

void main() {
    vec3 scene_color = texture(u_scene_color, v_uv).rgb;

    if (u_fog_enable == 0) {
        out_color = vec4(scene_color, 1.0);
        return;
    }

    // Reconstruct the view-space ray for this pixel.
    vec2 ndc = v_uv * 2.0 - 1.0;
    float depth = texture(u_scene_depth, v_uv).r;
    // depth = 1/(2*d) for the near=0/far=infinity projection; clamp so the
    // far plane (depth=0) and z-bias artifacts can't produce NaN/infinity.
    float surface_dist = 1.0 / max(2.0 * depth, 1e-6);
    surface_dist = min(surface_dist, u_fog_end);
    vec3 ray_dir = normalize(vec3(ndc.x / u_proj00, ndc.y / u_proj11, -1.0));

    float march_start = max(u_fog_start, 0.0);
    float march_end = max(surface_dist, march_start);
    float step_size = (march_end - march_start) / float(u_steps);

    // God rays: light shafts from the sun, sampled once per pixel and added
    // to the composite weighted by the total fog along the ray.
    vec3 god = (u_god_rays == 1) ? god_rays(v_uv) : vec3(0.0);

    float transmittance = 1.0;
    vec3 scattered = vec3(0.0);

    for (int i = 0; i < 32; i++) {
        if (i >= u_steps)
            break;
        float t = march_start + step_size * (float(i) + 0.5);
        vec3 view_pos = ray_dir * t;

        // World-space position for a camera-independent density field.
        vec4 world = u_inv_view * vec4(view_pos, 1.0);
        vec3 world_pos = world.xyz / world.w;

        // Procedural density: base density modulated by FBM noise.
        float density = u_fog_density * (1.0 + u_noise_scale * (fbm(world_pos * u_noise_freq) - 0.5));
        density = max(density, 0.0);

        // Sun-driven in-scattering: fog brightens when looking toward the sun.
        float scattering = 0.6 + 0.4 * max(dot(ray_dir, u_sun_dir), 0.0);

        float step_transmittance = exp(-density * step_size);
        scattered += transmittance * (1.0 - step_transmittance) * u_fog_color * scattering;
        transmittance *= step_transmittance;
    }

    // Composite: scene attenuated by fog, plus in-scattering, plus god rays
    // (shaft light appears where fog is present along the ray).
    out_color = vec4(scene_color * transmittance + scattered + god * u_sun_color * (1.0 - transmittance), 1.0);
}