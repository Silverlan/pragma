set(PCK "icu")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES unicode/utf8.h
  HINTS
    ${PRAGMA_DEPS_DIR}/icu/include
)

find_library(${PCK}_LIBRARY
  NAMES icuuc
  HINTS
    ${PRAGMA_DEPS_DIR}/icu/lib
)

find_library(${PCK}_LIBRARY_75
  NAMES libicuuc.so.75
  HINTS
    ${PRAGMA_DEPS_DIR}/icu/lib
)

find_library(${PCK}_DATA_LIBRARY
  NAMES icudata
  HINTS
    ${PRAGMA_DEPS_DIR}/icu/lib
)

find_library(${PCK}_DATA_LIBRARY_75
  NAMES libicudata.so.75
  HINTS
    ${PRAGMA_DEPS_DIR}/icu/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY ${PCK}_LIBRARY_75 ${PCK}_DATA_LIBRARY ${PCK}_DATA_LIBRARY_75 ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY} ${${PCK}_LIBRARY_75} ${${PCK}_DATA_LIBRARY} ${${PCK}_DATA_LIBRARY_75})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
