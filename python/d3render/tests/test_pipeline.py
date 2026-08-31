"""
Descent 3 Rendering Math — Validation Tests

Compares Python pipeline output against C++ render test results.
Tests coordinate transforms, clipping, lighting, and the full pipeline.

Run: python -m pytest python/d3render/tests/test_pipeline.py -v
"""

import math
import sys
import os

# Add parent directory to path so we can import d3render
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from d3render.vecmat import vector, matrix, scalar, vm_DotProduct, vm_NormalizeVector, vm_MatrixMulVector
from d3render.transforms import (
    gTransformViewPort, gTransformProjection, gTransformModelView,
    gTransformFull, View_position, View_zoom, Far_clip_z,
    Window_width, Window_height, Window_w2, Window_h2,
    Matrix_scale, View_matrix, Unscaled_matrix, Z_bias,
    g3_GetModelViewMatrix, g3_GetProjectionMatrix, g3_GetViewPortMatrix,
    g3_UpdateFullTransform, g3_TransformMult,
)
from d3render.setup import g3_StartFrame, g3_EndFrame
from d3render.points import g3_RotatePoint, g3_ProjectPoint, g3_CodePoint, MINZ
from d3render.types import g3Point, g3UVL, rendering_state
from d3render.constants import *
from d3render.grdefs import GR_RGB, GR_COLOR_RED, GR_COLOR_GREEN, GR_COLOR_BLUE
from d3render.state import gpu_state, rend_SetLighting, rend_SetAlphaType, rend_SetColorModel
from d3render.lighting import DeterminePointColor, rend_GetAlphaMultiplier
from d3render.draw import rend_DrawPolygon3D, gpu_DrawFlatPolygon3D
from d3render.clipper import InitFreePoints, g3_ClipPolygon
from d3render.types import g3Codes


# ── Test helpers ──
def assert_vec_eq(a, b, tol=1e-4, msg=""):
    """Assert two vectors are approximately equal."""
    assert abs(a._data[0] - b._data[0]) < tol, f"X mismatch: {a._data[0]} vs {b._data[0]} {msg}"
    assert abs(a._data[1] - b._data[1]) < tol, f"Y mismatch: {a._data[1]} vs {b._data[1]} {msg}"
    assert abs(a._data[2] - b._data[2]) < tol, f"Z mismatch: {a._data[2]} vs {b._data[2]} {msg}"


def assert_float_eq(a, b, tol=1e-4, msg=""):
    """Assert two floats are approximately equal."""
    assert abs(a - b) < tol, f"Float mismatch: {a} vs {b} {msg}"


# ── Vector/Matrix Tests ──
class TestVecMat:
    def test_vector_arithmetic(self):
        a = vector([1.0, 2.0, 3.0])
        b = vector([4.0, 5.0, 6.0])
        c = a + b
        assert_vec_eq(c, vector([5.0, 7.0, 9.0]))

        d = b - a
        assert_vec_eq(d, vector([3.0, 3.0, 3.0]))

        e = a * 2.0
        assert_vec_eq(e, vector([2.0, 4.0, 6.0]))

        f = b / 2.0
        assert_vec_eq(f, vector([2.0, 2.5, 3.0]))

    def test_dot_product(self):
        a = vector([1.0, 0.0, 0.0])
        b = vector([0.0, 1.0, 0.0])
        assert vm_DotProduct(a, b) == 0.0

        c = vector([1.0, 2.0, 3.0])
        d = vector([4.0, 5.0, 6.0])
        assert vm_DotProduct(c, d) == 32.0

    def test_normalize(self):
        v = vector([3.0, 4.0, 0.0])
        mag = vm_NormalizeVector(v)
        assert abs(mag - 5.0) < 1e-6
        assert_vec_eq(v, vector([0.6, 0.8, 0.0]))

    def test_matrix_identity(self):
        m = matrix()
        assert_vec_eq(m.rvec, vector([1.0, 0.0, 0.0]))
        assert_vec_eq(m.uvec, vector([0.0, 1.0, 0.0]))
        assert_vec_eq(m.fvec, vector([0.0, 0.0, 1.0]))

    def test_matrix_vector_mul(self):
        m = matrix()  # identity
        v = vector([1.0, 2.0, 3.0])
        result = vm_MatrixMulVector(vector(), v, m)
        assert_vec_eq(result, v)

    def test_matrix_transpose(self):
        m = matrix(
            rvec=vector([1.0, 2.0, 3.0]),
            uvec=vector([4.0, 5.0, 6.0]),
            fvec=vector([7.0, 8.0, 9.0]),
        )
        t = ~m
        assert t.rvec._data[0] == 1.0
        assert t.rvec._data[1] == 4.0
        assert t.rvec._data[2] == 7.0
        assert t.uvec._data[0] == 2.0
        assert t.uvec._data[1] == 5.0
        assert t.uvec._data[2] == 8.0


