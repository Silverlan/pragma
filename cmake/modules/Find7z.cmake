set(PCK "7z")

if (${PCK}_FOUND)
  return()
endif()

find_library(${PCK}_LIBRARY
  NAMES 7z 7z.so
  HINTS
    ${PRAGMA_DEPS_DIR}/7z/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
endif()
