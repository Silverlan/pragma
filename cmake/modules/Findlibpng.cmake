set(PCK "libpng")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES png.h
  HINTS
    ${PRAGMA_DEPS_DIR}/libpng/include
)

find_library(${PCK}_LIBRARY
  NAMES png
  HINTS
    ${PRAGMA_DEPS_DIR}/libpng/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
