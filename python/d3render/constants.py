"""
Descent 3 Rendering Constants

All enum values, flags, and constants from lib/renderer.h and lib/3d.h.
"""

# ── Point flags (lib/3d.h) ──
PF_PROJECTED   = 0x01  # has been projected, so sx,sy valid
PF_FAR_ALPHA   = 0x02  # past fog zone
PF_TEMP_POINT  = 0x04  # created during clip
PF_UV          = 0x08  # has uv values set
PF_L           = 0x10  # has lighting values set
PF_RGBA        = 0x20  # has RGBA lighting values set
PF_UV2         = 0x40  # has lightmap uvs as well
PF_ORIGPOINT   = 0x80  # has original (pre-rotated) point

# ── Clipping codes (lib/3d.h) ──
CC_OFF_LEFT    = 0x01
CC_OFF_RIGHT   = 0x02
CC_OFF_BOT     = 0x04
CC_OFF_TOP     = 0x08
CC_OFF_FAR     = 0x10
CC_OFF_CUSTOM  = 0x20
CC_BEHIND      = 0x80

# ── Renderer type (lib/renderer.h) ──
RENDERER_OPENGL   = 2
RENDERER_DIRECT3D = 3
RENDERER_GLIDE    = 4  # Unused
RENDERER_NONE     = 5
RENDERER_MESA     = 6

# ── Lighting states (lib/renderer.h) ──
LS_NONE          = 0  # No lighting
LS_GOURAUD       = 1  # Per-vertex lighting
LS_PHONG         = 2  # Same as GOURAUD in this codebase
LS_FLAT_GOURAUD  = 3  # Per-face lighting

# ── Color models (lib/renderer.h) ──
CM_MONO  = 0  # Single intensity channel
CM_RGB   = 1  # Full RGB

# ── Texture types (lib/renderer.h) ──
TT_FLAT                = 0  # Solid color
TT_LINEAR              = 1  # Linear filtering
TT_PERSPECTIVE         = 2  # Perspective-correct
TT_LINEAR_SPECIAL      = 3  # Linear special
TT_PERSPECTIVE_SPECIAL = 4  # Perspective special

# ── Wrap types (lib/renderer.h) ──
WT_WRAP   = 0  # GL_REPEAT
WT_CLAMP  = 1  # GL_CLAMP_TO_EDGE
WT_WRAP_V = 2  # GL_REPEAT on T, GL_CLAMP_TO_EDGE on S

# ── Alpha types (lib/renderer.h) ──
ATF_CONSTANT  = 0x01  # Alpha flag: constant
ATF_TEXTURE   = 0x02  # Alpha flag: texture
ATF_VERTEX    = 0x04  # Alpha flag: vertex

AT_ALWAYS                    = 0
AT_CONSTANT                  = 1
AT_TEXTURE                   = 2
AT_CONSTANT_TEXTURE          = 3
AT_VERTEX                    = 4
AT_CONSTANT_VERTEX           = 5
AT_TEXTURE_VERTEX            = 6
AT_CONSTANT_TEXTURE_VERTEX   = 7
AT_LIGHTMAP_BLEND            = 8
AT_SATURATE_TEXTURE          = 9
AT_SATURATE_VERTEX           = 12
AT_SATURATE_CONSTANT_VERTEX  = 13
AT_SATURATE_TEXTURE_VERTEX   = 14
AT_SPECULAR                  = 32
AT_LIGHTMAP_BLEND_SATURATE   = 33

# ── Map types (lib/renderer.h) ──
MAP_TYPE_BITMAP   = 0
MAP_TYPE_LIGHTMAP = 1

# ── Overlay types (lib/renderer.h) ──
OT_NONE   = 0  # No overlay
OT_BLEND  = 1  # Draw lightmap after main texture

# ── Clear flags (lib/renderer.h) ──
RF_CLEAR_ZBUFFER = 0x01

# ── Texture dimensions (lib/renderer.h) ──
TEXTURE_WIDTH  = 128
TEXTURE_HEIGHT = 128

# ── Clipper limits ──
MAX_POINTS_IN_POLY = 100
