set(PCK "geometric_tools")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES Mathematics/Math.h
  HINTS
    ${PRAGMA_DEPS_DIR}/geometrictools/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
