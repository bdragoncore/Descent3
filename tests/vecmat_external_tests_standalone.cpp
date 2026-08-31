/*
 * D3 Coverage Tests - lib/vecmat_external module (Standalone)
 * 
 * Tests for external vector/matrix operations (DLL interface functions).
 * Uses seeded RNG for reproducible test data.
 */

#include <gtest/gtest.h>
#include <cmath>
#include <random>

// ============================================================================
// D3 Types and Implementation (matching lib/vecmat_external.h)
// ============================================================================

typedef unsigned short angle;

typedef struct {
  angle p, h, b;
} angvec;

typedef struct {
  float x, y, z;
} vector;

typedef struct {
  float x, y, z, kat_pad;
} vector4;

typedef struct {
  float xyz[3];
} vector_array;

typedef struct {
  vector rvec, uvec, fvec;
} matrix;

inline matrix make_matrix(float rvec_x, float rvec_y, float rvec_z,
                         float uvec_x, float uvec_y, float uvec_z,
                         float fvec_x, float fvec_y, float fvec_z) {
    matrix m;
    m.rvec.x = rvec_x; m.rvec.y = rvec_y; m.rvec.z = rvec_z;
    m.uvec.x = uvec_x; m.uvec.y = uvec_y; m.uvec.z = uvec_z;
    m.fvec.x = fvec_x; m.fvec.y = fvec_y; m.fvec.z = fvec_z;
    return m;
}

inline matrix identity_matrix() {
    return make_matrix(1,0,0, 0,1,0, 0,0,1);
}

inline void vm_MakeZero(vector *v) { v->x = v->y = v->z = 0; }
inline void vm_MakeZero(angvec *a) { a->p = a->h = a->b = 0; }

inline bool operator==(vector a, vector b) {
  if (a.x == b.x && a.y == b.y && a.z == b.z)
    return true;
  return false;
}

inline bool operator!=(vector a, vector b) {
  return !(a == b);
}

inline vector operator+(vector a, vector b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  return a;
}

inline vector operator+=(vector &a, vector b) { return (a = a + b); }

inline matrix operator+(matrix a, matrix b) {
  a.rvec += b.rvec;
  a.uvec += b.uvec;
  a.fvec += b.fvec;
  return a;
}

inline matrix operator+=(matrix &a, matrix b) { return (a = a + b); }

inline vector operator-(vector a, vector b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  return a;
}

inline vector operator-=(vector &a, vector b) { return (a = a - b); }

inline matrix operator-(matrix a, matrix b) {
  a.rvec = a.rvec - b.rvec;
  a.uvec = a.uvec - b.uvec;
  a.fvec = a.fvec - b.fvec;
  return a;
}

inline matrix operator-=(matrix &a, matrix b) { return (a = a - b); }

inline float operator*(vector u, vector v) { return (u.x * v.x) + (u.y * v.y) + (u.z * v.z); }

inline vector operator*(vector v, float s) {
  v.x *= s;
  v.y *= s;
  v.z *= s;
  return v;
}

inline vector operator*=(vector &v, float s) { return (v = v * s); }

inline vector operator*(float s, vector v) { return v * s; }

inline matrix operator*(float s, matrix m) {
  m.fvec = m.fvec * s;
  m.uvec = m.uvec * s;
  m.rvec = m.rvec * s;
  return m;
}

inline matrix operator*(matrix m, float s) { return s * m; }

inline matrix operator*=(matrix &m, float s) { return (m = m * s); }

inline vector operator/(vector src, float n) {
  src.x /= n;
  src.y /= n;
  src.z /= n;
  return src;
}

inline vector operator/=(vector &src, float n) { return (src = src / n); }

inline matrix operator/(matrix src, float n) {
  src.fvec = src.fvec / n;
  src.rvec = src.rvec / n;
  src.uvec = src.uvec / n;
  return src;
}

inline matrix operator/=(matrix &src, float n) { return (src = src / n); }

inline vector operator^(vector u, vector v) {
  vector dest;
  dest.x = (u.y * v.z) - (u.z * v.y);
  dest.y = (u.z * v.x) - (u.x * v.z);
  dest.z = (u.x * v.y) - (u.y * v.x);
  return dest;
}

inline matrix operator~(matrix m) {
  float t;
  t = m.uvec.x;
  m.uvec.x = m.rvec.y;
  m.rvec.y = t;
  t = m.fvec.x;
  m.fvec.x = m.rvec.z;
  m.rvec.z = t;
  t = m.fvec.y;
  m.fvec.y = m.uvec.z;
  m.uvec.z = t;
  return m;
}

