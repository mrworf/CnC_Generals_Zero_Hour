# M30's offline, reviewed public bgfx runtime. No system fallback or fetch.
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../third_party/bgfx_shaderc.lock" pin_lines
  REGEX "^(BGFX_REV|BX_REV|BIMG_REV|LICENSE_SHA256)=")
foreach(pin_line IN LISTS pin_lines)
  string(REPLACE "=" ";" pin_pair "${pin_line}")
  list(GET pin_pair 0 pin_name)
  list(GET pin_pair 1 pin_value)
  set("${pin_name}" "${pin_value}")
endforeach()
set(ZH_BGFX_SOURCE_ROOT "${CMAKE_SOURCE_DIR}/build/bgfx-toolchain/source"
  CACHE PATH "Pinned public bgfx/bx/bimg sibling source root")
foreach(project IN ITEMS bgfx bx bimg)
  string(TOUPPER "${project}" project_upper)
  execute_process(COMMAND git -C "${ZH_BGFX_SOURCE_ROOT}/${project}" rev-parse HEAD
    RESULT_VARIABLE pin_result OUTPUT_VARIABLE actual_revision
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  if(NOT pin_result EQUAL 0 OR NOT actual_revision STREQUAL "${${project_upper}_REV}")
    message(FATAL_ERROR "Missing/wrong pinned ${project} runtime source; run tools/renderer/bootstrap_bgfx_shaderc.sh --acquire")
  endif()
  if(NOT EXISTS "${ZH_BGFX_SOURCE_ROOT}/${project}/LICENSE")
    message(FATAL_ERROR "Missing pinned ${project} license")
  endif()
  file(SHA256 "${ZH_BGFX_SOURCE_ROOT}/${project}/LICENSE" actual_license)
  if(NOT actual_license STREQUAL "${LICENSE_SHA256}")
    message(FATAL_ERROR "${project} license differs from reviewed pin")
  endif()
  execute_process(COMMAND git -C "${ZH_BGFX_SOURCE_ROOT}/${project}" ls-files --others --exclude-standard
    OUTPUT_VARIABLE untracked_source OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT untracked_source STREQUAL "")
    message(FATAL_ERROR "${project} source contains unreviewed untracked files")
  endif()
endforeach()
execute_process(COMMAND git -C "${ZH_BGFX_SOURCE_ROOT}/bgfx" diff --binary
  OUTPUT_VARIABLE bgfx_diff)
file(READ "${CMAKE_SOURCE_DIR}/third_party/bgfx_shaderc_glsl.patch" reviewed_shaderc_diff)
if(NOT bgfx_diff STREQUAL reviewed_shaderc_diff)
  message(FATAL_ERROR "bgfx source differs from the reviewed shaderc-only patch")
endif()
foreach(project IN ITEMS bx bimg)
  execute_process(COMMAND git -C "${ZH_BGFX_SOURCE_ROOT}/${project}" diff --quiet
    RESULT_VARIABLE source_dirty)
  if(NOT source_dirty EQUAL 0)
    message(FATAL_ERROR "${project} source differs from reviewed pin")
  endif()
endforeach()
set(ZH_BGFX_RUNTIME_LIBRARY
  "${ZH_BGFX_SOURCE_ROOT}/bgfx/.build/linux64_gcc/bin/libbgfx-shared-libRelease.so")
if(NOT EXISTS "${ZH_BGFX_RUNTIME_LIBRARY}")
  message(FATAL_ERROR "Missing pinned bgfx runtime; run tools/renderer/bootstrap_bgfx_runtime.sh")
endif()
add_library(zh_bgfx_runtime SHARED IMPORTED GLOBAL)
set_target_properties(zh_bgfx_runtime PROPERTIES
  IMPORTED_LOCATION "${ZH_BGFX_RUNTIME_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${ZH_BGFX_SOURCE_ROOT}/bgfx/include;${ZH_BGFX_SOURCE_ROOT}/bx/include;${ZH_BGFX_SOURCE_ROOT}/bimg/include")
