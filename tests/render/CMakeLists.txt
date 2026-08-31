# D3 Render Tests - Headless rendering tests using Mesa llvmpipe
# Render tests are split into multiple executables, one per suite.

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

set(D3_RENDER_SUITES
    egl
    gl
    game
    hud
)

set(D3_RENDER_SUITE_SOURCES
    egl               d3_render_tests_egl.cpp
    gl                d3_render_tests_gl.cpp
    game              d3_render_tests_game.cpp
    hud               d3_render_tests_hud.cpp
)

function(add_d3_render_test_executable suite_name)
    set(options)
    set(oneValueArgs SOURCE)
    set(multiValueArgs)
    cmake_parse_arguments(SUITE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(target_name "d3_render_tests_${suite_name}")
    add_executable(${target_name}
        d3_render_tests.cpp
        ${SUITE_SOURCE}
    )

    # Frame pointer and debug for unwinding/symbols; -finstrument-functions only on suite sources
    target_compile_options(${target_name} PRIVATE
        -fno-omit-frame-pointer
        -g
    )

    # Apply -finstrument-functions only to the suite source (not main or base)
    set_property(SOURCE ${SUITE_SOURCE}
        APPEND PROPERTY COMPILE_OPTIONS "-finstrument-functions"
    )

    # Export symbols to .dynsym so dladdr() can resolve names at runtime for tracing
    target_link_options(${target_name} PRIVATE
        -rdynamic
    )

    # Note: We link mesaopengl to override renderer symbols from descent3.
    # MesaOpenGL uses EGL pbuffer for headless rendering.
    # Use --allow-multiple-definition to let first definition win (mesaopengl)
    target_link_libraries(${target_name} PRIVATE
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

    target_include_directories(${target_name} PRIVATE 
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

    target_compile_definitions(${target_name} PRIVATE 
        -DDEBUGBREAK_AS_ASSERT
        -DPOSIX
        -D__LINUX__
    )

    # Register each suite as a separate CTest
    add_test(NAME ${target_name} COMMAND ${target_name})
    set_tests_properties(${target_name} PROPERTIES
        TIMEOUT 120
    )
endfunction()

foreach(suite ${D3_RENDER_SUITES})
    list(FIND D3_RENDER_SUITE_SOURCES ${suite} SUITE_INDEX)
    if(SUITE_INDEX EQUAL -1)
        message(FATAL_ERROR "No source mapped for render suite '${suite}'")
    endif()
    math(EXPR SOURCE_INDEX "${SUITE_INDEX} + 1")
    list(GET D3_RENDER_SUITE_SOURCES ${SOURCE_INDEX} SUITE_SOURCE)
    add_d3_render_test_executable(${suite} SOURCE ${SUITE_SOURCE})
endforeach()

# Convenience target that builds all render test binaries
add_custom_target(d3_render_tests_all DEPENDS
    d3_render_tests_egl
    d3_render_tests_gl
    d3_render_tests_game
    d3_render_tests_hud
)

