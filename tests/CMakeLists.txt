# Coverage tests CMakeLists.txt
# Build with: cmake -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON ..

# Put all test executables in tests/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# Platform defines so tests that include lib/ (crossplat.h, bitmap.h) compile on Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  add_compile_definitions(POSIX __LINUX__)
endif()

# Find GTest (includes GMock on Arch Linux)
find_package(GTest REQUIRED)

# Note: On Arch, GMock is available as GTest::gmock (not GMock::gmock)

# Test data generators (header-only, included in test sources)
# Using object library to ensure consistent compilation
add_library(test_data INTERFACE)
target_include_directories(test_data INTERFACE 
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# vecmat tests - DISABLED: new vecmat API uses method calls
# add_executable(vecmat_tests
#     vecmat_tests.cpp
# )
# target_link_libraries(vecmat_tests 
#     GTest::gtest_main
#     test_data
#     vecmat
#     fix
#     ddebug
# )
# target_include_directories(vecmat_tests PRIVATE ${PROJECT_SOURCE_DIR}/lib)
# gtest_discover_tests(vecmat_tests)

# physics tests - DISABLED: uses old vecmat API
# add_executable(physics_tests
add_executable(physics_tests
    physics_tests.cpp
)
target_link_libraries(physics_tests 
    GTest::gtest_main
    test_data
    physics
    vecmat
    fix
)
target_include_directories(physics_tests PRIVATE ${PROJECT_SOURCE_DIR}/lib)
gtest_discover_tests(physics_tests)

# ============================================================================
# CFile Mock Tests
# Uses OBJECT library to ensure mock symbols take precedence over real cfile
# ============================================================================

# Mock cfile implementation (compiled as OBJECT - linked FIRST in tests)
add_library(mock_cfile OBJECT
    mock_cfile.cpp
)
target_include_directories(mock_cfile PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# CFile mock tests - linked with mock object FIRST (wins symbol conflicts)
add_executable(cfile_mock_tests
    cfile_mock_tests.cpp
)

# Mock only (no real cfile) to avoid duplicate symbols; mock provides full API
target_compile_definitions(cfile_mock_tests PRIVATE D3_MOCK_CFILE_STANDALONE)
target_link_libraries(cfile_mock_tests PRIVATE
    $<TARGET_OBJECTS:mock_cfile>
    GTest::gtest_main
    GTest::gmock
)

gtest_discover_tests(cfile_mock_tests)

# ============================================================================
# CFile Standalone Tests (using mock infrastructure)
# ============================================================================

add_executable(cfile_coverage_tests
    cfile_tests_standalone.cpp
)

target_compile_definitions(cfile_coverage_tests PRIVATE D3_MOCK_CFILE_STANDALONE)
target_link_libraries(cfile_coverage_tests PRIVATE
    $<TARGET_OBJECTS:mock_cfile>
    GTest::gtest_main
)

gtest_discover_tests(cfile_coverage_tests)

# ============================================================================
# Stub libraries for dependencies (ddebug, ddio, args)
# ============================================================================

# C stubs (no name mangling)
add_library(test_stubs_c OBJECT
    stubs.c
)
target_include_directories(test_stubs_c PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
)

# C++ stubs (cfile functions) - compiled as static lib with C linkage
add_library(test_stubs_cpp STATIC
    mock_cfile.cpp
)
target_include_directories(test_stubs_cpp PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# Force C linkage for all cfile functions
target_compile_options(test_stubs_cpp PRIVATE -x c++)

# ============================================================================
# Bitmap Allocation Tests
# Disabled: tests expect bm_used() and bm_AllocLoadIFFAnim() which are not in
# the public lib/bitmap.h API. Re-enable when those are added to the bitmap library.
# ============================================================================
# add_executable(bitmap_alloc_tests
#     bitmap_alloc_tests.cpp
#     stubs.c
#     mock_cfile.cpp
# )
# target_link_libraries(bitmap_alloc_tests PRIVATE
#     GTest::gtest_main
#     bitmap
#     mem
#     fix
# )
# target_include_directories(bitmap_alloc_tests PRIVATE
#     ${PROJECT_SOURCE_DIR}/lib
#     ${PROJECT_SOURCE_DIR}/bitmap
#     ${PROJECT_SOURCE_DIR}/cfile
# )
# gtest_discover_tests(bitmap_alloc_tests)

# ============================================================================
# Bitmap Loader Tests
# ============================================================================

add_executable(bitmap_loader_tests
    bitmap_loader_tests.cpp
    stubs.c
)

# Mock and stubs first so -Wl,--allow-multiple-definition uses them over cfile/ddebug/ddio from bitmap
target_link_options(bitmap_loader_tests PRIVATE "LINKER:--allow-multiple-definition")
target_link_libraries(bitmap_loader_tests PRIVATE
    $<TARGET_OBJECTS:mock_cfile>
    GTest::gtest_main
    mem
    fix
    bitmap
)

target_include_directories(bitmap_loader_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
)

gtest_discover_tests(bitmap_loader_tests)

# ============================================================================
# IFF Loader Tests
# Tests IFF format (PBM/ILBM) loading from in-memory buffers
# ============================================================================

add_executable(iff_loader_tests
    iff_loader_tests.cpp
    stubs.c
)

target_link_options(iff_loader_tests PRIVATE "LINKER:--allow-multiple-definition")
target_link_libraries(iff_loader_tests PRIVATE
    $<TARGET_OBJECTS:mock_cfile>
    GTest::gtest_main
    mem
    fix
    bitmap
)

target_include_directories(iff_loader_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
)

gtest_discover_tests(iff_loader_tests)

# ============================================================================
# PCX Loader Tests
# Tests PCX format (8-bit and 24-bit) loading from in-memory buffers
# ============================================================================

add_executable(pcx_loader_tests
    pcx_loader_tests.cpp
    stubs.c
)

target_link_options(pcx_loader_tests PRIVATE "LINKER:--allow-multiple-definition")
target_link_libraries(pcx_loader_tests PRIVATE
    $<TARGET_OBJECTS:mock_cfile>
    GTest::gtest_main
    mem
    fix
    bitmap
)

target_include_directories(pcx_loader_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
)

gtest_discover_tests(pcx_loader_tests)

# ============================================================================
# Manage Tests (tracklock functions)
# Tests manage module's tracklock functionality
# Uses minimal tracklock implementation to avoid heavy dependencies
# ============================================================================

add_executable(manage_tests
    manage_tests.cpp
    manage_tracklocks.cpp
    stubs.c
)

target_link_options(manage_tests PRIVATE "LINKER:--allow-multiple-definition")
target_link_libraries(manage_tests PRIVATE
    $<TARGET_OBJECTS:mock_cfile>
    GTest::gtest_main
    mem
    fix
)

target_include_directories(manage_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/linux
)

# Define __LINUX__ for Linux builds
target_compile_definitions(manage_tests PRIVATE __LINUX__)

gtest_discover_tests(manage_tests)

# ============================================================================
# PART 4: DESCENT3/ SUB-MODULES - External Header Tests
# These tests verify constants and flags defined in external headers
# ============================================================================

# Weapon external tests (weapon indices)
add_executable(weapon_external_tests
    weapon_external_tests_standalone.cpp
)
target_link_libraries(weapon_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(weapon_external_tests)

# Damage external tests (damage types)
add_executable(damage_external_tests
    damage_external_tests_standalone.cpp
)
target_link_libraries(damage_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(damage_external_tests)

# LevelGoal external tests (goal flags and masks)
add_executable(levelgoal_external_tests
    levelgoal_external_tests_standalone.cpp
)
target_link_libraries(levelgoal_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(levelgoal_external_tests)

# Room external tests (room/face/portal flags)
add_executable(room_external_tests
    room_external_tests_standalone.cpp
)
target_link_libraries(room_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(room_external_tests)

# Object external tests (object types and flags)
add_executable(object_external_tests
    object_external_tests_standalone.cpp
)
target_link_libraries(object_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(object_external_tests)

# Object constants tests (from object.h - handle masks, lighting types, flags)
add_executable(object_constants_tests
    object_constants_tests.cpp
)
target_link_libraries(object_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(object_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(object_constants_tests)

# Player and Controls constants tests
add_executable(player_controls_constants_tests
    player_controls_constants_tests.cpp
)
target_link_libraries(player_controls_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(player_controls_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(player_controls_constants_tests)

# Ship constants tests
add_executable(ship_constants_tests
    ship_constants_tests.cpp
)
target_link_libraries(ship_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(ship_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(ship_constants_tests)

# Weapon constants tests
add_executable(weapon_constants_tests
    weapon_constants_tests.cpp
)
target_link_libraries(weapon_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(weapon_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(weapon_constants_tests)

# AI constants tests
add_executable(ai_constants_tests
    ai_constants_tests.cpp
)
target_link_libraries(ai_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(ai_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(ai_constants_tests)

# World & Environment constants tests
add_executable(world_constants_tests
    world_constants_tests.cpp
)
target_link_libraries(world_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(world_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(world_constants_tests)

# Game Loop & State constants tests
add_executable(gamestate_constants_tests
    gamestate_constants_tests.cpp
)
target_link_libraries(gamestate_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(gamestate_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(gamestate_constants_tests)

# Pilot & Config constants tests
add_executable(pilot_config_constants_tests
    pilot_config_constants_tests.cpp
)
target_link_libraries(pilot_config_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(pilot_config_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(pilot_config_constants_tests)

# Other Systems constants tests (ambient, lighting, powerup, marker)
add_executable(othersystems_constants_tests
    othersystems_constants_tests.cpp
)
target_link_libraries(othersystems_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(othersystems_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(othersystems_constants_tests)

# TelCom constants tests
add_executable(telcom_constants_tests
    telcom_constants_tests.cpp
)
target_link_libraries(telcom_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(telcom_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(telcom_constants_tests)

# Visual Effects constants tests
add_executable(vfx_constants_tests
    vfx_constants_tests.cpp
)
target_link_libraries(vfx_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(vfx_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(vfx_constants_tests)

# ---- tracer library — compiled WITHOUT -finstrument-functions ----
add_library(tracer STATIC
    tracer/tracer.cpp
)
# PUBLIC so that render_listener.h (which includes tracer.h) is visible
# to any target that links tracer
target_include_directories(tracer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tracer)
# PRIVATE: dl is an implementation detail of tracer.cpp, not part of its API
target_link_libraries(tracer PRIVATE dl)

# Consolidated render tests using OffscreenGL + Mesa llvmpipe
add_subdirectory(render)
