set(PCK "libdecor")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES libdecor.h
  HINTS
    ${PRAGMA_DEPS_DIR}/libdecor/include
)

find_path(${PCK}_PLUGIN_DIR
  NAMES cairo
  HINTS
    ${PRAGMA_DEPS_DIR}/libdecor/lib/plugins
)

find_library(${PCK}_LIBRARY
  NAMES decor-0
  HINTS
    ${PRAGMA_DEPS_DIR}/libdecor/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
