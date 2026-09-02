set(HEADERS
  ddsndgeometry.h
  hlsoundlib.h
  mixer.h
  openalsound.h
  sdlsound.h
  sndrender.h
  soundload.h
  ssl_lib.h)
set(CPPS
  hlsoundlib.cpp
  sndrender.cpp
  soundload.cpp
  ddsoundload.cpp
  ssl_lib.cpp
  mixer.cpp
  sdlgeometry.cpp
  sdlsound.cpp
)

# Optional OpenAL backend
find_package(OpenAL QUIET)
if(OpenAL_FOUND)
  list(APPEND CPPS openalsound.cpp)
  list(APPEND HEADERS openalsound.h)
endif()

add_library(sndlib STATIC ${HEADERS} ${CPPS})
target_link_libraries(sndlib PRIVATE
  cfile
  ddebug
  ddio
  logger
  mem
  stream_audio
  physics
  SDL3::SDL3
)
if(OpenAL_FOUND)
  target_link_libraries(sndlib PRIVATE OpenAL::OpenAL)
  target_compile_definitions(sndlib PRIVATE HAVE_OPENAL=1)
endif()
target_include_directories(sndlib PUBLIC
  $<BUILD_INTERFACE:
  ${PROJECT_SOURCE_DIR}/sndlib
  >
  ${SDL3_INCLUDE_DIRS}
)