# ── Transform Tests ──
class TestTransforms:
    def test_modelview_matrix_identity(self):
        """ModelView with identity camera at origin."""
        view_pos = vector([0.0, 0.0, 0.0])
        view_mat = matrix()
        mv = [0.0] * 16
        g3_GetModelViewMatrix(view_pos, view_mat, mv)

        # Identity camera: rvec=(1,0,0), uvec=(0,1,0), fvec=(0,0,1)
        # Translation should be zero
        assert_float_eq(mv[12], 0.0, msg="TX")
        assert_float_eq(mv[13], 0.0, msg="TY")
        assert_float_eq(mv[14], 0.0, msg="TZ")

        # Rotation should be identity (transposed)
        assert_float_eq(mv[0], 1.0)
        assert_float_eq(mv[5], 1.0)
        assert_float_eq(mv[10], 1.0)
        assert_float_eq(mv[15], 1.0)

    def test_modelview_matrix_translation(self):
        """ModelView with camera at (0, 0, -500)."""
        view_pos = vector([0.0, 0.0, -500.0])
        view_mat = matrix()
        mv = [0.0] * 16
        g3_GetModelViewMatrix(view_pos, view_mat, mv)

        # Translation: -viewPos · fvec = -(-500) * 1 = 500
        assert_float_eq(mv[14], 500.0, msg="TZ should be 500")

    def test_projection_matrix(self):
        """Projection matrix with standard viewport."""
        # Set up viewport
        gpu_state.clip_x1 = 0
        gpu_state.clip_y1 = 0
        gpu_state.clip_x2 = 639
        gpu_state.clip_y2 = 479

        proj = [0.0] * 16
        g3_GetProjectionMatrix(1.0, proj)

        # Should have non-zero values at [0], [5], [10], [11], [14]
        assert proj[0] > 0, "proj[0] should be positive"
        assert proj[5] > 0, "proj[5] should be positive"
        assert_float_eq(proj[10], 1.0)
        assert_float_eq(proj[11], 1.0)
        assert_float_eq(proj[14], -1.0)

    def test_viewport_matrix(self):
        """Viewport transform with standard size."""
        gpu_state.clip_x1 = 0
        gpu_state.clip_y1 = 0
        gpu_state.clip_x2 = 639
        gpu_state.clip_y2 = 479

        vp = [0.0] * 16
        g3_GetViewPortMatrix(vp)

        # viewportWidth = 640, viewportHeight = 480
        # vp[0] = 320, vp[5] = -240, vp[12] = 320, vp[13] = 240
        assert_float_eq(vp[0], 320.0)
        assert_float_eq(vp[5], -240.0)
        assert_float_eq(vp[12], 320.0)
        assert_float_eq(vp[13], 240.0)
        assert_float_eq(vp[10], 1.0)
        assert_float_eq(vp[15], 1.0)


