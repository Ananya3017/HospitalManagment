# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\HospitalManagmentQt_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\HospitalManagmentQt_autogen.dir\\ParseCache.txt"
  "HospitalManagmentQt_autogen"
  )
endif()
