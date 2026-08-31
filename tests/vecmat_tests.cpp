/*
 * D3 Coverage Tests - vecmat module
 * 
 * Tests for vector/matrix math operations.
 * Uses seeded RNG for reproducible test data.
 */

#include <gtest/gtest.h>
#include "math_generators.cpp"

#include "vecmat.h"
#include "fix.h"
#include <cmath>
#include <vector>
#include <cstdio>

// Stub definitions for missing functions/macros
#define ASSERT(x) do { } while(0)
#define Int3() do { } while(0)
#define debug_break() do { } while(0)

void AssertionFailed(const char* expr, const char* file, int line) { }
void Int3MessageBox(const char* text, int line) { }
void Debug_break() { }
void DebugBreak_callback_stop() { }
void DebugBreak_callback_resume() { }

// Tolerance for floating-point comparisons
static constexpr float FLOAT_TOLERANCE = 1e-5f;

// ============================================================================
// vm_MakeIdentity Tests
// ============================================================================

class VmMakeIdentityTest : public ::testing::Test {
protected:
    MatrixGenerator mgen;
};

TEST_F(VmMakeIdentityTest, CreatesIdentityMatrix) {
    matrix m;
    vm_MakeIdentity(&m);
    
    EXPECT_FLOAT_EQ(m.rvec.x, 1.0f);
    EXPECT_FLOAT_EQ(m.rvec.y, 0.0f);
    EXPECT_FLOAT_EQ(m.rvec.z, 0.0f);
    EXPECT_FLOAT_EQ(m.uvec.x, 0.0f);
    EXPECT_FLOAT_EQ(m.uvec.y, 1.0f);
    EXPECT_FLOAT_EQ(m.uvec.z, 0.0f);
    EXPECT_FLOAT_EQ(m.fvec.x, 0.0f);
    EXPECT_FLOAT_EQ(m.fvec.y, 0.0f);
    EXPECT_FLOAT_EQ(m.fvec.z, 1.0f);
}

// ============================================================================
// vm_MakeZero Tests
// ============================================================================

class VmMakeZeroTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmMakeZeroTest, MakesVectorZero) {
    vector v = vgen.random();
    vm_MakeZero(&v);
    
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(VmMakeZeroTest, MakesAngvecZero) {
    angvec a = { 12345, 54321, 32767 };
    vm_MakeZero(&a);
    
    EXPECT_EQ(a.p, 0u);
    EXPECT_EQ(a.h, 0u);
    EXPECT_EQ(a.b, 0u);
}

// ============================================================================
// vm_GetMagnitude Tests
// ============================================================================

class VmGetMagnitudeTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmGetMagnitudeTest, ReturnsZeroForZeroVector) {
    vector v = {0.0f, 0.0f, 0.0f};
    float mag = vm_GetMagnitude(&v);
    EXPECT_FLOAT_EQ(mag, 0.0f);
}

TEST_F(VmGetMagnitudeTest, ReturnsCorrectMagnitude) {
    vector v = {3.0f, 4.0f, 0.0f};
    float mag = vm_GetMagnitude(&v);
    EXPECT_NEAR(mag, 5.0f, FLOAT_TOLERANCE);
}

TEST_F(VmGetMagnitudeTest, ReturnsCorrectMagnitude3D) {
    vector v = {1.0f, 2.0f, 2.0f};
    float mag = vm_GetMagnitude(&v);
    float expected = sqrtf(9.0f);  // 1+4+4 = 9
    EXPECT_NEAR(mag, expected, FLOAT_TOLERANCE);
}

TEST_F(VmGetMagnitudeTest, RandomVectors) {
    for (int i = 0; i < 100; i++) {
        vector v = vgen.random(-100.0f, 100.0f);
        if (v.x == 0 && v.y == 0 && v.z == 0) continue;
        
        float mag = vm_GetMagnitude(&v);
        float expected = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        EXPECT_NEAR(mag, expected, FLOAT_TOLERANCE) 
            << "Magnitude mismatch for vector (" << v.x << ", " << v.y << ", " << v.z << ")";
    }
}

// ============================================================================
// vm_DotProduct Tests
// ============================================================================

class VmDotProductTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmDotProductTest, ZeroVectorGivesZero) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {0.0f, 0.0f, 0.0f};
    float dot = vm_DotProduct(&a, &b);
    EXPECT_FLOAT_EQ(dot, 0.0f);
}

TEST_F(VmDotProductTest, OrthogonalVectorsGiveZero) {
    vector a = {1.0f, 0.0f, 0.0f};
    vector b = {0.0f, 1.0f, 0.0f};
    float dot = vm_DotProduct(&a, &b);
    EXPECT_FLOAT_EQ(dot, 0.0f);
}

TEST_F(VmDotProductTest, ParallelVectorsGiveProductOfMagnitudes) {
    vector a = {2.0f, 0.0f, 0.0f};
    vector b = {3.0f, 0.0f, 0.0f};
    float dot = vm_DotProduct(&a, &b);
    EXPECT_FLOAT_EQ(dot, 6.0f);
}

TEST_F(VmDotProductTest, RandomVectors) {
    for (int i = 0; i < 100; i++) {
        vector a = vgen.random(-100.0f, 100.0f);
        vector b = vgen.random(-100.0f, 100.0f);
        
        float dot = vm_DotProduct(&a, &b);
        float expected = a.x*b.x + a.y*b.y + a.z*b.z;
        EXPECT_NEAR(dot, expected, FLOAT_TOLERANCE);
    }
}

// ============================================================================
// vm_CrossProduct Tests
// ============================================================================

class VmCrossProductTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmCrossProductTest, PerpendicularToInputVectors) {
    vector a = {1.0f, 0.0f, 0.0f};
    vector b = {0.0f, 1.0f, 0.0f};
    vector c;
    vm_CrossProduct(&c, &a, &b);
    
    // Result should be (0, 0, 1)
    EXPECT_NEAR(c.x, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(c.y, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(c.z, 1.0f, FLOAT_TOLERANCE);
}

TEST_F(VmCrossProductTest, AntiCommutative) {
    vector a = vgen.random();
    vector b = vgen.random();
    vector c1, c2;
    
    vm_CrossProduct(&c1, &a, &b);
    vm_CrossProduct(&c2, &b, &a);
    
    EXPECT_NEAR(c1.x, -c2.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(c1.y, -c2.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(c1.z, -c2.z, FLOAT_TOLERANCE);
}

TEST_F(VmCrossProductTest, ZeroVectorForParallel) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {2.0f, 4.0f, 6.0f};  // Parallel to a
    vector c;
    vm_CrossProduct(&c, &a, &b);
    
    EXPECT_FLOAT_EQ(c.x, 0.0f);
    EXPECT_FLOAT_EQ(c.y, 0.0f);
    EXPECT_FLOAT_EQ(c.z, 0.0f);
}

// ============================================================================
// vm_SubVectors Tests
// ============================================================================

class VmSubVectorsTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmSubVectorsTest, BasicSubtraction) {
    vector a = {5.0f, 10.0f, 15.0f};
    vector b = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_SubVectors(&result, &a, &b);
    
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 8.0f);
    EXPECT_FLOAT_EQ(result.z, 12.0f);
}

TEST_F(VmSubVectorsTest, SubtractFromSelfGivesZero) {
    vector a = vgen.random();
    vector result;
    vm_SubVectors(&result, &a, &a);
    
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

// ============================================================================
// vm_AddVectors Tests
// ============================================================================

class VmAddVectorsTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmAddVectorsTest, BasicAddition) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, 5.0f, 6.0f};
    vector result;
    vm_AddVectors(&result, &a, &b);
    
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}

TEST_F(VmAddVectorsTest, AddZeroGivesOriginal) {
    vector a = vgen.random();
    vector b = {0.0f, 0.0f, 0.0f};
    vector result;
    vm_AddVectors(&result, &a, &b);
    
    EXPECT_FLOAT_EQ(result.x, a.x);
    EXPECT_FLOAT_EQ(result.y, a.y);
    EXPECT_FLOAT_EQ(result.z, a.z);
}

// ============================================================================
// vm_ScaleVector Tests
// ============================================================================

class VmScaleVectorTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmScaleVectorTest, ScaleByTwo) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_ScaleVector(&result, &a, 2.0f);
    
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

TEST_F(VmScaleVectorTest, ScaleByZeroGivesZero) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_ScaleVector(&result, &a, 0.0f);
    
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

TEST_F(VmScaleVectorTest, ScaleByNegative) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_ScaleVector(&result, &a, -1.0f);
    
    EXPECT_FLOAT_EQ(result.x, -1.0f);
    EXPECT_FLOAT_EQ(result.y, -2.0f);
    EXPECT_FLOAT_EQ(result.z, -3.0f);
}

// ============================================================================
// vm_NormalizeVector Tests
// ============================================================================

class VmNormalizeVectorTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmNormalizeVectorTest, UnitVectorStaysUnit) {
    vector v = {1.0f, 0.0f, 0.0f};
    float old_mag = vm_NormalizeVector(&v);
    EXPECT_FLOAT_EQ(old_mag, 1.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(VmNormalizeVectorTest, ReturnsOldMagnitude) {
    vector v = {3.0f, 4.0f, 0.0f};
    float old_mag = vm_NormalizeVector(&v);
    EXPECT_NEAR(old_mag, 5.0f, FLOAT_TOLERANCE);
}

TEST_F(VmNormalizeVectorTest, ResultIsUnitLength) {
    vector v = {3.0f, 4.0f, 0.0f};
    vm_NormalizeVector(&v);
    float mag = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    EXPECT_NEAR(mag, 1.0f, FLOAT_TOLERANCE);
}

TEST_F(VmNormalizeVectorTest, RandomVectors) {
    for (int i = 0; i < 100; i++) {
        vector v = vgen.random(-100.0f, 100.0f);
        // Avoid near-zero vectors
        if (fabsf(v.x) < 0.01f && fabsf(v.y) < 0.01f && fabsf(v.z) < 0.01f) continue;
        
        float old_mag = vm_NormalizeVector(&v);
        float new_mag = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        
        EXPECT_NEAR(new_mag, 1.0f, FLOAT_TOLERANCE)
            << "Normalized vector not unit length for seed iteration " << i;
    }
}

// ============================================================================
// vm_MatrixMulVector Tests
// ============================================================================

class VmMatrixMulVectorTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
    MatrixGenerator mgen;
};

TEST_F(VmMatrixMulVectorTest, IdentityMatrixPreservesVector) {
    vector v = {1.0f, 2.0f, 3.0f};
    matrix m = IDENTITY_MATRIX;
    vector result;
    vm_MatrixMulVector(&result, &v, &m);
    
    EXPECT_FLOAT_EQ(result.x, v.x);
    EXPECT_FLOAT_EQ(result.y, v.y);
    EXPECT_FLOAT_EQ(result.z, v.z);
}

TEST_F(VmMatrixMulVectorTest, ZeroMatrixGivesZero) {
    vector v = {1.0f, 2.0f, 3.0f};
    matrix m = { {0,0,0}, {0,0,0}, {0,0,0} };
    vector result;
    vm_MatrixMulVector(&result, &v, &m);
    
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

// ============================================================================
// vm_MatrixMul Tests
// ============================================================================

class VmMatrixMulTest : public ::testing::Test {
protected:
    MatrixGenerator mgen;
};

TEST_F(VmMatrixMulTest, IdentityTimesIdentityIsIdentity) {
    matrix a = IDENTITY_MATRIX;
    matrix b = IDENTITY_MATRIX;
    matrix result;
    vm_MatrixMul(&result, &a, &b);
    
    matrix expected = IDENTITY_MATRIX;
    EXPECT_NEAR(result.rvec.x, expected.rvec.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.rvec.y, expected.rvec.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.uvec.x, expected.uvec.x, FLOAT_TOLERANCE);
}

TEST_F(VmMatrixMulTest, AnyMatrixTimesIdentityIsSame) {
    matrix a = mgen.random();
    matrix b = IDENTITY_MATRIX;
    matrix result;
    vm_MatrixMul(&result, &a, &b);
    
    EXPECT_NEAR(result.rvec.x, a.rvec.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.rvec.y, a.rvec.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.rvec.z, a.rvec.z, FLOAT_TOLERANCE);
}

// ============================================================================
// vm_TransposeMatrix Tests
// ============================================================================

class VmTransposeMatrixTest : public ::testing::Test {
protected:
    MatrixGenerator mgen;
};

TEST_F(VmTransposeMatrixTest, IdentityIsSelfTranspose) {
    matrix m = IDENTITY_MATRIX;
    matrix original = m;
    vm_TransposeMatrix(&m);
    
    EXPECT_FLOAT_EQ(m.rvec.x, original.rvec.x);
    EXPECT_FLOAT_EQ(m.rvec.y, original.rvec.y);
}

TEST_F(VmTransposeMatrixTest, TransposeReversesRowsColumns) {
    matrix m = mgen.random();
    matrix original = m;
    vm_TransposeMatrix(&m);
    
    // After transpose: m.rvec.x should become original.rvec.x (diagonal stays)
    // But m.rvec.y (was original.uvec.x) should become original.rvec.y
    EXPECT_NEAR(m.rvec.y, original.uvec.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.rvec.z, original.fvec.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.uvec.x, original.rvec.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.uvec.z, original.fvec.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.fvec.x, original.rvec.z, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.fvec.y, original.uvec.z, FLOAT_TOLERANCE);
}

// ============================================================================
// vm_AnglesToMatrix Tests
// ============================================================================

class VmAnglesToMatrixTest : public ::testing::Test {};

TEST_F(VmAnglesToMatrixTest, ZeroAnglesGivesIdentity) {
    matrix m;
    vm_AnglesToMatrix(&m, 0, 0, 0);
    
    EXPECT_NEAR(m.rvec.x, 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.uvec.y, 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.fvec.z, 1.0f, FLOAT_TOLERANCE);
}

// ============================================================================
// vm_VectorDistance Tests
// ============================================================================

class VmVectorDistanceTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmVectorDistanceTest, SamePointGivesZero) {
    vector p = vgen.random();
    float dist = vm_VectorDistance(&p, &p);
    EXPECT_FLOAT_EQ(dist, 0.0f);
}

TEST_F(VmVectorDistanceTest, KnownDistance) {
    vector a = {0.0f, 0.0f, 0.0f};
    vector b = {3.0f, 4.0f, 0.0f};
    float dist = vm_VectorDistance(&a, &b);
    EXPECT_NEAR(dist, 5.0f, FLOAT_TOLERANCE);
}

TEST_F(VmVectorDistanceTest, Symmetric) {
    vector a = vgen.random();
    vector b = vgen.random();
    float d1 = vm_VectorDistance(&a, &b);
    float d2 = vm_VectorDistance(&b, &a);
    EXPECT_FLOAT_EQ(d1, d2);
}

// ============================================================================
// vm_GetNormalizedDir Tests
// ============================================================================

class VmGetNormalizedDirTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmGetNormalizedDirTest, ReturnsCorrectDistance) {
    vector start = {0.0f, 0.0f, 0.0f};
    vector end = {3.0f, 4.0f, 0.0f};
    vector dir;
    float dist = vm_GetNormalizedDir(&dir, &end, &start);
    
