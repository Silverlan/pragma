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

set(LIB_VARS "${PCK}_LIBRARY")
set(LIB_VALUES "${${PCK}_LIBRARY}")

set(REQ_VARS ${PCK}_INCLUDE_DIR)
if(UNIX)
  find_library(${PCK}_LIBRARY_78
    NAMES libicuuc.so.78
    HINTS
      ${PRAGMA_DEPS_DIR}/icu/lib
  )

  find_library(${PCK}_DATA_LIBRARY
    NAMES icudata
    HINTS
      ${PRAGMA_DEPS_DIR}/icu/lib
  )

  find_library(${PCK}_DATA_LIBRARY_78
    NAMES libicudata.so.78
    HINTS
      ${PRAGMA_DEPS_DIR}/icu/lib
  )
  set(LIB_VARS ${LIB_VARS} ${PCK}_LIBRARY_78 ${PCK}_DATA_LIBRARY ${PCK}_DATA_LIBRARY_78)
  set(LIB_VALUES ${LIB_VALUES} ${${PCK}_LIBRARY_78} ${${PCK}_DATA_LIBRARY} ${${PCK}_DATA_LIBRARY_78})
  set(REQ_VARS ${REQ_VARS} ${LIB_VARS})
else()
  find_file(
    ${PCK}_ICUUC_RUNTIME
    NAMES icuuc78.dll
    HINTS ${PRAGMA_DEPS_DIR}/icu/bin
  )
  find_file(
    ${PCK}_ICUDT_RUNTIME
    NAMES icudt78.dll
    HINTS ${PRAGMA_DEPS_DIR}/icu/bin
  )
  set(REQ_VARS ${REQ_VARS} ${PCK}_ICUUC_RUNTIME ${PCK}_ICUDT_RUNTIME)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${REQ_VARS}
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${LIB_VALUES})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
  if(WIN32)
    set(${PCK}_RUNTIME  ${${PCK}_ICUUC_RUNTIME} ${${PCK}_ICUDT_RUNTIME})
  endif()
endif()
