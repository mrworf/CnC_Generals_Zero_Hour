cmake_minimum_required(VERSION 3.25)

project(GeneralsZeroHourNative VERSION 0.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ZhHostChecks)
include(ZhDependencies)

zh_require_x86_64("${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Zero Hour host architecture: ${CMAKE_SYSTEM_PROCESSOR}")

option(ZH_BUILD_TESTS "Build asset-free native-port tests" ON)
option(ZH_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ZH_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ZH_ENABLE_LAN "Build LAN support when its milestone is available" ON)
option(ZH_ENABLE_GPU_TESTS "Enable tests that require a real GPU" OFF)
option(ZH_ENABLE_RETAIL_TESTS "Enable tests that require user-owned retail data" OFF)

set(ZH_RETAIL_ZH_DATA "" CACHE PATH "Local Zero Hour retail data root (retail tests only)")
set(ZH_RETAIL_GENERALS_DATA "" CACHE PATH "Local Generals retail data root (retail tests only)")
set(ZH_RETAIL_LANGUAGE "" CACHE STRING "Retail language name (retail tests only)")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

zh_find_distribution_dependencies()
find_package(Threads REQUIRED)

execute_process(
  COMMAND git rev-parse --short=12 HEAD
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  RESULT_VARIABLE ZH_GIT_RESULT
  OUTPUT_VARIABLE ZH_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(NOT ZH_GIT_RESULT EQUAL 0)
  set(ZH_GIT_REVISION "unknown")
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
configure_file(cmake/build_metadata.h.in generated/build_metadata.h @ONLY)

add_library(zh_build_options INTERFACE)
target_compile_options(zh_build_options INTERFACE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fno-fast-math;-ffp-contract=off>
)
if(ZH_ENABLE_ASAN)
  target_compile_options(zh_build_options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(zh_build_options INTERFACE -fsanitize=address)
endif()
if(ZH_ENABLE_UBSAN)
  target_compile_options(zh_build_options INTERFACE -fsanitize=undefined -fno-omit-frame-pointer)
  target_link_options(zh_build_options INTERFACE -fsanitize=undefined)
endif()

include(cmake/LegacySourceInventory.cmake)

function(zh_add_bootstrap_component target inventory_variable)
  add_library(${target} STATIC src/bootstrap/component.cpp)
  target_compile_definitions(${target} PRIVATE
    ZH_COMPONENT_NAME="${target}"
    ZH_COMPONENT_SYMBOL=${target}_bootstrap_component
  )
  target_link_libraries(${target} PUBLIC zh_build_options)
  set_property(TARGET ${target} PROPERTY ZH_LEGACY_SOURCE_INVENTORY "${${inventory_variable}}")
endfunction()

zh_add_bootstrap_component(zh_compression ZH_LEGACY_ZH_COMPRESSION_SOURCES)
zh_add_bootstrap_component(zh_wwsupport ZH_LEGACY_ZH_WWSUPPORT_SOURCES)
zh_add_bootstrap_component(zh_w3d ZH_LEGACY_ZH_W3D_SOURCES)
zh_add_bootstrap_component(zh_wwshade ZH_LEGACY_ZH_WWSHADE_SOURCES)
zh_add_bootstrap_component(zh_game_engine ZH_LEGACY_ZH_GAME_ENGINE_SOURCES)
zh_add_bootstrap_component(zh_platform_sdl ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_renderer_sdl_gpu ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_audio_miniaudio ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_video_ffmpeg ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_os_posix ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_game_device ZH_LEGACY_ZH_GAME_DEVICE_SOURCES)
zh_add_bootstrap_component(zh_null_platform ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_null_renderer ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_null_audio ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_null_video ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_test_support ZH_EMPTY_LEGACY_SOURCES)

target_link_libraries(zh_compression PUBLIC ZLIB::ZLIB)
target_link_libraries(zh_wwsupport PUBLIC Freetype::Freetype Fontconfig::Fontconfig)
target_link_libraries(zh_w3d PUBLIC zh_wwsupport)
target_link_libraries(zh_wwshade PUBLIC zh_w3d)
target_link_libraries(zh_game_engine PUBLIC zh_compression zh_wwsupport)
target_link_libraries(zh_platform_sdl PUBLIC SDL3::SDL3)
target_link_libraries(zh_renderer_sdl_gpu PUBLIC SDL3::SDL3 zh_w3d zh_wwshade)
target_link_libraries(zh_video_ffmpeg PUBLIC FFmpeg::avformat FFmpeg::avcodec FFmpeg::avutil FFmpeg::swscale FFmpeg::swresample zh_audio_miniaudio)
target_link_libraries(zh_os_posix PUBLIC Threads::Threads)
target_link_libraries(zh_game_device PUBLIC zh_platform_sdl zh_renderer_sdl_gpu zh_audio_miniaudio zh_video_ffmpeg zh_os_posix)

set(ZH_BOOTSTRAP_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/shaders/bootstrap/bootstrap.vert")
set(ZH_BOOTSTRAP_SPIRV "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/bootstrap.vert.spv")
add_custom_command(
  OUTPUT "${ZH_BOOTSTRAP_SPIRV}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders"
  COMMAND "${ZH_GLSLC_EXECUTABLE}" -fshader-stage=vert "${ZH_BOOTSTRAP_SHADER}" -o "${ZH_BOOTSTRAP_SPIRV}"
  DEPENDS "${ZH_BOOTSTRAP_SHADER}"
  COMMENT "Compiling bootstrap GLSL to SPIR-V"
  VERBATIM
)
add_custom_target(zh_bootstrap_shaders ALL DEPENDS "${ZH_BOOTSTRAP_SPIRV}")
add_dependencies(zh_renderer_sdl_gpu zh_bootstrap_shaders)

add_executable(zh_main src/bootstrap/main.cpp)
target_include_directories(zh_main PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(zh_main PRIVATE
  zh_build_options zh_game_engine zh_game_device zh_null_platform zh_null_renderer
  zh_null_audio zh_null_video zh_test_support
)
set_property(TARGET zh_main PROPERTY ZH_LEGACY_SOURCE_INVENTORY "${ZH_LEGACY_ZH_MAIN_SOURCES}")

install(TARGETS zh_main RUNTIME DESTINATION bin)
install(FILES "${ZH_BOOTSTRAP_SPIRV}" DESTINATION share/generals-zero-hour/shaders)
install(FILES LICENSE.md DESTINATION share/licenses/generals-zero-hour)

if(ZH_BUILD_TESTS)
  enable_testing()
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_test(NAME m0_bootstrap_smoke COMMAND zh_main --bootstrap-smoke)
  set_tests_properties(m0_bootstrap_smoke PROPERTIES LABELS foundation PASS_REGULAR_EXPRESSION "asset-free bootstrap graph: ok")
  add_test(NAME m0_bootstrap_rejects_gameplay COMMAND zh_main)
  set_tests_properties(m0_bootstrap_rejects_gameplay PROPERTIES LABELS foundation WILL_FAIL TRUE)
  add_test(NAME m0_manifest_inventory COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/legacy_manifest_inventory.py" --check)
  set_tests_properties(m0_manifest_inventory PROPERTIES LABELS foundation)
  add_test(NAME m0_manifest_inventory_negative COMMAND "${Python3_EXECUTABLE}" -m unittest tests.m0.test_legacy_manifest_inventory.LegacyManifestInventoryTest.test_incomplete_inventory_is_rejected)
  set_tests_properties(m0_manifest_inventory_negative PROPERTIES LABELS foundation WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME m0_unsupported_architecture_probe COMMAND "${CMAKE_COMMAND}"
    -DZH_PROBE_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/tests/m0/probe_architecture.cmake
    -DZH_PROBE_ARGUMENT=-DZH_PROBE_PROCESSOR=aarch64
    "-DZH_EXPECTED=Unsupported architecture 'aarch64'"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/m0/expect_probe_failure.cmake")
  set_tests_properties(m0_unsupported_architecture_probe PROPERTIES LABELS foundation)
  add_test(NAME m0_missing_glslc_probe COMMAND "${CMAKE_COMMAND}"
    -DZH_PROBE_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/tests/m0/probe_glslc.cmake
    -DZH_PROBE_ARGUMENT=-DZH_DEPENDENCY_PROBE_EMPTY=ON
    "-DZH_EXPECTED=Missing required program 'glslc'"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/m0/expect_probe_failure.cmake")
  set_tests_properties(m0_missing_glslc_probe PROPERTIES LABELS foundation)
endif()
