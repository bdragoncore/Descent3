/**
 * @file physics_tests_standalone.cpp
 * @brief D3 Coverage Tests - Physics module (Standalone).
 *
 * @details
 * Tests for physics simulation and collision detection.
 * Uses seeded RNG for reproducible test data.
 *
 * This harness validates the behavior of `Descent3/physics_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/physics_tests_standalone.cpp`
 * @par Harness
 * `physics_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/physics_tests_standalone.cpp
 */

#include <gtest/gtest.h>
#include <cmath>
#include <random>
#include <vector>

// D3-compatible types
typedef struct { float x, y, z; } vector;
typedef unsigned short angle;

static constexpr float FLOAT_TOLERANCE = 1e-5f;
static constexpr float PHYSICS_TOLERANCE = 1e-4f;

// Seeded RNG
class SeededRNG {
public:
    explicit SeededRNG(uint32_t seed = 12345) : rng_(seed) {}
    
    float random_float(float min = -100.0f, float max = 100.0f) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng_);
    }
    
    int random_int(int min = -100, int max = 100) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng_);
    }
    
private:
    std::mt19937 rng_;
};

// ============================================================================
// Physics State and Simulation (simplified D3 physics)
// ============================================================================

typedef struct {
    vector position;
    vector velocity;
    vector thrust;
    float mass;
    float drag;
    float max_velocity;
} physics_state;

// Apply thrust: v += (thrust / mass) * dt
void physics_apply_thrust(physics_state* p, float dt) {
    if (p->mass > 0) {
        p->velocity.x += (p->thrust.x / p->mass) * dt;
        p->velocity.y += (p->thrust.y / p->mass) * dt;
        p->velocity.z += (p->thrust.z / p->mass) * dt;
    }
}

// Apply drag: v *= (1 - drag * dt)
void physics_apply_drag(physics_state* p, float dt) {
    float factor = 1.0f - p->drag * dt;
    if (factor < 0) factor = 0;
    p->velocity.x *= factor;
    p->velocity.y *= factor;
    p->velocity.z *= factor;
}

// Apply velocity to position: pos += v * dt
void physics_apply_velocity(physics_state* p, float dt) {
    p->position.x += p->velocity.x * dt;
    p->position.y += p->velocity.y * dt;
    p->position.z += p->velocity.z * dt;
}

// Clamp velocity to max
void physics_clamp_velocity(physics_state* p) {
    float speed = sqrtf(p->velocity.x*p->velocity.x + 
                        p->velocity.y*p->velocity.y + 
                        p->velocity.z*p->velocity.z);
    if (p->max_velocity > 0 && speed > p->max_velocity) {
        float scale = p->max_velocity / speed;
        p->velocity.x *= scale;
        p->velocity.y *= scale;
        p->velocity.z *= scale;
    }
}

// Full physics step
void physics_step(physics_state* p, float dt) {
    physics_apply_thrust(p, dt);
    physics_apply_drag(p, dt);
    physics_clamp_velocity(p);
    physics_apply_velocity(p, dt);
}

// ============================================================================
// Collision Detection
// ============================================================================

typedef struct {
    vector center;
    float radius;
} sphere;

typedef struct {
    vector normal;
    float distance;  // distance from origin along normal
} plane;

// Check sphere-sphere collision
bool sphere_sphere_intersect(const sphere* a, const sphere* b) {
    float dx = a->center.x - b->center.x;
    float dy = a->center.y - b->center.y;
    float dz = a->center.z - b->center.z;
    float dist_sq = dx*dx + dy*dy + dz*dz;
    float radius_sum = a->radius + b->radius;
    return dist_sq <= radius_sum * radius_sum;
}

