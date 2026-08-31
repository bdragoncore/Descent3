"""
Descent 3 Rendering Globals

Shared state variables to break circular imports between lighting.py and state.py.
"""

from __future__ import annotations

from d3render.types import rendering_state, renderer_preferred_state, tRendererStats
from d3render.constants import *
from d3render.grdefs import ddgr_color

# ── Global state ──
gpu_state = rendering_state()
gpu_preferred_state = renderer_preferred_state()
gpu_Alpha_factor: float = 1.0
gpu_Alpha_multiplier: float = 1.0

# Overlay state
gpu_Overlay_map: int = -1
gpu_Overlay_type: int = OT_NONE

# Renderer status
Renderer_initted: int = 0
Renderer_type: int = RENDERER_MESA

# Frame stats
gpu_last_frame_polys_drawn: int = 0
gpu_last_frame_verts_processed: int = 0
gpu_last_uploaded: int = 0

# Error message
Renderer_error_message: str = "Generic renderer error"

# Feature flags
UseHardware: bool = True
NoLightmaps: bool = False
StateLimited: bool = False
UseWBuffer: bool = False

# Z-bias
Z_bias: float = 0.0
