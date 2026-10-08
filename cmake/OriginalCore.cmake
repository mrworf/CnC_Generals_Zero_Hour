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

add_library(original_data STATIC
  "${ZH_CODE}/GameEngine/Source/Common/System/File.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/RAMFile.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeDataFile.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeFileSystem.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/SubsystemBase.cpp")
target_sources(original_data PRIVATE "${ZH_CODE}/GameEngine/Source/GameClient/CSF.cpp")
target_sources(original_data PRIVATE
  "${ZH_CODE}/GameEngine/Source/GameClient/StringCatalog.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/INI/INICore.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/GameText.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/LanguageFilter.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/NativeTextPaths.cpp")
target_link_libraries(original_data PUBLIC original_core)
target_compile_options(original_data PRIVATE -Wall -Wextra -Wno-unknown-pragmas)
add_executable(original_data_fixture tests/original/data.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_data_fixture PRIVATE original_data)
add_executable(original_data_audit tests/original/data_audit.cpp)
target_link_libraries(original_data_audit PRIVATE original_data)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME original_data_audit_wrapper COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_SOURCE_DIR}/tests/original/test_data_audit.py" --binary "$<TARGET_FILE:original_data_audit>")
set_tests_properties(original_data_audit_wrapper PROPERTIES LABELS "original;data" TIMEOUT 60
  ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
foreach(family IN ITEMS roots archives ram catalogs text ini fault_mount fault_ram fault_csf fault_map fault_filter fault_ini)
  add_test(NAME original_data_${family} COMMAND original_data_fixture "${family}")
  set_tests_properties(original_data_${family} PROPERTIES LABELS "original;data" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
