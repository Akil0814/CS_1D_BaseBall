# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CS_1D_BaseBall_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CS_1D_BaseBall_autogen.dir\\ParseCache.txt"
  "CS_1D_BaseBall_autogen"
  )
endif()
