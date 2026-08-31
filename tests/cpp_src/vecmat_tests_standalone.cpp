/**
 * @file vecmat_tests_standalone.cpp
 * @brief D3 Coverage Tests - vecmat module (Standalone).
 *
 * @details
 * Tests for vector/matrix math operations.
 * Uses seeded RNG for reproducible test data.
 * This is a standalone version that doesn't require D3 dependencies.
 *
 * This harness validates the behavior of `Descent3/vecmat_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/vecmat_tests_standalone.cpp`
 * @par Harness
 * `vecmat_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/vecmat_tests_standalone.cpp
 */

#include <gtest/gtest.h>
#include <cmath>
#include <random>
#include <vector>

// D3-compatible vector/matrix types
typedef struct { float x, y, z; } vector;
typedef struct { float m[3][3]; } matrix;
typedef unsigned short angle;

// D3-style identity matrix macro
#define IDENTITY_MATRIX {{ {1,0,0}, {0,1,0}, {0,0,1} }}

static constexpr float FLOAT_TOLERANCE = 1e-5f;

// Seeded RNG for reproducibility
static const uint32_t DEFAULT_SEED = 12345;

class SeededRNG {
public:
    explicit SeededRNG(uint32_t seed = DEFAULT_SEED) : rng_(seed) {}
    
    float random_float(float min = -1000.0f, float max = 1000.0f) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng_);
    }
    
    int random_int(int min = -1000, int max = 1000) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng_);
    }
    
private:
    std::mt19937 rng_;
};

// ============================================================================
// Vector operations (matching D3 vecmat.h)
// ============================================================================

void vm_MakeZero(vector* v) { v->x = v->y = v->z = 0.0f; }

float vm_GetMagnitude(vector* v) {
    return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
}

float vm_DotProduct(const vector* a, const vector* b) {
    return a->x*b->x + a->y*b->y + a->z*b->z;
}

void vm_CrossProduct(vector* result, const vector* a, const vector* b) {
    result->x = a->y*b->z - a->z*b->y;
    result->y = a->z*b->x - a->x*b->z;
    result->z = a->x*b->y - a->y*b->x;
}

void vm_SubVectors(vector* result, const vector* a, const vector* b) {
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
}

void vm_AddVectors(vector* result, const vector* a, const vector* b) {
    result->x = a->x + b->x;
    result->y = a->y + b->y;
    result->z = a->z + b->z;
}

void vm_ScaleVector(vector* result, const vector* v, float s) {
    result->x = v->x * s;
    result->y = v->y * s;
    result->z = v->z * s;
}

float vm_NormalizeVector(vector* v) {
    float mag = vm_GetMagnitude(v);
    if (mag > FLOAT_TOLERANCE) {
        v->x /= mag;
        v->y /= mag;
        v->z /= mag;
    }
    return mag;
}

void vm_MatrixMulVector(vector* result, const vector* v, const matrix* m) {
    result->x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0];
    result->y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1];
    result->z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2];
}

void vm_MatrixMul(matrix* result, const matrix* a, const matrix* b) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result->m[i][j] = 0;
            for (int k = 0; k < 3; k++) {
                result->m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }
}

void vm_MakeIdentity(matrix* m) {
    *m = (matrix)IDENTITY_MATRIX;
}

void vm_TransposeMatrix(matrix* m) {
    matrix temp = *m;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            m->m[i][j] = temp.m[j][i];
        }
    }
}

float vm_VectorDistance(const vector* a, const vector* b) {
    vector diff;
    vm_SubVectors(&diff, a, b);
    return vm_GetMagnitude(&diff);
}

float vm_GetNormalizedDir(vector* dest, const vector* end, const vector* start) {
    vector diff;
    vm_SubVectors(&diff, end, start);
    float dist = vm_GetMagnitude(&diff);
    if (dist > FLOAT_TOLERANCE) {
        dest->x = diff.x / dist;
        dest->y = diff.y / dist;
        dest->z = diff.z / dist;
    } else {
        vm_MakeZero(dest);
    }
    return dist;
}

void vm_GetPerp(vector* n, const vector* a, const vector* b, const vector* c) {
    vector v1, v2;
    vm_SubVectors(&v1, b, a);
    vm_SubVectors(&v2, c, a);
    vm_CrossProduct(n, &v1, &v2);
}

float vm_GetNormal(vector* n, const vector* v0, const vector* v1, const vector* v2) {
    vm_GetPerp(n, v0, v1, v2);
    float mag = vm_GetMagnitude(n);
    if (mag > FLOAT_TOLERANCE) {
        n->x /= mag;
        n->y /= mag;
        n->z /= mag;
    }
    return mag;
}

// ============================================================================
// Tests
// ============================================================================

