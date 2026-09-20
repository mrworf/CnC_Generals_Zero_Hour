execute_process(
  COMMAND "${CMAKE_COMMAND}" "${ZH_PROBE_ARGUMENT}" -P "${ZH_PROBE_SCRIPT}"
  RESULT_VARIABLE probe_result
  OUTPUT_VARIABLE probe_output
  ERROR_VARIABLE probe_error
)
set(probe_log "${probe_output}${probe_error}")
if(probe_result EQUAL 0)
  message(FATAL_ERROR "Negative probe unexpectedly succeeded: ${ZH_PROBE_SCRIPT}")
endif()
if(NOT probe_log MATCHES "${ZH_EXPECTED}")
  message(FATAL_ERROR "Negative probe did not emit '${ZH_EXPECTED}':\n${probe_log}")
endif()
message(STATUS "Observed expected failure: ${ZH_EXPECTED}")