inline vector operator-(vector a) {
  a.x *= -1;
  a.y *= -1;
  a.z *= -1;
  return a;
}

inline vector operator*(vector v, matrix m) {
  vector result;
  result.x = v * m.rvec;
  result.y = v * m.uvec;
  result.z = v * m.fvec;
  return result;
}

inline float vm_Dot3Vector(float x, float y, float z, vector *v) { return (x * v->x) + (y * v->y) + (z * v->z); }

// ============================================================================
// Test Fixtures
// ============================================================================

class VextTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    VextTest() : rng_(SEED) {}
};

// ============================================================================
// Tests - vm_MakeZero
// ============================================================================

TEST_F(VextTest, MakeZeroVector) {
    vector v = {1.0f, 2.0f, 3.0f};
    vm_MakeZero(&v);
    
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(VextTest, MakeZeroAngvec) {
    angvec a = {100, 200, 300};
    vm_MakeZero(&a);
    
    EXPECT_EQ(a.p, 0);
    EXPECT_EQ(a.h, 0);
    EXPECT_EQ(a.b, 0);
}

// ============================================================================
// Tests - Vector equality
// ============================================================================

TEST_F(VextTest, VectorEquality_Same) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {1.0f, 2.0f, 3.0f};
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST_F(VextTest, VectorEquality_Different) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {1.0f, 2.0f, 4.0f};
    
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST_F(VextTest, VectorEquality_Zero) {
    vector a = {0.0f, 0.0f, 0.0f};
    vector b = {0.0f, 0.0f, 0.0f};
    
    EXPECT_TRUE(a == b);
}

// ============================================================================
// Tests - Vector addition
// ============================================================================

TEST_F(VextTest, VectorAdd) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, 5.0f, 6.0f};
    vector c = a + b;
    
    EXPECT_FLOAT_EQ(c.x, 5.0f);
    EXPECT_FLOAT_EQ(c.y, 7.0f);
    EXPECT_FLOAT_EQ(c.z, 9.0f);
}

TEST_F(VextTest, VectorAddAssign) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, 5.0f, 6.0f};
    a += b;
    
    EXPECT_FLOAT_EQ(a.x, 5.0f);
    EXPECT_FLOAT_EQ(a.y, 7.0f);
    EXPECT_FLOAT_EQ(a.z, 9.0f);
}

TEST_F(VextTest, VectorAdd_ZeroIdentity) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector zero = {0.0f, 0.0f, 0.0f};
    vector c = a + zero;
    
    EXPECT_EQ(a, c);
}

// ============================================================================
// Tests - Vector subtraction
// ============================================================================

TEST_F(VextTest, VectorSub) {
    vector a = {5.0f, 7.0f, 9.0f};
    vector b = {1.0f, 2.0f, 3.0f};
    vector c = a - b;
    
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 5.0f);
    EXPECT_FLOAT_EQ(c.z, 6.0f);
}

TEST_F(VextTest, VectorSubAssign) {
    vector a = {5.0f, 7.0f, 9.0f};
    vector b = {1.0f, 2.0f, 3.0f};
    a -= b;
    
    EXPECT_FLOAT_EQ(a.x, 4.0f);
    EXPECT_FLOAT_EQ(a.y, 5.0f);
    EXPECT_FLOAT_EQ(a.z, 6.0f);
}

TEST_F(VextTest, VectorSub_Self) {
    vector a = {5.0f, 7.0f, 9.0f};
    vector c = a - a;
    
    EXPECT_FLOAT_EQ(c.x, 0.0f);
    EXPECT_FLOAT_EQ(c.y, 0.0f);
    EXPECT_FLOAT_EQ(c.z, 0.0f);
}

// ============================================================================
// Tests - Vector dot product
// ============================================================================

TEST_F(VextTest, VectorDotProduct) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, 5.0f, 6.0f};
    float dot = a * b;
    
    EXPECT_FLOAT_EQ(dot, 32.0f); // 1*4 + 2*5 + 3*6 = 32
}

TEST_F(VextTest, VectorDotProduct_Orthogonal) {
    vector a = {1.0f, 0.0f, 0.0f};
    vector b = {0.0f, 1.0f, 0.0f};
    float dot = a * b;
    
    EXPECT_FLOAT_EQ(dot, 0.0f);
}

