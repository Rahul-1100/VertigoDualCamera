# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\VertigoDualCamera_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\VertigoDualCamera_autogen.dir\\ParseCache.txt"
  "VertigoDualCamera_autogen"
  )
endif()
