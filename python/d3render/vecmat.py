"""
Descent 3 Vector/Matrix Math

Complete port of vecmat.h / vecmat_external.h.
All vm_* functions, vector/matrix types, and operator overloads.

Angles are in radians (C++ uses fixed-point where 0x10000 = 2π).
"""

from __future__ import annotations
import math
import random
from dataclasses import dataclass, field
from typing import Tuple, List

# ── Type aliases ──
scalar = float
angle = float  # radians (C++ uses fixed-point: 0x10000 = 2π)


# ── Vector (3D) ──
@dataclass
class vector:
    """3D vector with x, y, z components."""
    _data: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0])

    def __post_init__(self):
        if len(self._data) != 3:
            self._data = list(self._data) + [0.0] * (3 - len(self._data))

    def x(self) -> float:
        return self._data[0]

    def x(self, v: float):
        self._data[0] = v

    def y(self) -> float:
        return self._data[1]

    def y(self, v: float):
        self._data[1] = v

    def z(self) -> float:
        return self._data[2]

    def z(self, v: float):
        self._data[2] = v

    def __repr__(self) -> str:
        return f"vector({self._data[0]:.4f}, {self._data[1]:.4f}, {self._data[2]:.4f})"

    def __eq__(self, other) -> bool:
        if not isinstance(other, vector):
            return False
        return (abs(self._data[0] - other._data[0]) < 1e-6 and
                abs(self._data[1] - other._data[1]) < 1e-6 and
                abs(self._data[2] - other._data[2]) < 1e-6)

    # ── Arithmetic ──
    def __add__(self, other) -> vector:
        if isinstance(other, vector):
            return vector([
                self._data[0] + other._data[0],
                self._data[1] + other._data[1],
                self._data[2] + other._data[2],
            ])
        return NotImplemented

    def __sub__(self, other) -> vector:
        if isinstance(other, vector):
            return vector([
                self._data[0] - other._data[0],
                self._data[1] - other._data[1],
                self._data[2] - other._data[2],
            ])
        return NotImplemented

    def __neg__(self) -> vector:
        return vector([-self._data[0], -self._data[1], -self._data[2]])

    def __mul__(self, other):
        if isinstance(other, (int, float)):
            return vector([self._data[0] * other, self._data[1] * other, self._data[2] * other])
        if isinstance(other, matrix):
            return vm_MatrixMulVector(self, other)
        return NotImplemented

    def __rmul__(self, other):
        if isinstance(other, (int, float)):
            return self.__mul__(other)
        return NotImplemented

    def __truediv__(self, other) -> vector:
        if isinstance(other, (int, float)):
            return vector([self._data[0] / other, self._data[1] / other, self._data[2] / other])
        return NotImplemented

    def __imul__(self, other):
        if isinstance(other, (int, float)):
            self._data[0] *= other
            self._data[1] *= other
            self._data[2] *= other
            return self
        return NotImplemented

    def __itruediv__(self, other):
        if isinstance(other, (int, float)):
            self._data[0] /= other
            self._data[1] /= other
            self._data[2] /= other
            return self
        return NotImplemented

    def __iadd__(self, other):
        if isinstance(other, vector):
            self._data[0] += other._data[0]
            self._data[1] += other._data[1]
            self._data[2] += other._data[2]
            return self
        return NotImplemented

    def __isub__(self, other):
        if isinstance(other, vector):
            self._data[0] -= other._data[0]
            self._data[1] -= other._data[1]
            self._data[2] -= other._data[2]
            return self
        return NotImplemented


# ── Matrix (3×3 rotation) ──
@dataclass
class matrix:
    """3×3 rotation matrix stored as three row vectors (rvec, uvec, fvec)."""
    rvec: vector = field(default_factory=lambda: vector([1.0, 0.0, 0.0]))
    uvec: vector = field(default_factory=lambda: vector([0.0, 1.0, 0.0]))
    fvec: vector = field(default_factory=lambda: vector([0.0, 0.0, 1.0]))

    def __repr__(self) -> str:
        return (f"matrix(\n"
                f"  r={self.rvec}\n"
                f"  u={self.uvec}\n"
                f"  f={self.fvec})")

    def __invert__(self) -> matrix:
        """Transpose: ~m."""
        return vm_TransposeMatrix(self)

    def __mul__(self, other) -> matrix:
        if isinstance(other, matrix):
            return vm_MatrixMul(self, other)
        return NotImplemented