TEST_F(VextTest, VectorDotProduct_Parallel) {
    vector a = {1.0f, 1.0f, 1.0f};
    vector b = {2.0f, 2.0f, 2.0f};
    float dot = a * b;
    
    EXPECT_FLOAT_EQ(dot, 6.0f);
}

TEST_F(VextTest, VectorDotProduct_Self) {
    vector a = {3.0f, 4.0f, 0.0f};
    float dot = a * a;
    
    EXPECT_FLOAT_EQ(dot, 25.0f); // 9 + 16 = 25 (magnitude squared)
}

// ============================================================================
// Tests - Vector scalar multiplication
// ============================================================================

TEST_F(VextTest, VectorScalarMul) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector c = a * 2.0f;
    
    EXPECT_FLOAT_EQ(c.x, 2.0f);
    EXPECT_FLOAT_EQ(c.y, 4.0f);
    EXPECT_FLOAT_EQ(c.z, 6.0f);
}

TEST_F(VextTest, VectorScalarMul_Commutative) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector c1 = a * 2.0f;
    vector c2 = 2.0f * a;
    
    EXPECT_EQ(c1, c2);
}

TEST_F(VextTest, VectorScalarMulAssign) {
    vector a = {1.0f, 2.0f, 3.0f};
    a *= 2.0f;
    
    EXPECT_FLOAT_EQ(a.x, 2.0f);
    EXPECT_FLOAT_EQ(a.y, 4.0f);
    EXPECT_FLOAT_EQ(a.z, 6.0f);
}

TEST_F(VextTest, VectorScalarMul_Zero) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector c = a * 0.0f;
    
    EXPECT_FLOAT_EQ(c.x, 0.0f);
    EXPECT_FLOAT_EQ(c.y, 0.0f);
    EXPECT_FLOAT_EQ(c.z, 0.0f);
}

// ============================================================================
// Tests - Vector scalar division
// ============================================================================

TEST_F(VextTest, VectorScalarDiv) {
    vector a = {2.0f, 4.0f, 6.0f};
    vector c = a / 2.0f;
    
    EXPECT_FLOAT_EQ(c.x, 1.0f);
    EXPECT_FLOAT_EQ(c.y, 2.0f);
    EXPECT_FLOAT_EQ(c.z, 3.0f);
}

TEST_F(VextTest, VectorScalarDivAssign) {
    vector a = {2.0f, 4.0f, 6.0f};
    a /= 2.0f;
    
    EXPECT_FLOAT_EQ(a.x, 1.0f);
    EXPECT_FLOAT_EQ(a.y, 2.0f);
    EXPECT_FLOAT_EQ(a.z, 3.0f);
}

// ============================================================================
// Tests - Vector cross product
// ============================================================================

TEST_F(VextTest, VectorCrossProduct) {
    vector u = {1.0f, 0.0f, 0.0f};
    vector v = {0.0f, 1.0f, 0.0f};
    vector c = u ^ v;
    
    EXPECT_FLOAT_EQ(c.x, 0.0f);
    EXPECT_FLOAT_EQ(c.y, 0.0f);
    EXPECT_FLOAT_EQ(c.z, 1.0f);
}

TEST_F(VextTest, VectorCrossProduct_AntiCommutative) {
    vector u = {1.0f, 0.0f, 0.0f};
    vector v = {0.0f, 1.0f, 0.0f};
    vector c1 = u ^ v;
    vector c2 = v ^ u;
    
    EXPECT_EQ(c1.x, -c2.x);
    EXPECT_EQ(c1.y, -c2.y);
    EXPECT_EQ(c1.z, -c2.z);
}

TEST_F(VextTest, VectorCrossProduct_Parallel) {
    vector u = {1.0f, 2.0f, 3.0f};
    vector v = {2.0f, 4.0f, 6.0f};
    vector c = u ^ v;
    
    EXPECT_FLOAT_EQ(c.x, 0.0f);
    EXPECT_FLOAT_EQ(c.y, 0.0f);
    EXPECT_FLOAT_EQ(c.z, 0.0f);
}

// ============================================================================
// Tests - Vector negation
// ============================================================================

TEST_F(VextTest, VectorNegate) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector c = -a;
    
    EXPECT_FLOAT_EQ(c.x, -1.0f);
    EXPECT_FLOAT_EQ(c.y, -2.0f);
    EXPECT_FLOAT_EQ(c.z, -3.0f);
}

TEST_F(VextTest, VectorNegate_Double) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector c = -(-a);
    
    EXPECT_EQ(a, c);
}