    EXPECT_NEAR(dist, 5.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(dir.x, 0.6f, FLOAT_TOLERANCE);
    EXPECT_NEAR(dir.y, 0.8f, FLOAT_TOLERANCE);
    EXPECT_NEAR(dir.z, 0.0f, FLOAT_TOLERANCE);
}

TEST_F(VmGetNormalizedDirTest, DirectionIsNormalized) {
    for (int i = 0; i < 50; i++) {
        vector start = vgen.random(-100.0f, 100.0f);
        vector end = vgen.random(-100.0f, 100.0f);
        
        // Avoid same point
        if (start.x == end.x && start.y == end.y && start.z == end.z) continue;
        
        vector dir;
        vm_GetNormalizedDir(&dir, &end, &start);
        
        float mag = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        EXPECT_NEAR(mag, 1.0f, FLOAT_TOLERANCE);
    }
}

// ============================================================================
// vm_GetPerp Tests (3 points to normal)
// ============================================================================

class VmGetPerpTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmGetPerpTest, XYPlaneGivesZNormal) {
    vector a = {0.0f, 0.0f, 0.0f};
    vector b = {1.0f, 0.0f, 0.0f};
    vector c = {0.0f, 1.0f, 0.0f};
    vector n;
    vm_GetPerp(&n, &a, &b, &c);
    
