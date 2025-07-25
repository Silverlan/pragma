set(PCK "zlib")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES zlib.h
  HINTS
    ${PRAGMA_DEPS_DIR}/zlib/include
)

# Make sure to find the static version
set(_old_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")

find_library(${PCK}_LIBRARY
  NAMES z
  HINTS
    ${PRAGMA_DEPS_DIR}/zlib/lib
)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${_old_suffixes})

set(REQ_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR)
if(WIN32)
  find_file(
    ${PCK}_RUNTIME
    NAMES z.dll
    HINTS ${PRAGMA_DEPS_DIR}/zlib/bin
  )
  set(REQ_VARS ${REQ_VARS} ${PCK}_RUNTIME)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${REQ_VARS}
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})

  set(ZLIB_INCLUDE_DIRS ${${PCK}_INCLUDE_DIRS})
  set(ZLIB_LIBRARIES ${${PCK}_LIBRARY})
endif()
