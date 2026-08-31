"""
Descent 3 Rendering State Machine

rendering_state, all setter functions (rend_SetLighting, rend_SetAlphaType,
rend_SetZBufferState, etc.), and state-dependent globals.

Ported from renderer/HardwareBaseGPU.cpp, renderer/MesaOpenGL.cpp,
and renderer/HardwareOpenGL.cpp.
"""

from __future__ import annotations

from d3render.globals import (
    gpu_state, gpu_preferred_state, gpu_Alpha_factor, gpu_Alpha_multiplier,
    gpu_Overlay_map, gpu_Overlay_type, Renderer_initted, Renderer_type,
    gpu_last_frame_polys_drawn, gpu_last_frame_verts_processed, gpu_last_uploaded,
    Renderer_error_message, UseHardware, NoLightmaps, StateLimited, UseWBuffer,
    Z_bias,
)
from d3render.types import rendering_state, renderer_preferred_state, tRendererStats
from d3render.constants import *
from d3render.grdefs import ddgr_color
from d3render.lighting import rend_GetAlphaMultiplier


# ── State setters ──
def rend_SetLighting(state: int) -> None:
    """Set lighting state (LS_NONE, LS_GOURAUD, LS_FLAT_GOURAUD)."""
    global gpu_state
    if state == gpu_state.cur_light_state:
        return
    gpu_state.cur_light_state = state


def rend_SetColorModel(state: int) -> None:
    """Set color model (CM_MONO, CM_RGB)."""
    global gpu_state
    gpu_state.cur_color_model = state


def rend_SetTextureType(state: int) -> None:
    """Set texture type. Also sets cur_texture_quality.

    TT_FLAT → quality=0 (flat path)
    Others → quality=2 (textured path)
    """
    global gpu_state
    if state == gpu_state.cur_texture_type:
        return

    if state == TT_FLAT:
        gpu_state.cur_texture_quality = 0
    else:
        gpu_state.cur_texture_quality = 2

    gpu_state.cur_texture_type = state


def rend_SetFlatColor(color: ddgr_color) -> None:
    """Set flat fill color."""
    global gpu_state
    gpu_state.cur_color = color


def rend_SetAlphaType(atype: int) -> None:
    """Set alpha blending type.

    Updates gpu_state.cur_alpha_type and recomputes gpu_Alpha_multiplier.
    C++ source: renderer/MesaOpenGL.cpp:1092
    """
    global gpu_state, gpu_Alpha_multiplier
    if atype == gpu_state.cur_alpha_type:
        return

    gpu_state.cur_alpha_type = atype
    gpu_Alpha_multiplier = rend_GetAlphaMultiplier()


def rend_SetAlphaValue(val: int) -> None:
    """Set constant alpha value (0-255).

    Recomputes gpu_Alpha_multiplier.
    """
    global gpu_state, gpu_Alpha_multiplier
    gpu_state.cur_alpha = val
    gpu_Alpha_multiplier = rend_GetAlphaMultiplier()


def rend_SetAlphaFactor(val: float) -> None:
    """Set overall alpha scale factor (motion blur)."""
    global gpu_Alpha_factor
    gpu_Alpha_factor = max(0.0, min(1.0, val))


def rend_GetAlphaFactor() -> float:
    """Get alpha factor."""
    return gpu_Alpha_factor


def rend_SetZBufferState(state: int) -> None:
    """Enable/disable z-buffer.

    In the Python port, this only updates the state tracker.
    """
    global gpu_state
    if state == gpu_state.cur_zbuffer_state:
        return
    gpu_state.cur_zbuffer_state = state


def rend_SetBilinearState(state: int) -> None:
    """Set bilinear filtering state."""
    global gpu_state
    gpu_state.cur_bilinear_state = state


def rend_SetFiltering(state: int) -> None:
    """Set bilinear filtering (alias for rend_SetBilinearState)."""
    global gpu_state
    gpu_state.cur_bilinear_state = state


def rend_SetWrapType(val: int) -> None:
    """Set texture wrap type (WT_WRAP, WT_CLAMP, WT_WRAP_V)."""
    global gpu_state
    gpu_state.cur_wrap_type = val


def rend_SetZBias(z_bias: float) -> None:
    """Set z-bias for depth offset."""
    global Z_bias
    if Z_bias != z_bias:
        Z_bias = z_bias
        from d3render.transforms import g3_GetModelViewMatrix, gTransformModelView
        from d3render.transforms import View_position, Unscaled_matrix
        from d3render.transforms import g3_UpdateFullTransform, g3_ForceTransformRefresh
        g3_GetModelViewMatrix(View_position, Unscaled_matrix, gTransformModelView)
        g3_UpdateFullTransform()
        g3_ForceTransformRefresh()


def rend_SetOverlayMap(handle: int) -> None:
    """Set overlay texture map handle."""
    global gpu_Overlay_map
    gpu_Overlay_map = handle