    EXPECT_NEAR(n.x, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(n.y, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(n.z, 1.0f, FLOAT_TOLERANCE);
}

// ============================================================================
// vm_GetNormal Tests (3 points to normalized normal)
// ============================================================================

class VmGetNormalTest : public ::testing::Test {
protected:
    VectorGenerator vgen;
};

TEST_F(VmGetNormalTest, XYPlaneGivesNormalizedZ) {
    vector a = {0.0f, 0.0f, 0.0f};
    vector b = {1.0f, 0.0f, 0.0f};
    vector c = {0.0f, 1.0f, 0.0f};
    vector n;
    float mag = vm_GetNormal(&n, &a, &b, &c);
    
    EXPECT_NEAR(n.x, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(n.y, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(n.z, 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(mag, 1.0f, FLOAT_TOLERANCE);
}

// ============================================================================
// vm_ExtractAnglesFromMatrix Tests
// ============================================================================

class VmExtractAnglesFromMatrixTest : public ::testing::Test {};

TEST_F(VmExtractAnglesFromMatrixTest, IdentityGivesZeroAngles) {
    matrix m = IDENTITY_MATRIX;
    angvec a;
    vm_ExtractAnglesFromMatrix(&a, &m);
    
    // Identity matrix should give zero angles (or small values)
    EXPECT_NEAR((float)a.p, 0.0f, 100.0f);  // Allow some tolerance
    EXPECT_NEAR((float)a.h, 0.0f, 100.0f);
    EXPECT_NEAR((float)a.b, 0.0f, 100.0f);
}

// ============================================================================
// vm_Orthogonalize Tests
// ============================================================================

class VmOrthogonalizeTest : public ::testing::Test {
protected:
    MatrixGenerator mgen;
};

TEST_F(VmOrthogonalizeTest, IdentityStaysIdentity) {
    matrix m = IDENTITY_MATRIX;
    vm_Orthogonalize(&m);
    
    EXPECT_NEAR(m.rvec.x, 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.uvec.y, 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(m.fvec.z, 1.0f, FLOAT_TOLERANCE);
}

// ============================================================================
// vm_SinCos Tests
// ============================================================================

class VmSinCosTest : public ::testing::Test {};

TEST_F(VmSinCosTest, ZeroGivesZeroSinOneCos) {
    float sin_val, cos_val;
    vm_SinCos(0, &sin_val, &cos_val);
    
    EXPECT_NEAR(sin_val, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(cos_val, 1.0f, FLOAT_TOLERANCE);
}

TEST_F(VmSinCosTest, 90Degrees) {
    float sin_val, cos_val;
    // 90 degrees = 16384 in angle units
    vm_SinCos(16384, &sin_val, &cos_val);
    
    EXPECT_NEAR(sin_val, 1.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(cos_val, 0.0f, FLOAT_TOLERANCE);
}

// ============================================================================
// vm_GetSlope Tests
// ============================================================================

class VmGetSlopeTest : public ::testing::Test {};

TEST_F(VmGetSlopeTest, SimpleSlope) {
    float slope = vm_GetSlope(0.0f, 0.0f, 1.0f, 1.0f);
    EXPECT_FLOAT_EQ(slope, 1.0f);
}

TEST_F(VmGetSlopeTest, NegativeSlope) {
    float slope = vm_GetSlope(0.0f, 0.0f, 1.0f, -1.0f);
    EXPECT_FLOAT_EQ(slope, -1.0f);
}

TEST_F(VmGetSlopeTest, VerticalLineGivesInfinity) {
    float slope = vm_GetSlope(1.0f, 0.0f, 1.0f, 1.0f);
    // Should be a large value (approaching infinity)
    EXPECT_GT(fabsf(slope), 10000.0f);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
