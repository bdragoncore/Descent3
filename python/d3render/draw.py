"""
Descent 3 Draw Functions

rend_DrawPolygon3D, rend_DrawPolygon2D, rend_DrawMultitexturePolygon3D,
gpu_DrawFlatPolygon3D.

Ported from renderer/HardwareBaseGPU.cpp.
"""

from __future__ import annotations
from typing import List, Tuple

from d3render.types import g3Point, PosColorUVVertex, PosColorUV2Vertex, color_array, tex_array
from d3render.constants import *
from d3render.lighting import DeterminePointColor
from d3render.state import gpu_state, gpu_Overlay_map, gpu_Overlay_type
from d3render.transforms import g3_RefreshTransforms


def rend_DrawPolygon3D(handle: int, p: List[g3Point], nv: int,
                       map_type: int = MAP_TYPE_BITMAP) -> List[PosColorUVVertex]:
    """Draw a 3D polygon with texture.

    Returns the vertex array that would be sent to the GPU.
    C++ source: renderer/HardwareBaseGPU.cpp:479
    """
    # Refresh transforms for 3D rendering
    g3_RefreshTransforms(usePassthru=False)

    # Flat path: no texture
    if gpu_state.cur_texture_quality == 0:
        return gpu_DrawFlatPolygon3D(p, nv)

    # Multitexture path
    if gpu_Overlay_type != OT_NONE:
        return rend_DrawMultitexturePolygon3D(handle, p, nv, map_type)

    # Standard textured path
    vertices = []
    for i in range(nv):
        pnt = p[i]
        assert pnt.p3_flags & PF_ORIGPOINT, f"Vertex {i} missing PF_ORIGPOINT"

        color = DeterminePointColor(pnt, disableGouraud=False,
                                     checkTextureQuality=False,
                                     flatColorForNoLight=True)

        v = PosColorUVVertex(
            pos=list(pnt.p3_vecPreRot),
            color=color,
            uv=tex_array(s=pnt.p3_u, t=pnt.p3_v, r=0.0, w=1.0),
        )
        vertices.append(v)

    return vertices


def gpu_DrawFlatPolygon3D(p: List[g3Point], nv: int) -> List[PosColorUVVertex]:
    """Draw a flat (untextured) polygon.

    Returns the vertex array.
    C++ source: renderer/HardwareBaseGPU.cpp (Mesa) / HardwareOpenGL.cpp:1183
    """
    vertices = []
    for i in range(nv):
        pnt = p[i]
        color = DeterminePointColor(pnt, disableGouraud=True,
                                     checkTextureQuality=True,
                                     flatColorForNoLight=False)

        v = PosColorUVVertex(
            pos=list(pnt.p3_vecPreRot),
            color=color,
            uv=tex_array(s=0.0, t=0.0, r=0.0, w=1.0),
        )
        vertices.append(v)

    return vertices


def rend_DrawMultitexturePolygon3D(handle: int, p: List[g3Point], nv: int,
                                    map_type: int) -> List[PosColorUV2Vertex]:
    """Draw a polygon with multitexture (lightmap overlay).

    Returns the dual-UV vertex array.
    C++ source: renderer/HardwareBaseGPU.cpp:534
    """
    # Lightmap UV scaling (simplified — no actual lightmap data in Python port)
    # In C++: GameLightmaps[gpu_Overlay_map].square_res, .width, .height
    square_res = 128  # default
    lightmap_width = 128
    lightmap_height = 128

    one_over_square_res = 1.0 / square_res
    xscalar = lightmap_width * one_over_square_res
    yscalar = lightmap_height * one_over_square_res

    vertices = []
    for i in range(nv):
        pnt = p[i]
        assert pnt.p3_flags & PF_ORIGPOINT, f"Vertex {i} missing PF_ORIGPOINT"

        color = DeterminePointColor(pnt, disableGouraud=True,
                                     checkTextureQuality=False,
                                     flatColorForNoLight=False)

        v = PosColorUV2Vertex(
            pos=list(pnt.p3_vecPreRot),
            color=color,
            uv0=tex_array(s=pnt.p3_u, t=pnt.p3_v, r=0.0, w=1.0),
            uv1=tex_array(s=pnt.p3_u2 * xscalar, t=pnt.p3_v2 * yscalar, r=0.0, w=1.0),
        )
        vertices.append(v)

    return vertices


def rend_DrawPolygon2D(handle: int, p: List[g3Point], nv: int,
                       map_type: int = MAP_TYPE_BITMAP) -> List[PosColorUVVertex]:
    """Draw a 2D polygon (screen-space, with texture).

    Returns the vertex array.
    C++ source: renderer/HardwareBaseGPU.cpp:419
    """
    # Refresh transforms for 2D (passthru = ortho projection)
    g3_RefreshTransforms(usePassthru=True)

    # Add clip offset to screen coords
    xAdd = gpu_state.clip_x1
    yAdd = gpu_state.clip_y1

    vertices = []
    for i in range(nv):
        pnt = p[i]

        color = DeterminePointColor(pnt, disableGouraud=False,
                                     checkTextureQuality=True,
                                     flatColorForNoLight=False)

        v = PosColorUVVertex(
            pos=[pnt.p3_sx + xAdd, pnt.p3_sy + yAdd, 0.0],
            color=color,
            uv=tex_array(s=pnt.p3_u, t=pnt.p3_v, r=0.0, w=1.0),
        )
        vertices.append(v)

    return vertices