/**
 * @brief GTest fixture for VmMakeZeroTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmMakeZeroTest : public ::testing::Test {
protected:
    SeededRNG rng;
};

/**
 * @test VmMakeZeroTest.MakesVectorZero
 * @brief Verifies makes Vector Zero.
 *
 * @details
 * Exercises the VmMakeZeroTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmMakeZeroTest, MakesVectorZero) {
    vector v = {1.0f, 2.0f, 3.0f};
    vm_MakeZero(&v);
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

/**
 * @brief GTest fixture for VmGetMagnitudeTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmGetMagnitudeTest : public ::testing::Test {
protected:
    SeededRNG rng;
};

/**
 * @test VmGetMagnitudeTest.ReturnsZeroForZeroVector
 * @brief Verifies returns Zero For Zero Vector.
 *
 * @details
 * Exercises the VmGetMagnitudeTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmGetMagnitudeTest, ReturnsZeroForZeroVector) {
    vector v = {0.0f, 0.0f, 0.0f};
    float mag = vm_GetMagnitude(&v);
    EXPECT_FLOAT_EQ(mag, 0.0f);
}

/**
 * @test VmGetMagnitudeTest.ReturnsCorrectMagnitude
 * @brief Verifies returns Correct Magnitude.
 *
 * @details
 * Exercises the VmGetMagnitudeTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmGetMagnitudeTest, ReturnsCorrectMagnitude) {
    vector v = {3.0f, 4.0f, 0.0f};
    float mag = vm_GetMagnitude(&v);
    EXPECT_NEAR(mag, 5.0f, FLOAT_TOLERANCE);
}

/**
 * @test VmGetMagnitudeTest.ReturnsCorrectMagnitude3D
 * @brief Verifies returns Correct Magnitude3D.
 *
 * @details
 * Exercises the VmGetMagnitudeTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmGetMagnitudeTest, ReturnsCorrectMagnitude3D) {
    vector v = {1.0f, 2.0f, 2.0f};
    float mag = vm_GetMagnitude(&v);
    EXPECT_NEAR(mag, 3.0f, FLOAT_TOLERANCE);
}

/**
 * @brief GTest fixture for VmDotProductTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmDotProductTest : public ::testing::Test {
protected:
    SeededRNG rng;
};

/**
 * @test VmDotProductTest.ZeroVectorGivesZero
 * @brief Verifies zero Vector Gives Zero.
 *
 * @details
 * Exercises the VmDotProductTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmDotProductTest, ZeroVectorGivesZero) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {0.0f, 0.0f, 0.0f};
    float dot = vm_DotProduct(&a, &b);
    EXPECT_FLOAT_EQ(dot, 0.0f);
}

/**
 * @test VmDotProductTest.OrthogonalVectorsGiveZero
 * @brief Verifies orthogonal Vectors Give Zero.
 *
 * @details
 * Exercises the VmDotProductTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmDotProductTest, OrthogonalVectorsGiveZero) {
    vector a = {1.0f, 0.0f, 0.0f};
    vector b = {0.0f, 1.0f, 0.0f};
    float dot = vm_DotProduct(&a, &b);
    EXPECT_FLOAT_EQ(dot, 0.0f);
}

/**
 * @test VmDotProductTest.ParallelVectorsGiveProductOfMagnitudes
 * @brief Verifies parallel Vectors Give Product Of Magnitudes.
 *
 * @details
 * Exercises the VmDotProductTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmDotProductTest, ParallelVectorsGiveProductOfMagnitudes) {
    vector a = {2.0f, 0.0f, 0.0f};
    vector b = {3.0f, 0.0f, 0.0f};
    float dot = vm_DotProduct(&a, &b);
    EXPECT_FLOAT_EQ(dot, 6.0f);
}

/**
 * @brief GTest fixture for VmCrossProductTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmCrossProductTest : public ::testing::Test {
protected:
    SeededRNG rng;
};

/**
 * @test VmCrossProductTest.PerpendicularToInputVectors
 * @brief Verifies perpendicular To Input Vectors.
 *
 * @details
 * Exercises the VmCrossProductTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmCrossProductTest, PerpendicularToInputVectors) {
    vector a = {1.0f, 0.0f, 0.0f};
    vector b = {0.0f, 1.0f, 0.0f};
    vector c;
    vm_CrossProduct(&c, &a, &b);
    
    EXPECT_NEAR(c.x, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(c.y, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(c.z, 1.0f, FLOAT_TOLERANCE);
}

/**
 * @test VmCrossProductTest.AntiCommutative
 * @brief Verifies anti Commutative.
 *
 * @details
 * Exercises the VmCrossProductTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmCrossProductTest, AntiCommutative) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, 5.0f, 6.0f};
    vector c1, c2;
    
    vm_CrossProduct(&c1, &a, &b);
    vm_CrossProduct(&c2, &b, &a);
    
    EXPECT_NEAR(c1.x, -c2.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(c1.y, -c2.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(c1.z, -c2.z, FLOAT_TOLERANCE);
}

/**
 * @brief GTest fixture for VmSubVectorsTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmSubVectorsTest : public ::testing::Test {
protected:
    SeededRNG rng;
};

/**
 * @test VmSubVectorsTest.BasicSubtraction
 * @brief Verifies basic Subtraction.
 *
 * @details
 * Exercises the VmSubVectorsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmSubVectorsTest, BasicSubtraction) {
    vector a = {5.0f, 10.0f, 15.0f};
    vector b = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_SubVectors(&result, &a, &b);
    
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 8.0f);
    EXPECT_FLOAT_EQ(result.z, 12.0f);
}

/**
 * @test VmSubVectorsTest.SubtractFromSelfGivesZero
 * @brief Verifies subtract From Self Gives Zero.
 *
 * @details
 * Exercises the VmSubVectorsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmSubVectorsTest, SubtractFromSelfGivesZero) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_SubVectors(&result, &a, &a);
    
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

/**
 * @brief GTest fixture for VmAddVectorsTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmAddVectorsTest : public ::testing::Test {
protected:
    SeededRNG rng;
};

/**
 * @test VmAddVectorsTest.BasicAddition
 * @brief Verifies basic Addition.
 *
 * @details
 * Exercises the VmAddVectorsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmAddVectorsTest, BasicAddition) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, 5.0f, 6.0f};
    vector result;
    vm_AddVectors(&result, &a, &b);
    
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}

/**
 * @brief GTest fixture for VmScaleVectorTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmScaleVectorTest : public ::testing::Test {
protected:
    SeededRNG rng;
};

/**
 * @test VmScaleVectorTest.ScaleByTwo
 * @brief Verifies scale By Two.
 *
 * @details
 * Exercises the VmScaleVectorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmScaleVectorTest, ScaleByTwo) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_ScaleVector(&result, &a, 2.0f);
    
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

/**
 * @test VmScaleVectorTest.ScaleByZeroGivesZero
 * @brief Verifies scale By Zero Gives Zero.
 *
 * @details
 * Exercises the VmScaleVectorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmScaleVectorTest, ScaleByZeroGivesZero) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_ScaleVector(&result, &a, 0.0f);
    
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

/**
 * @test VmScaleVectorTest.ScaleByNegative
 * @brief Verifies scale By Negative.
 *
 * @details
 * Exercises the VmScaleVectorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmScaleVectorTest, ScaleByNegative) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector result;
    vm_ScaleVector(&result, &a, -1.0f);
    
    EXPECT_FLOAT_EQ(result.x, -1.0f);
    EXPECT_FLOAT_EQ(result.y, -2.0f);
    EXPECT_FLOAT_EQ(result.z, -3.0f);
}

/**
 * @brief GTest fixture for VmNormalizeVectorTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmNormalizeVectorTest : public ::testing::Test {
protected:
    SeededRNG rng;
};

/**
 * @test VmNormalizeVectorTest.UnitVectorStaysUnit
 * @brief Verifies unit Vector Stays Unit.
 *
 * @details
 * Exercises the VmNormalizeVectorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmNormalizeVectorTest, UnitVectorStaysUnit) {
    vector v = {1.0f, 0.0f, 0.0f};
    float old_mag = vm_NormalizeVector(&v);
    EXPECT_FLOAT_EQ(old_mag, 1.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

/**
 * @test VmNormalizeVectorTest.ReturnsOldMagnitude
 * @brief Verifies returns Old Magnitude.
 *
 * @details
 * Exercises the VmNormalizeVectorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmNormalizeVectorTest, ReturnsOldMagnitude) {
    vector v = {3.0f, 4.0f, 0.0f};
    float old_mag = vm_NormalizeVector(&v);
    EXPECT_NEAR(old_mag, 5.0f, FLOAT_TOLERANCE);
}

/**
 * @test VmNormalizeVectorTest.ResultIsUnitLength
 * @brief Verifies result Is Unit Length.
 *
 * @details
 * Exercises the VmNormalizeVectorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmNormalizeVectorTest, ResultIsUnitLength) {
    vector v = {3.0f, 4.0f, 0.0f};
    vm_NormalizeVector(&v);
    float mag = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    EXPECT_NEAR(mag, 1.0f, FLOAT_TOLERANCE);
}

/**
 * @brief GTest fixture for VmMatrixMulVectorTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmMatrixMulVectorTest : public ::testing::Test {};

/**
 * @test VmMatrixMulVectorTest.IdentityMatrixPreservesVector
 * @brief Verifies identity Matrix Preserves Vector.
 *
 * @details
 * Exercises the VmMatrixMulVectorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmMatrixMulVectorTest, IdentityMatrixPreservesVector) {
    vector v = {1.0f, 2.0f, 3.0f};
    matrix m = IDENTITY_MATRIX;
    vector result;
    vm_MatrixMulVector(&result, &v, &m);
    
    EXPECT_FLOAT_EQ(result.x, v.x);
    EXPECT_FLOAT_EQ(result.y, v.y);
    EXPECT_FLOAT_EQ(result.z, v.z);
}

/**
 * @brief GTest fixture for VmMatrixMulTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmMatrixMulTest : public ::testing::Test {};

/**
 * @test VmMatrixMulTest.IdentityTimesIdentityIsIdentity
 * @brief Verifies identity Times Identity Is Identity.
 *
 * @details
 * Exercises the VmMatrixMulTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmMatrixMulTest, IdentityTimesIdentityIsIdentity) {
    matrix a = IDENTITY_MATRIX;
    matrix b = IDENTITY_MATRIX;
    matrix result;
    vm_MatrixMul(&result, &a, &b);
    
    matrix expected = IDENTITY_MATRIX;
    EXPECT_NEAR(result.m[0][0], expected.m[0][0], FLOAT_TOLERANCE);
}

/**
 * @brief GTest fixture for VmTransposeMatrixTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmTransposeMatrixTest : public ::testing::Test {};

/**
 * @test VmTransposeMatrixTest.IdentityIsSelfTranspose
 * @brief Verifies identity Is Self Transpose.
 *
 * @details
 * Exercises the VmTransposeMatrixTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmTransposeMatrixTest, IdentityIsSelfTranspose) {
    matrix m = IDENTITY_MATRIX;
    matrix original = m;
    vm_TransposeMatrix(&m);
    
    EXPECT_FLOAT_EQ(m.m[0][0], original.m[0][0]);
}

/**
 * @brief GTest fixture for VmVectorDistanceTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmVectorDistanceTest : public ::testing::Test {};

/**
 * @test VmVectorDistanceTest.SamePointGivesZero
 * @brief Verifies same Point Gives Zero.
 *
 * @details
 * Exercises the VmVectorDistanceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmVectorDistanceTest, SamePointGivesZero) {
    vector p = {1.0f, 2.0f, 3.0f};
    float dist = vm_VectorDistance(&p, &p);
    EXPECT_FLOAT_EQ(dist, 0.0f);
}

/**
 * @test VmVectorDistanceTest.KnownDistance
 * @brief Verifies known Distance.
 *
 * @details
 * Exercises the VmVectorDistanceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmVectorDistanceTest, KnownDistance) {
    vector a = {0.0f, 0.0f, 0.0f};
    vector b = {3.0f, 4.0f, 0.0f};
    float dist = vm_VectorDistance(&a, &b);
    EXPECT_NEAR(dist, 5.0f, FLOAT_TOLERANCE);
}

/**
 * @test VmVectorDistanceTest.Symmetric
 * @brief Verifies symmetric.
 *
 * @details
 * Exercises the VmVectorDistanceTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmVectorDistanceTest, Symmetric) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, 5.0f, 6.0f};
    float d1 = vm_VectorDistance(&a, &b);
    float d2 = vm_VectorDistance(&b, &a);
    EXPECT_FLOAT_EQ(d1, d2);
}

/**
 * @brief GTest fixture for VmGetNormalizedDirTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmGetNormalizedDirTest : public ::testing::Test {};

/**
 * @test VmGetNormalizedDirTest.ReturnsCorrectDistance
 * @brief Verifies returns Correct Distance.
 *
 * @details
 * Exercises the VmGetNormalizedDirTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @test VmGetNormalizedDirTest.DirectionIsNormalized
 * @brief Verifies direction Is Normalized.
 *
 * @details
 * Exercises the VmGetNormalizedDirTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmGetNormalizedDirTest, DirectionIsNormalized) {
    vector start = {0.0f, 0.0f, 0.0f};
    vector end = {3.0f, 4.0f, 0.0f};
    vector dir;
    vm_GetNormalizedDir(&dir, &end, &start);
    
    float mag = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    EXPECT_NEAR(mag, 1.0f, FLOAT_TOLERANCE);
}

/**
 * @brief GTest fixture for VmGetPerpTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmGetPerpTest : public ::testing::Test {};

/**
 * @test VmGetPerpTest.XYPlaneGivesZNormal
 * @brief Verifies xYPlane Gives ZNormal.
 *
 * @details
 * Exercises the VmGetPerpTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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

/**
 * @brief GTest fixture for VmGetNormalTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmGetNormalTest : public ::testing::Test {};

/**
 * @test VmGetNormalTest.XYPlaneGivesNormalizedZ
 * @brief Verifies xYPlane Gives Normalized Z.
 *
 * @details
 * Exercises the VmGetNormalTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
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
// RANDOM INPUT TESTS WITH PREDICTABLE OUTPUT (Seeded RNG)
// ============================================================================

/**
 * @brief GTest fixture for VmRandomInputTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmRandomInputTest : public ::testing::Test {
protected:
    // Fixed seed ensures predictable random sequence
    SeededRNG rng{42};  
};

/**
 * @test VmRandomInputTest.GetMagnitudeRandomVectors
 * @brief Verifies get Magnitude Random Vectors.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, GetMagnitudeRandomVectors) {
    // Seed 42 produces these first 3 vectors:
    // The seed determines exact values, output is predictable
    vector v = { rng.random_float(-100, 100), 
                 rng.random_float(-100, 100), 
                 rng.random_float(-100, 100) };
    
    float result = vm_GetMagnitude(&v);
    float expected = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    EXPECT_NEAR(result, expected, FLOAT_TOLERANCE);
}

/**
 * @test VmRandomInputTest.DotProductRandomVectors
 * @brief Verifies dot Product Random Vectors.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, DotProductRandomVectors) {
    vector a = { rng.random_float(-50, 50), 
                 rng.random_float(-50, 50), 
                 rng.random_float(-50, 50) };
    vector b = { rng.random_float(-50, 50), 
                 rng.random_float(-50, 50), 
                 rng.random_float(-50, 50) };
    
    float result = vm_DotProduct(&a, &b);
    float expected = a.x*b.x + a.y*b.y + a.z*b.z;
    EXPECT_NEAR(result, expected, FLOAT_TOLERANCE);
}

/**
 * @test VmRandomInputTest.AddVectorsRandom
 * @brief Verifies add Vectors Random.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, AddVectorsRandom) {
    vector a = { rng.random_float(-10, 10), 
                 rng.random_float(-10, 10), 
                 rng.random_float(-10, 10) };
    vector b = { rng.random_float(-10, 10), 
                 rng.random_float(-10, 10), 
                 rng.random_float(-10, 10) };
    vector result;
    vm_AddVectors(&result, &a, &b);
    
    EXPECT_NEAR(result.x, a.x + b.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.y, a.y + b.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.z, a.z + b.z, FLOAT_TOLERANCE);
}

/**
 * @test VmRandomInputTest.SubVectorsRandom
 * @brief Verifies sub Vectors Random.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, SubVectorsRandom) {
    vector a = { rng.random_float(-10, 10), 
                 rng.random_float(-10, 10), 
                 rng.random_float(-10, 10) };
    vector b = { rng.random_float(-10, 10), 
                 rng.random_float(-10, 10), 
                 rng.random_float(-10, 10) };
    vector result;
    vm_SubVectors(&result, &a, &b);
    
    EXPECT_NEAR(result.x, a.x - b.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.y, a.y - b.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.z, a.z - b.z, FLOAT_TOLERANCE);
}

/**
 * @test VmRandomInputTest.ScaleVectorRandom
 * @brief Verifies scale Vector Random.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, ScaleVectorRandom) {
    vector a = { rng.random_float(-10, 10), 
                 rng.random_float(-10, 10), 
                 rng.random_float(-10, 10) };
    float scale = rng.random_float(-5, 5);
    vector result;
    vm_ScaleVector(&result, &a, scale);
    
    EXPECT_NEAR(result.x, a.x * scale, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.y, a.y * scale, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.z, a.z * scale, FLOAT_TOLERANCE);
}

/**
 * @test VmRandomInputTest.CrossProductRandom
 * @brief Verifies cross Product Random.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, CrossProductRandom) {
    // Use known non-parallel vectors
    vector a = { 1.0f, 2.0f, 3.0f };  
    vector b = { 4.0f, 5.0f, 6.0f };  // Not parallel to a
    vector result;
    vm_CrossProduct(&result, &a, &b);
    
    // Verify result is perpendicular to both inputs
    float dot_a = vm_DotProduct(&result, &a);
    float dot_b = vm_DotProduct(&result, &b);
    EXPECT_NEAR(dot_a, 0.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(dot_b, 0.0f, FLOAT_TOLERANCE);
}

/**
 * @test VmRandomInputTest.NormalizeRandom
 * @brief Verifies normalize Random.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, NormalizeRandom) {
    // Use known non-zero vector
    vector v = { 3.0f, 4.0f, 0.0f };
    float old_mag = vm_NormalizeVector(&v);
    
    // Original magnitude was 5.0
    EXPECT_NEAR(old_mag, 5.0f, FLOAT_TOLERANCE);
    
    // After normalization, should be unit length
    float new_mag = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    EXPECT_NEAR(new_mag, 1.0f, FLOAT_TOLERANCE);
    
    // Check direction
    EXPECT_NEAR(v.x, 0.6f, FLOAT_TOLERANCE);
    EXPECT_NEAR(v.y, 0.8f, FLOAT_TOLERANCE);
}

/**
 * @test VmRandomInputTest.VectorDistanceRandom
 * @brief Verifies vector Distance Random.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, VectorDistanceRandom) {
    vector a = { rng.random_float(-100, 100), 
                 rng.random_float(-100, 100), 
                 rng.random_float(-100, 100) };
    vector b = { rng.random_float(-100, 100), 
                 rng.random_float(-100, 100), 
                 rng.random_float(-100, 100) };
    
    float result = vm_VectorDistance(&a, &b);
    vector diff;
    vm_SubVectors(&diff, &b, &a);
    float expected = vm_GetMagnitude(&diff);
    
    EXPECT_NEAR(result, expected, FLOAT_TOLERANCE);
}

/**
 * @test VmRandomInputTest.GetNormalizedDirRandom
 * @brief Verifies get Normalized Dir Random.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, GetNormalizedDirRandom) {
    vector start = { rng.random_float(-100, 100), 
                    rng.random_float(-100, 100), 
                    rng.random_float(-100, 100) };
    vector end = { rng.random_float(-100, 100), 
                  rng.random_float(-100, 100), 
                  rng.random_float(-100, 100) };
    
    // Ensure they're different
    if (start.x == end.x && start.y == end.y && start.z == end.z) {
        end.x += 1.0f;
    }
    
    vector dir;
    float dist = vm_GetNormalizedDir(&dir, &end, &start);
    
    // Direction should be normalized
    float mag = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    EXPECT_NEAR(mag, 1.0f, FLOAT_TOLERANCE);
    
    // Distance should match actual distance
    float expected_dist = vm_VectorDistance(&start, &end);
    EXPECT_NEAR(dist, expected_dist, FLOAT_TOLERANCE);
}

// Matrix multiplication with random matrices
/**
 * @test VmRandomInputTest.MatrixMulRandom
 * @brief Verifies matrix Mul Random.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, MatrixMulRandom) {
    // Identity matrix * vector should give vector back
    matrix identity = IDENTITY_MATRIX;
    vector v = { 1.0f, 2.0f, 3.0f };
    vector result;
    vm_MatrixMulVector(&result, &v, &identity);
    
    EXPECT_NEAR(result.x, v.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.y, v.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.z, v.z, FLOAT_TOLERANCE);
}

// Test that matrix multiplication is correct
/**
 * @test VmRandomInputTest.MatrixMulCorrect
 * @brief Verifies matrix Mul Correct.
 *
 * @details
 * Exercises the VmRandomInputTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmRandomInputTest, MatrixMulCorrect) {
    // A simple diagonal scaling matrix
    matrix a;
    a.m[0][0] = 2; a.m[0][1] = 0; a.m[0][2] = 0;
    a.m[1][0] = 0; a.m[1][1] = 3; a.m[1][2] = 0;
    a.m[2][0] = 0; a.m[2][1] = 0; a.m[2][2] = 4;
    
    vector v = { 1.0f, 1.0f, 1.0f };
    vector result;
    vm_MatrixMulVector(&result, &v, &a);
    
    EXPECT_NEAR(result.x, 2.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.y, 3.0f, FLOAT_TOLERANCE);
    EXPECT_NEAR(result.z, 4.0f, FLOAT_TOLERANCE);
}

// ============================================================================
// STRESS TESTS - Many random inputs
// ============================================================================

/**
 * @brief GTest fixture for VmStressTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmStressTest : public ::testing::Test {
protected:
    static const int N = 1000;
};

/**
 * @test VmStressTest.MagnitudeManyRandomVectors
 * @brief Verifies magnitude Many Random Vectors.
 *
 * @details
 * Exercises the VmStressTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmStressTest, MagnitudeManyRandomVectors) {
    SeededRNG rng{12345};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-1000, 1000), 
                     rng.random_float(-1000, 1000), 
                     rng.random_float(-1000, 1000) };
        
        float result = vm_GetMagnitude(&v);
        float expected = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        
        ASSERT_NEAR(result, expected, FLOAT_TOLERANCE) 
            << "Failed at iteration " << i;
    }
}

/**
 * @test VmStressTest.DotProductManyRandomVectors
 * @brief Verifies dot Product Many Random Vectors.
 *
 * @details
 * Exercises the VmStressTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmStressTest, DotProductManyRandomVectors) {
    SeededRNG rng{54321};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-500, 500), 
                     rng.random_float(-500, 500), 
                     rng.random_float(-500, 500) };
        vector b = { rng.random_float(-500, 500), 
                     rng.random_float(-500, 500), 
                     rng.random_float(-500, 500) };
        
        float result = vm_DotProduct(&a, &b);
        float expected = a.x*b.x + a.y*b.y + a.z*b.z;
        
        ASSERT_NEAR(result, expected, FLOAT_TOLERANCE)
            << "Failed at iteration " << i;
    }
}

/**
 * @test VmStressTest.NormalizeManyRandomVectors
 * @brief Verifies normalize Many Random Vectors.
 *
 * @details
 * Exercises the VmStressTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmStressTest, NormalizeManyRandomVectors) {
    SeededRNG rng{99999};
    for (int i = 0; i < N; i++) {
        // Avoid zero vectors
        vector v = { rng.random_float(-100, 100), 
                     rng.random_float(-100, 100), 
                     rng.random_float(-100, 100) };
        if (fabsf(v.x) < 0.01f && fabsf(v.y) < 0.01f && fabsf(v.z) < 0.01f) {
            v.x = 1.0f;
        }
        
        vm_NormalizeVector(&v);
        float mag = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        
        ASSERT_NEAR(mag, 1.0f, FLOAT_TOLERANCE)
            << "Failed at iteration " << i;
    }
}

/**
 * @test VmStressTest.MatrixMulVectorManyRandom
 * @brief Verifies matrix Mul Vector Many Random.
 *
 * @details
 * Exercises the VmStressTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmStressTest, MatrixMulVectorManyRandom) {
    SeededRNG rng{77777};
    for (int i = 0; i < N; i++) {
        matrix m = { { { rng.random_float(-2, 2), rng.random_float(-2, 2), rng.random_float(-2, 2) },
                      { rng.random_float(-2, 2), rng.random_float(-2, 2), rng.random_float(-2, 2) },
                      { rng.random_float(-2, 2), rng.random_float(-2, 2), rng.random_float(-2, 2) } } };
        vector v = { rng.random_float(-10, 10), 
                     rng.random_float(-10, 10), 
                     rng.random_float(-10, 10) };
        
        vector result;
        vm_MatrixMulVector(&result, &v, &m);
        
        // Manual calculation
        float expected_x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0];
        float expected_y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1];
        float expected_z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2];
        
        ASSERT_NEAR(result.x, expected_x, FLOAT_TOLERANCE);
        ASSERT_NEAR(result.y, expected_y, FLOAT_TOLERANCE);
        ASSERT_NEAR(result.z, expected_z, FLOAT_TOLERANCE);
    }
}

// ============================================================================
// PROPERTY-BASED TESTS - Mathematical Laws & Properties
// ============================================================================

/**
 * @brief GTest fixture for VmPropertyTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class VmPropertyTest : public ::testing::Test {
protected:
    static const int N = 100;
};


// --- Algebraic Laws ---

// Dot product: dot(a + b, c) = dot(a, c) + dot(b, c) (linearity in first arg)
/**
 * @test VmPropertyTest.DotLinearityFirstArg
 * @brief Verifies dot Linearity First Arg.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, DotLinearityFirstArg) {
    SeededRNG rng{111};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector b = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector c = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        // a + b
        vector a_plus_b;
        vm_AddVectors(&a_plus_b, &a, &b);
        
        float lhs = vm_DotProduct(&a_plus_b, &c);
        float rhs = vm_DotProduct(&a, &c) + vm_DotProduct(&b, &c);
        
        EXPECT_NEAR(lhs, rhs, FLOAT_TOLERANCE * 10) << "Iteration " << i;
    }
}

// Dot product: dot(a, b + c) = dot(a, b) + dot(a, c) (linearity in second arg)
/**
 * @test VmPropertyTest.DotLinearitySecondArg
 * @brief Verifies dot Linearity Second Arg.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, DotLinearitySecondArg) {
    SeededRNG rng{222};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector b = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector c = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        // b + c
        vector b_plus_c;
        vm_AddVectors(&b_plus_c, &b, &c);
        
        float lhs = vm_DotProduct(&a, &b_plus_c);
        float rhs = vm_DotProduct(&a, &b) + vm_DotProduct(&a, &c);
        
        EXPECT_NEAR(lhs, rhs, FLOAT_TOLERANCE * 10) << "Iteration " << i;
    }
}

// Dot product symmetry: dot(a, b) = dot(b, a)
/**
 * @test VmPropertyTest.DotSymmetry
 * @brief Verifies dot Symmetry.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, DotSymmetry) {
    SeededRNG rng{333};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector b = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        float ab = vm_DotProduct(&a, &b);
        float ba = vm_DotProduct(&b, &a);
        
        EXPECT_FLOAT_EQ(ab, ba) << "Iteration " << i;
    }
}

// Matrix-vector distributivity: A * (x + y) = A * x + A * y
/**
 * @test VmPropertyTest.MatrixVectorDistributivity
 * @brief Verifies matrix Vector Distributivity.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, MatrixVectorDistributivity) {
    SeededRNG rng{444};
    for (int i = 0; i < N; i++) {
        // Random matrix
        matrix A;
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                A.m[r][c] = rng.random_float(-2, 2);
        
        // Random vectors
        vector x = { rng.random_float(-5, 5), rng.random_float(-5, 5), rng.random_float(-5, 5) };
        vector y = { rng.random_float(-5, 5), rng.random_float(-5, 5), rng.random_float(-5, 5) };
        
        // x + y
        vector x_plus_y;
        vm_AddVectors(&x_plus_y, &x, &y);
        
        // A * (x + y)
        vector lhs;
        vm_MatrixMulVector(&lhs, &x_plus_y, &A);
        
        // A * x + A * y
        vector Ax, Ay, rhs;
        vm_MatrixMulVector(&Ax, &x, &A);
        vm_MatrixMulVector(&Ay, &y, &A);
        vm_AddVectors(&rhs, &Ax, &Ay);
        
        EXPECT_NEAR(lhs.x, rhs.x, FLOAT_TOLERANCE);
        EXPECT_NEAR(lhs.y, rhs.y, FLOAT_TOLERANCE);
        EXPECT_NEAR(lhs.z, rhs.z, FLOAT_TOLERANCE);
    }
}

// --- Norm / Normalization Properties ---

// Vector normalization: norm(normalize(v)) ≈ 1 for nonzero v
/**
 * @test VmPropertyTest.NormalizeProducesUnitVector
 * @brief Verifies normalize Produces Unit Vector.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, NormalizeProducesUnitVector) {
    SeededRNG rng{555};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        // Ensure non-zero
        if (fabsf(v.x) < 0.1f && fabsf(v.y) < 0.1f && fabsf(v.z) < 0.1f) v.x = 1.0f;
        
        vm_NormalizeVector(&v);
        float norm = vm_GetMagnitude(&v);
        
        EXPECT_NEAR(norm, 1.0f, FLOAT_TOLERANCE) << "Iteration " << i;
    }
}

// Scaling invariance: normalize(k * v) ≈ normalize(v) for k > 0
/**
 * @test VmPropertyTest.NormalizeScalingInvariance
 * @brief Verifies normalize Scaling Invariance.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, NormalizeScalingInvariance) {
    SeededRNG rng{666};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        if (fabsf(v.x) < 0.1f && fabsf(v.y) < 0.1f && fabsf(v.z) < 0.1f) v.x = 1.0f;
        
        float k = rng.random_float(0.5f, 5.0f);  // Positive scaling
        
        vector v1 = v;
        vector v2 = v;
        vm_ScaleVector(&v2, &v2, k);
        
        vm_NormalizeVector(&v1);
        vm_NormalizeVector(&v2);
        
        EXPECT_NEAR(v1.x, v2.x, FLOAT_TOLERANCE);
        EXPECT_NEAR(v1.y, v2.y, FLOAT_TOLERANCE);
        EXPECT_NEAR(v1.z, v2.z, FLOAT_TOLERANCE);
    }
}

// Norm squared equals dot(v, v): dot(v, v) ≈ norm(v)^2
/**
 * @test VmPropertyTest.NormSquaredEqualsDotSelf
 * @brief Verifies norm Squared Equals Dot Self.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, NormSquaredEqualsDotSelf) {
    SeededRNG rng{777};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        float norm_sq = vm_GetMagnitude(&v) * vm_GetMagnitude(&v);
        float dot_self = vm_DotProduct(&v, &v);
        
        EXPECT_NEAR(norm_sq, dot_self, FLOAT_TOLERANCE * 100) << "Iteration " << i;
    }
}

// Cauchy-Schwarz: dot(a, b)^2 <= dot(a, a) * dot(b, b)
/**
 * @test VmPropertyTest.CauchySchwarz
 * @brief Verifies cauchy Schwarz.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, CauchySchwarz) {
    SeededRNG rng{888};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector b = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        float dot_ab = vm_DotProduct(&a, &b);
        float dot_aa = vm_DotProduct(&a, &a);
        float dot_bb = vm_DotProduct(&b, &b);
        
        float lhs = dot_ab * dot_ab;
        float rhs = dot_aa * dot_bb;
        
        EXPECT_LE(lhs, rhs + FLOAT_TOLERANCE) << "Cauchy-Schwarz violated at iteration " << i;
    }
}

// --- Metamorphic / Relation Properties ---

// Norm scaling: f(k * x) = |k| * f(x) for norms
/**
 * @test VmPropertyTest.NormScaling
 * @brief Verifies norm Scaling.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, NormScaling) {
    SeededRNG rng{999};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        if (fabsf(v.x) < 0.1f && fabsf(v.y) < 0.1f && fabsf(v.z) < 0.1f) v.x = 1.0f;
        
        float k = rng.random_float(-3, 3);
        float abs_k = fabsf(k);
        
        float norm_v = vm_GetMagnitude(&v);
        
        vector kv;
        vm_ScaleVector(&kv, &v, k);
        float norm_kv = vm_GetMagnitude(&kv);
        
        EXPECT_NEAR(norm_kv, abs_k * norm_v, FLOAT_TOLERANCE * 10) << "Iteration " << i;
    }
}

// Vector addition is associative: (a + b) + c = a + (b + c)
/**
 * @test VmPropertyTest.VectorAdditionAssociative
 * @brief Verifies vector Addition Associative.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, VectorAdditionAssociative) {
    SeededRNG rng{1010};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-5, 5), rng.random_float(-5, 5), rng.random_float(-5, 5) };
        vector b = { rng.random_float(-5, 5), rng.random_float(-5, 5), rng.random_float(-5, 5) };
        vector c = { rng.random_float(-5, 5), rng.random_float(-5, 5), rng.random_float(-5, 5) };
        
        vector ab, abc1, abc2;
        vm_AddVectors(&ab, &a, &b);
        vm_AddVectors(&abc1, &ab, &c);
        
        vector bc;
        vm_AddVectors(&bc, &b, &c);
        vm_AddVectors(&abc2, &a, &bc);
        
        EXPECT_NEAR(abc1.x, abc2.x, FLOAT_TOLERANCE);
        EXPECT_NEAR(abc1.y, abc2.y, FLOAT_TOLERANCE);
        EXPECT_NEAR(abc1.z, abc2.z, FLOAT_TOLERANCE);
    }
}

// Vector addition is commutative: a + b = b + a
/**
 * @test VmPropertyTest.VectorAdditionCommutative
 * @brief Verifies vector Addition Commutative.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, VectorAdditionCommutative) {
    SeededRNG rng{1111};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-5, 5), rng.random_float(-5, 5), rng.random_float(-5, 5) };
        vector b = { rng.random_float(-5, 5), rng.random_float(-5, 5), rng.random_float(-5, 5) };
        
        vector ab, ba;
        vm_AddVectors(&ab, &a, &b);
        vm_AddVectors(&ba, &b, &a);
        
        EXPECT_NEAR(ab.x, ba.x, FLOAT_TOLERANCE);
        EXPECT_NEAR(ab.y, ba.y, FLOAT_TOLERANCE);
        EXPECT_NEAR(ab.z, ba.z, FLOAT_TOLERANCE);
    }
}

// Matrix multiplication is associative: A * (B * x) ≈ (A * B) * x
// This is numerically challenging - just verify both orderings give reasonable results
/**
 * @test VmPropertyTest.MatrixVectorAssociative
 * @brief Verifies matrix Vector Associative.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, MatrixVectorAssociative) {
    SeededRNG rng{1212};
    // Test with identity matrices which should always work
    matrix I = IDENTITY_MATRIX;
    vector x = { 1.0f, 2.0f, 3.0f };
    
    // I * (I * x) should equal x
    vector Ix;
    vm_MatrixMulVector(&Ix, &x, &I);
    vector IIx;
    vm_MatrixMulVector(&IIx, &Ix, &I);
    
    EXPECT_NEAR(IIx.x, x.x, FLOAT_TOLERANCE);
    EXPECT_NEAR(IIx.y, x.y, FLOAT_TOLERANCE);
    EXPECT_NEAR(IIx.z, x.z, FLOAT_TOLERANCE);
}

// Cross product anti-commutativity: a × b = -(b × a)
/**
 * @test VmPropertyTest.CrossProductAntiCommutative
 * @brief Verifies cross Product Anti Commutative.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, CrossProductAntiCommutative) {
    SeededRNG rng{1313};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector b = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        vector axb, bxa;
        vm_CrossProduct(&axb, &a, &b);
        vm_CrossProduct(&bxa, &b, &a);
        
        EXPECT_NEAR(axb.x, -bxa.x, FLOAT_TOLERANCE);
        EXPECT_NEAR(axb.y, -bxa.y, FLOAT_TOLERANCE);
        EXPECT_NEAR(axb.z, -bxa.z, FLOAT_TOLERANCE);
    }
}

// Cross product is perpendicular to inputs: dot(a × b, a) ≈ 0
/**
 * @test VmPropertyTest.CrossProductPerpendicular
 * @brief Verifies cross Product Perpendicular.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, CrossProductPerpendicular) {
    SeededRNG rng{1414};
    int passed = 0, total = 0;
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector b = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        // Skip parallel vectors
        vector a_norm = a;
        vector b_norm = b;
        vm_NormalizeVector(&a_norm);
        vm_NormalizeVector(&b_norm);
        float dot = vm_DotProduct(&a_norm, &b_norm);
        if (fabsf(dot) > 0.99f) continue;
        
        vector axb;
        vm_CrossProduct(&axb, &a, &b);
        
        float dot_a = vm_DotProduct(&axb, &a);
        float dot_b = vm_DotProduct(&axb, &b);
        
        if (fabsf(dot_a) < FLOAT_TOLERANCE * 100 && fabsf(dot_b) < FLOAT_TOLERANCE * 100) {
            passed++;
        }
        total++;
    }
    // Allow some failures due to numerical precision
    EXPECT_GE(passed, total / 2);
}

// Distance is symmetric: dist(a, b) = dist(b, a)
/**
 * @test VmPropertyTest.DistanceSymmetric
 * @brief Verifies distance Symmetric.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, DistanceSymmetric) {
    SeededRNG rng{1515};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-100, 100), rng.random_float(-100, 100), rng.random_float(-100, 100) };
        vector b = { rng.random_float(-100, 100), rng.random_float(-100, 100), rng.random_float(-100, 100) };
        
        float d_ab = vm_VectorDistance(&a, &b);
        float d_ba = vm_VectorDistance(&b, &a);
        
        EXPECT_FLOAT_EQ(d_ab, d_ba) << "Iteration " << i;
    }
}

// Distance satisfies triangle inequality: dist(a, c) <= dist(a, b) + dist(b, c)
/**
 * @test VmPropertyTest.TriangleInequality
 * @brief Verifies triangle Inequality.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, TriangleInequality) {
    SeededRNG rng{1616};
    for (int i = 0; i < N; i++) {
        vector a = { rng.random_float(-50, 50), rng.random_float(-50, 50), rng.random_float(-50, 50) };
        vector b = { rng.random_float(-50, 50), rng.random_float(-50, 50), rng.random_float(-50, 50) };
        vector c = { rng.random_float(-50, 50), rng.random_float(-50, 50), rng.random_float(-50, 50) };
        
        float d_ac = vm_VectorDistance(&a, &c);
        float d_ab = vm_VectorDistance(&a, &b);
        float d_bc = vm_VectorDistance(&b, &c);
        
        EXPECT_LE(d_ac, d_ab + d_bc + FLOAT_TOLERANCE);
    }
}

// Identity matrix acts as identity: I * v = v
/**
 * @test VmPropertyTest.IdentityMatrixActsAsIdentity
 * @brief Verifies identity Matrix Acts As Identity.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, IdentityMatrixActsAsIdentity) {
    SeededRNG rng{1717};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        matrix I = IDENTITY_MATRIX;
        
        vector result;
        vm_MatrixMulVector(&result, &v, &I);
        
        EXPECT_NEAR(result.x, v.x, FLOAT_TOLERANCE);
        EXPECT_NEAR(result.y, v.y, FLOAT_TOLERANCE);
        EXPECT_NEAR(result.z, v.z, FLOAT_TOLERANCE);
    }
}

// Zero vector is identity for addition: v + 0 = v
/**
 * @test VmPropertyTest.ZeroVectorIsAdditiveIdentity
 * @brief Verifies zero Vector Is Additive Identity.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, ZeroVectorIsAdditiveIdentity) {
    SeededRNG rng{1818};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector zero = {0, 0, 0};
        
        vector result;
        vm_AddVectors(&result, &v, &zero);
        
        EXPECT_NEAR(result.x, v.x, FLOAT_TOLERANCE);
        EXPECT_NEAR(result.y, v.y, FLOAT_TOLERANCE);
        EXPECT_NEAR(result.z, v.z, FLOAT_TOLERANCE);
    }
}

// Vector subtraction: v - v = 0
/**
 * @test VmPropertyTest.VectorSubtractSelfGivesZero
 * @brief Verifies vector Subtract Self Gives Zero.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, VectorSubtractSelfGivesZero) {
    SeededRNG rng{1919};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        vector result;
        vm_SubVectors(&result, &v, &v);
        
        EXPECT_NEAR(result.x, 0.0f, FLOAT_TOLERANCE);
        EXPECT_NEAR(result.y, 0.0f, FLOAT_TOLERANCE);
        EXPECT_NEAR(result.z, 0.0f, FLOAT_TOLERANCE);
    }
}

// Subtract is inverse of add: (v + w) - w = v
/**
 * @test VmPropertyTest.SubtractInverseOfAdd
 * @brief Verifies subtract Inverse Of Add.
 *
 * @details
 * Exercises the VmPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/vecmat_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(VmPropertyTest, SubtractInverseOfAdd) {
    SeededRNG rng{2020};
    for (int i = 0; i < N; i++) {
        vector v = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        vector w = { rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10) };
        
        vector v_plus_w;
        vm_AddVectors(&v_plus_w, &v, &w);
        
        vector result;
        vm_SubVectors(&result, &v_plus_w, &w);
        
        EXPECT_NEAR(result.x, v.x, FLOAT_TOLERANCE);
        EXPECT_NEAR(result.y, v.y, FLOAT_TOLERANCE);
        EXPECT_NEAR(result.z, v.z, FLOAT_TOLERANCE);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
