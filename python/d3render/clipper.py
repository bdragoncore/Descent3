"""
Descent 3 Polygon Clipping

Sutherland-Hodgman clipping against 6 frustum planes.
g3_ClipPolygon, ClipLine, ClipEdge, ClipFarEdge, ClipCustomEdge,
ClipPlane, InitFreePoints, GetTempPoint, FreeTempPoint, CheckTempPoints.

Ported from renderer/HardwareClipper.cpp.
"""

from __future__ import annotations
from typing import List, Tuple, Optional

from d3render.vecmat import vector, scalar, vm_Dot3Product
from d3render.transforms import (
    Far_clip_z, Clip_custom, Clip_plane, Clip_plane_point, Matrix_scale,
)
from d3render.types import g3Point
from d3render.constants import *
from d3render.points import g3_CodePoint


# ── Temp point pool ──
MAX_POINTS_IN_POLY = 100
_temp_points: List[g3Point] = []
_free_points: List[Optional[g3Point]] = []
_free_point_num: int = 0


def InitFreePoints() -> None:
    """Initialize the temp point pool. Called at start of each frame."""
    global _free_point_num, _free_points, _temp_points
    _temp_points = [g3Point() for _ in range(MAX_POINTS_IN_POLY)]
    _free_points = list(_temp_points)
    _free_point_num = 0


def GetTempPoint() -> g3Point:
    """Allocate a temp point from the pool."""
    global _free_point_num
    assert _free_point_num < MAX_POINTS_IN_POLY, "clipper: temp point pool exhausted"
    p = _free_points[_free_point_num]
    _free_point_num += 1
    p.p3_flags = PF_TEMP_POINT
    return p


def FreeTempPoint(p: g3Point) -> None:
    """Free a temp point back to the pool."""
    global _free_point_num
    assert p.p3_flags & PF_TEMP_POINT, "clipper: freeing non-temp point"
    _free_point_num -= 1
    _free_points[_free_point_num] = p
    p.p3_flags &= ~PF_TEMP_POINT


def CheckTempPoints() -> None:
    """Verify all temp points are freed (debug only)."""
    if _free_point_num != 0:
        # In debug mode this would Int3()
        InitFreePoints()


def g3_FreeTempPoints(pointlist: List[g3Point], nv: int) -> None:
    """Free all temp points in a point list."""
    for i in range(nv):
        if pointlist[i].p3_flags & PF_TEMP_POINT:
            FreeTempPoint(pointlist[i])
    CheckTempPoints()


# ── Edge clipping ──
def _interpolate(on_pnt: g3Point, off_pnt: g3Point, k: float) -> g3Point:
    """Interpolate between two points with factor k, preserving flags."""
    tmp = GetTempPoint()
    tmp.p3_vec = [
        on_pnt.p3_vec[0] + (off_pnt.p3_vec[0] - on_pnt.p3_vec[0]) * k,
        on_pnt.p3_vec[1] + (off_pnt.p3_vec[1] - on_pnt.p3_vec[1]) * k,
        on_pnt.p3_vec[2] + (off_pnt.p3_vec[2] - on_pnt.p3_vec[2]) * k,
    ]
    tmp.p3_x = tmp.p3_vec[0]
    tmp.p3_y = tmp.p3_vec[1]
    tmp.p3_z = tmp.p3_vec[2]

    if on_pnt.p3_flags & PF_UV:
        tmp.p3_u = on_pnt.p3_u + (off_pnt.p3_u - on_pnt.p3_u) * k
        tmp.p3_v = on_pnt.p3_v + (off_pnt.p3_v - on_pnt.p3_v) * k
        tmp.p3_flags |= PF_UV

    if on_pnt.p3_flags & PF_UV2:
        tmp.p3_u2 = on_pnt.p3_u2 + (off_pnt.p3_u2 - on_pnt.p3_u2) * k
        tmp.p3_v2 = on_pnt.p3_v2 + (off_pnt.p3_v2 - on_pnt.p3_v2) * k
        tmp.p3_flags |= PF_UV2

    if on_pnt.p3_flags & PF_L:
        tmp.p3_l = on_pnt.p3_l + (off_pnt.p3_l - on_pnt.p3_l) * k
        tmp.p3_flags |= PF_L

    if on_pnt.p3_flags & PF_RGBA:
        tmp.p3_r = on_pnt.p3_r + (off_pnt.p3_r - on_pnt.p3_r) * k
        tmp.p3_g = on_pnt.p3_g + (off_pnt.p3_g - on_pnt.p3_g) * k
        tmp.p3_b = on_pnt.p3_b + (off_pnt.p3_b - on_pnt.p3_b) * k
        tmp.p3_a = on_pnt.p3_a + (off_pnt.p3_a - on_pnt.p3_a) * k
        tmp.p3_flags |= PF_RGBA

    return tmp