// ============================================================================
// Tests - Matrix addition
// ============================================================================

TEST_F(VextTest, MatrixAdd) {
    matrix a, b, c;
    a.rvec = (vector){1,0,0}; a.uvec = (vector){0,1,0}; a.fvec = (vector){0,0,1};
    b.rvec = (vector){2,0,0}; b.uvec = (vector){0,2,0}; b.fvec = (vector){0,0,2};
    c = a + b;
    
    EXPECT_FLOAT_EQ(c.rvec.x, 3.0f);
    EXPECT_FLOAT_EQ(c.uvec.y, 3.0f);
    EXPECT_FLOAT_EQ(c.fvec.z, 3.0f);
}

TEST_F(VextTest, MatrixAddAssign) {
    matrix a = identity_matrix();
    matrix b = make_matrix(2,0,0, 0,2,0, 0,0,2);
    a += b;
    
    EXPECT_FLOAT_EQ(a.rvec.x, 3.0f);
}

// ============================================================================
// Tests - Matrix scalar multiplication
// ============================================================================

TEST_F(VextTest, MatrixScalarMul) {
    matrix m = identity_matrix();
    matrix c = m * 2.0f;
    
    EXPECT_FLOAT_EQ(c.rvec.x, 2.0f);
    EXPECT_FLOAT_EQ(c.uvec.y, 2.0f);
    EXPECT_FLOAT_EQ(c.fvec.z, 2.0f);
}

TEST_F(VextTest, MatrixScalarMul_Commutative) {
    matrix m = identity_matrix();
    matrix c1 = m * 2.0f;
    matrix c2 = 2.0f * m;
    
    EXPECT_EQ(c1.rvec.x, c2.rvec.x);
}

// ============================================================================
// Tests - Matrix transpose
// ============================================================================

TEST_F(VextTest, MatrixTranspose) {
    matrix m = make_matrix(1,2,3, 4,5,6, 7,8,9);
    matrix t = ~m;
    
    // After transpose: t.rvec = {1,4,7}, t.uvec = {2,5,8}, t.fvec = {3,6,9}
    EXPECT_FLOAT_EQ(t.rvec.x, 1.0f);
    EXPECT_FLOAT_EQ(t.rvec.y, 4.0f);
    EXPECT_FLOAT_EQ(t.rvec.z, 7.0f);
}

TEST_F(VextTest, MatrixTranspose_Identity) {
    matrix i = identity_matrix();
    matrix t = ~i;
    
    // Identity transpose is itself
    EXPECT_FLOAT_EQ(t.rvec.x, 1.0f);
    EXPECT_FLOAT_EQ(t.uvec.y, 1.0f);
    EXPECT_FLOAT_EQ(t.fvec.z, 1.0f);
}

// ============================================================================
// Tests - Matrix scalar division
// ============================================================================

TEST_F(VextTest, MatrixScalarDiv) {
    matrix m = make_matrix(2,0,0, 0,4,0, 0,0,6);
    matrix c = m / 2.0f;
    
    EXPECT_FLOAT_EQ(c.rvec.x, 1.0f);
    EXPECT_FLOAT_EQ(c.uvec.y, 2.0f);
    EXPECT_FLOAT_EQ(c.fvec.z, 3.0f);
}

// ============================================================================
// Tests - Vector-matrix multiplication
// ============================================================================

TEST_F(VextTest, VectorMatrixMul_Identity) {
    vector v = {1.0f, 2.0f, 3.0f};
    matrix i = identity_matrix();
    vector c = v * i;
    
    EXPECT_EQ(v, c);
}

TEST_F(VextTest, VectorMatrixMul_Scale) {
    vector v = {1.0f, 1.0f, 1.0f};
    matrix s = make_matrix(2,0,0, 0,2,0, 0,0,2);
    vector c = v * s;
    
    EXPECT_FLOAT_EQ(c.x, 2.0f);
    EXPECT_FLOAT_EQ(c.y, 2.0f);
    EXPECT_FLOAT_EQ(c.z, 2.0f);
}

// ============================================================================
// Tests - vm_Dot3Vector
// ============================================================================

TEST_F(VextTest, Dot3Vector) {
    vector v = {4.0f, 5.0f, 6.0f};
    float result = vm_Dot3Vector(1.0f, 2.0f, 3.0f, &v);
    
    EXPECT_FLOAT_EQ(result, 32.0f); // 1*4 + 2*5 + 3*6 = 32
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