# ── Angle vector (pitch, heading, bank) ──
@dataclass
class angvec:
    p: angle = 0.0  # pitch
    h: angle = 0.0  # heading
    b: angle = 0.0  # bank


# ── Identity matrix constant ──
Identity_matrix = matrix(
    rvec=vector([1.0, 0.0, 0.0]),
    uvec=vector([0.0, 1.0, 0.0]),
    fvec=vector([0.0, 0.0, 1.0]),
)


# ── Vector operations ──
def vm_MakeZero(v: vector) -> None:
    """Set vector to {0, 0, 0}."""
    v._data[0] = 0.0
    v._data[1] = 0.0
    v._data[2] = 0.0


def vm_MakeIdentity(m: matrix) -> None:
    """Make matrix an identity matrix."""
    m.rvec._data[0] = 1.0; m.rvec._data[1] = 0.0; m.rvec._data[2] = 0.0
    m.uvec._data[0] = 0.0; m.uvec._data[1] = 1.0; m.uvec._data[2] = 0.0
    m.fvec._data[0] = 0.0; m.fvec._data[1] = 0.0; m.fvec._data[2] = 1.0


def vm_ClearMatrix(m: matrix) -> None:
    """Clear matrix to all zeros."""
    vm_MakeZero(m.rvec)
    vm_MakeZero(m.uvec)
    vm_MakeZero(m.fvec)


def vm_DotProduct(a: vector, b: vector) -> scalar:
    """Dot product of two 3D vectors."""
    return a._data[0] * b._data[0] + a._data[1] * b._data[1] + a._data[2] * b._data[2]


def vm_Dot3Product(a: vector, b: vector) -> scalar:
    """Alias for vm_DotProduct (3-component dot product)."""
    return vm_DotProduct(a, b)


def vm_CrossProduct(result: vector, a: vector, b: vector) -> None:
    """Cross product: result = a × b."""
    result._data[0] = a._data[1] * b._data[2] - a._data[2] * b._data[1]
    result._data[1] = a._data[2] * b._data[0] - a._data[0] * b._data[2]
    result._data[2] = a._data[0] * b._data[1] - a._data[1] * b._data[0]


def vm_AddVectors(result: vector, a: vector, b: vector) -> None:
    """result = a + b."""
    result._data[0] = a._data[0] + b._data[0]
    result._data[1] = a._data[1] + b._data[1]
    result._data[2] = a._data[2] + b._data[2]


def vm_SubVectors(result: vector, a: vector, b: vector) -> None:
    """result = a - b."""
    result._data[0] = a._data[0] - b._data[0]
    result._data[1] = a._data[1] - b._data[1]
    result._data[2] = a._data[2] - b._data[2]


def vm_ScaleVector(result: vector, v: vector, s: scalar) -> None:
    """result = v * s."""
    result._data[0] = v._data[0] * s
    result._data[1] = v._data[1] * s
    result._data[2] = v._data[2] * s


def vm_ScaleAddVector(d: vector, p: vector, v: vector, s: scalar) -> None:
    """d = p + v * s."""
    d._data[0] = p._data[0] + v._data[0] * s
    d._data[1] = p._data[1] + v._data[1] * s
    d._data[2] = p._data[2] + v._data[2] * s


def vm_DivVector(result: vector, v: vector, s: scalar) -> None:
    """result = v / s."""
    result._data[0] = v._data[0] / s
    result._data[1] = v._data[1] / s
    result._data[2] = v._data[2] / s


def vm_CenterVector(v: vector) -> None:
    """Set vector to 0,0,0 (alias for vm_MakeZero)."""
    vm_MakeZero(v)


def vm_GetMagnitude(v: vector) -> scalar:
    """Euclidean magnitude (uses sqrt)."""
    return math.sqrt(v._data[0] * v._data[0] + v._data[1] * v._data[1] + v._data[2] * v._data[2])


def vm_GetMagnitudeFast(v: vector) -> scalar:
    """Fast magnitude approximation (Manhattan distance)."""
    return abs(v._data[0]) + abs(v._data[1]) + abs(v._data[2])


