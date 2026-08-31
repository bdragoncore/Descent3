"""
Descent 3 Transform Pipeline

g3_GetModelViewMatrix, g3_GetProjectionMatrix, g3_GetViewPortMatrix,
g3_TransformMult, g3_TransformTrans, g3_UpdateFullTransform,
g3_RefreshTransforms, g3_ForceTransformRefresh.

Ported from renderer/HardwareTransforms.cpp and renderer/HardwareSetup.cpp.
"""

from __future__ import annotations
import math
from typing import List, Tuple

from d3render.vecmat import vector, matrix, scalar, vm_Dot3Product, vm_TransposeMatrix
from d3render.constants import *

# ── 4×4 transform matrices (stored as flat 16-element lists, row-major) ──
gTransformViewPort: List[float] = [0.0] * 16
gTransformProjection: List[float] = [0.0] * 16
gTransformModelView: List[float] = [0.0] * 16
gTransformFull: List[float] = [0.0] * 16

# ── Frame globals ──
View_position: vector = vector()
View_zoom: float = 1.0
Far_clip_z: float = float('inf')

Window_width: int = 640
Window_height: int = 480
Window_w2: float = 320.0
Window_h2: float = 240.0

Matrix_scale: vector = vector([1.0, 1.0, 1.0])
View_matrix: matrix = matrix()
Unscaled_matrix: matrix = matrix()

# Custom clip plane
Clip_custom: int = 0
Clip_plane_distance: float = 0.0
Clip_plane: vector = vector()
Clip_plane_point: vector = vector()

# Transform pass-through state
_sUseTransformPassthru: int = -1

# Z-bias
Z_bias: float = 0.0


def g3_GetModelViewMatrix(viewPos: vector, viewMatrix: matrix, mvMat: List[float]) -> None:
    """Build the 4×4 modelview matrix (row-major, 16 floats).

    This is the camera transform: world → camera space.
    The matrix is transposed relative to OpenGL convention because
    D3 stores row-major but GL reads column-major.

    C++ source: renderer/HardwareTransforms.cpp:29
    """
    localOrient = matrix(
        rvec=vector(list(viewMatrix.rvec._data)),
        uvec=vector(list(viewMatrix.uvec._data)),
        fvec=vector(list(viewMatrix.fvec._data)),
    )
    localPos = vector([-viewPos._data[0], -viewPos._data[1], -viewPos._data[2]])

    # Row-major layout matching C++:
    # [ r.x  u.x  f.x  0   ]
    # [ r.y  u.y  f.y  0   ]
    # [ r.z  u.z  f.z  0   ]
    # [ T.x  T.y  T.z  1   ]
    mvMat[0]  = localOrient.rvec._data[0]
    mvMat[1]  = localOrient.uvec._data[0]
    mvMat[2]  = localOrient.fvec._data[0]
    mvMat[3]  = 0.0
    mvMat[4]  = localOrient.rvec._data[1]
    mvMat[5]  = localOrient.uvec._data[1]
    mvMat[6]  = localOrient.fvec._data[1]
    mvMat[7]  = 0.0
    mvMat[8]  = localOrient.rvec._data[2]
    mvMat[9]  = localOrient.uvec._data[2]
    mvMat[10] = localOrient.fvec._data[2]
    mvMat[11] = 0.0
    mvMat[12] = vm_Dot3Product(localPos, localOrient.rvec)
    mvMat[13] = vm_Dot3Product(localPos, localOrient.uvec)
    mvMat[14] = vm_Dot3Product(localPos, localOrient.fvec) + Z_bias
    mvMat[15] = 1.0


def g3_TransformMult(res: List[List[float]], a: List[List[float]], b: List[List[float]]) -> None:
    """4×4 matrix multiplication: res = a × b.

    Both a and b are 4×4 matrices stored as list-of-lists (row-major).
    C++ source: renderer/HardwareTransforms.cpp:50
    """
    temp = [[0.0] * 4 for _ in range(4)]
    for y in range(4):
        for x in range(4):
            temp[y][x] = (a[y][0] * b[0][x] + a[y][1] * b[1][x] +
                          a[y][2] * b[2][x] + a[y][3] * b[3][x])
    for y in range(4):
        for x in range(4):
            res[y][x] = temp[y][x]


def g3_TransformTrans(res: List[List[float]], t: List[List[float]]) -> None:
    """4×4 matrix transpose: res = t^T."""
    temp = [[0.0] * 4 for _ in range(4)]
    for y in range(4):
        for x in range(4):
            temp[x][y] = t[y][x]
    for y in range(4):
        for x in range(4):
            res[y][x] = temp[y][x]