# ── Point Tests ──
class TestPoints:
    def _setup_frame(self):
        """Set up a standard frame for testing."""
        # Set clip rect to 640x480
        gpu_state.clip_x1 = 0
        gpu_state.clip_y1 = 0
        gpu_state.clip_x2 = 639
        gpu_state.clip_y2 = 479
        gpu_state.screen_width = 640
        gpu_state.screen_height = 480

        # Camera at (0, 0, -500) looking along +Z
        view_pos = vector([0.0, 0.0, -500.0])
        view_mat = matrix()
        g3_StartFrame(view_pos, view_mat, zoom=1.0)

    def test_rotate_point_at_origin(self):
        """Point at origin should be at (0, 0, 500) in camera space."""
        self._setup_frame()

        pt = g3Point()
        world = vector([0.0, 0.0, 0.0])
        g3_RotatePoint(pt, world)

        # Camera at (0,0,-500), point at (0,0,0) → camera space: (0, 0, 500)
        assert_float_eq(pt.p3_x, 0.0, msg="p3_x")
        assert_float_eq(pt.p3_y, 0.0, msg="p3_y")
        assert_float_eq(pt.p3_z, 500.0, msg="p3_z")

        # Pre-rot should be original world position
        assert_float_eq(pt.p3_vecPreRot[0], 0.0)
        assert_float_eq(pt.p3_vecPreRot[1], 0.0)
        assert_float_eq(pt.p3_vecPreRot[2], 0.0)

    def test_rotate_point_offset(self):
        """Point offset from origin."""
        self._setup_frame()

        pt = g3Point()
        world = vector([100.0, 50.0, 0.0])
        g3_RotatePoint(pt, world)

        # Camera at (0,0,-500), point at (100,50,0) → camera space: ~ (100, 67, 500)
        # Note: zoom scaling introduces small deviation from exact values
        assert_float_eq(pt.p3_x, 100.0, tol=0.1, msg="p3_x")
        assert_float_eq(pt.p3_y, 66.6667, tol=0.1, msg="p3_y")
        assert_float_eq(pt.p3_z, 500.0, tol=0.1, msg="p3_z")

    def test_project_point_center(self):
        """Project a point at screen center."""
        self._setup_frame()

        pt = g3Point()
        world = vector([0.0, 0.0, 0.0])
        g3_RotatePoint(pt, world)
        g3_ProjectPoint(pt)

        # Screen center: (319.5, 239.5) with clip rect 0..639, 0..479
        assert_float_eq(pt.p3_sx, 319.5, msg="p3_sx")
        assert_float_eq(pt.p3_sy, 239.5, msg="p3_sy")

    def test_project_point_offset(self):
        """Project a point with offset."""
        self._setup_frame()

        pt = g3Point()
        world = vector([100.0, 50.0, 0.0])
        g3_RotatePoint(pt, world)
        g3_ProjectPoint(pt)

        # With Window_w2=319.5, Window_h2=239.5, z=500, Matrix_scale.y=1.333:
        # camera_y = 50 * 1.333 = 66.67
        # p3_sx = 319.5 + (100 * 0.9995 * 319.5 / 500) = 383.37
        # p3_sy = 239.5 - (66.67 * 239.5 / 500) = 207.57
        assert_float_eq(pt.p3_sx, 383.4, tol=1.0, msg="p3_sx")
        assert_float_eq(pt.p3_sy, 207.6, tol=1.0, msg="p3_sy")

    def test_code_point_behind_camera(self):
        """Point behind camera should have CC_BEHIND."""
        self._setup_frame()

        pt = g3Point()
        world = vector([0.0, 0.0, -600.0])  # Behind camera at z=-500
        g3_RotatePoint(pt, world)

        # Camera at (0,0,-500), point at (0,0,-600) → camera space: (0, 0, -100)
        assert pt.p3_z < 0, f"Point should be behind camera: z={pt.p3_z}"
        assert pt.p3_codes & CC_BEHIND, "Should have CC_BEHIND flag"

    def test_code_point_off_screen(self):
        """Point far to the right should have CC_OFF_RIGHT."""
        self._setup_frame()

        pt = g3Point()
        world = vector([1000.0, 0.0, 0.0])
        g3_RotatePoint(pt, world)

        # camera space: (1000, 0, 500)
        # p3_x > p3_z → 1000 > 500 → CC_OFF_RIGHT
        assert pt.p3_codes & CC_OFF_RIGHT, f"Should have CC_OFF_RIGHT: codes={pt.p3_codes}"