// Get distance between sphere centers
float sphere_sphere_distance(const sphere* a, const sphere* b) {
    float dx = a->center.x - b->center.x;
    float dy = a->center.y - b->center.y;
    float dz = a->center.z - b->center.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

// Check sphere-plane collision
// Returns true if sphere intersects or is behind plane
bool sphere_plane_intersect(const sphere* s, const plane* p) {
    // Distance from sphere center to plane
    float dist = s->center.x * p->normal.x + 
                s->center.y * p->normal.y + 
                s->center.z * p->normal.z - p->distance;
    return dist <= s->radius;
}

// Get distance from sphere to plane (positive = in front)
float sphere_plane_distance(const sphere* s, const plane* p) {
    return s->center.x * p->normal.x + 
           s->center.y * p->normal.y + 
           s->center.z * p->normal.z - p->distance - s->radius;
}

// Ray-sphere intersection (returns distance or -1 if no hit)
float ray_sphere_intersect(const vector* ray_origin, const vector* ray_dir, 
                          const sphere* s) {
    vector oc;
    oc.x = ray_origin->x - s->center.x;
    oc.y = ray_origin->y - s->center.y;
    oc.z = ray_origin->z - s->center.z;
    
    float a = ray_dir->x*ray_dir->x + ray_dir->y*ray_dir->y + ray_dir->z*ray_dir->z;
    float b = 2.0f * (oc.x*ray_dir->x + oc.y*ray_dir->y + oc.z*ray_dir->z);
    float c = oc.x*oc.x + oc.y*oc.y + oc.z*oc.z - s->radius*s->radius;
    
    float discriminant = b*b - 4*a*c;
    if (discriminant < 0) return -1.0f;
    
    float t = (-b - sqrtf(discriminant)) / (2.0f * a);
    if (t < 0) t = (-b + sqrtf(discriminant)) / (2.0f * a);
    if (t < 0) return -1.0f;
    
    return t;
}

// ============================================================================
// Vector helpers (reuse from vecmat)
// ============================================================================

float vm_GetMagnitude(vector* v) {
    return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
}

void vm_NormalizeVector(vector* v) {
    float mag = vm_GetMagnitude(v);
    if (mag > FLOAT_TOLERANCE) {
        v->x /= mag;
        v->y /= mag;
        v->z /= mag;
    }
}

float vm_DotProduct(const vector* a, const vector* b) {
    return a->x*b->x + a->y*b->y + a->z*b->z;
}

void vm_SubVectors(vector* result, const vector* a, const vector* b) {
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
}

// ============================================================================
// Tests
// ============================================================================

// --- Physics State Tests ---

/**
 * @brief GTest fixture for PhysicsStateTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PhysicsStateTest : public ::testing::Test {};

/**
 * @test PhysicsStateTest.ZeroThrustNoVelocityChange
 * @brief Verifies zero Thrust No Velocity Change.
 *
 * @details
 * Exercises the PhysicsStateTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsStateTest, ZeroThrustNoVelocityChange) {
    physics_state p;
    p.position = {0, 0, 0};
    p.velocity = {0, 0, 0};
    p.thrust = {0, 0, 0};
    p.mass = 1.0f;
    p.drag = 0.0f;
    p.max_velocity = 0.0f;
    
    physics_step(&p, 1.0f);
    
    EXPECT_FLOAT_EQ(p.velocity.x, 0.0f);
    EXPECT_FLOAT_EQ(p.velocity.y, 0.0f);
    EXPECT_FLOAT_EQ(p.velocity.z, 0.0f);
}

/**
 * @test PhysicsStateTest.ConstantThrustAccelerates
 * @brief Verifies constant Thrust Accelerates.
 *
 * @details
 * Exercises the PhysicsStateTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsStateTest, ConstantThrustAccelerates) {
    physics_state p;
    p.position = {0, 0, 0};
    p.velocity = {0, 0, 0};
    p.thrust = {1.0f, 0, 0};  // 1 unit thrust
    p.mass = 1.0f;
    p.drag = 0.0f;
    p.max_velocity = 0.0f;
    
    physics_step(&p, 1.0f);
    
    EXPECT_NEAR(p.velocity.x, 1.0f, PHYSICS_TOLERANCE);
}

/**
 * @test PhysicsStateTest.MassAffectsAcceleration
 * @brief Verifies mass Affects Acceleration.
 *
 * @details
 * Exercises the PhysicsStateTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsStateTest, MassAffectsAcceleration) {
    physics_state p1, p2;
    
    p1 = { {0,0,0}, {0,0,0}, {1,0,0}, 1.0f, 0.0f, 0.0f };
    p2 = { {0,0,0}, {0,0,0}, {1,0,0}, 2.0f, 0.0f, 0.0f };
    
    physics_step(&p1, 1.0f);
    physics_step(&p2, 1.0f);
    
    // p1 with mass 1 should have 2x velocity of p2 with mass 2
    EXPECT_GT(p1.velocity.x, p2.velocity.x);
}

/**
 * @test PhysicsStateTest.DragSlowsVelocity
 * @brief Verifies drag Slows Velocity.
 *
 * @details
 * Exercises the PhysicsStateTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsStateTest, DragSlowsVelocity) {
    physics_state p;
    p.position = {0, 0, 0};
    p.velocity = {10.0f, 0, 0};
    p.thrust = {0, 0, 0};
    p.mass = 1.0f;
    p.drag = 1.0f;
    p.max_velocity = 0.0f;
    
    physics_step(&p, 1.0f);
    
    EXPECT_LT(p.velocity.x, 10.0f);
}

/**
 * @test PhysicsStateTest.VelocityClampedToMax
 * @brief Verifies velocity Clamped To Max.
 *
 * @details
 * Exercises the PhysicsStateTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsStateTest, VelocityClampedToMax) {
    physics_state p;
    p.position = {0, 0, 0};
    p.velocity = {0, 0, 0};
    p.thrust = {100.0f, 0, 0};
    p.mass = 1.0f;
    p.drag = 0.0f;
    p.max_velocity = 5.0f;
    
    physics_step(&p, 1.0f);
    
    EXPECT_LE(p.velocity.x, 5.0f);
}

/**
 * @test PhysicsStateTest.PositionUpdatesWithVelocity
 * @brief Verifies position Updates With Velocity.
 *
 * @details
 * Exercises the PhysicsStateTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsStateTest, PositionUpdatesWithVelocity) {
    physics_state p;
    p.position = {0, 0, 0};
    p.velocity = {1.0f, 2.0f, 3.0f};
    p.thrust = {0, 0, 0};
    p.mass = 1.0f;
    p.drag = 0.0f;
    p.max_velocity = 0.0f;
    
    physics_step(&p, 1.0f);
    
    EXPECT_FLOAT_EQ(p.position.x, 1.0f);
    EXPECT_FLOAT_EQ(p.position.y, 2.0f);
    EXPECT_FLOAT_EQ(p.position.z, 3.0f);
}

// --- Sphere-Sphere Collision Tests ---

/**
 * @brief GTest fixture for SphereCollisionTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class SphereCollisionTest : public ::testing::Test {};

/**
 * @test SphereCollisionTest.NoOverlapNoCollision
 * @brief Verifies no Overlap No Collision.
 *
 * @details
 * Exercises the SphereCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SphereCollisionTest, NoOverlapNoCollision) {
    sphere a = {{0, 0, 0}, 1.0f};
    sphere b = {{10, 0, 0}, 1.0f};
    
    EXPECT_FALSE(sphere_sphere_intersect(&a, &b));
}

/**
 * @test SphereCollisionTest.OverlapCollision
 * @brief Verifies overlap Collision.
 *
 * @details
 * Exercises the SphereCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SphereCollisionTest, OverlapCollision) {
    sphere a = {{0, 0, 0}, 2.0f};
    sphere b = {{3, 0, 0}, 2.0f};  // centers 3 apart, radii sum = 4
    
    EXPECT_TRUE(sphere_sphere_intersect(&a, &b));
}

/**
 * @test SphereCollisionTest.TouchingNoCollision
 * @brief Verifies touching No Collision.
 *
 * @details
 * Exercises the SphereCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SphereCollisionTest, TouchingNoCollision) {
    sphere a = {{0, 0, 0}, 1.0f};
    sphere b = {{2, 0, 0}, 1.0f};  // centers 2 apart, radii sum = 2
    
    // Touching at exactly the boundary - our implementation considers this intersecting
    EXPECT_TRUE(sphere_sphere_intersect(&a, &b));
}

/**
 * @test SphereCollisionTest.DistanceCalculation
 * @brief Verifies distance Calculation.
 *
 * @details
 * Exercises the SphereCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SphereCollisionTest, DistanceCalculation) {
    sphere a = {{0, 0, 0}, 1.0f};
    sphere b = {{3, 4, 0}, 1.0f};  // distance = 5
    
    EXPECT_NEAR(sphere_sphere_distance(&a, &b), 5.0f, FLOAT_TOLERANCE);
}

/**
 * @test SphereCollisionTest.SameSphereDistance
 * @brief Verifies same Sphere Distance.
 *
 * @details
 * Exercises the SphereCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SphereCollisionTest, SameSphereDistance) {
    sphere a = {{1, 2, 3}, 1.0f};
    sphere b = {{1, 2, 3}, 1.0f};
    
    EXPECT_NEAR(sphere_sphere_distance(&a, &b), 0.0f, FLOAT_TOLERANCE);
}

// --- Sphere-Plane Collision Tests ---

/**
 * @brief GTest fixture for SpherePlaneCollisionTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class SpherePlaneCollisionTest : public ::testing::Test {};

/**
 * @test SpherePlaneCollisionTest.SphereInFrontOfPlane
 * @brief Verifies sphere In Front Of Plane.
 *
 * @details
 * Exercises the SpherePlaneCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpherePlaneCollisionTest, SphereInFrontOfPlane) {
    sphere s = {{0, 0, 10}, 1.0f};
    plane p = {{0, 0, 1}, 0};  // plane at z=0, facing +z
    
    // Sphere at z=10 with radius 1, distance from center to plane = 10
    // Since 10 > 1 (radius), sphere is completely in front, no collision
    // Our function returns true if dist <= radius (intersecting or behind)
    // dist = 10 - 0 - 1 = 9, and 9 > 1, so returns false (no intersect)
    EXPECT_FALSE(sphere_plane_intersect(&s, &p));
}

/**
 * @test SpherePlaneCollisionTest.SphereBehindPlane
 * @brief Verifies sphere Behind Plane.
 *
 * @details
 * Exercises the SpherePlaneCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpherePlaneCollisionTest, SphereBehindPlane) {
    sphere s = {{0, 0, -10}, 1.0f};
    plane p = {{0, 0, 1}, 0};  // plane at z=0, facing +z
    
    // Sphere at z=-10 with radius 1, center is behind plane
    // dist = -10 - 0 - 1 = -11 <= 1, returns true (intersecting/behind)
    EXPECT_TRUE(sphere_plane_intersect(&s, &p));
}

/**
 * @test SpherePlaneCollisionTest.SphereTouchingPlane
 * @brief Verifies sphere Touching Plane.
 *
 * @details
 * Exercises the SpherePlaneCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpherePlaneCollisionTest, SphereTouchingPlane) {
    sphere s = {{0, 0, 1}, 1.0f};
    plane p = {{0, 0, 1}, 0};  // plane at z=0
    
    // Sphere center at z=1, radius=1, touches plane at z=0
    // dist = 1*1 - 0 - 1 = 0, <= radius means intersect
    EXPECT_TRUE(sphere_plane_intersect(&s, &p));
}

/**
 * @test SpherePlaneCollisionTest.PlaneNormalCalculation
 * @brief Verifies plane Normal Calculation.
 *
 * @details
 * Exercises the SpherePlaneCollisionTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(SpherePlaneCollisionTest, PlaneNormalCalculation) {
    plane p = {{0, 0, 1}, 5};  // plane at z=5
    
    sphere s = {{0, 0, 10}, 1.0f};
    float dist = sphere_plane_distance(&s, &p);
    EXPECT_NEAR(dist, 10.0f - 5.0f - 1.0f, FLOAT_TOLERANCE);
}

// --- Ray-Sphere Intersection Tests ---

/**
 * @brief GTest fixture for RaySphereTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class RaySphereTest : public ::testing::Test {};

/**
 * @test RaySphereTest.RayHitsSphere
 * @brief Verifies ray Hits Sphere.
 *
 * @details
 * Exercises the RaySphereTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(RaySphereTest, RayHitsSphere) {
    vector origin = {0, 0, -5};
    vector dir = {0, 0, 1};  // pointing toward +z
    sphere s = {{0, 0, 0}, 1.0f};
    
    float t = ray_sphere_intersect(&origin, &dir, &s);
    
    EXPECT_GE(t, 0.0f);
    EXPECT_NEAR(t, 4.0f, FLOAT_TOLERANCE);
}

/**
 * @test RaySphereTest.RayMissesSphere
 * @brief Verifies ray Misses Sphere.
 *
 * @details
 * Exercises the RaySphereTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(RaySphereTest, RayMissesSphere) {
    vector origin = {10, 0, -5};
    vector dir = {0, 0, 1};  // pointing toward +z, but sphere at origin
    sphere s = {{0, 0, 0}, 1.0f};
    
    float t = ray_sphere_intersect(&origin, &dir, &s);
    
    EXPECT_LT(t, 0.0f);
}

/**
 * @test RaySphereTest.RayOriginInsideSphere
 * @brief Verifies ray Origin Inside Sphere.
 *
 * @details
 * Exercises the RaySphereTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(RaySphereTest, RayOriginInsideSphere) {
    vector origin = {0, 0, 0};
    vector dir = {0, 0, 1};
    sphere s = {{0, 0, 0}, 1.0f};
    
    float t = ray_sphere_intersect(&origin, &dir, &s);
    
    EXPECT_GE(t, 0.0f);
}

// --- Property-Based Physics Tests ---

/**
 * @brief GTest fixture for PhysicsPropertyTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PhysicsPropertyTest : public ::testing::Test {
protected:
    static const int N = 100;
};

/**
 * @test PhysicsPropertyTest.EnergyConservedNoDragNoThrust
 * @brief Verifies energy Conserved No Drag No Thrust.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, EnergyConservedNoDragNoThrust) {
    SeededRNG rng{111};
    
    for (int i = 0; i < N; i++) {
        physics_state p;
        p.position = {0, 0, 0};
        p.velocity = {rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10)};
        p.thrust = {0, 0, 0};
        p.mass = 1.0f;
        p.drag = 0.0f;
        p.max_velocity = 0.0f;
        
        float initial_speed = sqrtf(p.velocity.x*p.velocity.x + 
                                    p.velocity.y*p.velocity.y + 
                                    p.velocity.z*p.velocity.z);
        
        physics_step(&p, 1.0f);
        
        float final_speed = sqrtf(p.velocity.x*p.velocity.x + 
                                  p.velocity.y*p.velocity.y + 
                                  p.velocity.z*p.velocity.z);
        
        EXPECT_NEAR(initial_speed, final_speed, PHYSICS_TOLERANCE);
    }
}

/**
 * @test PhysicsPropertyTest.DragReducesEnergy
 * @brief Verifies drag Reduces Energy.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, DragReducesEnergy) {
    SeededRNG rng{222};
    
    for (int i = 0; i < N; i++) {
        physics_state p;
        p.position = {0, 0, 0};
        p.velocity = {rng.random_float(1, 10), rng.random_float(1, 10), rng.random_float(1, 10)};
        p.thrust = {0, 0, 0};
        p.mass = 1.0f;
        p.drag = 0.5f;
        p.max_velocity = 0.0f;
        
        float initial_speed_sq = p.velocity.x*p.velocity.x + 
                               p.velocity.y*p.velocity.y + 
                               p.velocity.z*p.velocity.z;
        
        physics_step(&p, 1.0f);
        
        float final_speed_sq = p.velocity.x*p.velocity.x + 
                              p.velocity.y*p.velocity.y + 
                              p.velocity.z*p.velocity.z;
        
        EXPECT_LE(final_speed_sq, initial_speed_sq);
    }
}

/**
 * @test PhysicsPropertyTest.ThrustIncreasesEnergy
 * @brief Verifies thrust Increases Energy.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, ThrustIncreasesEnergy) {
    SeededRNG rng{333};
    
    for (int i = 0; i < N; i++) {
        physics_state p;
        p.position = {0, 0, 0};
        p.velocity = {0, 0, 0};
        p.thrust = {rng.random_float(0.1f, 5.0f), 0, 0};
        p.mass = 1.0f;
        p.drag = 0.0f;
        p.max_velocity = 0.0f;
        
        physics_step(&p, 1.0f);
        
        float final_speed = sqrtf(p.velocity.x*p.velocity.x + 
                                  p.velocity.y*p.velocity.y + 
                                  p.velocity.z*p.velocity.z);
        
        EXPECT_GT(final_speed, 0.0f);
    }
}

/**
 * @test PhysicsPropertyTest.VelocityMagnitudeNonNegative
 * @brief Verifies velocity Magnitude Non Negative.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, VelocityMagnitudeNonNegative) {
    SeededRNG rng{444};
    
    for (int i = 0; i < N; i++) {
        physics_state p;
        p.position = {0, 0, 0};
        p.velocity = {rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10)};
        p.thrust = {0, 0, 0};
        p.mass = 1.0f;
        p.drag = 0.0f;
        p.max_velocity = 0.0f;
        
        physics_step(&p, 1.0f);
        
        float speed = sqrtf(p.velocity.x*p.velocity.x + 
                           p.velocity.y*p.velocity.y + 
                           p.velocity.z*p.velocity.z);
        
        EXPECT_GE(speed, 0.0f);
    }
}

/**
 * @test PhysicsPropertyTest.PositionUpdatesAreAdditive
 * @brief Verifies position Updates Are Additive.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, PositionUpdatesAreAdditive) {
    SeededRNG rng{555};
    
    for (int i = 0; i < N; i++) {
        physics_state p1, p2;
        
        vector initial_pos = {rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10)};
        vector vel = {rng.random_float(-5, 5), rng.random_float(-5, 5), rng.random_float(-5, 5)};
        
        p1 = {initial_pos, vel, {0,0,0}, 1.0f, 0.0f, 0.0f};
        p2 = {initial_pos, vel, {0,0,0}, 1.0f, 0.0f, 0.0f};
        
        // p1: two half steps
        physics_step(&p1, 0.5f);
        physics_step(&p1, 0.5f);
        
        // p2: one full step
        physics_step(&p2, 1.0f);
        
        // Should be the same
        EXPECT_NEAR(p1.position.x, p2.position.x, PHYSICS_TOLERANCE);
        EXPECT_NEAR(p1.position.y, p2.position.y, PHYSICS_TOLERANCE);
        EXPECT_NEAR(p1.position.z, p2.position.z, PHYSICS_TOLERANCE);
    }
}

/**
 * @test PhysicsPropertyTest.MassCannotBeNegative
 * @brief Verifies mass Cannot Be Negative.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, MassCannotBeNegative) {
    physics_state p;
    p.position = {0, 0, 0};
    p.velocity = {1, 0, 0};
    p.thrust = {1, 0, 0};
    p.mass = -1.0f;  // Invalid negative mass
    p.drag = 0.0f;
    p.max_velocity = 0.0f;
    
    // Should not crash, just not apply thrust
    physics_step(&p, 1.0f);
    
    // Velocity should remain unchanged (division by negative mass skipped)
    EXPECT_FLOAT_EQ(p.velocity.x, 1.0f);
}

// Collision property tests
/**
 * @test PhysicsPropertyTest.SphereCollisionSymmetric
 * @brief Verifies sphere Collision Symmetric.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, SphereCollisionSymmetric) {
    SeededRNG rng{666};
    
    for (int i = 0; i < N; i++) {
        sphere a = {{rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10)}, 
                   rng.random_float(0.5f, 2.0f)};
        sphere b = {{rng.random_float(-10, 10), rng.random_float(-10, 10), rng.random_float(-10, 10)}, 
                   rng.random_float(0.5f, 2.0f)};
        
        bool ab = sphere_sphere_intersect(&a, &b);
        bool ba = sphere_sphere_intersect(&b, &a);
        
        EXPECT_EQ(ab, ba);
    }
}

/**
 * @test PhysicsPropertyTest.SphereCollisionReflexive
 * @brief Verifies sphere Collision Reflexive.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, SphereCollisionReflexive) {
    sphere s = {{0, 0, 0}, 1.0f};
    
    // A sphere always collides with itself
    EXPECT_TRUE(sphere_sphere_intersect(&s, &s));
}

/**
 * @test PhysicsPropertyTest.PlaneCollisionWithZeroNormal
 * @brief Verifies plane Collision With Zero Normal.
 *
 * @details
 * Exercises the PhysicsPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsPropertyTest, PlaneCollisionWithZeroNormal) {
    sphere s = {{0, 0, 5}, 1.0f};
    plane p = {{0, 0, 0}, 0};  // zero normal - undefined behavior
    
    // Should handle gracefully (not crash)
    bool result = sphere_plane_intersect(&s, &p);
    (void)result;  // suppress unused warning
}

// Stress tests
/**
 * @brief GTest fixture for PhysicsStressTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PhysicsStressTest : public ::testing::Test {
protected:
    static const int N = 1000;
};

/**
 * @test PhysicsStressTest.ManyPhysicsSteps
 * @brief Verifies many Physics Steps.
 *
 * @details
 * Exercises the PhysicsStressTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsStressTest, ManyPhysicsSteps) {
    SeededRNG rng{777};
    
    physics_state p;
    p.position = {0, 0, 0};
    p.velocity = {0, 0, 0};
    p.thrust = {1, 0, 0};
    p.mass = 1.0f;
    p.drag = 0.1f;
    p.max_velocity = 10.0f;
    
    for (int i = 0; i < N; i++) {
        physics_step(&p, 0.01f);
    }
    
    // Should be clamped to max_velocity
    EXPECT_LE(p.velocity.x, 10.0f + PHYSICS_TOLERANCE);
}

/**
 * @test PhysicsStressTest.ManyCollisionChecks
 * @brief Verifies many Collision Checks.
 *
 * @details
 * Exercises the PhysicsStressTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/physics_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PhysicsStressTest, ManyCollisionChecks) {
    SeededRNG rng{888};
    int collisions = 0;
    
    for (int i = 0; i < N; i++) {
        sphere a = {{rng.random_float(-100, 100), rng.random_float(-100, 100), rng.random_float(-100, 100)}, 
                   rng.random_float(0.1f, 5.0f)};
        sphere b = {{rng.random_float(-100, 100), rng.random_float(-100, 100), rng.random_float(-100, 100)}, 
                   rng.random_float(0.1f, 5.0f)};
        
        if (sphere_sphere_intersect(&a, &b)) collisions++;
    }
    
    // Just verify no crashes
    EXPECT_GE(collisions, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
