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
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/data/corpus/english-manifest.tsv" ZH_CORPUS_MANIFEST_TEXT)
configure_file(cmake/corpus_manifest.h.in generated/corpus_manifest.h @ONLY)

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

add_library(zh_data STATIC src/data/config.cpp src/data/big_archive.cpp src/data/vfs.cpp src/data/inventory.cpp)
target_include_directories(zh_data PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_data PUBLIC zh_os_posix zh_build_options)

add_library(zh_persistence STATIC src/persistence/save.cpp src/persistence/replay.cpp)
target_include_directories(zh_persistence PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_persistence PUBLIC zh_wwsupport zh_build_options)

add_library(zh_simulation STATIC src/simulation/determinism.cpp)
target_include_directories(zh_simulation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_simulation PUBLIC zh_persistence zh_build_options)

add_library(zh_lan STATIC
  src/lan/harness.cpp
  src/lan/posix_udp.cpp
  src/lan/protocol.cpp
  src/lan/session.cpp
  src/lan/virtual_transport.cpp
)
target_include_directories(zh_lan PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_lan PUBLIC zh_wwsupport zh_os_posix zh_build_options)

target_sources(zh_compression PRIVATE src/foundation/compression.cpp)
target_include_directories(zh_compression PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(zh_wwsupport PRIVATE
  src/foundation/byte_codec.cpp
  src/foundation/format.cpp
  src/foundation/numeric.cpp
  src/foundation/unicode.cpp
)
target_include_directories(zh_wwsupport PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(zh_renderer_sdl_gpu PRIVATE
  src/renderer/contract.cpp
  src/renderer/dx8_state_cache.cpp
  src/renderer/recording_device.cpp
  src/renderer/shader_registry.cpp
  src/renderer/texture_loader.cpp
)
target_include_directories(zh_renderer_sdl_gpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(zh_ui STATIC src/ui/renderer.cpp src/ui/font.cpp src/ui/session.cpp)
target_include_directories(zh_ui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_ui PUBLIC zh_renderer_sdl_gpu zh_platform_sdl zh_data zh_build_options Freetype::Freetype Fontconfig::Fontconfig)

add_library(zh_world STATIC src/world/scene.cpp src/world/recorder.cpp src/world/corpus.cpp)
target_include_directories(zh_world PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_world PUBLIC zh_renderer_sdl_gpu zh_build_options)

add_library(zh_effects STATIC src/effects/registry.cpp src/effects/recorder.cpp)
target_include_directories(zh_effects PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_effects PUBLIC zh_renderer_sdl_gpu zh_wwshade zh_build_options)

target_sources(zh_platform_sdl PRIVATE src/platform/sdl_events.cpp src/platform/sdl_window.cpp)
target_include_directories(zh_platform_sdl PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_platform_sdl PUBLIC zh_wwsupport)

target_sources(zh_os_posix PRIVATE src/foundation/platform.cpp)
target_include_directories(zh_os_posix PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(zh_null_platform PRIVATE src/headless/null_platform.cpp)
target_sources(zh_null_renderer PRIVATE src/headless/null_renderer.cpp)
target_sources(zh_null_audio PRIVATE src/headless/null_audio.cpp)
target_sources(zh_null_video PRIVATE src/headless/null_video.cpp)
foreach(null_target zh_null_platform zh_null_renderer zh_null_audio zh_null_video)
  target_include_directories(${null_target} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
endforeach()

target_link_libraries(zh_compression PUBLIC ZLIB::ZLIB zh_wwsupport)
target_link_libraries(zh_wwsupport PUBLIC Freetype::Freetype Fontconfig::Fontconfig)
target_link_libraries(zh_w3d PUBLIC zh_wwsupport)
target_link_libraries(zh_wwshade PUBLIC zh_w3d)
target_link_libraries(zh_game_engine PUBLIC zh_compression zh_wwsupport)
target_link_libraries(zh_platform_sdl PUBLIC SDL3::SDL3)
target_link_libraries(zh_renderer_sdl_gpu PUBLIC SDL3::SDL3 zh_w3d zh_wwshade)
target_link_libraries(zh_video_ffmpeg PUBLIC FFmpeg::avformat FFmpeg::avcodec FFmpeg::avutil FFmpeg::swscale FFmpeg::swresample zh_audio_miniaudio)
target_link_libraries(zh_os_posix PUBLIC Threads::Threads)
target_link_libraries(zh_game_device PUBLIC zh_platform_sdl zh_renderer_sdl_gpu zh_effects zh_audio_miniaudio zh_video_ffmpeg zh_os_posix)

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

set(ZH_RENDERER_SHADER_NAMES
  ui.vert ui.frag
  terrain.vert terrain.frag
  water.vert water.frag
  points.vert points.frag
  wwshade.vert wwshade.frag
)
set(ZH_RENDERER_SPIRV)
foreach(shader_name IN LISTS ZH_RENDERER_SHADER_NAMES)
  set(shader_source "${CMAKE_CURRENT_SOURCE_DIR}/shaders/renderer/${shader_name}")
  set(shader_output "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/renderer/${shader_name}.spv")
  add_custom_command(
    OUTPUT "${shader_output}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/renderer"
    COMMAND "${ZH_GLSLC_EXECUTABLE}" "${shader_source}" -o "${shader_output}"
    DEPENDS "${shader_source}"
    COMMENT "Compiling renderer shader ${shader_name}"
    VERBATIM
  )
  list(APPEND ZH_RENDERER_SPIRV "${shader_output}")
endforeach()
add_custom_target(zh_renderer_shaders ALL DEPENDS ${ZH_RENDERER_SPIRV})
add_dependencies(zh_renderer_sdl_gpu zh_renderer_shaders)

set(ZH_EFFECT_SHADER_NAMES
  projected.vert projected.frag
  bump_environment.vert bump_environment.frag
  stealth.vert stealth.frag
  post_effect.vert post_effect.frag
)
set(ZH_EFFECT_SPIRV)
foreach(shader_name IN LISTS ZH_EFFECT_SHADER_NAMES)
  set(shader_source "${CMAKE_CURRENT_SOURCE_DIR}/shaders/effects/${shader_name}")
  set(shader_output "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/effects/${shader_name}.spv")
  add_custom_command(
    OUTPUT "${shader_output}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/effects"
    COMMAND "${ZH_GLSLC_EXECUTABLE}" "${shader_source}" -o "${shader_output}"
    DEPENDS "${shader_source}"
    COMMENT "Compiling effect shader ${shader_name}"
    VERBATIM
  )
  list(APPEND ZH_EFFECT_SPIRV "${shader_output}")
endforeach()
add_custom_target(zh_effect_shaders ALL DEPENDS ${ZH_EFFECT_SPIRV})
add_dependencies(zh_effects zh_renderer_shaders zh_effect_shaders)

add_executable(zh_main src/bootstrap/main.cpp src/headless/runtime.cpp)
target_include_directories(zh_main PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated" "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_main PRIVATE
  zh_build_options zh_game_engine zh_game_device zh_null_platform zh_null_renderer
  zh_null_audio zh_null_video zh_test_support zh_data
  zh_lan
)
set_property(TARGET zh_main PROPERTY ZH_LEGACY_SOURCE_INVENTORY "${ZH_LEGACY_ZH_MAIN_SOURCES}")

install(TARGETS zh_main RUNTIME DESTINATION bin)
install(FILES "${ZH_BOOTSTRAP_SPIRV}" DESTINATION share/generals-zero-hour/shaders)
install(FILES ${ZH_RENDERER_SPIRV} DESTINATION share/generals-zero-hour/shaders/renderer)
install(FILES ${ZH_EFFECT_SPIRV} DESTINATION share/generals-zero-hour/shaders/effects)
install(FILES LICENSE.md DESTINATION share/licenses/generals-zero-hour)
install(FILES data/corpus/english-manifest.tsv DESTINATION share/generals-zero-hour/corpus)
install(FILES data/corpus/world-rendering-manifest.tsv DESTINATION share/generals-zero-hour/corpus)
install(FILES data/corpus/effects-manifest.tsv DESTINATION share/generals-zero-hour/corpus)

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

  add_test(NAME renderer_inventory COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/renderer_inventory.py" --check)
  set_tests_properties(renderer_inventory PROPERTIES LABELS renderer-contract)
  add_test(NAME renderer_inventory_negative COMMAND "${Python3_EXECUTABLE}" -m unittest tests.m2.test_renderer_inventory)
  set_tests_properties(renderer_inventory_negative PROPERTIES LABELS renderer-contract WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME renderer_public_headers COMMAND "${Python3_EXECUTABLE}" -m unittest tests.m2.test_renderer_public_headers)
  set_tests_properties(renderer_public_headers PROPERTIES LABELS renderer-contract WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

  add_executable(renderer_contract_tests tests/renderer/test_renderer_contract.cpp)
  target_link_libraries(renderer_contract_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_contract COMMAND renderer_contract_tests)
  set_tests_properties(renderer_contract PROPERTIES LABELS renderer-contract)

  add_executable(renderer_recording_device_tests tests/renderer/test_recording_device.cpp)
  target_link_libraries(renderer_recording_device_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_recording_device COMMAND renderer_recording_device_tests)
  set_tests_properties(renderer_recording_device PROPERTIES LABELS renderer-contract)

  add_executable(renderer_texture_loader_tests tests/renderer/test_texture_loader.cpp)
  target_link_libraries(renderer_texture_loader_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_texture_loader COMMAND renderer_texture_loader_tests)
  set_tests_properties(renderer_texture_loader PROPERTIES LABELS renderer-contract)

  add_executable(renderer_dx8_state_cache_tests tests/renderer/test_dx8_state_cache.cpp)
  target_link_libraries(renderer_dx8_state_cache_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_dx8_state_cache COMMAND renderer_dx8_state_cache_tests)
  set_tests_properties(renderer_dx8_state_cache PROPERTIES LABELS renderer-contract)

  add_executable(renderer_shader_registry_tests tests/renderer/test_shader_registry.cpp)
  target_compile_definitions(renderer_shader_registry_tests PRIVATE
    ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    ZH_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
  )
  target_link_libraries(renderer_shader_registry_tests PRIVATE zh_renderer_sdl_gpu)
  add_dependencies(renderer_shader_registry_tests zh_renderer_shaders)
  add_test(NAME renderer_shader_registry COMMAND renderer_shader_registry_tests)
  set_tests_properties(renderer_shader_registry PROPERTIES LABELS renderer-contract)

  add_executable(ui_renderer_tests tests/ui/test_ui_renderer.cpp)
  target_link_libraries(ui_renderer_tests PRIVATE zh_ui)
  add_test(NAME ui_renderer COMMAND ui_renderer_tests)
  set_tests_properties(ui_renderer PROPERTIES LABELS "renderer-contract;ui")

  add_executable(ui_font_tests tests/ui/test_ui_font.cpp)
  target_link_libraries(ui_font_tests PRIVATE zh_ui)
  add_test(NAME ui_font COMMAND ui_font_tests)
  set_tests_properties(ui_font PROPERTIES LABELS ui)

  add_executable(ui_session_tests tests/ui/test_ui_session.cpp)
  target_compile_definitions(ui_session_tests PRIVATE ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(ui_session_tests PRIVATE zh_ui)
  add_test(NAME ui_session COMMAND ui_session_tests)
  set_tests_properties(ui_session PROPERTIES LABELS "platform;renderer-contract;ui")

  add_executable(world_scene_tests tests/world/test_world_scene.cpp)
  target_link_libraries(world_scene_tests PRIVATE zh_world)
  add_test(NAME world_scene COMMAND world_scene_tests)
  set_tests_properties(world_scene PROPERTIES LABELS renderer-contract)

  add_executable(world_recorder_tests tests/world/test_world_recorder.cpp)
  target_link_libraries(world_recorder_tests PRIVATE zh_world)
  add_test(NAME world_recorder COMMAND world_recorder_tests)
  set_tests_properties(world_recorder PROPERTIES LABELS renderer-contract)

  add_executable(world_corpus_tests tests/world/test_world_corpus.cpp)
  target_compile_definitions(world_corpus_tests PRIVATE ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(world_corpus_tests PRIVATE zh_world)
  add_test(NAME world_corpus COMMAND world_corpus_tests)
  set_tests_properties(world_corpus PROPERTIES LABELS renderer-contract)

  add_executable(effects_registry_tests tests/effects/test_effect_registry.cpp)
  target_compile_definitions(effects_registry_tests PRIVATE
    ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    ZH_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
  )
  target_link_libraries(effects_registry_tests PRIVATE zh_effects)
  add_dependencies(effects_registry_tests zh_renderer_shaders zh_effect_shaders)
  add_test(NAME effects_registry COMMAND effects_registry_tests)
  set_tests_properties(effects_registry PROPERTIES LABELS renderer-contract)

  add_executable(effects_recorder_tests tests/effects/test_effect_recorder.cpp)
  target_compile_definitions(effects_recorder_tests PRIVATE ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(effects_recorder_tests PRIVATE zh_effects)
  add_test(NAME effects_recorder COMMAND effects_recorder_tests)
  set_tests_properties(effects_recorder PROPERTIES LABELS renderer-contract)

  add_executable(foundation_abi_codec_tests tests/foundation/test_abi_codec.cpp)
  target_link_libraries(foundation_abi_codec_tests PRIVATE zh_wwsupport)
  add_test(NAME foundation_abi_codec COMMAND foundation_abi_codec_tests)
  set_tests_properties(foundation_abi_codec PROPERTIES LABELS foundation)

  add_executable(foundation_platform_tests tests/foundation/test_platform.cpp)
  target_link_libraries(foundation_platform_tests PRIVATE zh_os_posix)
  add_test(NAME foundation_platform COMMAND foundation_platform_tests)
  set_tests_properties(foundation_platform PROPERTIES LABELS foundation)

  add_executable(foundation_compression_tests tests/foundation/test_compression.cpp)
  target_link_libraries(foundation_compression_tests PRIVATE zh_compression)
  add_test(NAME foundation_compression COMMAND foundation_compression_tests)
  set_tests_properties(foundation_compression PROPERTIES LABELS foundation)

  add_executable(headless_runtime_tests
    tests/headless/test_headless_runtime.cpp
    src/headless/runtime.cpp
  )
  target_link_libraries(headless_runtime_tests PRIVATE
    zh_os_posix zh_null_platform zh_null_renderer zh_null_audio zh_null_video zh_lan
  )
  add_test(NAME headless_runtime COMMAND headless_runtime_tests)
  set_tests_properties(headless_runtime PROPERTIES LABELS headless)

  add_test(NAME headless_process_smoke COMMAND zh_main --headless --ticks 2
    --state-dir "${CMAKE_CURRENT_BINARY_DIR}/headless-process-smoke")
  set_tests_properties(headless_process_smoke PROPERTIES
    LABELS headless
    PASS_REGULAR_EXPRESSION "headless: completed ticks=2"
  )

  add_test(NAME headless_process_isolation COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/headless/test_process_isolation.py"
    --executable "$<TARGET_FILE:zh_main>")
  set_tests_properties(headless_process_isolation PROPERTIES LABELS headless TIMEOUT 30)

  add_executable(lan_protocol_tests tests/lan/test_protocol.cpp)
  target_link_libraries(lan_protocol_tests PRIVATE zh_lan)
  add_test(NAME lan_protocol COMMAND lan_protocol_tests)
  set_tests_properties(lan_protocol PROPERTIES LABELS lan)

  add_executable(lan_virtual_tests tests/lan/test_virtual_transport.cpp)
  target_link_libraries(lan_virtual_tests PRIVATE zh_lan)
  add_test(NAME lan_virtual COMMAND lan_virtual_tests)
  set_tests_properties(lan_virtual PROPERTIES LABELS "lan;headless")

  add_executable(lan_posix_udp_tests tests/lan/test_posix_udp.cpp)
  target_link_libraries(lan_posix_udp_tests PRIVATE zh_lan)
  add_test(NAME lan_posix_udp COMMAND lan_posix_udp_tests)
  set_tests_properties(lan_posix_udp PROPERTIES LABELS "lan;headless" TIMEOUT 10)

  add_test(NAME lan_two_processes COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/lan/test_two_processes.py"
    --executable "$<TARGET_FILE:zh_main>")
  set_tests_properties(lan_two_processes PROPERTIES LABELS "lan;headless" TIMEOUT 20 RUN_SERIAL TRUE)

  add_executable(platform_sdl_event_tests tests/platform/test_sdl_events.cpp)
  target_link_libraries(platform_sdl_event_tests PRIVATE zh_platform_sdl)
  add_test(NAME platform_sdl_events COMMAND platform_sdl_event_tests)
  set_tests_properties(platform_sdl_events PROPERTIES LABELS platform)

  add_executable(platform_sdl_window_tests tests/platform/test_sdl_window.cpp)
  target_link_libraries(platform_sdl_window_tests PRIVATE zh_platform_sdl)
  add_test(NAME platform_sdl_window_errors COMMAND platform_sdl_window_tests)
  set_tests_properties(platform_sdl_window_errors PROPERTIES LABELS platform)

  add_executable(zh_platform_smoke tests/platform/platform_smoke.cpp)
  target_link_libraries(zh_platform_smoke PRIVATE zh_platform_sdl)

  add_executable(data_config_tests tests/data/test_data_config.cpp)
  target_link_libraries(data_config_tests PRIVATE zh_data)
  add_test(NAME data_config COMMAND data_config_tests)
  set_tests_properties(data_config PROPERTIES LABELS data)

  add_executable(data_vfs_tests tests/data/test_vfs.cpp)
  target_link_libraries(data_vfs_tests PRIVATE zh_data)
  add_test(NAME data_vfs COMMAND data_vfs_tests)
  set_tests_properties(data_vfs PROPERTIES LABELS data)

  add_executable(data_inventory_tests tests/data/test_inventory.cpp)
  target_link_libraries(data_inventory_tests PRIVATE zh_data)
  add_test(NAME data_inventory COMMAND data_inventory_tests)
  set_tests_properties(data_inventory PROPERTIES LABELS data)

  add_executable(persistence_save_tests tests/persistence/test_save.cpp)
  target_link_libraries(persistence_save_tests PRIVATE zh_persistence)
  add_test(NAME persistence_save COMMAND persistence_save_tests)
  set_tests_properties(persistence_save PROPERTIES LABELS "foundation;determinism")

  add_executable(persistence_replay_tests tests/persistence/test_replay.cpp)
  target_link_libraries(persistence_replay_tests PRIVATE zh_persistence)
  add_test(NAME persistence_replay COMMAND persistence_replay_tests)
  set_tests_properties(persistence_replay PROPERTIES LABELS "foundation;determinism")

  add_executable(determinism_scenario_tests tests/determinism/test_scenario.cpp)
  target_link_libraries(determinism_scenario_tests PRIVATE zh_simulation)
  add_test(NAME determinism_scenario COMMAND determinism_scenario_tests
    --emit-result "${CMAKE_CURRENT_BINARY_DIR}/determinism-result.txt")
  set_tests_properties(determinism_scenario PROPERTIES LABELS determinism)
  add_test(NAME determinism_report_comparison COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.determinism.test_compare_presets)
  set_tests_properties(determinism_report_comparison PROPERTIES
    LABELS determinism
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )

  if(ZH_ENABLE_RETAIL_TESTS)
    if(NOT IS_ABSOLUTE "${ZH_RETAIL_ZH_DATA}" OR NOT IS_ABSOLUTE "${ZH_RETAIL_GENERALS_DATA}" OR ZH_RETAIL_LANGUAGE STREQUAL "")
      message(FATAL_ERROR "Retail tests require absolute ZH_RETAIL_ZH_DATA, absolute ZH_RETAIL_GENERALS_DATA, and ZH_RETAIL_LANGUAGE")
    endif()
    add_test(NAME retail_data_verification COMMAND zh_main --verify-data
      --zh-data "${ZH_RETAIL_ZH_DATA}" --generals-data "${ZH_RETAIL_GENERALS_DATA}" --language "${ZH_RETAIL_LANGUAGE}")
    set_tests_properties(retail_data_verification PROPERTIES LABELS data
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      PASS_REGULAR_EXPRESSION "devices skipped;corpus verification ok")
  endif()
endif()