def ClipFarEdge(on_pnt: g3Point, off_pnt: g3Point) -> g3Point:
    """Clip an edge against the far plane."""
    z_on = on_pnt.p3_z
    z_off = off_pnt.p3_z
    k = 1.0 - ((z_off - Far_clip_z) / (z_off - z_on))

    tmp = _interpolate(on_pnt, off_pnt, k)
    g3_CodePoint(tmp)
    return tmp


def ClipCustomEdge(on_pnt: g3Point, off_pnt: g3Point) -> g3Point:
    """Clip an edge against the custom clip plane."""
    tmp = GetTempPoint()

    ray_direction = vector([
        (off_pnt.p3_vec[0] - on_pnt.p3_vec[0]) / Matrix_scale._data[0],
        (off_pnt.p3_vec[1] - on_pnt.p3_vec[1]) / Matrix_scale._data[1],
        (off_pnt.p3_vec[2] - on_pnt.p3_vec[2]) / Matrix_scale._data[2],
    ])

    w = vector([
        (on_pnt.p3_vec[0] - Clip_plane_point._data[0]) / Matrix_scale._data[0],
        (on_pnt.p3_vec[1] - Clip_plane_point._data[1]) / Matrix_scale._data[1],
        (on_pnt.p3_vec[2] - Clip_plane_point._data[2]) / Matrix_scale._data[2],
    ])

    den = -vm_Dot3Product(Clip_plane, ray_direction)
    if den == 0.0:
        k = 1.0
    else:
        num = vm_Dot3Product(Clip_plane, w)
        k = num / den

    tmp.p3_vec = [
        on_pnt.p3_vec[0] + (off_pnt.p3_vec[0] - on_pnt.p3_vec[0]) * k,
        on_pnt.p3_vec[1] + (off_pnt.p3_vec[1] - on_pnt.p3_vec[1]) * k,
        on_pnt.p3_vec[2] + (off_pnt.p3_vec[2] - on_pnt.p3_vec[2]) * k,
    ]
    tmp.p3_x = tmp.p3_vec[0]
    tmp.p3_y = tmp.p3_vec[1]
    tmp.p3_z = tmp.p3_vec[2]

    if on_pnt.p3_flags & PF_UV:
        tmp.p3_u = on_pnt.p3_u + (off_pnt.p3_u - on_pnt.p3_u) * k
        tmp.p3_v = on_pnt.p3_v + (off_pnt.p3_v - on_pnt.p3_v) * k
        tmp.p3_flags |= PF_UV

    if on_pnt.p3_flags & PF_UV2:
        tmp.p3_u2 = on_pnt.p3_u2 + (off_pnt.p3_u2 - on_pnt.p3_u2) * k
        tmp.p3_v2 = on_pnt.p3_v2 + (off_pnt.p3_v2 - on_pnt.p3_v2) * k
        tmp.p3_flags |= PF_UV2

    if on_pnt.p3_flags & PF_L:
        tmp.p3_l = on_pnt.p3_l + (off_pnt.p3_l - on_pnt.p3_l) * k
        tmp.p3_flags |= PF_L

    if on_pnt.p3_flags & PF_RGBA:
        tmp.p3_r = on_pnt.p3_r + (off_pnt.p3_r - on_pnt.p3_r) * k
        tmp.p3_g = on_pnt.p3_g + (off_pnt.p3_g - on_pnt.p3_g) * k
        tmp.p3_b = on_pnt.p3_b + (off_pnt.p3_b - on_pnt.p3_b) * k
        tmp.p3_a = on_pnt.p3_a + (off_pnt.p3_a - on_pnt.p3_a) * k
        tmp.p3_flags |= PF_RGBA

    g3_CodePoint(tmp)
    return tmp


