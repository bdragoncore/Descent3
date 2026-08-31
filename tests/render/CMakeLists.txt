# D3 Render Tests - Consolidated headless rendering tests using Mesa llvmpipe
# All render tests consolidated into a single executable: d3_render_tests

# Find required packages for headless rendering
find_package(PkgConfig REQUIRED)
pkg_check_modules(EGL REQUIRED IMPORTED_TARGET egl)
pkg_check_modules(PNG REQUIRED libpng)

# Shared render output directory for all rendering tests
set(RENDER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/tests/render_output")
file(MAKE_DIRECTORY ${RENDER_OUTPUT_DIR})

# Core init, OffscreenGL, and base class (compiled as static library to share across tests)
add_library(d3_render_test_base STATIC
    core_init.cpp
    render_test_base.cpp
    offscreen_gl.cpp
)

# Find vcpkg plog if available
find_path(VCPKG_PLOG_INCLUDE_DIR "plog/Log.h" PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build/vcpkg_installed/x64-linux/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg_installed/x64-linux/include
)

target_include_directories(d3_render_test_base PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/renderer
    ${EGL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VCPKG_PLOG_INCLUDE_DIR}
)

target_link_libraries(d3_render_test_base PUBLIC
    PkgConfig::EGL
    GL
    md5
)

target_compile_definitions(d3_render_test_base PUBLIC
    RENDER_OUTPUT_DIR="${RENDER_OUTPUT_DIR}"
)

target_compile_definitions(d3_render_test_base PRIVATE 
    -DDEBUGBREAK_AS_ASSERT
    -DPOSIX
    -D__LINUX__
)

# Main test executable - connects all render test suites to the gtest runner
add_executable(d3_render_tests
    d3_render_tests.cpp
    d3_render_tests_basic.cpp
    d3_render_tests_egl.cpp
    d3_render_tests_software_gl.cpp
    d3_render_tests_text.cpp
    d3_render_tests_mesa_legacy.cpp
    d3_render_tests_mesa_renderer_legacy.cpp
    d3_render_tests_polymodel_legacy.cpp
)

# Frame pointer and debug for unwinding/symbols; -finstrument-functions only on suite sources
target_compile_options(d3_render_tests PRIVATE
    -fno-omit-frame-pointer
    -g
)
# Apply -finstrument-functions only to test suite sources (not main or base)
set_property(SOURCE
    d3_render_tests_basic.cpp
    d3_render_tests_egl.cpp
    d3_render_tests_software_gl.cpp
    d3_render_tests_text.cpp
    d3_render_tests_mesa_legacy.cpp
    d3_render_tests_mesa_renderer_legacy.cpp
    d3_render_tests_polymodel_legacy.cpp
    APPEND PROPERTY COMPILE_OPTIONS "-finstrument-functions"
)

# Export symbols to .dynsym so dladdr() can resolve names at runtime for tracing
target_link_options(d3_render_tests PRIVATE
    -rdynamic
)

# Note: We link mesaopengl to override renderer symbols from descent3.
# MesaOpenGL uses EGL pbuffer for headless rendering.
# Use --allow-multiple-definition to let first definition win (mesaopengl)
target_link_libraries(d3_render_tests PRIVATE
    tracer
    GTest::gtest
    -Wl,--allow-multiple-definition
    -Wl,--start-group
    d3_render_test_base
    mesaopengl
    cfile
    bitmap
    mem
    fix
    ddebug
    ddio
    logger
    -Wl,--end-group
    -Wl,--whole-archive
    descent3
    linux
    -Wl,--no-whole-archive
    ${SDL3_LIBRARIES}
    ${PNG_LIBRARIES}
    PkgConfig::EGL
    GL
    GLU
)

target_include_directories(d3_render_tests PRIVATE 
    ${SDL3_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/tests
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/bitmap
)

target_compile_definitions(d3_render_tests PRIVATE 
    -DDEBUGBREAK_AS_ASSERT
    -DPOSIX
    -D__LINUX__
)

# Register test - env vars are hardcoded in d3_render_tests.cpp
add_test(NAME d3_render_tests COMMAND d3_render_tests)
set_tests_properties(d3_render_tests PROPERTIES
    TIMEOUT 120
)