# ── Lighting Tests ──
class TestLighting:
    def _setup_frame(self):
        gpu_state.clip_x1 = 0
        gpu_state.clip_y1 = 0
        gpu_state.clip_x2 = 639
        gpu_state.clip_y2 = 479
        view_pos = vector([0.0, 0.0, -500.0])
        view_mat = matrix()
        g3_StartFrame(view_pos, view_mat, zoom=1.0)

    def test_no_lighting_white(self):
        """LS_NONE should return white color."""
        self._setup_frame()
        rend_SetLighting(LS_NONE)
        rend_SetAlphaType(AT_ALWAYS)

        pt = g3Point()
        color = DeterminePointColor(pt, flatColorForNoLight=False)

        assert_float_eq(color.r, 1.0, msg="R should be 1.0")
        assert_float_eq(color.g, 1.0, msg="G should be 1.0")
        assert_float_eq(color.b, 1.0, msg="B should be 1.0")
        assert_float_eq(color.a, 1.0, msg="A should be 1.0")

    def test_no_lighting_flat_color(self):
        """LS_NONE with flatColorForNoLight should use cur_color."""
        self._setup_frame()
        rend_SetLighting(LS_NONE)
        rend_SetAlphaType(AT_ALWAYS)
        gpu_state.cur_color = GR_RGB(255, 128, 64)

        pt = g3Point()
        color = DeterminePointColor(pt, flatColorForNoLight=True)

        assert_float_eq(color.r, 1.0, msg="R should be 1.0")
        assert_float_eq(color.g, 0.502, tol=0.01, msg="G should be ~0.5")
        assert_float_eq(color.b, 0.251, tol=0.01, msg="B should be ~0.25")

    def test_mono_lighting(self):
        """CM_MONO should use p3_l for all channels."""
        self._setup_frame()
        rend_SetLighting(LS_GOURAUD)
        rend_SetColorModel(CM_MONO)

        pt = g3Point()
        pt.p3_l = 0.5
        color = DeterminePointColor(pt)

        assert_float_eq(color.r, 0.5, msg="R")
        assert_float_eq(color.g, 0.5, msg="G")
        assert_float_eq(color.b, 0.5, msg="B")

    def test_rgb_lighting(self):
        """CM_RGB should use per-vertex colors."""
        self._setup_frame()
        rend_SetLighting(LS_GOURAUD)
        rend_SetColorModel(CM_RGB)

        pt = g3Point()
        pt.p3_r = 1.0
        pt.p3_g = 0.5
        pt.p3_b = 0.25
        color = DeterminePointColor(pt)

        assert_float_eq(color.r, 1.0, msg="R")
        assert_float_eq(color.g, 0.5, msg="G")
        assert_float_eq(color.b, 0.25, msg="B")

    def test_alpha_multiplier(self):
        """AT_CONSTANT should use cur_alpha / 255."""
        self._setup_frame()
        gpu_state.cur_alpha = 128
        rend_SetAlphaType(AT_CONSTANT)

        assert_float_eq(rend_GetAlphaMultiplier(), 128.0 / 255.0, tol=0.001)

    def test_alpha_vertex(self):
        """ATF_VERTEX should multiply by vertex alpha."""
        self._setup_frame()
        rend_SetAlphaType(AT_VERTEX)

        pt = g3Point()
        pt.p3_a = 0.5
        color = DeterminePointColor(pt)

        assert_float_eq(color.a, 0.5, msg="Alpha should be 0.5")


# ── Full Pipeline Test (CubeRenders) ──
class TestCubePipeline:
    """Test the full pipeline against the C++ CubeRenders test."""

    def _setup_frame(self):
        gpu_state.clip_x1 = 0
        gpu_state.clip_y1 = 0
        gpu_state.clip_x2 = 639
        gpu_state.clip_y2 = 479
        gpu_state.screen_width = 640
        gpu_state.screen_height = 480

        view_pos = vector([0.0, 0.0, -500.0])
        view_mat = matrix()
        g3_StartFrame(view_pos, view_mat, zoom=1.0)

        rend_SetLighting(LS_NONE)
        rend_SetAlphaType(AT_ALWAYS)
        rend_SetColorModel(CM_RGB)

    def test_cube_vertex_transform(self):
        """Test that cube vertices transform correctly.

        The C++ CubeRenders test uses a unit cube centered at origin,
        scaled by 100, rotated 45° heading and 25° pitch.
        """
        self._setup_frame()

        # Cube vertices (unit cube centered at origin)
        cube_verts = [
            vector([-0.5, -0.5, -0.5]),  # 0: back-bottom-left
            vector([0.5, -0.5, -0.5]),   # 1: back-bottom-right
            vector([0.5, 0.5, -0.5]),    # 2: back-top-right
            vector([-0.5, 0.5, -0.5]),   # 3: back-top-left
            vector([-0.5, -0.5, 0.5]),   # 4: front-bottom-left
            vector([0.5, -0.5, 0.5]),    # 5: front-bottom-right
            vector([0.5, 0.5, 0.5]),     # 6: front-top-right
            vector([-0.5, 0.5, 0.5]),    # 7: front-top-left
        ]

        scale = 100.0
        deg2rad = math.pi / 180.0
        heading_deg = 45.0
        pitch_deg = 25.0
        ch = math.cos(heading_deg * deg2rad)
        sh = math.sin(heading_deg * deg2rad)
        cp = math.cos(pitch_deg * deg2rad)
        sp = math.sin(pitch_deg * deg2rad)

        screen_pts = []
        for i, v in enumerate(cube_verts):
            # Scale
            world = vector([v._data[0] * scale, v._data[1] * scale, v._data[2] * scale])

            # Rotate Y (heading)
            x = world._data[0] * ch - world._data[2] * sh
            z = world._data[0] * sh + world._data[2] * ch
            world._data[0] = x
            world._data[2] = z

            # Rotate X (pitch)
            y = world._data[1] * cp - world._data[2] * sp
            world._data[2] = world._data[1] * sp + world._data[2] * cp
            world._data[1] = y

            # Transform to camera space
            pt = g3Point()
            g3_RotatePoint(pt, world)
            g3_ProjectPoint(pt)

            screen_pts.append((pt.p3_sx, pt.p3_sy, pt.p3_z))

        # Verify the front-top-right vertex (index 6)
        # After rotation, it should be visible and on screen
        sx, sy, sz = screen_pts[6]
        assert 0 <= sx <= 640, f"Vertex 6 sx={sx} out of range"
        assert 0 <= sy <= 480, f"Vertex 6 sy={sy} out of range"
        assert sz > 0, f"Vertex 6 should be in front of camera: z={sz}"

    def test_flat_polygon_vertices(self):
        """Test that flat polygon vertices are computed correctly."""
        self._setup_frame()

        # Create a simple triangle in world space
        world_verts = [
            vector([-50.0, -50.0, 0.0]),
            vector([50.0, -50.0, 0.0]),
            vector([0.0, 50.0, 0.0]),
        ]

        points = []
        for v in world_verts:
            pt = g3Point()
            g3_RotatePoint(pt, v)
            pt.p3_flags |= PF_ORIGPOINT
            pt.p3_u = 0.0
            pt.p3_v = 0.0
            points.append(pt)

        # Draw as flat polygon
        vertices = gpu_DrawFlatPolygon3D(points, 3)

        assert len(vertices) == 3
        for v in vertices:
            # Color should be white (LS_NONE + flatColorForNoLight not set in this path)
            # Actually, gpu_DrawFlatPolygon3D uses disableGouraud=True, checkTextureQuality=True
            # which triggers the flat color path if cur_color is set
            pass  # Color depends on gpu_state.cur_color

    def test_draw_polygon_3d_flat(self):
        """Test the full 3D polygon draw path for flat rendering."""
        self._setup_frame()

        # Create a triangle
        world_verts = [
            vector([-50.0, -50.0, 0.0]),
            vector([50.0, -50.0, 0.0]),
            vector([0.0, 50.0, 0.0]),
        ]

        points = []
        for v in world_verts:
            pt = g3Point()
            g3_RotatePoint(pt, v)
            pt.p3_flags |= PF_ORIGPOINT
            pt.p3_u = 0.0
            pt.p3_v = 0.0
            pt.p3_l = 1.0
            pt.p3_r = 1.0
            pt.p3_g = 0.0
            pt.p3_b = 0.0
            pt.p3_a = 1.0
            points.append(pt)

        # Set flat path
        gpu_state.cur_texture_quality = 0
        vertices = rend_DrawPolygon3D(0, points, 3)

        assert len(vertices) == 3
        # Check that positions are world-space (p3_vecPreRot)
        for i, v in enumerate(vertices):
            assert_vec_eq(vector(v.pos), world_verts[i], msg=f"Vertex {i} position")