def vm_NormalizeVector(v: vector) -> scalar:
    """Normalize vector in-place. Returns magnitude before normalization."""
    mag = vm_GetMagnitude(v)
    if mag > 0.0:
        inv = 1.0 / mag
        v._data[0] *= inv
        v._data[1] *= inv
        v._data[2] *= inv
    return mag


def vm_NormalizeVectorFast(v: vector) -> scalar:
    """Fast normalize using vm_GetMagnitudeFast."""
    mag = vm_GetMagnitudeFast(v)
    if mag > 0.0:
        inv = 1.0 / mag
        v._data[0] *= inv
        v._data[1] *= inv
        v._data[2] *= inv
    return mag


def vm_VectorDistance(a: vector, b: vector) -> scalar:
    """Euclidean distance between two vectors."""
    return vm_GetMagnitude(vector([
        a._data[0] - b._data[0],
        a._data[1] - b._data[1],
        a._data[2] - b._data[2],
    ]))


def vm_VectorDistanceQuick(a: vector, b: vector) -> scalar:
    """Fast distance approximation."""
    return vm_GetMagnitudeFast(vector([
        a._data[0] - b._data[0],
        a._data[1] - b._data[1],
        a._data[2] - b._data[2],
    ]))


def vm_GetNormalizedDir(dest: vector, end: vector, start: vector) -> scalar:
    """Compute normalized direction from start to end. Returns distance."""
    dest._data[0] = end._data[0] - start._data[0]
    dest._data[1] = end._data[1] - start._data[1]
    dest._data[2] = end._data[2] - start._data[2]
    return vm_NormalizeVector(dest)


def vm_GetNormalizedDirFast(dest: vector, end: vector, start: vector) -> scalar:
    """Fast normalized direction. Returns fast distance."""
    dest._data[0] = end._data[0] - start._data[0]
    dest._data[1] = end._data[1] - start._data[1]
    dest._data[2] = end._data[2] - start._data[2]
    return vm_NormalizeVectorFast(dest)


# ── Matrix operations ──
def vm_MatrixMulVector(result, v: vector, m: matrix):
    """Rotate a vector through a matrix: result = v * m.

    C++: dest->p3_vec = tempv * View_matrix
    This is: result[i] = Σj (v[j] * m.row[j][i])
    where row[0]=rvec, row[1]=uvec, row[2]=fvec

    Accepts result as either vector (with _data) or list.
    """
    r0 = v._data[0] * m.rvec._data[0] + v._data[1] * m.uvec._data[0] + v._data[2] * m.fvec._data[0]
    r1 = v._data[0] * m.rvec._data[1] + v._data[1] * m.uvec._data[1] + v._data[2] * m.fvec._data[1]
    r2 = v._data[0] * m.rvec._data[2] + v._data[1] * m.uvec._data[2] + v._data[2] * m.fvec._data[2]
    if isinstance(result, vector):
        result._data[0] = r0
        result._data[1] = r1
        result._data[2] = r2
    else:
        result[0] = r0
        result[1] = r1
        result[2] = r2
    return result


def vm_VectorMulTMatrix(result: vector, v: vector, m: matrix) -> None:
    """Multiply vector by transpose of matrix: result = v * m^T."""
    result._data[0] = v._data[0] * m.rvec._data[0] + v._data[1] * m.rvec._data[1] + v._data[2] * m.rvec._data[2]
    result._data[1] = v._data[0] * m.uvec._data[0] + v._data[1] * m.uvec._data[1] + v._data[2] * m.uvec._data[2]
    result._data[2] = v._data[0] * m.fvec._data[0] + v._data[1] * m.fvec._data[1] + v._data[2] * m.fvec._data[2]