def ClipEdge(plane_flag: int, on_pnt: g3Point, off_pnt: g3Point) -> g3Point:
    """Clip an edge against one frustum plane."""
    if plane_flag & CC_OFF_FAR:
        return ClipFarEdge(on_pnt, off_pnt)

    if (plane_flag & CC_OFF_CUSTOM) and Clip_custom:
        return ClipCustomEdge(on_pnt, off_pnt)

    # Determine which coordinate to use
    if plane_flag & (CC_OFF_RIGHT | CC_OFF_LEFT):
        a = on_pnt.p3_x
        b = off_pnt.p3_x
    else:
        a = on_pnt.p3_y
        b = off_pnt.p3_y

    if plane_flag & (CC_OFF_LEFT | CC_OFF_BOT):
        a = -a
        b = -b

    k = (a - on_pnt.p3_z) / ((a - on_pnt.p3_z) - b + off_pnt.p3_z)

    tmp = _interpolate(on_pnt, off_pnt, k)

    # Compute Z at the clipping plane
    if plane_flag & (CC_OFF_TOP | CC_OFF_BOT):
        tmp.p3_z = tmp.p3_y
    else:
        tmp.p3_z = tmp.p3_x

    if plane_flag & (CC_OFF_LEFT | CC_OFF_BOT):
        tmp.p3_z = -tmp.p3_z

    g3_CodePoint(tmp)
    return tmp


# ── Line clipping ──
def ClipLine(p0: List[g3Point], p1: List[g3Point], codes_or: int) -> None:
    """Clip a line to the viewing pyramid.

    Modifies p0[0] and p1[0] in place.
    C++ source: renderer/HardwareClipper.cpp
    """
    plane_flag = 1
    while plane_flag <= 32:
        if codes_or & plane_flag:
            if p0[0].p3_codes & plane_flag:
                # swap
                p0[0], p1[0] = p1[0], p0[0]

            old_p1 = p1[0]
            p1[0] = ClipEdge(plane_flag, p0[0], p1[0])

            codes_or = p0[0].p3_codes | p1[0].p3_codes

            if old_p1.p3_flags & PF_TEMP_POINT:
                FreeTempPoint(old_p1)

        plane_flag <<= 1


# ── Polygon clipping ──
def ClipPlane(plane_flag: int, src: List[g3Point], dest: List[g3Point],
              nv: int, cc: 'g3Codes') -> int:
    """Clip a polygon against one plane.

    Returns the number of vertices in the output.
    C++ source: renderer/HardwareClipper.cpp
    """
    from d3render.types import g3Codes
    cc.cc_and = 0xFF
    cc.cc_or = 0

    dest_count = 0
    for i in range(nv):
        prev = (i - 1) % nv
        next_i = (i + 1) % nv

        if src[i].p3_codes & plane_flag:
            # Vertex is outside
            if not (src[prev].p3_codes & plane_flag):
                # Edge enters: add intersection
                new_pt = ClipEdge(plane_flag, src[prev], src[i])
                dest[dest_count] = new_pt
                cc.cc_or |= new_pt.p3_codes
                cc.cc_and &= new_pt.p3_codes
                dest_count += 1

            if not (src[next_i].p3_codes & plane_flag):
                # Edge exits: add intersection
                new_pt = ClipEdge(plane_flag, src[next_i], src[i])
                dest[dest_count] = new_pt
                cc.cc_or |= new_pt.p3_codes
                cc.cc_and &= new_pt.p3_codes
                dest_count += 1

            if src[i].p3_flags & PF_TEMP_POINT:
                FreeTempPoint(src[i])
        else:
            # Vertex is inside: keep it
            dest[dest_count] = src[i]
            cc.cc_or |= src[i].p3_codes
            cc.cc_and &= src[i].p3_codes
            dest_count += 1

    return dest_count


# Temp buffers for clipping
_Vbuf0: List[g3Point] = [g3Point() for _ in range(MAX_POINTS_IN_POLY)]
_Vbuf1: List[g3Point] = [g3Point() for _ in range(MAX_POINTS_IN_POLY)]


def g3_ClipPolygon(pointlist: List[g3Point], nv: int, cc: 'g3Codes') -> Tuple[List[g3Point], int]:
    """Clip a polygon against all frustum planes.

    Returns (clipped_pointlist, new_nv).
    C++ source: renderer/HardwareClipper.cpp
    """
    from d3render.types import g3Codes

    # Reset temp point pool
    if _free_point_num != 0:
        _free_point_num = 0

    # Use double-buffered vertex arrays
    src = pointlist
    dest = _Vbuf0
    dest_idx = 0

    plane_flag = 1
    while plane_flag <= 32:
        if cc.cc_or & plane_flag:
            nv = ClipPlane(plane_flag, src, dest, nv, cc)

            if cc.cc_and:  # clipped away
                return dest, nv

            src = dest
            dest = _Vbuf1 if dest is _Vbuf0 else _Vbuf0

        plane_flag <<= 1

    return src, nv


def g3_SetFarClipZ(z: float) -> None:
    """Set the far clipping plane distance."""
    from d3render.transforms import Far_clip_z
    import d3render.transforms as _t
    _t.Far_clip_z = z
