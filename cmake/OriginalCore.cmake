find_package(Threads REQUIRED)
set(ZH_CODE "${CMAKE_SOURCE_DIR}/GeneralsMD/Code")
add_library(original_core STATIC
  "${ZH_CODE}/GameEngine/Source/Common/RandomValue.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/crc.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/Trig.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/AsciiString.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/UnicodeString.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/GameMemory.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/MemoryInit.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/CriticalSection.cpp")
target_include_directories(original_core PUBLIC
  "${ZH_CODE}/GameEngine/Include"
  "${ZH_CODE}/GameEngine/Include/Precompiled"
  "${ZH_CODE}/Libraries/Include")
target_compile_features(original_core PUBLIC cxx_std_20)
target_compile_options(original_core PRIVATE -Wall -Wextra -Wno-unknown-pragmas -ffp-contract=off)
target_link_libraries(original_core PUBLIC Threads::Threads)
if(ZH_SANITIZE)
  target_compile_options(original_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(original_core PUBLIC -fsanitize=address,undefined)
endif()
add_executable(original_core_fixture tests/original/core.cpp)
target_link_libraries(original_core_fixture PRIVATE original_core)
foreach(family IN ITEMS values random strings pools repeat)
  add_test(NAME original_core_${family} COMMAND original_core_fixture "${family}")
  set_tests_properties(original_core_${family} PROPERTIES LABELS "original;core" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