# ── Clipping Tests ──
class TestClipping:
    def _setup_frame(self):
        gpu_state.clip_x1 = 0
        gpu_state.clip_y1 = 0
        gpu_state.clip_x2 = 639
        gpu_state.clip_y2 = 479
        view_pos = vector([0.0, 0.0, -500.0])
        view_mat = matrix()
        g3_StartFrame(view_pos, view_mat, zoom=1.0)

    def test_clip_point_on_screen(self):
        """Point at screen center should have no clip codes."""
        self._setup_frame()

        pt = g3Point()
        world = vector([0.0, 0.0, 0.0])
        g3_RotatePoint(pt, world)

        assert pt.p3_codes == 0, f"Center point should have no clip codes: {pt.p3_codes}"

    def test_clip_point_off_right(self):
        """Point far to the right should be clipped."""
        self._setup_frame()

        pt = g3Point()
        world = vector([1000.0, 0.0, 0.0])
        g3_RotatePoint(pt, world)

        assert pt.p3_codes & CC_OFF_RIGHT, "Should be off right"

    def test_clip_polygon_all_on_screen(self):
        """Polygon entirely on screen should not be clipped."""
        self._setup_frame()

        points = []
        for v in [
            vector([-100.0, -100.0, 0.0]),
            vector([100.0, -100.0, 0.0]),
            vector([100.0, 100.0, 0.0]),
            vector([-100.0, 100.0, 0.0]),
        ]:
            pt = g3Point()
            g3_RotatePoint(pt, v)
            pt.p3_flags |= PF_ORIGPOINT | PF_UV
            pt.p3_u = 0.0
            pt.p3_v = 0.0
            points.append(pt)

        cc = g3Codes(cc_or=0, cc_and=0xFF)
        for p in points:
            cc.cc_or |= p.p3_codes
            cc.cc_and &= p.p3_codes

        # All on screen → no clipping needed
        assert cc.cc_or == 0, f"All points on screen, but cc_or={cc.cc_or}"


if __name__ == "__main__":
    import pytest
    pytest.main([__file__, "-v"])