def vm_MatrixMul(dest: matrix, src0: matrix, src1: matrix) -> None:
    """Multiply two 3×3 matrices: dest = src0 * src1."""
    # src0 rows × src1 columns
    r0 = src0.rvec._data
    r1 = src0.uvec._data
    r2 = src0.fvec._data
    c0 = [src1.rvec._data[0], src1.uvec._data[0], src1.fvec._data[0]]
    c1 = [src1.rvec._data[1], src1.uvec._data[1], src1.fvec._data[1]]
    c2 = [src1.rvec._data[2], src1.uvec._data[2], src1.fvec._data[2]]

    dest.rvec._data[0] = r0[0]*c0[0] + r0[1]*c0[1] + r0[2]*c0[2]
    dest.rvec._data[1] = r0[0]*c1[0] + r0[1]*c1[1] + r0[2]*c1[2]
    dest.rvec._data[2] = r0[0]*c2[0] + r0[1]*c2[1] + r0[2]*c2[2]

    dest.uvec._data[0] = r1[0]*c0[0] + r1[1]*c0[1] + r1[2]*c0[2]
    dest.uvec._data[1] = r1[0]*c1[0] + r1[1]*c1[1] + r1[2]*c1[2]
    dest.uvec._data[2] = r1[0]*c2[0] + r1[1]*c2[1] + r1[2]*c2[2]

    dest.fvec._data[0] = r2[0]*c0[0] + r2[1]*c0[1] + r2[2]*c0[2]
    dest.fvec._data[1] = r2[0]*c1[0] + r2[1]*c1[1] + r2[2]*c1[2]
    dest.fvec._data[2] = r2[0]*c2[0] + r2[1]*c2[1] + r2[2]*c2[2]


def vm_MatrixMulTMatrix(dest: matrix, src0: matrix, src1: matrix) -> None:
    """Multiply matrix by transpose: dest = src0 * src1^T."""
    t = vm_TransposeMatrix(src1)
    vm_MatrixMul(dest, src0, t)


def vm_TransposeMatrix(m: matrix) -> matrix:
    """Return transpose of matrix."""
    return matrix(
        rvec=vector([m.rvec._data[0], m.uvec._data[0], m.fvec._data[0]]),
        uvec=vector([m.rvec._data[1], m.uvec._data[1], m.fvec._data[1]]),
        fvec=vector([m.rvec._data[2], m.uvec._data[2], m.fvec._data[2]]),
    )


def vm_Orthogonalize(m: matrix) -> None:
    """Ensure matrix is orthogonal (Gram-Schmidt)."""
    # Normalize right vector
    vm_NormalizeVector(m.rvec)

    # Make up vector orthogonal to right
    dot = vm_DotProduct(m.rvec, m.uvec)
    m.uvec._data[0] -= dot * m.rvec._data[0]
    m.uvec._data[1] -= dot * m.rvec._data[1]
    m.uvec._data[2] -= dot * m.rvec._data[2]
    vm_NormalizeVector(m.uvec)

    # Forward = right × up (already orthogonal)
    vm_CrossProduct(m.fvec, m.rvec, m.uvec)


# ── Angles and rotation ──
def vm_SinCos(a: angle, s: List[float], c: List[float]) -> None:
    """Compute sin and cos of angle. Results stored in s[0] and c[0]."""
    s[0] = math.sin(a)
    c[0] = math.cos(a)


def vm_AnglesToMatrix(m: matrix, p: angle, h: angle, b: angle) -> None:
    """Create rotation matrix from pitch, heading, bank angles (radians).

    Order: heading → pitch → bank (Y → X → Z).
    """
    sp, cp = math.sin(p), math.cos(p)
    sh, ch = math.sin(h), math.cos(h)
    sb, cb = math.sin(b), math.cos(b)

    m.rvec._data[0] = ch * cb + sh * sp * sb
    m.rvec._data[1] = sp * cb
    m.rvec._data[2] = sh * cb - ch * sp * sb

    m.uvec._data[0] = -ch * sb + sh * sp * cb
    m.uvec._data[1] = sp * sb
    m.uvec._data[2] = -sh * sb - ch * sp * cb

    m.fvec._data[0] = sh * cp
    m.fvec._data[1] = -cp
    m.fvec._data[2] = ch * cp


def vm_VectorToMatrix(m: matrix, fvec: vector, uvec: vector = None, rvec: vector = None) -> None:
    """Compute a matrix from one or two vectors.

    If only fvec: computes uvec and rvec automatically.
    If fvec + uvec: computes rvec from cross product.
    If fvec + uvec + rvec: uses all three, orthogonalizes.
    """
    # Normalize forward
    f = vector(list(fvec._data))
    vm_NormalizeVector(f)
    m.fvec = f

    if uvec is not None:
        u = vector(list(uvec._data))
    else:
        # Default up: try (0,1,0), fallback to (1,0,0) if parallel
        if abs(f._data[1]) > 0.9999:
            u = vector([1.0, 0.0, 0.0])
        else:
            u = vector([0.0, 1.0, 0.0])

    # Compute right = up × forward
    r = vector()
    vm_CrossProduct(r, u, f)
    vm_NormalizeVector(r)
    m.rvec = r

    # Recompute up = forward × right
    vm_CrossProduct(u, f, r)
    m.uvec = u

    if rvec is not None:
        m.rvec = vector(list(rvec._data))


