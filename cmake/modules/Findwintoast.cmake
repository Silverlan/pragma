set(PCK "wintoast")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES wintoastlib.h
  HINTS
    ${PRAGMA_DEPS_DIR}/WinToast/include
)

find_library(${PCK}_LIBRARY
  NAMES WinToast
  HINTS
    ${PRAGMA_DEPS_DIR}/WinToast/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
