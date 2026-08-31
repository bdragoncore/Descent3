"""
Cross-validation: compare Python port output against C++ render test state dump.

Run: python -m pytest python/d3render/tests/test_cross_validate.py -v

Requires: build/tests/render_output/cross_validate.json from C++ test run.
"""

import json
import math
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from d3render.vecmat import vector, matrix
from d3render.transforms import (
    g3_GetModelViewMatrix, g3_GetProjectionMatrix, g3_GetViewPortMatrix,
    g3_UpdateFullTransform, g3_TransformMult,
    gTransformViewPort, gTransformProjection, gTransformModelView, gTransformFull,
    Matrix_scale, Window_w2, Window_h2,
)
from d3render.setup import g3_StartFrame, g3_EndFrame
from d3render.points import g3_RotatePoint, g3_ProjectPoint, g3_CodePoint
from d3render.types import g3Point
from d3render.constants import PF_PROJECTED

# Test fixtures - match C++ tests/render/fixtures/cube.h exactly
CUBE_VERTS = [
    (-1, -1, -1),  # 0
    ( 1, -1, -1),  # 1
    ( 1,  1, -1),  # 2
    (-1,  1, -1),  # 3
    (-1, -1,  1),  # 4
    ( 1, -1,  1),  # 5
    ( 1,  1,  1),  # 6
    (-1,  1,  1),  # 7
]
CUBE_NUM_VERTS = 8

CUBE_FACE = [
    [0, 1, 2, 3],  # front
    [5, 4, 7, 6],  # back
    [1, 5, 6, 2],  # right
    [4, 0, 3, 7],  # left
    [3, 2, 6, 7],  # top
    [4, 5, 1, 0],  # bottom
]
CUBE_NUM_FACES = 6


def load_cpp_state():
    """Load the C++ cross-validation JSON dump."""
    paths = [
        "build/tests/cross_validate.json",
        "build/tests/render_output/cross_validate.json",
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "build", "tests", "cross_validate.json"),
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "build", "tests", "render_output", "cross_validate.json"),
    ]
    for p in paths:
        p = os.path.normpath(p)
        if os.path.exists(p):
            with open(p) as f:
                return json.load(f)
    raise FileNotFoundError(f"cross_validate.json not found. Searched: {paths}")


def assert_float_eq(a, b, tol=1e-4, msg=""):
    assert abs(a - b) < tol, f"{msg}: {a} vs {b} (diff={abs(a-b):.2e})"


def assert_vec_eq(a, b, tol=1e-4, msg=""):
    for i, name in enumerate("xyz"):
        assert abs(a._data[i] - b._data[i]) < tol, f"{msg} {name}: {a._data[i]} vs {b._data[i]}"


def mat_get(mat_flat, r, c):
    """Access a 4x4 matrix stored as a flat 16-element list."""
    return mat_flat[r * 4 + c]