def vm_VectorAngleToMatrix(m: matrix, v: vector, a: angle) -> None:
    """Compute matrix from axis vector and rotation angle (radians)."""
    vm_NormalizeVector(v)
    c = math.cos(a)
    s = math.sin(a)
    t = 1.0 - c
    x, y, z = v._data[0], v._data[1], v._data[2]

    m.rvec._data[0] = t * x * x + c
    m.rvec._data[1] = t * x * y + s * z
    m.rvec._data[2] = t * x * z - s * y

    m.uvec._data[0] = t * x * y - s * z
    m.uvec._data[1] = t * y * y + c
    m.uvec._data[2] = t * y * z + s * x

    m.fvec._data[0] = t * x * z + s * y
    m.fvec._data[1] = t * y * z - s * x
    m.fvec._data[2] = t * z * z + c


def vm_SinCosToMatrix(m: matrix, sinp: scalar, cosp: scalar,
                      sinb: scalar, cosb: scalar,
                      sinh: scalar, cosh: scalar) -> None:
    """Create matrix from precomputed sin/cos values."""
    m.rvec._data[0] = cosh * cosb + sinh * sinp * sinb
    m.rvec._data[1] = sinp * cosb
    m.rvec._data[2] = sinh * cosb - cosh * sinp * sinb

    m.uvec._data[0] = -cosh * sinb + sinh * sinp * cosb
    m.uvec._data[1] = sinp * sinb
    m.uvec._data[2] = -sinh * sinb - cosh * sinp * cosb

    m.fvec._data[0] = sinh * cosp
    m.fvec._data[1] = -cosp
    m.fvec._data[2] = cosh * cosp


def vm_ExtractAnglesFromMatrix(a: angvec, m: matrix) -> angvec:
    """Extract pitch, heading, bank angles from rotation matrix."""
    # Pitch from forward vector's Y component
    a.p = math.asin(-m.fvec._data[1])

    # Heading from forward X and Z
    cp = math.cos(a.p)
    if abs(cp) > 0.001:
        a.h = math.atan2(m.fvec._data[0], m.fvec._data[2])
    else:
        a.h = 0.0

    # Bank from Right Y and Up Y
    if abs(cp) > 0.001:
        a.b = math.atan2(m.rvec._data[1], m.uvec._data[1])
    else:
        a.b = 0.0

    return a


# ── Geometry ──
def vm_GetPerp(n: vector, a: vector, b: vector, c: vector) -> None:
    """Compute perpendicular (normal) vector from three points."""
    ab = vector([b._data[0] - a._data[0], b._data[1] - a._data[1], b._data[2] - a._data[2]])
    ac = vector([c._data[0] - a._data[0], c._data[1] - a._data[1], c._data[2] - a._data[2]])
    vm_CrossProduct(n, ab, ac)


def vm_GetNormal(n: vector, v0: vector, v1: vector, v2: vector) -> scalar:
    """Compute normalized surface normal from three points. Returns magnitude."""
    vm_GetPerp(n, v0, v1, v2)
    return vm_NormalizeVector(n)


def vm_GetCentroid(centroid: vector, src: List[vector], nv: int) -> scalar:
    """Compute centroid of polygon vertices. Returns average distance."""
    vm_MakeZero(centroid)
    total_dist = 0.0
    for i in range(nv):
        centroid._data[0] += src[i]._data[0]
        centroid._data[1] += src[i]._data[1]
        centroid._data[2] += src[i]._data[2]
        total_dist += vm_GetMagnitude(src[i])

    centroid._data[0] /= nv
    centroid._data[1] /= nv
    centroid._data[2] /= nv
    return total_dist / nv


