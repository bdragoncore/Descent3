cmake_path(SET GENERATED_HEADERS NORMALIZE ${CMAKE_CURRENT_BINARY_DIR}/generated)
cmake_path(SET SHADERS_H NORMALIZE ${GENERATED_HEADERS}/shaders.h)
set(SHADERS
  shaders/fragment.glsl
  shaders/vertex.glsl
)
add_custom_command(
  OUTPUT ${SHADERS_H}
  COMMAND ${CMAKE_COMMAND} -Dshaders_h=${SHADERS_H} -P ../cmake/shaders.cmake -- ${SHADERS}
  DEPENDS ../cmake/shaders.cmake ${SHADERS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(HEADERS
  dyna_gl.h
  HardwareInternal.h
  HardwareOpenGL.h
  ShaderProgram.h
  ${SHADERS_H}
)
set(CPPS
  HardwareClipper.cpp
  HardwareDraw.cpp
  HardwareGlobalVars.cpp
  HardwareInstance.cpp
  HardwareOpenGL.cpp
  HardwareBaseGPU.cpp
  HardwarePoints.cpp
  HardwareSetup.cpp
  HardwareTransforms.cpp
)

add_library(renderer STATIC ${HEADERS} ${CPPS})
target_include_directories(renderer PRIVATE ${GENERATED_HEADERS} ${PROJECT_BINARY_DIR}/lib ${SDL3_INCLUDE_DIRS})
target_link_libraries(renderer PRIVATE
  glm::glm
  SDL3::SDL3
  bitmap
  ddebug
  ddio
  logger
  mem
  misc
  module
  rtperformance
)

# Mesa OpenGL software renderer library
# ====================================
# This library provides headless OpenGL rendering using Mesa's software rasterizer
# (llvmpipe) for unit testing purposes. It uses EGL with surfaceless platform to
# create an offscreen OpenGL context without requiring a display server.

# Find EGL and OpenGL using pkg-config for headless rendering
find_package(PkgConfig REQUIRED)
pkg_check_modules(EGL REQUIRED IMPORTED_TARGET egl)
pkg_check_modules(OpenGL REQUIRED IMPORTED_TARGET opengl)

add_library(mesaopengl STATIC MesaOpenGL.cpp MesaOpenGL.h)
target_include_directories(mesaopengl PRIVATE ${GENERATED_HEADERS} ${SDL3_INCLUDE_DIRS})

# Define RENDERER_MESA_ENABLED to enable Mesa-specific symbols
# This allows the Mesa renderer to override the HardwareOpenGL renderer
target_compile_definitions(mesaopengl PRIVATE RENDERER_MESA_ENABLED)

# Link against EGL and OpenGL for headless rendering
# EGL provides the surfaceless platform for headless OpenGL context creation
# OpenGL provides the rendering API
target_link_libraries(mesaopengl PRIVATE
  glm::glm
  SDL3::SDL3
  PkgConfig::EGL
  PkgConfig::OpenGL
  bitmap
  ddebug
  ddio
  logger
  mem
  misc
  module
  rtperformance
)
