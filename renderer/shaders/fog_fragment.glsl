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
 * VOLUMETRIC FOG PASS (Phase 4)
 * =============================
 * Ray-marches a procedural 3D density field per-pixel and composites the
 * accumulated in-scattering over the rendered scene.  Runs as a full-screen
 * pass after the scene is resolved to a texture.
 *
 * Phase 2 added light interaction: the in-scattering term is driven by the
 * level's actual sun (direction + color, set via rend_SetSunLight) instead
 * of a hardcoded direction, and screen-space god rays (light shafts) are
 * sampled from the bright areas of the resolved scene along the line toward
 * the sun's screen position.
 *
 * Phase 3 animates the density field: the FBM noise is advected by a wind
 * vector over time (u_wind * u_time), so the fog drifts and rolls instead
 * of being a static field.
 *
 * Phase 4 adds per-sector density volumes: each fogged room contributes an
 * AABB (u_volume_min/u_volume_max) with a density and color.  Where a
 * ray-march sample is inside a volume, its density is added to the field and
 * its color overrides the base fog color, so indoor fog varies per sector.
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
uniform float u_time;         // seconds since start (fog animation)
uniform vec3 u_wind;          // world-space drift direction for the density field
uniform int u_num_volumes;    // per-sector fog volumes (Phase 4)
uniform vec4 u_volume_min[16];   // min xyz + density
uniform vec4 u_volume_max[16];   // max xyz + unused
uniform vec4 u_volume_color[16]; // rgb + unused
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

// Samples the per-sector fog volumes at a world-space point, returning the
// added density and the fog color to use (the last volume containing the
// point wins).
void sample_volumes(vec3 p, out float density, out vec3 color) {
    density = 0.0;
    color = vec3(0.0);
    for (int i = 0; i < 16; i++) {
        if (i >= u_num_volumes)
            break;
        vec3 mn = u_volume_min[i].xyz;
        vec3 mx = u_volume_max[i].xyz;
        if (p.x >= mn.x && p.x <= mx.x && p.y >= mn.y && p.y <= mx.y && p.z >= mn.z && p.z <= mx.z) {
            density += u_volume_min[i].w;
            color = u_volume_color[i].rgb;
        }
    }
}

// Ray/AABB slab test.  Returns true and the entry/exit distances if the ray
// (origin ro, direction rd) intersects the box [mn, mx].
bool ray_hits_volume(vec3 ro, vec3 rd, vec3 mn, vec3 mx, out float t_near, out float t_far) {
    // BUGFIX #10: rd can have a zero component (axis-aligned camera), which
    // makes 1.0/rd inf and (mn-ro)*inf NaN when ro is exactly on the slab
    // face.  NaN propagates through min/max and can poison the march on some
    // drivers.  Clamp the divisor to a large finite value and restore the
    // sign so the slab test stays finite and correct for parallel rays.
    vec3 inv = 1.0 / max(abs(rd), vec3(1e-6));
    inv = mix(-inv, inv, step(0.0, rd));
    vec3 t0 = (mn - ro) * inv;
    vec3 t1 = (mx - ro) * inv;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    t_near = max(max(tmin.x, tmin.y), tmin.z);
    t_far = min(min(tmax.x, tmax.y), tmax.z);
    return t_far >= t_near;
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

    // BUGFIX #10: when the base density is zero (volume-only fog), the FBM
    // noise is multiplied by zero and contributes nothing, yet it dominated
    // the pass cost (24 hash calls per sample), locking up rendering on real
    // hardware.  Skip it entirely, and skip the march for rays that miss
    // every volume so non-fogged pixels do zero work.
    bool volume_only = (u_fog_density <= 0.0);
    bool march_needed = true;
    if (volume_only) {
        vec3 cam_pos = (u_inv_view * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
        vec3 world_dir = normalize((u_inv_view * vec4(ray_dir, 0.0)).xyz);
        march_needed = false;
        for (int i = 0; i < 16; i++) {
            if (i >= u_num_volumes)
                break;
            float t_near, t_far;
            if (ray_hits_volume(cam_pos, world_dir, u_volume_min[i].xyz, u_volume_max[i].xyz, t_near, t_far)) {
                if (t_far >= 0.0) {
                    march_needed = true;
                    break;
                }
            }
        }
    }

    // God rays: light shafts from the sun, sampled once per pixel and added
    // to the composite weighted by the total fog along the ray.
    vec3 god = (u_god_rays == 1) ? god_rays(v_uv) : vec3(0.0);

    float transmittance = 1.0;
    vec3 scattered = vec3(0.0);

    for (int i = 0; i < 32; i++) {
        if (i >= u_steps)
            break;
        if (!march_needed)
            break;
        float t = march_start + step_size * (float(i) + 0.5);
        vec3 view_pos = ray_dir * t;

        // World-space position for a camera-independent density field.
        vec4 world = u_inv_view * vec4(view_pos, 1.0);
        vec3 world_pos = world.xyz / world.w;

        // Per-sector fog volumes: added density + color override.
        float vol_density;
        vec3 vol_color;
        sample_volumes(world_pos, vol_density, vol_color);

        // Procedural density: base density modulated by FBM noise, advected
        // by the wind over time so the fog drifts and rolls, plus the
        // per-sector volume density.  Skipped entirely for volume-only fog
        // (u_fog_density == 0) where the noise is multiplied by zero.
        float density = vol_density;
        if (!volume_only) {
            vec3 sample_pos = world_pos * u_noise_freq + u_wind * u_time;
            density = u_fog_density * (1.0 + u_noise_scale * (fbm(sample_pos) - 0.5)) + vol_density;
        }
        density = max(density, 0.0);

        // Fog color: the volume color wins where a volume is present.
        vec3 fog_color = u_fog_color;
        if (vol_density > 0.0)
            fog_color = vol_color;

        // Sun-driven in-scattering: fog brightens when looking toward the sun.
        float scattering = 0.6 + 0.4 * max(dot(ray_dir, u_sun_dir), 0.0);

        float step_transmittance = exp(-density * step_size);
        scattered += transmittance * (1.0 - step_transmittance) * fog_color * scattering;
        transmittance *= step_transmittance;

        // BUGFIX #10: once the fog is effectively opaque, the remaining steps
        // contribute <1% of the scene; stop marching to bound the pass cost.
        if (transmittance < 0.01)
            break;
    }

    // Composite: scene attenuated by fog, plus in-scattering, plus god rays
    // (shaft light appears where fog is present along the ray).
    out_color = vec4(scene_color * transmittance + scattered + god * u_sun_color * (1.0 - transmittance), 1.0);
}