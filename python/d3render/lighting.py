"""
Descent 3 Lighting and Color Determination

DeterminePointColor, rend_GetAlphaMultiplier.
Ported from renderer/HardwareBaseGPU.cpp.
"""

from __future__ import annotations

from d3render.types import g3Point, color_array
from d3render.constants import *
from d3render.grdefs import GR_COLOR_RED, GR_COLOR_GREEN, GR_COLOR_BLUE
from d3render.globals import gpu_state, gpu_Alpha_factor, gpu_Alpha_multiplier


def rend_GetAlphaMultiplier() -> float:
    """Return the alpha multiplier based on current alpha type.

    C++ source: renderer/HardwareBaseGPU.cpp:69
    """
    atype = gpu_state.cur_alpha_type
    if atype in (AT_ALWAYS, AT_TEXTURE, AT_VERTEX, AT_TEXTURE_VERTEX,
                 AT_SATURATE_VERTEX, AT_SATURATE_TEXTURE_VERTEX, AT_SPECULAR):
        return 1.0
    elif atype in (AT_CONSTANT, AT_CONSTANT_TEXTURE, AT_CONSTANT_TEXTURE_VERTEX,
                   AT_CONSTANT_VERTEX, AT_LIGHTMAP_BLEND, AT_LIGHTMAP_BLEND_SATURATE,
                   AT_SATURATE_TEXTURE, AT_SATURATE_CONSTANT_VERTEX):
        return gpu_state.cur_alpha / 255.0
    else:
        return 0.0  # Unknown type


def DeterminePointColor(pnt: g3Point, disableGouraud: bool = False,
                        checkTextureQuality: bool = False,
                        flatColorForNoLight: bool = False) -> color_array:
    """Determine the final color for a vertex based on lighting and alpha modes.

    Parameters:
        pnt: The g3Point to compute color for.
        disableGouraud: If True, skip flat gouraud path (used by gpu_DrawFlatPolygon3D).
        checkTextureQuality: If True, check texture quality for flat color.
        flatColorForNoLight: If True, use flat color when LS_NONE.

    Returns:
        color_array with r, g, b, a components (0.0-1.0).

    C++ source: renderer/HardwareBaseGPU.cpp:455
    """
    # Compute alpha
    alpha = gpu_Alpha_multiplier * gpu_Alpha_factor
    if gpu_state.cur_alpha_type & ATF_VERTEX:
        alpha *= pnt.p3_a

    # Determine RGB
    if ((gpu_state.cur_light_state == LS_FLAT_GOURAUD and not disableGouraud) or
        (gpu_state.cur_texture_quality == 0 and checkTextureQuality) or
        (gpu_state.cur_light_state == LS_NONE and flatColorForNoLight)):
        # Flat color override
        return color_array(
            r=GR_COLOR_RED(gpu_state.cur_color) / 255.0,
            g=GR_COLOR_GREEN(gpu_state.cur_color) / 255.0,
            b=GR_COLOR_BLUE(gpu_state.cur_color) / 255.0,
            a=alpha,
        )
    elif gpu_state.cur_light_state == LS_NONE:
        # No lighting: white
        return color_array(r=1.0, g=1.0, b=1.0, a=alpha)
    elif gpu_state.cur_color_model == CM_MONO:
        # Mono: intensity
        return color_array(r=pnt.p3_l, g=pnt.p3_l, b=pnt.p3_l, a=alpha)
    else:
        # RGB: per-vertex color
        return color_array(r=pnt.p3_r, g=pnt.p3_g, b=pnt.p3_b, a=alpha)
