# Coverage tests CMakeLists.txt
# Build with: cmake -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON ..

# Put all test executables in tests/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# Platform defines so tests that include lib/ (crossplat.h, bitmap.h) compile.
# Mirrors the platform block in the top-level CMakeLists.txt. Tests are added
# via add_subdirectory(tests) before the parent's add_compile_definitions, so
# they do NOT inherit those defines.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  add_compile_definitions(POSIX __LINUX__)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_compile_definitions(POSIX MACOSX=1)
endif()
# On Windows, WIN32 is defined by the compiler/platform automatically.
# NOMINMAX prevents the windows.h min/max macros from breaking std::min/std::max.
if(WIN32)
  add_compile_definitions(NOMINMAX)
endif()

# Linker flag to allow multiple definitions (needed when stubs override real libs).
# GNU ld uses --allow-multiple-definition; Apple ld does not support this
# (multiply_defined is obsolete/ignored on modern ld64). Tests that need this
# flag are guarded to non-Apple platforms below.
if(NOT APPLE)
  set(ALLOW_MULTI_DEF_LINKER_FLAG -Wl,--allow-multiple-definition)
else()
  set(ALLOW_MULTI_DEF_LINKER_FLAG "")
endif()

# stdc++fs is needed for std::filesystem on older GCC but does not exist on
# macOS (libc++) or MSVC (built-in). Create an INTERFACE target that
# conditionally provides it.
add_library(stdcppfs INTERFACE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
  target_link_libraries(stdcppfs INTERFACE stdc++fs)
endif()

# Find GTest (includes GMock on Arch Linux)
find_package(GTest REQUIRED)

# BUGFIX #509: Verify the MSVC runtime library is set to static CRT.
# The dynamic debug CRT (ucrtbased.dll) is not redistributable, so Debug
# builds could not start on machines without the Windows SDK. The root
# CMakeLists.txt sets CMAKE_MSVC_RUNTIME_LIBRARY to "MultiThreaded$<$<CONFIG:Debug>:Debug>"
# (/MT for Release, /MTd for Debug). This test fails at configure time if
# that setting is ever removed or changed back to dynamic.
if(MSVC)
  if(NOT CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreaded")
    message(FATAL_ERROR "BUGFIX #509 regression: CMAKE_MSVC_RUNTIME_LIBRARY must be static CRT (MultiThreaded/MultiThreadedDebug), got '${CMAKE_MSVC_RUNTIME_LIBRARY}'. Dynamic CRT (/MDd) requires non-redistributable ucrtbased.dll.")
  endif()
endif()

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
#     cpp_src/vecmat_tests.cpp
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
    cpp_src/physics_tests.cpp
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

# vecmat utility tests - cover the remaining vm_* functions against the real library
add_executable(vecmat_util_tests
    cpp_src/vecmat_util_tests.cpp
)
target_link_libraries(vecmat_util_tests
    GTest::gtest_main
    test_data
    vecmat
)
target_include_directories(vecmat_util_tests PRIVATE ${PROJECT_SOURCE_DIR}/lib)
gtest_discover_tests(vecmat_util_tests)

# ============================================================================
# Standalone utility tests (self-contained, gtest only)
# ============================================================================

set(STANDALONE_UTIL_TESTS
    args_tests_standalone
    byteswap_tests_standalone
    fix_tests_standalone
    macros_tests_standalone
    md5_tests_standalone
    psclass_tests_standalone
    psglob_tests_standalone
    psrand_tests_standalone
    pstring_tests_standalone
    vecmat_tests_standalone
    vecmat_external_tests_standalone
)

foreach(t ${STANDALONE_UTIL_TESTS})
    if(t STREQUAL "psglob_tests_standalone")
        # psglob tests exercise the real misc library (PSGlobMatch)
        add_executable(${t} cpp_src/${t}.cpp)
        target_link_libraries(${t} PRIVATE GTest::gtest_main misc)
        gtest_discover_tests(${t})
    else()
        add_executable(${t} cpp_src/${t}.cpp)
        target_link_libraries(${t} PRIVATE GTest::gtest_main)
        gtest_discover_tests(${t})
    endif()
endforeach()

# byteswap_tests exercises the real lib/byteswap.h header
add_executable(byteswap_tests cpp_src/byteswap_tests.cpp)
target_link_libraries(byteswap_tests PRIVATE GTest::gtest_main test_data)
gtest_discover_tests(byteswap_tests)

# psclass tests exercise the real lib/psclass.h template containers
add_executable(psclass_real_tests cpp_src/psclass_real_tests.cpp)
target_link_libraries(psclass_real_tests PRIVATE GTest::gtest_main test_data)
gtest_discover_tests(psclass_real_tests)

# Real-library tests for the small utility libraries
add_executable(fix_real_tests cpp_src/fix_real_tests.cpp)
target_link_libraries(fix_real_tests PRIVATE GTest::gtest_main test_data fix)
gtest_discover_tests(fix_real_tests)

add_executable(psrand_real_tests cpp_src/psrand_real_tests.cpp)
target_link_libraries(psrand_real_tests PRIVATE GTest::gtest_main misc)
gtest_discover_tests(psrand_real_tests)

add_executable(mem_real_tests cpp_src/mem_real_tests.cpp)
target_link_libraries(mem_real_tests PRIVATE GTest::gtest_main mem)
target_include_directories(mem_real_tests PRIVATE ${PROJECT_SOURCE_DIR}/mem ${PROJECT_SOURCE_DIR}/lib)
gtest_discover_tests(mem_real_tests)

add_executable(lib_util_tests cpp_src/lib_util_tests.cpp)
target_link_libraries(lib_util_tests PRIVATE GTest::gtest_main test_data logger)
target_include_directories(lib_util_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/logger
)
gtest_discover_tests(lib_util_tests)

# ddio_file_tests: use lnxfile.cpp on POSIX, winfile.cpp on Windows
if(WIN32)
  set(DDIO_FILE_TEST_SOURCES
      cpp_src/ddio_file_tests.cpp
      ${PROJECT_SOURCE_DIR}/ddio/winfile.cpp
      ${PROJECT_SOURCE_DIR}/ddio/key.cpp)
else()
  set(DDIO_FILE_TEST_SOURCES
      cpp_src/ddio_file_tests.cpp
      ${PROJECT_SOURCE_DIR}/ddio/lnxfile.cpp
      ${PROJECT_SOURCE_DIR}/ddio/key.cpp)
endif()
add_executable(ddio_file_tests ${DDIO_FILE_TEST_SOURCES})
target_link_libraries(ddio_file_tests PRIVATE GTest::gtest_main logger ddebug mem)
target_include_directories(ddio_file_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/ddebug
)
gtest_discover_tests(ddio_file_tests)

add_executable(unzip_real_tests cpp_src/unzip_real_tests.cpp)
target_link_libraries(unzip_real_tests PRIVATE GTest::gtest_main unzip)
target_include_directories(unzip_real_tests PRIVATE ${PROJECT_SOURCE_DIR}/unzip)
gtest_discover_tests(unzip_real_tests)

add_executable(list_real_tests
    cpp_src/list_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/list.cpp
)
target_link_libraries(list_real_tests PRIVATE GTest::gtest_main mem logger ddebug)
target_include_directories(list_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/ddebug
)
# list.cpp calls Int3() on duplicate AddListItem; in tests that assert triggers
# SDL_assert(false) and aborts. Define RELEASE so Int3 becomes no-op for the test.
target_compile_definitions(list_real_tests PRIVATE RELEASE)
gtest_discover_tests(list_real_tests)

add_executable(small_game_real_tests
    cpp_src/small_game_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/difficulty.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gamepath.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/megacell.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gamefile.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/special_face.cpp
)
target_link_libraries(small_game_real_tests PRIVATE GTest::gtest_main mem logger ddebug bitmap)
target_include_directories(small_game_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
)
target_compile_definitions(small_game_real_tests PRIVATE RELEASE)
gtest_discover_tests(small_game_real_tests)

add_executable(door_ship_vclip_real_tests
    cpp_src/door_ship_vclip_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/door.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/ship.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/vclip.cpp
)
target_link_libraries(door_ship_vclip_real_tests PRIVATE GTest::gtest_main mem logger ddebug bitmap cfile)
target_include_directories(door_ship_vclip_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/object
)
target_compile_definitions(door_ship_vclip_real_tests PRIVATE RELEASE)
gtest_discover_tests(door_ship_vclip_real_tests)

add_executable(aiambient_real_tests
    cpp_src/aiambient_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/aiambient.cpp
)
target_link_libraries(aiambient_real_tests PRIVATE GTest::gtest_main misc mem logger cfile)
target_include_directories(aiambient_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/object
)
target_compile_definitions(aiambient_real_tests PRIVATE RELEASE)
gtest_discover_tests(aiambient_real_tests)

add_executable(aiterrain_real_tests
    cpp_src/aiterrain_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/aiterrain.cpp
)
target_link_libraries(aiterrain_real_tests PRIVATE GTest::gtest_main vecmat)
target_include_directories(aiterrain_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
)
target_compile_definitions(aiterrain_real_tests PRIVATE RELEASE)
gtest_discover_tests(aiterrain_real_tests)

add_executable(trigger_real_tests
    cpp_src/trigger_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/trigger.cpp
)
target_link_libraries(trigger_real_tests PRIVATE GTest::gtest_main logger ddebug mem)
target_include_directories(trigger_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/module
)
target_compile_definitions(trigger_real_tests PRIVATE RELEASE)
gtest_discover_tests(trigger_real_tests)

add_executable(marker_real_tests
    cpp_src/marker_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/marker.cpp
)
target_link_libraries(marker_real_tests PRIVATE GTest::gtest_main logger ddebug)
target_include_directories(marker_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/linux
)
target_compile_definitions(marker_real_tests PRIVATE RELEASE)
gtest_discover_tests(marker_real_tests)

add_executable(gamespyutils_real_tests
    cpp_src/gamespyutils_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gamespyutils.cpp
)
target_link_libraries(gamespyutils_real_tests PRIVATE GTest::gtest_main)
target_include_directories(gamespyutils_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(gamespyutils_real_tests)

add_executable(args_real_tests
    cpp_src/args_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/args.cpp
)
target_link_libraries(args_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(args_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/logger
)
gtest_discover_tests(args_real_tests)

add_executable(objinfo_real_tests
    cpp_src/objinfo_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/objinfo.cpp
)
target_link_libraries(objinfo_real_tests PRIVATE GTest::gtest_main mem logger ddebug)
target_include_directories(objinfo_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/object
    ${PROJECT_SOURCE_DIR}/linux
)
target_compile_definitions(objinfo_real_tests PRIVATE RELEASE)
gtest_discover_tests(objinfo_real_tests)

add_executable(program_real_tests
    cpp_src/program_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/program.cpp
)
target_link_libraries(program_real_tests PRIVATE GTest::gtest_main logger ddebug)
target_include_directories(program_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/lib/linux
    ${PROJECT_BINARY_DIR}/lib
    ${PROJECT_BINARY_DIR}
)
target_compile_definitions(program_real_tests PRIVATE RELEASE)
gtest_discover_tests(program_real_tests)

add_executable(splinter_real_tests
    cpp_src/splinter_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/splinter.cpp
)
target_link_libraries(splinter_real_tests PRIVATE GTest::gtest_main logger ddebug vecmat)
target_include_directories(splinter_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/object
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/misc
)
target_compile_definitions(splinter_real_tests PRIVATE RELEASE)
gtest_discover_tests(splinter_real_tests)

add_executable(soar_real_tests
    cpp_src/soar_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/soar.cpp
)
target_link_libraries(soar_real_tests PRIVATE GTest::gtest_main)
target_include_directories(soar_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
)
gtest_discover_tests(soar_real_tests)

add_executable(bnode_real_tests
    cpp_src/bnode_real_tests.cpp
)
target_link_libraries(bnode_real_tests PRIVATE GTest::gtest_main)
target_include_directories(bnode_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
)
gtest_discover_tests(bnode_real_tests)

add_executable(gameevent_real_tests
    cpp_src/gameevent_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gameevent.cpp
)
target_link_libraries(gameevent_real_tests PRIVATE GTest::gtest_main mem logger)
target_include_directories(gameevent_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/vecmat
)
target_compile_definitions(gameevent_real_tests PRIVATE RELEASE)
gtest_discover_tests(gameevent_real_tests)

add_executable(postrender_real_tests
    cpp_src/postrender_real_tests.cpp
)
target_link_libraries(postrender_real_tests PRIVATE GTest::gtest_main)
target_include_directories(postrender_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
)
gtest_discover_tests(postrender_real_tests)

add_executable(subtitles_real_tests
    cpp_src/subtitles_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/subtitles.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/args.cpp
)
target_link_libraries(subtitles_real_tests PRIVATE GTest::gtest_main mem logger cfile)
target_include_directories(subtitles_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
)
target_compile_definitions(subtitles_real_tests PRIVATE RELEASE)
gtest_discover_tests(subtitles_real_tests)

add_executable(weather_real_tests
    cpp_src/weather_real_tests.cpp
    cpp_src/fake_hlsystem.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/weather.cpp
)
target_link_libraries(weather_real_tests PRIVATE GTest::gtest_main mem logger vecmat fix)
target_include_directories(weather_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
)
target_compile_definitions(weather_real_tests PRIVATE RELEASE)
gtest_discover_tests(weather_real_tests)

add_executable(voice_real_tests
    cpp_src/voice_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/voice.cpp
)
target_link_libraries(voice_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(voice_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/stream_audio
    ${PROJECT_SOURCE_DIR}/AudioEncode
)
target_compile_definitions(voice_real_tests PRIVATE RELEASE)
gtest_discover_tests(voice_real_tests)

add_executable(credits_real_tests
    cpp_src/credits_real_tests.cpp
    cpp_src/fake_hlsystem.cpp
)
target_link_libraries(credits_real_tests PRIVATE GTest::gtest_main logger ddebug mem fix vecmat cfile)
target_include_directories(credits_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
)
target_compile_definitions(credits_real_tests PRIVATE RELEASE)
gtest_discover_tests(credits_real_tests)

add_executable(scorch_real_tests
    cpp_src/scorch_real_tests.cpp
)
target_link_libraries(scorch_real_tests PRIVATE GTest::gtest_main logger ddebug mem fix vecmat cfile)
target_include_directories(scorch_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
)
target_compile_definitions(scorch_real_tests PRIVATE RELEASE)
gtest_discover_tests(scorch_real_tests)

add_executable(robotfire_real_tests
    cpp_src/robotfire_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/robotfire.cpp
)
target_link_libraries(robotfire_real_tests PRIVATE GTest::gtest_main logger ddebug mem fix vecmat cfile)
target_include_directories(robotfire_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(robotfire_real_tests PRIVATE RELEASE)
gtest_discover_tests(robotfire_real_tests)

# vclip_real_tests needs --allow-multiple-definition (GNU ld) which is not
# available on macOS (Apple ld ignores -multiply_defined). Skip on Apple.
if(NOT APPLE)
add_executable(vclip_real_tests
    cpp_src/vclip_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/vclip.cpp
)
# Note: cfile transitively links ddebug, whose Error() exits the process.
# The test provides its own recording Error(); --allow-multiple-definition
# makes the test's copy (first on the link line) win over ddebug's.
target_link_libraries(vclip_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_link_options(vclip_real_tests PRIVATE ${ALLOW_MULTI_DEF_LINKER_FLAG})
target_include_directories(vclip_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(vclip_real_tests PRIVATE RELEASE)
gtest_discover_tests(vclip_real_tests)
endif()

add_executable(spew_real_tests
    cpp_src/spew_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/spew.cpp
)
target_link_libraries(spew_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(spew_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(spew_real_tests PRIVATE RELEASE)
gtest_discover_tests(spew_real_tests)

add_executable(lightmap_info_real_tests
    cpp_src/lightmap_info_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/lightmap_info.cpp
)
target_link_libraries(lightmap_info_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(lightmap_info_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(lightmap_info_real_tests PRIVATE RELEASE)
gtest_discover_tests(lightmap_info_real_tests)

add_executable(gamepath_gamefile_real_tests
    cpp_src/gamepath_gamefile_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gamepath.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gamefile.cpp
)
target_link_libraries(gamepath_gamefile_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(gamepath_gamefile_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(gamepath_gamefile_real_tests PRIVATE RELEASE)
gtest_discover_tests(gamepath_gamefile_real_tests)

add_executable(ambient_real_tests
    cpp_src/ambient_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/ambient.cpp
)
target_link_libraries(ambient_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(ambient_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(ambient_real_tests PRIVATE RELEASE)
gtest_discover_tests(ambient_real_tests)

add_executable(objscript_real_tests
    cpp_src/objscript_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/ObjScript.cpp
)
target_link_libraries(objscript_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(objscript_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/scripts
    ${PROJECT_SOURCE_DIR}/module
)
target_compile_definitions(objscript_real_tests PRIVATE RELEASE)
gtest_discover_tests(objscript_real_tests)

add_executable(special_face_real_tests
    cpp_src/special_face_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/special_face.cpp
)
target_link_libraries(special_face_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(special_face_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(special_face_real_tests PRIVATE RELEASE)
gtest_discover_tests(special_face_real_tests)

add_executable(multi_save_setting_real_tests
    cpp_src/multi_save_setting_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/multi_save_setting.cpp
)
target_link_libraries(multi_save_setting_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(multi_save_setting_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(multi_save_setting_real_tests PRIVATE RELEASE)
gtest_discover_tests(multi_save_setting_real_tests)

add_executable(briefingparse_real_tests
    cpp_src/briefingparse_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/BriefingParse.cpp
)
target_link_libraries(briefingparse_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(briefingparse_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/grtext
)
target_compile_definitions(briefingparse_real_tests PRIVATE RELEASE)
gtest_discover_tests(briefingparse_real_tests)

add_executable(inventory_real_tests
    cpp_src/inventory_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/Inventory.cpp
)
target_link_libraries(inventory_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(inventory_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/module
)
target_compile_definitions(inventory_real_tests PRIVATE RELEASE)
gtest_discover_tests(inventory_real_tests)

add_executable(hudmessage_real_tests
    cpp_src/hudmessage_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/hudmessage.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/Inventory.cpp
)
target_link_libraries(hudmessage_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile)
target_include_directories(hudmessage_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
)
target_compile_definitions(hudmessage_real_tests PRIVATE RELEASE)
gtest_discover_tests(hudmessage_real_tests)

add_executable(pilot_class_real_tests
    cpp_src/pilot_class_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/pilot_class.cpp
)
target_link_libraries(pilot_class_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile ddio)
target_include_directories(pilot_class_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(pilot_class_real_tests PRIVATE RELEASE)
gtest_discover_tests(pilot_class_real_tests)

add_executable(localization_real_tests
    cpp_src/localization_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/localization.cpp
)
target_link_libraries(localization_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile ddio)
target_include_directories(localization_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
)
target_compile_definitions(localization_real_tests PRIVATE RELEASE)
gtest_discover_tests(localization_real_tests)

add_executable(attach_real_tests
    cpp_src/attach_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/attach.cpp
)
target_link_libraries(attach_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile ddio)
target_include_directories(attach_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/model
)
target_compile_definitions(attach_real_tests PRIVATE RELEASE)
gtest_discover_tests(attach_real_tests)

add_executable(doorway_real_tests
    cpp_src/doorway_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/doorway.cpp
)
target_link_libraries(doorway_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile ddio)
target_include_directories(doorway_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/scripts
    ${PROJECT_SOURCE_DIR}/module
)
target_compile_definitions(doorway_real_tests PRIVATE RELEASE)
gtest_discover_tests(doorway_real_tests)

add_executable(object_lighting_real_tests
    cpp_src/object_lighting_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/object_lighting.cpp
)
target_link_libraries(object_lighting_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile ddio)
target_include_directories(object_lighting_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(object_lighting_real_tests PRIVATE RELEASE)
gtest_discover_tests(object_lighting_real_tests)

add_executable(audiotaunts_real_tests
    cpp_src/audiotaunts_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/audiotaunts.cpp
)
target_link_libraries(audiotaunts_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile ddio)
target_include_directories(audiotaunts_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/stream_audio
    ${PROJECT_SOURCE_DIR}/AudioEncode
    ${PROJECT_SOURCE_DIR}/scripts
)
target_compile_definitions(audiotaunts_real_tests PRIVATE RELEASE)
gtest_discover_tests(audiotaunts_real_tests)

add_executable(pilotpics_real_tests
    cpp_src/pilotpics_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/PilotPicsAPI.cpp
)
target_link_libraries(pilotpics_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat cfile ddio)
target_include_directories(pilotpics_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/model
)
target_compile_definitions(pilotpics_real_tests PRIVATE RELEASE)
gtest_discover_tests(pilotpics_real_tests)

add_executable(gamefont_real_tests
    cpp_src/gamefont_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gamefont.cpp
)
target_link_libraries(gamefont_real_tests PRIVATE GTest::gtest_main logger mem fix)
target_include_directories(gamefont_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/fix
)
target_compile_definitions(gamefont_real_tests PRIVATE RELEASE)
gtest_discover_tests(gamefont_real_tests)

add_executable(cinematics_real_tests
    cpp_src/cinematics_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/cinematics.cpp
)
target_link_libraries(cinematics_real_tests PRIVATE GTest::gtest_main logger mem fix)
target_include_directories(cinematics_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/sndlib
)
target_compile_definitions(cinematics_real_tests PRIVATE RELEASE)
gtest_discover_tests(cinematics_real_tests)

add_executable(megacell_real_tests
    cpp_src/megacell_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/megacell.cpp
)
target_link_libraries(megacell_real_tests PRIVATE GTest::gtest_main logger mem fix)
target_include_directories(megacell_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/cfile
)
target_compile_definitions(megacell_real_tests PRIVATE RELEASE)
gtest_discover_tests(megacell_real_tests)

add_executable(debuggraph_real_tests
    cpp_src/debuggraph_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/debuggraph.cpp
)
target_link_libraries(debuggraph_real_tests PRIVATE GTest::gtest_main logger mem fix)
target_include_directories(debuggraph_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
)
target_compile_definitions(debuggraph_real_tests PRIVATE RELEASE)
gtest_discover_tests(debuggraph_real_tests)

add_executable(d3music_real_tests
    cpp_src/d3music_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/d3music.cpp
)
target_link_libraries(d3music_real_tests PRIVATE GTest::gtest_main logger mem fix)
target_include_directories(d3music_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/music
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/stream_audio
    ${PROJECT_SOURCE_DIR}/AudioEncode
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/misc
)
target_compile_definitions(d3music_real_tests PRIVATE RELEASE)
gtest_discover_tests(d3music_real_tests)

# d3movie_real_tests DISABLED: includes d3movie.cpp which references
# Render_preferred_state, rend_SetPreferredState, and bitmap globals
# (paged_in_count/paged_in_num) that are not stubbed. The test needs
# the full renderer + bitmap libraries, which pull in SDL3, GL, etc.
# Re-enable when those stubs are added or the test is restructured.
if(FALSE)
add_executable(d3movie_real_tests
    cpp_src/d3movie_real_tests.cpp
)
target_link_libraries(d3movie_real_tests PRIVATE GTest::gtest_main logger renderer)
target_include_directories(d3movie_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/libmve
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/mem
)
target_compile_definitions(d3movie_real_tests PRIVATE RELEASE)
gtest_discover_tests(d3movie_real_tests)
endif()

add_executable(newpyrogauges_real_tests
    cpp_src/newpyrogauges_real_tests.cpp
)
target_link_libraries(newpyrogauges_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(newpyrogauges_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/bitmap
)
target_compile_definitions(newpyrogauges_real_tests PRIVATE RELEASE)
gtest_discover_tests(newpyrogauges_real_tests)

add_executable(gametexture_real_tests
    cpp_src/gametexture_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gametexture.cpp
)
target_link_libraries(gametexture_real_tests PRIVATE GTest::gtest_main logger mem ddebug)
target_include_directories(gametexture_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/Descent3/lib
)
target_compile_definitions(gametexture_real_tests PRIVATE RELEASE)
gtest_discover_tests(gametexture_real_tests)

add_executable(telcomcargo_real_tests
    cpp_src/telcomcargo_real_tests.cpp
)
target_link_libraries(telcomcargo_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(telcomcargo_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/md
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/Descent3/lib
)
target_compile_definitions(telcomcargo_real_tests PRIVATE RELEASE)
gtest_discover_tests(telcomcargo_real_tests)

add_executable(help_real_tests
    cpp_src/help_real_tests.cpp
)
target_link_libraries(help_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(help_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
)
target_compile_definitions(help_real_tests PRIVATE RELEASE)
gtest_discover_tests(help_real_tests)

add_executable(help_linked_real_tests
    cpp_src/help_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/help.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/newui_core.cpp
)
target_link_libraries(help_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs ui)
target_include_directories(help_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_BINARY_DIR}/lib
)
target_compile_definitions(help_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(help_linked_real_tests)

add_executable(door_linked_real_tests
    cpp_src/door_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/door.cpp
)
target_link_libraries(door_linked_real_tests PRIVATE GTest::gtest_main logger mem ddebug)
target_include_directories(door_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/model
)
target_compile_definitions(door_linked_real_tests PRIVATE RELEASE)
gtest_discover_tests(door_linked_real_tests)

add_executable(scorch_linked_real_tests
    cpp_src/scorch_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/scorch.cpp
)
target_link_libraries(scorch_linked_real_tests PRIVATE GTest::gtest_main logger mem ddebug)
target_include_directories(scorch_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/manage
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/misc
)
target_compile_definitions(scorch_linked_real_tests PRIVATE RELEASE)
gtest_discover_tests(scorch_linked_real_tests)

add_executable(configitem_linked_real_tests
    cpp_src/configitem_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/ConfigItem.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/newui_core.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/newui.cpp
)
target_link_libraries(configitem_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs ui)
target_include_directories(configitem_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_BINARY_DIR}/lib
)
target_compile_definitions(configitem_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(configitem_linked_real_tests)

add_executable(gamecheat_linked_real_tests
    cpp_src/gamecheat_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/GameCheat.cpp
    ${PROJECT_SOURCE_DIR}/tests/cpp_src/fake_hlsystem.cpp
)
target_link_libraries(gamecheat_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(gamecheat_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_BINARY_DIR}/lib
)
target_compile_definitions(gamecheat_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(gamecheat_linked_real_tests)

add_executable(fireball_linked_real_tests
    cpp_src/fireball_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/fireball.cpp
    ${PROJECT_SOURCE_DIR}/tests/cpp_src/fake_hlsystem.cpp
)
target_link_libraries(fireball_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(fireball_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_BINARY_DIR}/lib
)
target_compile_definitions(fireball_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(fireball_linked_real_tests)

add_executable(weaponfire_linked_real_tests
    cpp_src/weaponfire_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/WeaponFire.cpp
    ${PROJECT_SOURCE_DIR}/tests/cpp_src/fake_hlsystem.cpp
)
target_link_libraries(weaponfire_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(weaponfire_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_BINARY_DIR}/lib
)
target_compile_definitions(weaponfire_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(weaponfire_linked_real_tests)

add_executable(dllwrappers_real_tests
    cpp_src/dllwrappers_real_tests.cpp
)
target_link_libraries(dllwrappers_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(dllwrappers_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
)
target_compile_definitions(dllwrappers_real_tests PRIVATE RELEASE)
gtest_discover_tests(dllwrappers_real_tests)

add_executable(sdlmain_real_tests
    cpp_src/sdlmain_real_tests.cpp
)
target_link_libraries(sdlmain_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(sdlmain_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
)
target_compile_definitions(sdlmain_real_tests PRIVATE RELEASE)
gtest_discover_tests(sdlmain_real_tests)

add_executable(buddymenu_real_tests
    cpp_src/buddymenu_real_tests.cpp
)
target_link_libraries(buddymenu_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(buddymenu_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
)
target_compile_definitions(buddymenu_real_tests PRIVATE RELEASE)
gtest_discover_tests(buddymenu_real_tests)

add_executable(newui_filedlg_real_tests
    cpp_src/newui_filedlg_real_tests.cpp
)
target_link_libraries(newui_filedlg_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(newui_filedlg_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/logger
)
target_compile_definitions(newui_filedlg_real_tests PRIVATE RELEASE)
gtest_discover_tests(newui_filedlg_real_tests)

add_executable(slew_real_tests
    cpp_src/slew_real_tests.cpp
)
target_link_libraries(slew_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(slew_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(slew_real_tests PRIVATE RELEASE)
gtest_discover_tests(slew_real_tests)

add_executable(mmitem_real_tests
    cpp_src/mmitem_real_tests.cpp
)
target_link_libraries(mmitem_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(mmitem_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(mmitem_real_tests PRIVATE RELEASE)
gtest_discover_tests(mmitem_real_tests)

add_executable(smallviews_real_tests
    cpp_src/smallviews_real_tests.cpp
)
target_link_libraries(smallviews_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(smallviews_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(smallviews_real_tests PRIVATE RELEASE)
gtest_discover_tests(smallviews_real_tests)

add_executable(cockpit_real_tests
    cpp_src/cockpit_real_tests.cpp
)
target_link_libraries(cockpit_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(cockpit_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(cockpit_real_tests PRIVATE RELEASE)
gtest_discover_tests(cockpit_real_tests)

add_executable(d3forcefeedback_real_tests
    cpp_src/d3forcefeedback_real_tests.cpp
)
target_link_libraries(d3forcefeedback_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(d3forcefeedback_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
)
target_compile_definitions(d3forcefeedback_real_tests PRIVATE RELEASE)
gtest_discover_tests(d3forcefeedback_real_tests)

add_executable(gamecheat_real_tests
    cpp_src/gamecheat_real_tests.cpp
)
target_link_libraries(gamecheat_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(gamecheat_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(gamecheat_real_tests PRIVATE RELEASE)
gtest_discover_tests(gamecheat_real_tests)

add_executable(screens_real_tests
    cpp_src/screens_real_tests.cpp
)
target_link_libraries(screens_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(screens_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(screens_real_tests PRIVATE RELEASE)
gtest_discover_tests(screens_real_tests)

add_executable(multiclient_real_tests
    cpp_src/multiclient_real_tests.cpp
)
target_link_libraries(multiclient_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(multiclient_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(multiclient_real_tests PRIVATE RELEASE)
gtest_discover_tests(multiclient_real_tests)

add_executable(telcomgoals_real_tests
    cpp_src/telcomgoals_real_tests.cpp
)
target_link_libraries(telcomgoals_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(telcomgoals_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(telcomgoals_real_tests PRIVATE RELEASE)
gtest_discover_tests(telcomgoals_real_tests)

add_executable(game2dll_real_tests
    cpp_src/game2dll_real_tests.cpp
)
target_link_libraries(game2dll_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(game2dll_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
)
target_compile_definitions(game2dll_real_tests PRIVATE RELEASE)
gtest_discover_tests(game2dll_real_tests)

add_executable(telcomautomap_real_tests
    cpp_src/telcomautomap_real_tests.cpp
)
target_link_libraries(telcomautomap_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(telcomautomap_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(telcomautomap_real_tests PRIVATE RELEASE)
gtest_discover_tests(telcomautomap_real_tests)

add_executable(terrainsearch_real_tests
    cpp_src/terrainsearch_real_tests.cpp
)
target_link_libraries(terrainsearch_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(terrainsearch_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(terrainsearch_real_tests PRIVATE RELEASE)
gtest_discover_tests(terrainsearch_real_tests)

add_executable(multiconnect_real_tests
    cpp_src/multiconnect_real_tests.cpp
)
target_link_libraries(multiconnect_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(multiconnect_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(multiconnect_real_tests PRIVATE RELEASE)
gtest_discover_tests(multiconnect_real_tests)

add_executable(hotspotmap_real_tests
    cpp_src/hotspotmap_real_tests.cpp
)
target_link_libraries(hotspotmap_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(hotspotmap_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(hotspotmap_real_tests PRIVATE RELEASE)
gtest_discover_tests(hotspotmap_real_tests)

add_executable(object_real_tests
    cpp_src/object_real_tests.cpp
)
target_link_libraries(object_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(object_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(object_real_tests PRIVATE RELEASE)
gtest_discover_tests(object_real_tests)

add_executable(matcen_real_tests
    cpp_src/matcen_real_tests.cpp
)
target_link_libraries(matcen_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(matcen_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(matcen_real_tests PRIVATE RELEASE)
gtest_discover_tests(matcen_real_tests)

add_executable(loadstate_real_tests
    cpp_src/loadstate_real_tests.cpp
)
target_link_libraries(loadstate_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(loadstate_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(loadstate_real_tests PRIVATE RELEASE)
gtest_discover_tests(loadstate_real_tests)

add_executable(viseffect_real_tests
    cpp_src/viseffect_real_tests.cpp
)
target_link_libraries(viseffect_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(viseffect_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(viseffect_real_tests PRIVATE RELEASE)
gtest_discover_tests(viseffect_real_tests)

add_executable(gamecinematics_real_tests
    cpp_src/gamecinematics_real_tests.cpp
)
target_link_libraries(gamecinematics_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(gamecinematics_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(gamecinematics_real_tests PRIVATE RELEASE)
gtest_discover_tests(gamecinematics_real_tests)

add_executable(gamesequence_real_tests
    cpp_src/gamesequence_real_tests.cpp
)
target_link_libraries(gamesequence_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(gamesequence_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(gamesequence_real_tests PRIVATE RELEASE)
gtest_discover_tests(gamesequence_real_tests)

add_executable(fireball_real_tests
    cpp_src/fireball_real_tests.cpp
)
target_link_libraries(fireball_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(fireball_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(fireball_real_tests PRIVATE RELEASE)
gtest_discover_tests(fireball_real_tests)

add_executable(boa_real_tests
    cpp_src/boa_real_tests.cpp
)
target_link_libraries(boa_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(boa_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(boa_real_tests PRIVATE RELEASE)
gtest_discover_tests(boa_real_tests)

add_executable(multisafe_real_tests
    cpp_src/multisafe_real_tests.cpp
)
target_link_libraries(multisafe_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(multisafe_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(multisafe_real_tests PRIVATE RELEASE)
gtest_discover_tests(multisafe_real_tests)

add_executable(multiserver_real_tests
    cpp_src/multiserver_real_tests.cpp
)
target_link_libraries(multiserver_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(multiserver_real_tests)

add_executable(gameloop_real_tests
    cpp_src/gameloop_real_tests.cpp
)
target_link_libraries(gameloop_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(gameloop_real_tests)

add_executable(player_real_tests
    cpp_src/player_real_tests.cpp
)
target_link_libraries(player_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(player_real_tests)

# Linked real tests require GNU ld features (--wrap, --start-group/--end-group)
# that Apple's ld64 does not support. Skip on Apple.
if(NOT APPLE)
add_executable(player_linked_real_tests
    cpp_src/player_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/Player.cpp
    ${PROJECT_SOURCE_DIR}/tests/cpp_src/fake_hlsystem.cpp
)
target_link_libraries(player_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(player_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_BINARY_DIR}/lib
)
target_compile_definitions(player_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(player_linked_real_tests)

add_executable(loadlevel_linked_real_tests
    cpp_src/loadlevel_linked_real_tests.cpp
)
target_link_libraries(loadlevel_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(loadlevel_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(loadlevel_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(loadlevel_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME loadlevel_linked_real_tests COMMAND loadlevel_linked_real_tests)

add_executable(telcom_linked_real_tests
    cpp_src/telcom_linked_real_tests.cpp
)
target_link_libraries(telcom_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(telcom_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(telcom_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(telcom_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME telcom_linked_real_tests COMMAND telcom_linked_real_tests)

add_executable(multi_linked_real_tests
    cpp_src/multi_linked_real_tests.cpp
)
target_link_libraries(multi_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(multi_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(multi_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(multi_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME multi_linked_real_tests COMMAND multi_linked_real_tests)

add_executable(mission_linked_real_tests
    cpp_src/mission_linked_real_tests.cpp
)
target_link_libraries(mission_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(mission_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(mission_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(mission_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME mission_linked_real_tests COMMAND mission_linked_real_tests)

add_executable(pilot_linked_real_tests
    cpp_src/pilot_linked_real_tests.cpp
)
target_link_libraries(pilot_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(pilot_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(pilot_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(pilot_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME pilot_linked_real_tests COMMAND pilot_linked_real_tests)

add_executable(newui_linked_real_tests
    cpp_src/newui_linked_real_tests.cpp
)
target_link_libraries(newui_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(newui_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(newui_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(newui_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME newui_linked_real_tests COMMAND newui_linked_real_tests)

add_executable(osiris_linked_real_tests
    cpp_src/osiris_linked_real_tests.cpp
)
target_link_libraries(osiris_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(osiris_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(osiris_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(osiris_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME osiris_linked_real_tests COMMAND osiris_linked_real_tests)

add_executable(matcen_linked_real_tests
    cpp_src/matcen_linked_real_tests.cpp
)
target_link_libraries(matcen_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(matcen_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(matcen_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(matcen_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME matcen_linked_real_tests COMMAND matcen_linked_real_tests)

add_executable(gamesequence_linked_real_tests
    cpp_src/gamesequence_linked_real_tests.cpp
)
target_link_libraries(gamesequence_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(gamesequence_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(gamesequence_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(gamesequence_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME gamesequence_linked_real_tests COMMAND gamesequence_linked_real_tests)

add_executable(osiris_predefs_linked_real_tests
    cpp_src/osiris_predefs_linked_real_tests.cpp
)
target_link_libraries(osiris_predefs_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(osiris_predefs_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(osiris_predefs_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(osiris_predefs_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME osiris_predefs_linked_real_tests COMMAND osiris_predefs_linked_real_tests)

add_executable(loadstate_linked_real_tests
    cpp_src/loadstate_linked_real_tests.cpp
)
target_link_libraries(loadstate_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(loadstate_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(loadstate_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(loadstate_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME loadstate_linked_real_tests COMMAND loadstate_linked_real_tests)

add_executable(gameloop_linked_real_tests
    cpp_src/gameloop_linked_real_tests.cpp
)
target_link_libraries(gameloop_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(gameloop_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(gameloop_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(gameloop_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME gameloop_linked_real_tests COMMAND gameloop_linked_real_tests)

add_executable(multi_server_linked_real_tests
    cpp_src/multi_server_linked_real_tests.cpp
)
target_link_libraries(multi_server_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(multi_server_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(multi_server_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(multi_server_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME multi_server_linked_real_tests COMMAND multi_server_linked_real_tests)

add_executable(ctlconfig_linked_real_tests
    cpp_src/ctlconfig_linked_real_tests.cpp
)
target_link_libraries(ctlconfig_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(ctlconfig_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(ctlconfig_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(ctlconfig_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME ctlconfig_linked_real_tests COMMAND ctlconfig_linked_real_tests)

add_executable(controls_linked_real_tests
    cpp_src/controls_linked_real_tests.cpp
)
target_link_libraries(controls_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(controls_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(controls_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(controls_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME controls_linked_real_tests COMMAND controls_linked_real_tests)

add_executable(gauges_linked_real_tests
    cpp_src/gauges_linked_real_tests.cpp
)
target_link_libraries(gauges_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(gauges_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(gauges_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(gauges_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME gauges_linked_real_tests COMMAND gauges_linked_real_tests)

add_executable(multi_client_linked_real_tests
    cpp_src/multi_client_linked_real_tests.cpp
)
target_link_libraries(multi_client_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(multi_client_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(multi_client_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(multi_client_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME multi_client_linked_real_tests COMMAND multi_client_linked_real_tests)

add_executable(multi_connect_linked_real_tests
    cpp_src/multi_connect_linked_real_tests.cpp
)
target_link_libraries(multi_connect_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(multi_connect_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(multi_connect_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(multi_connect_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME multi_connect_linked_real_tests COMMAND multi_connect_linked_real_tests)

add_executable(ctlcfgelem_linked_real_tests
    cpp_src/ctlcfgelem_linked_real_tests.cpp
)
target_link_libraries(ctlcfgelem_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(ctlcfgelem_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(ctlcfgelem_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(ctlcfgelem_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME ctlcfgelem_linked_real_tests COMMAND ctlcfgelem_linked_real_tests)

add_executable(multi_dll_mgr_linked_real_tests
    cpp_src/multi_dll_mgr_linked_real_tests.cpp
)
target_link_libraries(multi_dll_mgr_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(multi_dll_mgr_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(multi_dll_mgr_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(multi_dll_mgr_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME multi_dll_mgr_linked_real_tests COMMAND multi_dll_mgr_linked_real_tests)

add_executable(mission_download_linked_real_tests
    cpp_src/mission_download_linked_real_tests.cpp
)
target_link_libraries(mission_download_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(mission_download_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(mission_download_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(mission_download_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME mission_download_linked_real_tests COMMAND mission_download_linked_real_tests)

add_executable(multisafe_server_linked_real_tests
    cpp_src/multisafe_server_linked_real_tests.cpp
)
target_link_libraries(multisafe_server_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(multisafe_server_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(multisafe_server_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(multisafe_server_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME multisafe_server_linked_real_tests COMMAND multisafe_server_linked_real_tests)

add_executable(osirisloadandbind_linked_real_tests
    cpp_src/osirisloadandbind_linked_real_tests.cpp
)
target_link_libraries(osirisloadandbind_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(osirisloadandbind_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(osirisloadandbind_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(osirisloadandbind_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME osirisloadandbind_linked_real_tests COMMAND osirisloadandbind_linked_real_tests)

add_executable(soar_helpers_linked_real_tests
    cpp_src/soar_helpers_linked_real_tests.cpp
)
target_link_libraries(soar_helpers_linked_real_tests PRIVATE
    GTest::gtest_main
    -Wl,--start-group
    descent3
    2dlib AudioEncode bitmap cfile ddebug ddio fix grtext inetfile libmve libacm
    linux logger manage mem md5 misc model module music networking physics
    renderer rtperformance sndlib stream_audio ui unzip vecmat m ${CMAKE_DL_LIBS}
    -Wl,--end-group)
target_link_options(soar_helpers_linked_real_tests PRIVATE LINKER:--wrap,_ZN5pilot10initializeEv)
target_include_directories(soar_helpers_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/md5
    ${PROJECT_SOURCE_DIR}/build/lib
)
target_compile_definitions(soar_helpers_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ MEM_USE_RTL PRIMARY_HOG="d3-linux.hog")
add_test(NAME soar_helpers_linked_real_tests COMMAND soar_helpers_linked_real_tests)
endif() # NOT APPLE

add_executable(loadlevel_real_tests
    cpp_src/loadlevel_real_tests.cpp
)
target_link_libraries(loadlevel_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(loadlevel_real_tests)

add_executable(telcom_real_tests
    cpp_src/telcom_real_tests.cpp
)
target_link_libraries(telcom_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(telcom_real_tests)

add_executable(osirisload_real_tests
    cpp_src/osirisload_real_tests.cpp
)
target_link_libraries(osirisload_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(osirisload_real_tests)

add_executable(aimain_real_tests
    cpp_src/aimain_real_tests.cpp
)
target_link_libraries(aimain_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(aimain_real_tests)

add_executable(weaponfire_real_tests
    cpp_src/weaponfire_real_tests.cpp
)
target_link_libraries(weaponfire_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(weaponfire_real_tests)

add_executable(pilot_real_tests
    cpp_src/pilot_real_tests.cpp
)
target_link_libraries(pilot_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(pilot_real_tests)

add_executable(multi_real_tests
    cpp_src/multi_real_tests.cpp
)
target_link_libraries(multi_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(multi_real_tests)

add_executable(terrainrender_real_tests
    cpp_src/terrainrender_real_tests.cpp
)
target_link_libraries(terrainrender_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(terrainrender_real_tests)

add_executable(osiris_predefs_real_tests
    cpp_src/osiris_predefs_real_tests.cpp
)
target_link_libraries(osiris_predefs_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(osiris_predefs_real_tests)

add_executable(room_real_tests
    cpp_src/room_real_tests.cpp
)
target_link_libraries(room_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(room_real_tests)

add_executable(damage_real_tests
    cpp_src/damage_real_tests.cpp
)
target_link_libraries(damage_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(damage_real_tests)

add_executable(levelgoal_real_tests
    cpp_src/levelgoal_real_tests.cpp
)
target_link_libraries(levelgoal_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(levelgoal_real_tests)

add_executable(bsp_real_tests
    cpp_src/bsp_real_tests.cpp
)
target_link_libraries(bsp_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(bsp_real_tests)

add_executable(terrain_real_tests
    cpp_src/terrain_real_tests.cpp
)
target_link_libraries(terrain_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(terrain_real_tests)

add_executable(aigoal_real_tests
    cpp_src/aigoal_real_tests.cpp
)
target_link_libraries(aigoal_real_tests PRIVATE GTest::gtest_main logger)
gtest_discover_tests(aigoal_real_tests)

add_executable(gamesave_real_tests
    cpp_src/gamesave_real_tests.cpp
)
target_link_libraries(gamesave_real_tests PRIVATE GTest::gtest_main logger cfile)
target_include_directories(gamesave_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
)
target_compile_definitions(gamesave_real_tests PRIVATE RELEASE)
gtest_discover_tests(gamesave_real_tests)

add_executable(procedurals_real_tests
    cpp_src/procedurals_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/procedurals.cpp
)
target_link_libraries(procedurals_real_tests PRIVATE GTest::gtest_main logger mem cfile bitmap ddebug)
target_include_directories(procedurals_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
)
target_compile_definitions(procedurals_real_tests PRIVATE RELEASE)
gtest_discover_tests(procedurals_real_tests)

add_executable(hud_real_tests
    cpp_src/hud_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/hud.cpp
)
target_link_libraries(hud_real_tests PRIVATE GTest::gtest_main logger mem cfile ddebug)
target_include_directories(hud_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/ui
)
target_compile_definitions(hud_real_tests PRIVATE RELEASE)
gtest_discover_tests(hud_real_tests)

add_executable(telcomeffects_real_tests
    cpp_src/telcomeffects_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/TelComEffects.cpp
)
target_link_libraries(telcomeffects_real_tests PRIVATE GTest::gtest_main logger mem cfile ddebug)
target_include_directories(telcomeffects_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/sndlib
)
target_compile_definitions(telcomeffects_real_tests PRIVATE RELEASE)
gtest_discover_tests(telcomeffects_real_tests)

add_executable(objectlighting_real_tests
    cpp_src/objectlighting_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/object_lighting.cpp
)
target_link_libraries(objectlighting_real_tests PRIVATE GTest::gtest_main logger mem cfile ddebug)
target_include_directories(objectlighting_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/physics
)
target_compile_definitions(objectlighting_real_tests PRIVATE RELEASE)
gtest_discover_tests(objectlighting_real_tests)

add_executable(objinit_real_tests
    cpp_src/objinit_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/ObjInit.cpp
)
target_link_libraries(objinit_real_tests PRIVATE GTest::gtest_main logger mem vecmat)
target_include_directories(objinit_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(objinit_real_tests PRIVATE RELEASE POSIX __LINUX__)
gtest_discover_tests(objinit_real_tests)

add_executable(aipath_real_tests
    cpp_src/aipath_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/aipath.cpp
)
target_link_libraries(aipath_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(aipath_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(aipath_real_tests PRIVATE RELEASE POSIX __LINUX__)
gtest_discover_tests(aipath_real_tests)

add_executable(telcomeffectsrender_real_tests
    cpp_src/telcomeffectsrender_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/TelComEffects.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/TelcomEffectsRender.cpp
)
target_link_libraries(telcomeffectsrender_real_tests PRIVATE GTest::gtest_main logger mem cfile ddebug vecmat)
target_include_directories(telcomeffectsrender_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(telcomeffectsrender_real_tests PRIVATE RELEASE POSIX __LINUX__)
gtest_discover_tests(telcomeffectsrender_real_tests)

add_executable(huddisplay_real_tests
    cpp_src/huddisplay_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/huddisplay.cpp
)
target_link_libraries(huddisplay_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat)
target_include_directories(huddisplay_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(huddisplay_real_tests PRIVATE RELEASE POSIX __LINUX__)
gtest_discover_tests(huddisplay_real_tests)

add_executable(descent_real_tests
    cpp_src/descent_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/descent.cpp
)
target_link_libraries(descent_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(descent_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(descent_real_tests PRIVATE RELEASE POSIX __LINUX__)
gtest_discover_tests(descent_real_tests)

add_executable(renderobject_real_tests
    cpp_src/renderobject_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/renderobject.cpp
)
target_link_libraries(renderobject_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat)
target_include_directories(renderobject_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(renderobject_real_tests PRIVATE RELEASE POSIX __LINUX__)
gtest_discover_tests(renderobject_real_tests)

add_executable(dedicated_server_real_tests
    cpp_src/dedicated_server_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/dedicated_server.cpp
)
target_link_libraries(dedicated_server_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat)
target_include_directories(dedicated_server_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(dedicated_server_real_tests PRIVATE RELEASE POSIX __LINUX__)
gtest_discover_tests(dedicated_server_real_tests)

add_executable(demofile_real_tests
    cpp_src/demofile_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/demofile.cpp
)
target_link_libraries(demofile_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(demofile_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(demofile_real_tests PRIVATE RELEASE POSIX __LINUX__)
gtest_discover_tests(demofile_real_tests)

add_executable(init_real_tests
    cpp_src/init_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/init.cpp
)
target_link_libraries(init_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(init_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(init_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(init_real_tests)

add_executable(multi_ui_real_tests
    cpp_src/multi_ui_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/multi_ui.cpp
)
target_link_libraries(multi_ui_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(multi_ui_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(multi_ui_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(multi_ui_real_tests)

add_executable(gamefile_real_tests
    cpp_src/gamefile_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gamefile.cpp
)
target_link_libraries(gamefile_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(gamefile_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(gamefile_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(gamefile_real_tests)

add_executable(gamepath_real_tests
    cpp_src/gamepath_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/gamepath.cpp
)
target_link_libraries(gamepath_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(gamepath_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(gamepath_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(gamepath_real_tests)

add_executable(door_real_tests
    cpp_src/door_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/door.cpp
)
target_link_libraries(door_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(door_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(door_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(door_real_tests)

add_executable(ship_real_tests
    cpp_src/ship_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/ship.cpp
)
target_link_libraries(ship_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(ship_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(ship_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(ship_real_tests)

add_executable(pilotpicsapi_real_tests
    cpp_src/pilotpicsapi_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/PilotPicsAPI.cpp
)
target_link_libraries(pilotpicsapi_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(pilotpicsapi_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(pilotpicsapi_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(pilotpicsapi_real_tests)

add_executable(weapon_real_tests
    cpp_src/weapon_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/weapon.cpp
)
target_link_libraries(weapon_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(weapon_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(weapon_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(weapon_real_tests)

add_executable(newuicore_real_tests
    cpp_src/newuicore_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/newui_core.cpp
)
target_link_libraries(newuicore_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs ui)
target_include_directories(newuicore_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(newuicore_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(newuicore_real_tests)

add_executable(config_real_tests
    cpp_src/config_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/config.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/newui_core.cpp
)
target_link_libraries(config_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs ui)

add_executable(menu_real_tests
    cpp_src/menu_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/menu.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/newui_core.cpp
)
target_link_libraries(menu_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs ui)

target_include_directories(menu_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(menu_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(menu_real_tests)

add_executable(game_real_tests
    cpp_src/game_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/game.cpp
)
target_link_libraries(game_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat bitmap stdcppfs)
target_include_directories(game_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(game_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(game_real_tests)

add_executable(bnode_linked_real_tests
    cpp_src/bnode_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/bnode.cpp
)
target_link_libraries(bnode_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(bnode_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(bnode_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(bnode_linked_real_tests)

add_executable(boa_linked_real_tests
    cpp_src/boa_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/BOA.cpp
)
target_link_libraries(boa_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(boa_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(boa_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(boa_linked_real_tests)

add_executable(aigoal_linked_real_tests
    cpp_src/aigoal_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/AIGoal.cpp
)
target_link_libraries(aigoal_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(aigoal_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(aigoal_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(aigoal_linked_real_tests)

add_executable(lighting_linked_real_tests
    cpp_src/lighting_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/lighting.cpp
)
target_link_libraries(lighting_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(lighting_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(lighting_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(lighting_linked_real_tests)

add_executable(terrainrender_linked_real_tests
    cpp_src/terrainrender_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/terrainrender.cpp
)
target_link_libraries(terrainrender_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat stdcppfs)
target_include_directories(terrainrender_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(terrainrender_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(terrainrender_linked_real_tests)

add_executable(damage_linked_real_tests
    cpp_src/damage_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/damage.cpp
)
target_link_libraries(damage_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat misc stdcppfs)
target_include_directories(damage_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(damage_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(damage_linked_real_tests)

add_executable(object_linked_real_tests
    cpp_src/object_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/object.cpp
)
target_link_libraries(object_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat misc stdcppfs)
target_include_directories(object_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(object_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(object_linked_real_tests)

add_executable(aimain_linked_real_tests
    cpp_src/aimain_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/AImain.cpp
)
target_link_libraries(aimain_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat misc stdcppfs)
target_include_directories(aimain_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(aimain_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(aimain_linked_real_tests)

add_executable(room_linked_real_tests
    cpp_src/room_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/room.cpp
)
target_link_libraries(room_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat misc stdcppfs)
target_include_directories(room_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(room_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(room_linked_real_tests)

add_executable(bsp_linked_real_tests
    cpp_src/bsp_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/bsp.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/list.cpp
)
target_link_libraries(bsp_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat misc cfile ddebug stdcppfs)
target_include_directories(bsp_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(bsp_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(bsp_linked_real_tests)

add_executable(terrain_linked_real_tests
    cpp_src/terrain_linked_real_tests.cpp
    ${PROJECT_SOURCE_DIR}/Descent3/terrain.cpp
)
target_link_libraries(terrain_linked_real_tests PRIVATE GTest::gtest_main logger mem fix vecmat misc cfile bitmap ddebug stdcppfs)
target_include_directories(terrain_linked_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(terrain_linked_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(terrain_linked_real_tests)

target_include_directories(config_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/bitmap
    ${PROJECT_SOURCE_DIR}/2dlib
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/misc
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/grtext
    ${PROJECT_SOURCE_DIR}/physics
    ${PROJECT_SOURCE_DIR}/sndlib
    ${PROJECT_SOURCE_DIR}/vecmat
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/module
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/rtperformance
    ${CMAKE_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/builds/linux/vcpkg_installed/x64-linux/include
)
target_compile_definitions(config_real_tests PRIVATE RELEASE POSIX __LINUX__ PRIMARY_HOG="d3-linux.hog")
gtest_discover_tests(config_real_tests)

add_executable(mission_real_tests
    cpp_src/mission_real_tests.cpp
)
target_link_libraries(mission_real_tests PRIVATE GTest::gtest_main logger)
target_include_directories(mission_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Descent3
)
target_compile_definitions(mission_real_tests PRIVATE RELEASE)
gtest_discover_tests(mission_real_tests)

# ============================================================================
# CFile Mock Tests
# Uses OBJECT library to ensure mock symbols take precedence over real cfile
# ============================================================================

# Mock cfile implementation (compiled as OBJECT - linked FIRST in tests)
add_library(mock_cfile OBJECT
    cpp_src/mock_cfile.cpp
)
target_include_directories(mock_cfile PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# mock_cfile.hpp includes <gmock/gmock.h>; link GTest::gmock to get its
# include directory propagated to this object library.
target_link_libraries(mock_cfile PRIVATE GTest::gmock)

# CFile mock tests - linked with mock object FIRST (wins symbol conflicts)
add_executable(cfile_mock_tests
    cpp_src/cfile_mock_tests.cpp
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
    cpp_src/cfile_tests_standalone.cpp
)

target_compile_definitions(cfile_coverage_tests PRIVATE D3_MOCK_CFILE_STANDALONE)
target_link_libraries(cfile_coverage_tests PRIVATE
    $<TARGET_OBJECTS:mock_cfile>
    GTest::gtest_main
)

gtest_discover_tests(cfile_coverage_tests)

# ============================================================================
# CFile Full Mock Library (enhanced with HOG library and path support)
# ============================================================================

add_library(mock_cfile_full OBJECT
    cpp_src/mock_cfile_full.cpp
)
target_include_directories(mock_cfile_full PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# mock_cfile_full.hpp includes <gmock/gmock.h>; link GTest::gmock to get its
# include directory propagated to this object library.
target_link_libraries(mock_cfile_full PRIVATE GTest::gmock)

# ============================================================================
# CFile Integration Tests (using full mock with HOG library support)
# ============================================================================

add_executable(cfile_integration_tests
    cpp_src/cfile_integration_tests.cpp
)

target_compile_definitions(cfile_integration_tests PRIVATE D3_MOCK_CFILE_STANDALONE)
target_link_libraries(cfile_integration_tests PRIVATE
    $<TARGET_OBJECTS:mock_cfile_full>
    GTest::gtest_main
    GTest::gmock
)

gtest_discover_tests(cfile_integration_tests)

# cfile_real_tests uses --wrap for stdio functions (GNU ld only)
if(NOT APPLE)
# ============================================================================
# CFile Real Tests with Linker-Wrapped C IO Functions
# Tests the REAL cfile library with intercepted C stdio functions
# ============================================================================

# Compile cfile sources directly so linker wrapping applies to them
if(WIN32)
  set(CFILE_REAL_TEST_DDIO_SOURCES
      ${PROJECT_SOURCE_DIR}/ddio/winfile.cpp)
else()
  set(CFILE_REAL_TEST_DDIO_SOURCES
      ${PROJECT_SOURCE_DIR}/ddio/lnxfile.cpp)
endif()
add_executable(cfile_real_tests
    cpp_src/cfile_real_tests.cpp
    cpp_src/cfile_io_wrappers.cpp
    # Don't compile stubs.c - we link real ddebug library instead
    ${PROJECT_SOURCE_DIR}/cfile/cfile.cpp
    ${PROJECT_SOURCE_DIR}/cfile/hogfile.cpp
    ${PROJECT_SOURCE_DIR}/cfile/inffile.cpp
    # Compile ddio file functions directly so they use wrapped C functions
    ${CFILE_REAL_TEST_DDIO_SOURCES}
)

target_include_directories(cfile_real_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${PROJECT_SOURCE_DIR}/ddio
    ${PROJECT_SOURCE_DIR}/linux
    ${PROJECT_SOURCE_DIR}/logger
    ${PROJECT_SOURCE_DIR}/mem
    ${PROJECT_SOURCE_DIR}/ddebug
    ${PROJECT_SOURCE_DIR}/misc  # For pstring.h
    ${PROJECT_BINARY_DIR}/cfile
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Need to link libraries for cfile dependencies
target_link_libraries(cfile_real_tests PRIVATE
    ddebug
    ddio
    mem
    misc
    GTest::gtest_main
    GTest::gmock
)

# Linker wrapping for C stdio functions - applies to cfile/ddio sources compiled above
target_link_options(cfile_real_tests PRIVATE
    "LINKER:--wrap=fopen"
    "LINKER:--wrap=fclose"
    "LINKER:--wrap=fread"
    "LINKER:--wrap=fwrite"
    "LINKER:--wrap=fseek"
    "LINKER:--wrap=ftell"
    "LINKER:--wrap=feof"
    "LINKER:--wrap=rewind"
    "LINKER:--wrap=fputc"
    "LINKER:--wrap=vfprintf"
    "LINKER:--wrap=fileno"
    "LINKER:--wrap=fstat"
)

gtest_discover_tests(cfile_real_tests)
endif() # NOT APPLE

# ============================================================================
# Stub libraries for dependencies (ddebug, ddio, args)
# ============================================================================

# C stubs (no name mangling)
add_library(test_stubs_c OBJECT
    cpp_src/stubs.c
)
target_include_directories(test_stubs_c PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
)

# C++ stubs (cfile functions) - compiled as static lib with C linkage
add_library(test_stubs_cpp STATIC
    cpp_src/mock_cfile.cpp
)
target_include_directories(test_stubs_cpp PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/fix
    ${PROJECT_SOURCE_DIR}/cfile
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# mock_cfile.cpp includes mock_cfile.hpp which includes <gmock/gmock.h>
target_link_libraries(test_stubs_cpp PRIVATE GTest::gmock)
# Force C linkage for all cfile functions
target_compile_options(test_stubs_cpp PRIVATE -x c++)

# ============================================================================
# Bitmap Allocation Tests
# Disabled: tests expect bm_used() and bm_AllocLoadIFFAnim() which are not in
# the public lib/bitmap.h API. Re-enable when those are added to the bitmap library.
# ============================================================================
# add_executable(bitmap_alloc_tests
#     cpp_src/bitmap_alloc_tests.cpp
#     cpp_src/stubs.c
#     cpp_src/mock_cfile.cpp
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
# Uses --allow-multiple-definition (GNU ld only); skip on Apple.
# ============================================================================

if(NOT APPLE)
add_executable(bitmap_loader_tests
    cpp_src/bitmap_loader_tests.cpp
    cpp_src/stubs.c
)

# Mock and stubs first so allow-multiple-definition uses them over cfile/ddebug/ddio from bitmap
target_link_options(bitmap_loader_tests PRIVATE ${ALLOW_MULTI_DEF_LINKER_FLAG})
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
endif()

# ============================================================================
# IFF Loader Tests
# Tests IFF format (PBM/ILBM) loading from in-memory buffers
# Uses --allow-multiple-definition (GNU ld only); skip on Apple.
# ============================================================================

if(NOT APPLE)
add_executable(iff_loader_tests
    cpp_src/iff_loader_tests.cpp
    cpp_src/stubs.c
)

target_link_options(iff_loader_tests PRIVATE ${ALLOW_MULTI_DEF_LINKER_FLAG})
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
endif()

# ============================================================================
# PCX Loader Tests
# Tests PCX format (8-bit and 24-bit) loading from in-memory buffers
# Uses --allow-multiple-definition (GNU ld only); skip on Apple.
# ============================================================================

if(NOT APPLE)
add_executable(pcx_loader_tests
    cpp_src/pcx_loader_tests.cpp
    cpp_src/stubs.c
)

target_link_options(pcx_loader_tests PRIVATE ${ALLOW_MULTI_DEF_LINKER_FLAG})
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
endif()

# ============================================================================
# Manage Tests (tracklock functions)
# Tests manage module's tracklock functionality
# Uses minimal tracklock implementation to avoid heavy dependencies
# Uses --allow-multiple-definition (GNU ld only); skip on Apple.
# ============================================================================

if(NOT APPLE)
add_executable(manage_tests
    cpp_src/manage_tests.cpp
    cpp_src/manage_tracklocks.cpp
    cpp_src/stubs.c
)

target_link_options(manage_tests PRIVATE ${ALLOW_MULTI_DEF_LINKER_FLAG})
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
endif()

# ============================================================================
# PART 4: DESCENT3/ SUB-MODULES - External Header Tests
# These tests verify constants and flags defined in external headers
# ============================================================================

# Weapon external tests (weapon indices)
add_executable(weapon_external_tests
    cpp_src/weapon_external_tests_standalone.cpp
)
target_link_libraries(weapon_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(weapon_external_tests)

# Damage external tests (damage types)
add_executable(damage_external_tests
    cpp_src/damage_external_tests_standalone.cpp
)
target_link_libraries(damage_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(damage_external_tests)

# LevelGoal external tests (goal flags and masks)
add_executable(levelgoal_external_tests
    cpp_src/levelgoal_external_tests_standalone.cpp
)
target_link_libraries(levelgoal_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(levelgoal_external_tests)

# Room external tests (room/face/portal flags)
add_executable(room_external_tests
    cpp_src/room_external_tests_standalone.cpp
)
target_link_libraries(room_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(room_external_tests)

# Object external tests (object types and flags)
add_executable(object_external_tests
    cpp_src/object_external_tests_standalone.cpp
)
target_link_libraries(object_external_tests PRIVATE
    GTest::gtest_main
)
gtest_discover_tests(object_external_tests)

# Object constants tests (from object.h - handle masks, lighting types, flags)
add_executable(object_constants_tests
    cpp_src/object_constants_tests.cpp
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
    cpp_src/player_controls_constants_tests.cpp
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
    cpp_src/ship_constants_tests.cpp
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
    cpp_src/weapon_constants_tests.cpp
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
    cpp_src/ai_constants_tests.cpp
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
    cpp_src/world_constants_tests.cpp
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
    cpp_src/gamestate_constants_tests.cpp
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
    cpp_src/pilot_config_constants_tests.cpp
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
    cpp_src/othersystems_constants_tests.cpp
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
    cpp_src/telcom_constants_tests.cpp
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
    cpp_src/vfx_constants_tests.cpp
)
target_link_libraries(vfx_constants_tests PRIVATE
    GTest::gtest_main
)
target_include_directories(vfx_constants_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/Descent3
)
gtest_discover_tests(vfx_constants_tests)

# Consolidated render tests using OffscreenGL + Mesa llvmpipe
# Only build on Linux where PkgConfig + EGL + Mesa are available
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_subdirectory(render)
endif()

# Pre-existing known test failures (runtime crashes / incomplete stubs).
# Cannot use set_tests_properties here because gtest_discover_tests
# discovers tests at build time, not configure time.
# Instead, exclude them via ctest -E in the CI workflow (.github/workflows/build.yml).
