"""
Descent 3 Data Structures

g3Point, g3UVL, g3Codes, color_array, tex_array, PosColorUVVertex,
PosColorUV2Vertex, rendering_state, renderer_preferred_state.
Ported from lib/3d.h, renderer/HardwareInternal.h, lib/renderer.h.
"""

from __future__ import annotations
from dataclasses import dataclass, field
from typing import List, Optional
import copy

from d3render.constants import *
from d3render.grdefs import ddgr_color


# ── g3UVL — UV coordinates and lighting values (lib/3d.h) ──
@dataclass
class g3UVL:
    u: float = 0.0   # Primary U
    v: float = 0.0   # Primary V
    u2: float = 0.0  # Lightmap U
    v2: float = 0.0  # Lightmap V
    # Union: l (intensity) or r (red)
    l: float = 0.0   # Intensity lighting (CM_MONO)
    g: float = 0.0   # Green
    b: float = 0.0   # Blue
    a: float = 1.0   # Alpha

    @property
    def r(self) -> float:
        """Red channel (shares storage with l)."""
        return self.l

    @r.setter
    def r(self, value: float):
        self.l = value


# ── g3Codes — clipping codes (lib/3d.h) ──
@dataclass
class g3Codes:
    cc_or: int = 0   # OR of all vertex clip codes
    cc_and: int = 0xFF  # AND of all vertex clip codes


# ── g3Point — 3D point (lib/3d.h) ──
@dataclass
class g3Point:
    p3_sx: float = 0.0        # Screen X
    p3_sy: float = 0.0        # Screen Y
    p3_codes: int = 0         # Clipping codes (CC_*)
    p3_flags: int = 0         # Flags (PF_*)
    p3_vec: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0])       # Camera-space XYZ
    p3_vecPreRot: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0]) # World-space XYZ
    p3_uvl: g3UVL = field(default_factory=g3UVL)  # UV + lighting

    # Convenience accessors matching C++ macros
    @property
    def p3_x(self) -> float:
        return self.p3_vec[0]

    @p3_x.setter
    def p3_x(self, v: float):
        self.p3_vec[0] = v

    @property
    def p3_y(self) -> float:
        return self.p3_vec[1]

    @p3_y.setter
    def p3_y(self, v: float):
        self.p3_vec[1] = v

    @property
    def p3_z(self) -> float:
        return self.p3_vec[2]

    @p3_z.setter
    def p3_z(self, v: float):
        self.p3_vec[2] = v

    @property
    def p3_u(self) -> float:
        return self.p3_uvl.u

    @p3_u.setter
    def p3_u(self, v: float):
        self.p3_uvl.u = v

    @property
    def p3_v(self) -> float:
        return self.p3_uvl.v

    @p3_v.setter
    def p3_v(self, v: float):
        self.p3_uvl.v = v

    @property
    def p3_l(self) -> float:
        return self.p3_uvl.l

    @p3_l.setter
    def p3_l(self, v: float):
        self.p3_uvl.l = v

    @property
    def p3_r(self) -> float:
        return self.p3_uvl.r

    @p3_r.setter
    def p3_r(self, v: float):
        self.p3_uvl.r = v

    @property
    def p3_g(self) -> float:
        return self.p3_uvl.g

    @p3_g.setter
    def p3_g(self, v: float):
        self.p3_uvl.g = v

    @property
    def p3_b(self) -> float:
        return self.p3_uvl.b

    @p3_b.setter
    def p3_b(self, v: float):
        self.p3_uvl.b = v

    @property
    def p3_a(self) -> float:
        return self.p3_uvl.a

    @p3_a.setter
    def p3_a(self, v: float):
        self.p3_uvl.a = v

    @property
    def p3_u2(self) -> float:
        return self.p3_uvl.u2

    @p3_u2.setter
    def p3_u2(self, v: float):
        self.p3_uvl.u2 = v

    @property
    def p3_v2(self) -> float:
        return self.p3_uvl.v2

    @p3_v2.setter
    def p3_v2(self, v: float):
        self.p3_uvl.v2 = v

    def copy(self) -> g3Point:
        """Deep copy of this point."""
        return g3Point(
            p3_sx=self.p3_sx,
            p3_sy=self.p3_sy,
            p3_codes=self.p3_codes,
            p3_flags=self.p3_flags,
            p3_vec=list(self.p3_vec),
            p3_vecPreRot=list(self.p3_vecPreRot),
            p3_uvl=g3UVL(
                u=self.p3_uvl.u, v=self.p3_uvl.v,
                u2=self.p3_uvl.u2, v2=self.p3_uvl.v2,
                l=self.p3_uvl.l, g=self.p3_uvl.g,
                b=self.p3_uvl.b, a=self.p3_uvl.a,
            ),
        )


# ── Internal vertex types (renderer/HardwareInternal.h) ──
@dataclass
class color_array:
    r: float = 0.0
    g: float = 0.0
    b: float = 0.0
    a: float = 1.0


@dataclass
class tex_array:
    s: float = 0.0
    t: float = 0.0
    r: float = 0.0
    w: float = 1.0


@dataclass
class PosColorUVVertex:
    """Single-UV vertex for flat/textured rendering."""
    pos: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0])
    color: color_array = field(default_factory=color_array)
    uv: tex_array = field(default_factory=tex_array)


@dataclass
class PosColorUV2Vertex:
    """Dual-UV vertex for multitexture (lightmap) rendering."""
    pos: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0])
    color: color_array = field(default_factory=color_array)
    uv0: tex_array = field(default_factory=tex_array)
    uv1: tex_array = field(default_factory=tex_array)


# ── rendering_state (lib/renderer.h) ──
@dataclass
class rendering_state:
    initted: int = 0
    cur_bilinear_state: int = -1
    cur_zbuffer_state: int = -1
    cur_texture_type: int = TT_FLAT
    cur_color_model: int = CM_MONO
    cur_light_state: int = LS_GOURAUD
    cur_alpha_type: int = AT_TEXTURE
    cur_wrap_type: int = WT_WRAP
    cur_alpha: int = 255
    cur_color: ddgr_color = 0x00FFFFFF
    cur_texture_quality: int = -1
    clip_x1: int = 0
    clip_x2: int = 0
    clip_y1: int = 0
    clip_y2: int = 0
    screen_width: int = 640
    screen_height: int = 480


# ── renderer_preferred_state (lib/renderer.h) ──
@dataclass
class renderer_preferred_state:
    mipping: int = 0
    filtering: int = 1
    gamma: float = 1.5
    bit_depth: int = 32
    width: int = 800
    height: int = 600
    vsync_on: int = 0
    fullscreen: bool = False


# ── tRendererStats (lib/renderer.h) ──
@dataclass
class tRendererStats:
    poly_count: int = 0
    vert_count: int = 0
    texture_uploads: int = 0
