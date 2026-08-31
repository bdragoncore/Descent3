/*
 * D3 Coverage Tests - Test Data Generators
 * 
 * Provides deterministic random data generation for coverage tests.
 * Uses seeded RNG for reproducibility.
 */

#ifndef TEST_DATA_GENERATORS_H
#define TEST_DATA_GENERATORS_H

#include "vecmat.h"
#include "fix.h"
#include <cstdint>
#include <cmath>
#include <random>
#include <vector>

// Default seed for reproducibility
static constexpr uint32_t DEFAULT_SEED = 12345;

// Validity bounds
static constexpr float MIN_COMPONENT = -10000.0f;
static constexpr float MAX_COMPONENT = 10000.0f;
static constexpr float SMALL_FLOAT = 1e-6f;

class SeededRNG {
public:
    explicit SeededRNG(uint32_t seed = DEFAULT_SEED) : rng_(seed) {}
    
    uint32_t seed() const { return seed_; }
    void set_seed(uint32_t seed) { 
        seed_ = seed; 
        rng_.seed(seed); 
    }
    
    // Random float in range [min, max]
    float random_float(float min = -1000.0f, float max = 1000.0f) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng_);
    }
    
    // Random int in range [min, max]
    int random_int(int min = -1000, int max = 1000) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng_);
    }
    
    // Random bool
    bool random_bool() {
        return rng_() % 2 == 0;
    }

private:
    uint32_t seed_;
    std::mt19937 rng_;
};

// Vector generator
class VectorGenerator {
public:
    explicit VectorGenerator(uint32_t seed = DEFAULT_SEED) : rng_(seed) {}
    
    uint32_t seed() const { return rng_.seed(); }
    
    // Random vector with components in range [min, max]
    vector random(float min = -1000.0f, float max = 1000.0f) {
        return { rng_.random_float(min, max), 
                 rng_.random_float(min, max), 
                 rng_.random_float(min, max) };
    }
    
    // Zero vector (edge case)
    vector zero() { return {0.0f, 0.0f, 0.0f}; }
    
    // Unit vector (edge case)
    vector unit() { return {1.0f, 0.0f, 0.0f}; }
    
    // Near-zero vector (edge case)
    vector near_zero() { 
        return { SMALL_FLOAT, SMALL_FLOAT, SMALL_FLOAT }; 
    }
    
    // Boundary values (edge case)
    vector boundary_max() { 
        return { MAX_COMPONENT, MAX_COMPONENT, MAX_COMPONENT }; 
    }
    
    vector boundary_min() { 
        return { MIN_COMPONENT, MIN_COMPONENT, MIN_COMPONENT }; 
    }
    
    // Random normalized vector
    vector normalized() {
        vector v = random();
        float mag = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        if (mag > SMALL_FLOAT) {
            v.x /= mag; v.y /= mag; v.z /= mag;
        }
        return v;
    }
    
private:
    SeededRNG rng_;
};

// Matrix generator
class MatrixGenerator {
public:
    explicit MatrixGenerator(uint32_t seed = DEFAULT_SEED) : rng_(seed), vgen_(seed) {}
    
    // Random matrix
    matrix random() {
        matrix m;
        m.rvec = vgen_.random();
        m.uvec = vgen_.random();
        m.fvec = vgen_.random();
        return m;
    }
    
    // Identity matrix
    matrix identity() {
        return IDENTITY_MATRIX;
    }
    
    // Random orthogonal matrix (rotation)
    matrix rotation() {
        // Create a random rotation matrix using Euler angles
        matrix m;
        float p = rng_.random_float(0, 360) * (M_PI / 180.0f);
        float h = rng_.random_float(0, 360) * (M_PI / 180.0f);
        float b = rng_.random_float(0, 360) * (M_PI / 180.0f);
        vm_AnglesToMatrix(&m, (angle)(p * 65535.0f / (2.0f * M_PI)),
                                 (angle)(h * 65535.0f / (2.0f * M_PI)),
                                 (angle)(b * 65535.0f / (2.0f * M_PI)));
        return m;
    }
    
private:
    SeededRNG rng_;
    VectorGenerator vgen_;
};

// Angle generator
class AngleGenerator {
public:
    explicit AngleGenerator(uint32_t seed = DEFAULT_SEED) : rng_(seed) {}
    
    // Random angle
    angle random() {
        return (angle)rng_.random_int(0, 65535);
    }
    
    // Zero angle
    angle zero() { return 0; }
    
    // Common angles
    angle deg_0() { return 0; }
    angle deg_90() { return (angle)(16384); }  // 90 degrees in angle units
    angle deg_180() { return (angle)(32768); }
    angle deg_270() { return (angle)(49152); }
    angle deg_360() { return (angle)(65535); }

private:
    SeededRNG rng_;
};

// Angvec generator
class AngvecGenerator {
public:
    explicit AngvecGenerator(uint32_t seed = DEFAULT_SEED) : agen_(seed) {}
    
    angvec random() {
        return { agen_.random(), agen_.random(), agen_.random() };
    }
    
    angvec zero() {
        return { 0, 0, 0 };
    }

private:
    AngleGenerator agen_;
};

// Fixed-point generator
class FixGenerator {
public:
    explicit FixGenerator(uint32_t seed = DEFAULT_SEED) : rng_(seed) {}
    
    // Random fix value
    fix random() {
        return (fix)rng_.random_int(-100000, 100000);
    }
    
    // Zero
    fix zero() { return 0; }
    
    // One
    fix one() { return F1_0; }
    
    // Small value
    fix small() { return 100; }

private:
    SeededRNG rng_;
};

#endif // TEST_DATA_GENERATORS_H