def g3_UpdateFullTransform() -> None:
    """Compute combined transform: Full = ViewPort × Projection × ModelView.

    Result stored in gTransformFull as 4×4 list-of-lists.
    C++ source: renderer/HardwareTransforms.cpp:74
    """
    # Convert flat lists to 4×4
    def to_4x4(flat: List[float]) -> List[List[float]]:
        return [flat[i*4:(i+1)*4] for i in range(4)]

    def from_4x4(m: List[List[float]], flat: List[float]) -> None:
        for i in range(4):
            for j in range(4):
                flat[i*4 + j] = m[i][j]

    mv = to_4x4(gTransformModelView)
    proj = to_4x4(gTransformProjection)
    vp = to_4x4(gTransformViewPort)

    # Full = ModelView × Projection
    temp = [[0.0] * 4 for _ in range(4)]
    g3_TransformMult(temp, mv, proj)

    # Full = Full × ViewPort
    result = [[0.0] * 4 for _ in range(4)]
    g3_TransformMult(result, temp, vp)

    from_4x4(result, gTransformFull)


def g3_GetViewPortMatrix(viewMat: List[float]) -> None:
    """Build the 4×4 viewport transform matrix (NDC → screen).

    Y is flipped: screen Y increases downward.
    C++ source: renderer/HardwareSetup.cpp
    """
    viewportWidth = Window_width
    viewportHeight = Window_height
    viewportX = 0
    viewportY = 0

    # Use clip rect if available
    from d3render.state import gpu_state
    viewportWidth = gpu_state.clip_x2 - gpu_state.clip_x1 + 1
    viewportHeight = gpu_state.clip_y2 - gpu_state.clip_y1 + 1
    viewportX = gpu_state.clip_x1
    viewportY = gpu_state.clip_y1

    vwo2 = viewportWidth * 0.5
    vho2 = viewportHeight * 0.5

    # Zero out
    for i in range(16):
        viewMat[i] = 0.0

    viewMat[0]  = vwo2
    viewMat[5]  = -vho2  # Y flip
    viewMat[10] = 1.0
    viewMat[12] = vwo2 + viewportX
    viewMat[13] = vho2 + viewportY
    viewMat[15] = 1.0


def g3_GetProjectionMatrix(zoom: float, projMat: List[float]) -> None:
    """Build the 4×4 projection matrix (camera → clip).

    Infinite far plane projection. D3 uses left-handed +Z forward.
    C++ source: renderer/HardwareSetup.cpp:52
    """
    # Match C++ rend_GetProjectionParameters: reads from gpu_state clip rect
    from d3render.state import gpu_state
    viewportWidth = gpu_state.clip_x2 - gpu_state.clip_x1
    viewportHeight = gpu_state.clip_y2 - gpu_state.clip_y1

    s = viewportWidth / viewportHeight
    vertical_fov = zoom * 3.0 / 4.0

    # Zero out
    for i in range(16):
        projMat[i] = 0.0

    oOT = 1.0 / vertical_fov

    if s <= 1.0:
        projMat[0] = oOT
        projMat[5] = oOT * s
    else:
        projMat[0] = oOT / s
        projMat[5] = oOT

    # Infinite far plane:
    projMat[10] = 1.0
    projMat[11] = 1.0
    projMat[14] = -1.0


def g3_ForceTransformRefresh() -> None:
    """Force next g3_RefreshTransforms to re-upload matrices to GL."""
    global _sUseTransformPassthru
    _sUseTransformPassthru = -1


def g3_RefreshTransforms(usePassthru: bool) -> None:
    """Upload current transform matrices to GL (or set passthru).

    In the Python port, this is a no-op since we don't have GL.
    C++ source: renderer/HardwareTransforms.cpp:84
    """
    global _sUseTransformPassthru
    if _sUseTransformPassthru == 1 and usePassthru:
        return

    # In Python port: no GL to upload to
    _sUseTransformPassthru = 1 if usePassthru else 0


def g3_SetFarClipZ(z: float) -> None:
    """Set the far clipping plane distance."""
    global Far_clip_z
    Far_clip_z = z


def g3_ResetFarClipZ() -> None:
    """Reset far clip to infinity."""
    global Far_clip_z
    Far_clip_z = float('inf')


def g3_SetCustomClipPlane(state: int, pnt: vector, normal: vector) -> None:
    """Set up a custom clipping plane.

    g3_StartFrame must be called before this.
    C++ source: renderer/HardwareClipper.cpp
    """
    global Clip_custom, Clip_plane_point, Clip_plane
    Clip_custom = state
    if state:
        tempv = pnt - View_position
        # tempv = tempv * View_matrix  (vector * matrix)
        from d3render.vecmat import vm_MatrixMulVector
        Clip_plane_point = vm_MatrixMulVector(vector(), tempv, View_matrix)

        # Clip_plane = normal * Unscaled_matrix
        Clip_plane = vm_MatrixMulVector(vector(), normal, Unscaled_matrix)
        from d3render.vecmat import vm_NormalizeVector
        vm_NormalizeVector(Clip_plane)
