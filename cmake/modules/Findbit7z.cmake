set(PCK "bit7z")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES bit7z/bit7z.hpp
  HINTS
    ${PRAGMA_DEPS_DIR}/bit7z/include
)

find_library(${PCK}_LIBRARY
  NAMES bit7z
  HINTS
    ${PRAGMA_DEPS_DIR}/bit7z/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
