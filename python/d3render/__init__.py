"""
Descent 3 Rendering Math — Python Port

Complete port of the D3 rendering pipeline math for validation.
Covers vector/matrix operations, coordinate transforms, clipping,
lighting, and the full render state machine.

Usage:
    from d3render import *
    setup_start_frame(view_pos, view_matrix, zoom=1.0)
    pt = rotate_point(world_pos)
    project_point(pt)
    # pt.p3_sx, pt.p3_sy now contain screen coordinates
"""

from d3render.constants import *
from d3render.grdefs import *
from d3render.types import *
from d3render.vecmat import *
from d3render.transforms import *
from d3render.setup import *
from d3render.points import *
from d3render.clipper import *
from d3render.lighting import *
from d3render.state import *
from d3render.draw import *
