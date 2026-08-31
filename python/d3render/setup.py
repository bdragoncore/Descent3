"""
Descent 3 Frame Setup

g3_StartFrame, g3_EndFrame, and frame initialization.
Ported from renderer/HardwareSetup.cpp.
"""

from __future__ import annotations

from d3render.vecmat import vector, matrix, scalar
from d3render.transforms import (
    gTransformViewPort, gTransformProjection, gTransformModelView,
    gTransformFull, View_position, View_zoom, Far_clip_z,
    Window_width, Window_height, Window_w2, Window_h2,
    Matrix_scale, View_matrix, Unscaled_matrix,
    g3_GetViewPortMatrix, g3_GetProjectionMatrix, g3_GetModelViewMatrix,
    g3_UpdateFullTransform, g3_ResetFarClipZ,
)
import d3render.transforms as _transforms
from d3render.clipper import InitFreePoints


def g3_StartFrame(view_pos: vector, view_matrix: matrix, zoom: float) -> None:
    """Initialize the rendering frame.

    Sets up all transform matrices, viewport, and camera state.
    Must be called before any rendering.
    C++ source: renderer/HardwareSetup.cpp:86
    """
    # Build transform matrices
    g3_GetViewPortMatrix(gTransformViewPort)
    g3_GetProjectionMatrix(zoom, gTransformProjection)
    g3_GetModelViewMatrix(view_pos, view_matrix, gTransformModelView)
    g3_UpdateFullTransform()

    # Get viewport dimensions
    from d3render.state import gpu_state
    _transforms.Window_width = gpu_state.clip_x2 - gpu_state.clip_x1
    _transforms.Window_height = gpu_state.clip_y2 - gpu_state.clip_y1

    _transforms.Window_w2 = _transforms.Window_width * 0.5
    _transforms.Window_h2 = _transforms.Window_height * 0.5

    # Compute Matrix_scale based on aspect ratio
    s = _transforms.Window_height / _transforms.Window_width
    ms_x = s if s <= 1.0 else 1.0 / s
    Matrix_scale._data[0] = ms_x
    Matrix_scale._data[1] = 1.0
    Matrix_scale._data[2] = 1.0

    # Convert zoom to vertical FOV
    zoom *= 3.0 / 4.0

    global View_zoom, Unscaled_matrix, View_matrix
    View_position._data[0] = view_pos._data[0]
    View_position._data[1] = view_pos._data[1]
    View_position._data[2] = view_pos._data[2]
    View_zoom = zoom
    Unscaled_matrix.rvec._data[0] = view_matrix.rvec._data[0]
    Unscaled_matrix.rvec._data[1] = view_matrix.rvec._data[1]
    Unscaled_matrix.rvec._data[2] = view_matrix.rvec._data[2]
    Unscaled_matrix.uvec._data[0] = view_matrix.uvec._data[0]
    Unscaled_matrix.uvec._data[1] = view_matrix.uvec._data[1]
    Unscaled_matrix.uvec._data[2] = view_matrix.uvec._data[2]
    Unscaled_matrix.fvec._data[0] = view_matrix.fvec._data[0]
    Unscaled_matrix.fvec._data[1] = view_matrix.fvec._data[1]
    Unscaled_matrix.fvec._data[2] = view_matrix.fvec._data[2]

    # Scale the view matrix by zoom
    oOZ = 1.0 / View_zoom
    Matrix_scale._data[0] *= oOZ
    Matrix_scale._data[1] *= oOZ
    # Matrix_scale.z() = 1.0  (already set)

    View_matrix.rvec = Unscaled_matrix.rvec * Matrix_scale._data[0]
    View_matrix.uvec = Unscaled_matrix.uvec * Matrix_scale._data[1]
    View_matrix.fvec = Unscaled_matrix.fvec * Matrix_scale._data[2]

    # Initialize clipper temp point pool
    InitFreePoints()

    # Reset far clip
    g3_ResetFarClipZ()


def g3_EndFrame() -> None:
    """End the rendering frame. Verifies all temp points are freed."""
    from d3render.clipper import CheckTempPoints
    CheckTempPoints()


def g3_GetViewPosition(vp: vector) -> None:
    """Get current camera position."""
    vp._data[0] = View_position._data[0]
    vp._data[1] = View_position._data[1]
    vp._data[2] = View_position._data[2]


def g3_GetViewMatrix(mat: matrix) -> None:
    """Get current view matrix."""
    mat.rvec = vector(list(View_matrix.rvec._data))
    mat.uvec = vector(list(View_matrix.uvec._data))
    mat.fvec = vector(list(View_matrix.fvec._data))


def g3_GetUnscaledMatrix(mat: matrix) -> None:
    """Get the unscaled view matrix (before zoom)."""
    mat.rvec = vector(list(Unscaled_matrix.rvec._data))
    mat.uvec = vector(list(Unscaled_matrix.uvec._data))
    mat.fvec = vector(list(Unscaled_matrix.fvec._data))


def g3_GetMatrixScale(matrix_scale: vector) -> None:
    """Get the current matrix scale vector."""
    matrix_scale._data[0] = Matrix_scale._data[0]
    matrix_scale._data[1] = Matrix_scale._data[1]
    matrix_scale._data[2] = Matrix_scale._data[2]


def g3_SetAspectRatio(aspect: float) -> None:
    """Set user-specified aspect ratio (stored as w/h)."""
    global _sAspect
    _sAspect = aspect


def g3_GetAspectRatio() -> float:
    """Get user-specified aspect ratio."""
    return _sAspect


_sAspect: float = 0.0
