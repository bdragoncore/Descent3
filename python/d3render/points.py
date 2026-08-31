"""
Descent 3 Point Operations

g3_RotatePoint, g3_ProjectPoint, g3_CodePoint, g3_Point2Vec,
g3_CalcPointDepth, delta rotation functions.

Ported from renderer/HardwarePoints.cpp.
"""

from __future__ import annotations
from typing import List

from d3render.vecmat import vector, matrix, scalar, vm_MatrixMulVector, vm_NormalizeVector, vm_Dot3Product
from d3render.transforms import (
    View_position, View_matrix, Unscaled_matrix,
    Matrix_scale,
)
import d3render.transforms as T
from d3render.types import g3Point
from d3render.constants import *


# ── Minimum Z for "behind camera" check ──
MINZ: float = 0.0


def g3_CodePoint(p: g3Point) -> int:
    """Compute frustum clipping codes for a point.

    Returns the codes and stores them in p.p3_codes.
    C++ source: renderer/HardwarePoints.cpp:7
    """
    cc = 0

    if p.p3_x > p.p3_z:
        cc |= CC_OFF_RIGHT
    if p.p3_y > p.p3_z:
        cc |= CC_OFF_TOP
    if p.p3_x < -p.p3_z:
        cc |= CC_OFF_LEFT
    if p.p3_y < -p.p3_z:
        cc |= CC_OFF_BOT
    if p.p3_z < MINZ:
        cc |= CC_BEHIND
    if p.p3_z > T.Far_clip_z:
        cc |= CC_OFF_FAR

    # Custom clip plane check
    if T.Clip_custom:
        vec = vector([
            (p.p3_vec[0] - T.Clip_plane_point._data[0]) / Matrix_scale._data[0],
            (p.p3_vec[1] - T.Clip_plane_point._data[1]) / Matrix_scale._data[1],
            (p.p3_vec[2] - T.Clip_plane_point._data[2]) / Matrix_scale._data[2],
        ])
        dp = vm_Dot3Product(vec, T.Clip_plane)
        if dp < -0.005:
            cc |= CC_OFF_CUSTOM

    p.p3_codes = cc
    return cc


def g3_RotatePoint(dest: g3Point, src: vector) -> int:
    """Rotate a point from world space to camera space.

    Stores original world position in p3_vecPreRot,
    rotated camera-space position in p3_vec.
    Returns clipping codes.
    C++ source: renderer/HardwarePoints.cpp:28
    """
    # Store pre-rotated (world) position
    dest.p3_vecPreRot = list(src._data)

    # Find offset from camera position
    tempv = vector([
        src._data[0] - View_position._data[0],
        src._data[1] - View_position._data[1],
        src._data[2] - View_position._data[2],
    ])

    # Rotate by camera orientation
    vm_MatrixMulVector(dest.p3_vec, tempv, View_matrix)

    # Set flags
    dest.p3_flags = PF_ORIGPOINT

    return g3_CodePoint(dest)


def g3_ProjectPoint(p: g3Point) -> None:
    """Project a camera-space point to screen coordinates.

    Does nothing if already projected or behind camera.
    C++ source: renderer/HardwarePoints.cpp:43
    """
    if p.p3_flags & PF_PROJECTED:
        return
    if p.p3_codes & CC_BEHIND:
        return

    one_over_z = 1.0 / p.p3_z
    p.p3_sx = T.Window_w2 + (p.p3_x * (T.Window_w2 * one_over_z))
    p.p3_sy = T.Window_h2 - (p.p3_y * (T.Window_h2 * one_over_z))
    p.p3_flags |= PF_PROJECTED


def g3_Point2Vec(v: vector, sx: int, sy: int) -> None:
    """From a 2D screen point, compute the 3D vector through that point.

    C++ source: renderer/HardwarePoints.cpp:53
    """
    from d3render.vecmat import vm_TransposeMatrix, matrix

    tempv = vector()
    tempv._data[0] = ((sx - T.Window_w2) / T.Window_w2) * Matrix_scale._data[2] / Matrix_scale._data[0]
    tempv._data[1] = -((sy - T.Window_h2) / T.Window_h2) * Matrix_scale._data[2] / Matrix_scale._data[1]
    tempv._data[2] = 1.0

    vm_NormalizeVector(tempv)

    # Multiply by transpose of Unscaled_matrix
    tempm = vm_TransposeMatrix(Unscaled_matrix)
    vm_MatrixMulVector(v, tempv, tempm)


def g3_CalcPointDepth(pnt: vector) -> float:
    """Calculate the depth (Z) of a point in camera space.

    C++ source: renderer/HardwarePoints.cpp:124
    """
    return ((pnt._data[0] - View_position._data[0]) * View_matrix.fvec._data[0] +
            (pnt._data[1] - View_position._data[1]) * View_matrix.fvec._data[1] +
            (pnt._data[2] - View_position._data[2]) * View_matrix.fvec._data[2])


# ── Delta rotation functions ──
def g3_RotateDeltaX(dest: vector, dx: float) -> vector:
    """Rotate a delta along the X axis."""
    dest._data[0] = View_matrix.rvec._data[0] * dx
    dest._data[1] = View_matrix.uvec._data[0] * dx
    dest._data[2] = View_matrix.fvec._data[0] * dx
    return dest


def g3_RotateDeltaY(dest: vector, dy: float) -> vector:
    """Rotate a delta along the Y axis."""
    dest._data[0] = View_matrix.rvec._data[1] * dy
    dest._data[1] = View_matrix.uvec._data[1] * dy
    dest._data[2] = View_matrix.fvec._data[1] * dy
    return dest


def g3_RotateDeltaZ(dest: vector, dz: float) -> vector:
    """Rotate a delta along the Z axis."""
    dest._data[0] = View_matrix.rvec._data[2] * dz
    dest._data[1] = View_matrix.uvec._data[2] * dz
    dest._data[2] = View_matrix.fvec._data[2] * dz
    return dest


def g3_RotateDeltaVec(dest: vector, src: vector) -> vector:
    """Rotate a delta vector through the view matrix."""
    vm_MatrixMulVector(dest, src, View_matrix)
    return dest


def g3_AddDeltaVec(dest: g3Point, src: g3Point, deltav: vector) -> int:
    """Add a delta vector to a point."""
    dest.p3_vec = [
        src.p3_vec[0] + deltav._data[0],
        src.p3_vec[1] + deltav._data[1],
        src.p3_vec[2] + deltav._data[2],
    ]
    dest.p3_flags = 0  # not projected
    return g3_CodePoint(dest)