class TestCrossValidate:
    """Compare Python port against C++ state dump."""

    def setup_method(self):
        self.cpp = load_cpp_state()

    def _setup_frame(self):
        """Recreate the C++ frame setup: camera at (0,0,-500), identity view, zoom=1."""
        g3_EndFrame()  # reset any prior state
        # Set clip rect to match C++ test (640x480 logical: 0..639, 0..479)
        from d3render.state import gpu_state
        gpu_state.clip_x1 = 0
        gpu_state.clip_y1 = 0
        gpu_state.clip_x2 = 639
        gpu_state.clip_y2 = 479
        view_pos = vector([0.0, 0.0, -500.0])
        view_orient = matrix()  # defaults to identity
        g3_StartFrame(view_pos, view_orient, 1.0)

    def test_window_params(self):
        self._setup_frame()
        cpp = self.cpp
        assert_float_eq(Window_w2, cpp["window_w2"], tol=1.0, msg="Window_w2")
        assert_float_eq(Window_h2, cpp["window_h2"], tol=1.0, msg="Window_h2")
        assert_float_eq(Matrix_scale._data[0], cpp["matrix_scale_x"], tol=1e-4, msg="Matrix_scale.x")
        assert_float_eq(Matrix_scale._data[1], cpp["matrix_scale_y"], tol=1e-4, msg="Matrix_scale.y")

    def test_modelview_matrix(self):
        self._setup_frame()
        cpp = self.cpp["modelview"]
        for r in range(4):
            for c in range(4):
                assert_float_eq(
                    mat_get(gTransformModelView, r, c), cpp[r][c],
                    tol=1e-5, msg=f"ModelView[{r}][{c}]"
                )

    def test_projection_matrix(self):
        self._setup_frame()
        cpp = self.cpp["projection"]
        for r in range(4):
            for c in range(4):
                assert_float_eq(
                    mat_get(gTransformProjection, r, c), cpp[r][c],
                    tol=1e-5, msg=f"Projection[{r}][{c}]"
                )

    def test_viewport_matrix(self):
        self._setup_frame()
        cpp = self.cpp["viewport"]
        for r in range(4):
            for c in range(4):
                assert_float_eq(
                    mat_get(gTransformViewPort, r, c), cpp[r][c],
                    tol=1e-5, msg=f"Viewport[{r}][{c}]"
                )

    def test_full_transform(self):
        self._setup_frame()
        cpp = self.cpp["full_transform"]
        for r in range(4):
            for c in range(4):
                assert_float_eq(
                    mat_get(gTransformFull, r, c), cpp[r][c],
                    tol=1e-4, msg=f"FullTransform[{r}][{c}]"
                )

    def test_vertex_camera_space(self):
        """Compare camera-space coordinates after g3_RotatePoint."""
        self._setup_frame()

        deg2rad = math.pi / 180.0
        heading_deg = 45.0
        pitch_deg = 25.0
        ch = math.cos(heading_deg * deg2rad)
        sh = math.sin(heading_deg * deg2rad)
        cp = math.cos(pitch_deg * deg2rad)
        sp = math.sin(pitch_deg * deg2rad)
        scale = 100.0

        cpp_verts = self.cpp["vertices"]
        for i, (wx, wy, wz) in enumerate(CUBE_VERTS):
            world_pos = vector([wx * scale, wy * scale, wz * scale])

            # Rotate Y (heading)
            x = world_pos._data[0] * ch - world_pos._data[2] * sh
            z = world_pos._data[0] * sh + world_pos._data[2] * ch
            world_pos._data[0] = x
            world_pos._data[2] = z

            # Rotate X (pitch)
            y = world_pos._data[1] * cp - world_pos._data[2] * sp
            z = world_pos._data[1] * sp + world_pos._data[2] * cp
            world_pos._data[1] = y
            world_pos._data[2] = z

            pt = g3Point()
            g3_RotatePoint(pt, world_pos)

            cpp_cam = cpp_verts[i]["camera"]
            assert_float_eq(pt.p3_vec[0], cpp_cam[0], tol=1e-3, msg=f"Vertex {i} camera.x")
            assert_float_eq(pt.p3_vec[1], cpp_cam[1], tol=1e-3, msg=f"Vertex {i} camera.y")
            assert_float_eq(pt.p3_vec[2], cpp_cam[2], tol=1e-3, msg=f"Vertex {i} camera.z")

    def test_vertex_screen_space(self):
        """Compare screen-space coordinates after g3_ProjectPoint."""
        self._setup_frame()

        deg2rad = math.pi / 180.0
        heading_deg = 45.0
        pitch_deg = 25.0
        ch = math.cos(heading_deg * deg2rad)
        sh = math.sin(heading_deg * deg2rad)
        cp = math.cos(pitch_deg * deg2rad)
        sp = math.sin(pitch_deg * deg2rad)
        scale = 100.0

        cpp_verts = self.cpp["vertices"]
        for i, (wx, wy, wz) in enumerate(CUBE_VERTS):
            world_pos = vector([wx * scale, wy * scale, wz * scale])

            x = world_pos._data[0] * ch - world_pos._data[2] * sh
            z = world_pos._data[0] * sh + world_pos._data[2] * ch
            world_pos._data[0] = x
            world_pos._data[2] = z

            y = world_pos._data[1] * cp - world_pos._data[2] * sp
            z = world_pos._data[1] * sp + world_pos._data[2] * cp
            world_pos._data[1] = y
            world_pos._data[2] = z

            pt = g3Point()
            g3_RotatePoint(pt, world_pos)
            g3_ProjectPoint(pt)

            cpp_screen = cpp_verts[i]["screen"]
            assert_float_eq(pt.p3_sx, cpp_screen[0], tol=2.0, msg=f"Vertex {i} screen.x")
            assert_float_eq(pt.p3_sy, cpp_screen[1], tol=2.0, msg=f"Vertex {i} screen.y")

    def test_vertex_clip_codes(self):
        """Compare clip codes after g3_CodePoint."""
        self._setup_frame()

        deg2rad = math.pi / 180.0
        heading_deg = 45.0
        pitch_deg = 25.0
        ch = math.cos(heading_deg * deg2rad)
        sh = math.sin(heading_deg * deg2rad)
        cp = math.cos(pitch_deg * deg2rad)
        sp = math.sin(pitch_deg * deg2rad)
        scale = 100.0

        cpp_verts = self.cpp["vertices"]
        for i, (wx, wy, wz) in enumerate(CUBE_VERTS):
            world_pos = vector([wx * scale, wy * scale, wz * scale])

            x = world_pos._data[0] * ch - world_pos._data[2] * sh
            z = world_pos._data[0] * sh + world_pos._data[2] * ch
            world_pos._data[0] = x
            world_pos._data[2] = z

            y = world_pos._data[1] * cp - world_pos._data[2] * sp
            z = world_pos._data[1] * sp + world_pos._data[2] * cp
            world_pos._data[1] = y
            world_pos._data[2] = z

            pt = g3Point()
            g3_RotatePoint(pt, world_pos)
            g3_ProjectPoint(pt)
            g3_CodePoint(pt)

            cpp_code = cpp_verts[i]["clip_code"]
            assert pt.p3_codes == cpp_code, f"Vertex {i} clip_code: {pt.p3_codes} vs {cpp_code}"