def rend_SetOverlayType(type_: int) -> None:
    """Set overlay operation type (OT_NONE, OT_BLEND)."""
    global gpu_Overlay_type
    gpu_Overlay_type = type_


def rend_SetRendererType(state: int) -> None:
    """Set renderer type."""
    global Renderer_type
    Renderer_type = state


def rend_SetPreferredState(pref_state: renderer_preferred_state, reinit: bool = False) -> int:
    """Set preferred renderer state."""
    global gpu_preferred_state
    old_state = renderer_preferred_state(
        mipping=gpu_preferred_state.mipping,
        filtering=gpu_preferred_state.filtering,
        gamma=gpu_preferred_state.gamma,
        bit_depth=gpu_preferred_state.bit_depth,
        width=gpu_preferred_state.width,
        height=gpu_preferred_state.height,
        vsync_on=gpu_preferred_state.vsync_on,
        fullscreen=gpu_preferred_state.fullscreen,
    )

    gpu_preferred_state = renderer_preferred_state(
        mipping=pref_state.mipping,
        filtering=pref_state.filtering,
        gamma=pref_state.gamma,
        bit_depth=pref_state.bit_depth,
        width=pref_state.width,
        height=pref_state.height,
        vsync_on=pref_state.vsync_on,
        fullscreen=pref_state.fullscreen,
    )

    if gpu_state.initted:
        if old_state.fullscreen != pref_state.fullscreen:
            rend_SetFullScreen(pref_state.fullscreen)

        if (pref_state.width != gpu_state.screen_width or
            pref_state.height != gpu_state.screen_height or
            old_state.bit_depth != pref_state.bit_depth or reinit):
            rend_ReInit()

        if old_state.gamma != pref_state.gamma:
            rend_SetGammaValue(pref_state.gamma)

    return 1


def rend_SetGammaValue(val: float) -> None:
    """Set gamma value."""
    global gpu_preferred_state
    gpu_preferred_state.gamma = val


def rend_SetFullScreen(fullscreen: bool) -> None:
    """Toggle fullscreen (no-op in Python port)."""
    pass


def rend_ReInit() -> int:
    """Reinitialize renderer (no-op in Python port)."""
    return 1


def rend_GetRenderState(rstate: rendering_state) -> None:
    """Copy current state into rstate."""
    rstate.initted = gpu_state.initted
    rstate.cur_bilinear_state = gpu_state.cur_bilinear_state
    rstate.cur_zbuffer_state = gpu_state.cur_zbuffer_state
    rstate.cur_texture_type = gpu_state.cur_texture_type
    rstate.cur_color_model = gpu_state.cur_color_model
    rstate.cur_light_state = gpu_state.cur_light_state
    rstate.cur_alpha_type = gpu_state.cur_alpha_type
    rstate.cur_wrap_type = gpu_state.cur_wrap_type
    rstate.cur_alpha = gpu_state.cur_alpha
    rstate.cur_color = gpu_state.cur_color
    rstate.cur_texture_quality = gpu_state.cur_texture_quality
    rstate.clip_x1 = gpu_state.clip_x1
    rstate.clip_x2 = gpu_state.clip_x2
    rstate.clip_y1 = gpu_state.clip_y1
    rstate.clip_y2 = gpu_state.clip_y2
    rstate.screen_width = gpu_state.screen_width
    rstate.screen_height = gpu_state.screen_height


def rend_GetProjectionParameters() -> tuple:
    """Get projection dimensions (width, height)."""
    width = gpu_state.clip_x2 - gpu_state.clip_x1
    height = gpu_state.clip_y2 - gpu_state.clip_y1
    return width, height


def rend_GetProjectionScreenParameters() -> tuple:
    """Get full screen parameters (lx, ty, w, h)."""
    lx = gpu_state.clip_x1
    ty = gpu_state.clip_y1
    w = gpu_state.clip_x2 - gpu_state.clip_x1 + 1
    h = gpu_state.clip_y2 - gpu_state.clip_y1 + 1
    return lx, ty, w, h


def rend_GetAspectRatio() -> float:
    """Get aspect ratio."""
    return (3.0 * gpu_state.screen_width) / (4.0 * gpu_state.screen_height)


def rend_GetStatistics(stats: tRendererStats) -> None:
    """Get renderer statistics."""
    if Renderer_initted:
        stats.poly_count = gpu_last_frame_polys_drawn
        stats.vert_count = gpu_last_frame_verts_processed
        stats.texture_uploads = gpu_last_uploaded
    else:
        stats.poly_count = 0
        stats.vert_count = 0
        stats.texture_uploads = 0


def rend_GetErrorMessage() -> str:
    """Get error message."""
    return Renderer_error_message


def rend_SetErrorMessage(msg: str) -> None:
    """Set error message."""
    global Renderer_error_message
    Renderer_error_message = msg