def vm_GetCentroidFast(centroid: vector, src: List[vector], nv: int) -> scalar:
    """Fast centroid using vm_GetMagnitudeFast."""
    vm_MakeZero(centroid)
    total_dist = 0.0
    for i in range(nv):
        centroid._data[0] += src[i]._data[0]
        centroid._data[1] += src[i]._data[1]
        centroid._data[2] += src[i]._data[2]
        total_dist += vm_GetMagnitudeFast(src[i])

    centroid._data[0] /= nv
    centroid._data[1] /= nv
    centroid._data[2] /= nv
    return total_dist / nv


def vm_GetSlope(x1: scalar, y1: scalar, x2: scalar, y2: scalar) -> scalar:
    """Compute slope between two points."""
    dx = x2 - x1
    if dx == 0.0:
        return 0.0
    return (y2 - y1) / dx


def vm_DistToPlane(checkp: vector, norm: vector, planep: vector) -> scalar:
    """Distance from point to plane (defined by normal and a point on the plane)."""
    # plane equation: dot(norm, p - planep) = 0
    # distance = dot(norm, checkp - planep)
    diff = vector([
        checkp._data[0] - planep._data[0],
        checkp._data[1] - planep._data[1],
        checkp._data[2] - planep._data[2],
    ])
    return vm_DotProduct(norm, diff)


# ── Delta angles ──
def vm_DeltaAngVec(v0: vector, v1: vector, fvec: vector) -> angle:
    """Angle between two vectors, given a forward reference vector."""
    mag0 = vm_GetMagnitude(v0)
    mag1 = vm_GetMagnitude(v1)
    if mag0 == 0.0 or mag1 == 0.0:
        return 0.0
    dot = vm_DotProduct(v0, v1) / (mag0 * mag1)
    dot = max(-1.0, min(1.0, dot))  # clamp for acos
    return math.acos(dot)


def vm_DeltaAngVecNorm(v0: vector, v1: vector, fvec: vector) -> angle:
    """Angle between two already-normalized vectors."""
    dot = vm_DotProduct(v0, v1)
    dot = max(-1.0, min(1.0, dot))
    return math.acos(dot)


# ── Determinant and inverse ──
def calc_det_value(det: matrix) -> scalar:
    """Compute determinant of 3×3 matrix."""
    return (det.rvec._data[0] * (det.uvec._data[1] * det.fvec._data[2] - det.uvec._data[2] * det.fvec._data[1])
          - det.rvec._data[1] * (det.uvec._data[0] * det.fvec._data[2] - det.uvec._data[2] * det.fvec._data[0])
          + det.rvec._data[2] * (det.uvec._data[0] * det.fvec._data[1] - det.uvec._data[1] * det.fvec._data[0]))


def vm_MakeInverseMatrix(dest: matrix) -> None:
    """Compute inverse of 3×3 rotation matrix (transpose for orthogonal matrices)."""
    # For rotation matrices, inverse = transpose
    dest.rvec._data[0] = dest.rvec._data[0]  # identity for now
    # Actually, for a proper rotation matrix:
    t = vm_TransposeMatrix(dest)
    dest.rvec = vector(list(t.rvec._data))
    dest.uvec = vector(list(t.uvec._data))
    dest.fvec = vector(list(t.fvec._data))


# ── Random ──
def vm_MakeRandomVector(vec: vector) -> None:
    """Generate a random vector with components in [-RAND_MAX/2, RAND_MAX/2]."""
    vec._data[0] = random.uniform(-1.0, 1.0)
    vec._data[1] = random.uniform(-1.0, 1.0)
    vec._data[2] = random.uniform(-1.0, 1.0)


# ── Bounding sphere ──
def vm_ComputeBoundingSphere(center: vector, vecs: List[vector], num_verts: int) -> scalar:
    """Compute minimum bounding sphere of a set of points. Returns radius."""
    # Simple approach: centroid + max distance
    vm_MakeZero(center)
    for i in range(num_verts):
        center._data[0] += vecs[i]._data[0]
        center._data[1] += vecs[i]._data[1]
        center._data[2] += vecs[i]._data[2]
    center._data[0] /= num_verts
    center._data[1] /= num_verts
    center._data[2] /= num_verts

    max_dist = 0.0
    for i in range(num_verts):
        d = vm_VectorDistance(center, vecs[i])
        if d > max_dist:
            max_dist = d
    return max_dist
