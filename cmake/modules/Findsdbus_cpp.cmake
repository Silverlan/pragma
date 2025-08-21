set(PCK "sdbus_cpp")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES sdbus-c++/sdbus-c++.h
  HINTS
    ${PRAGMA_DEPS_DIR}/sdbus-cpp/include
)

find_library(${PCK}_LIBRARY
  NAMES sdbus-c++
  HINTS
    ${PRAGMA_DEPS_DIR}/sdbus-cpp/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
