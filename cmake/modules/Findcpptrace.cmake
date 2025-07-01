set(PCK "cpptrace")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES cpptrace/cpptrace.hpp
  HINTS
    ${PRAGMA_DEPS_DIR}/cpptrace/include
)

find_library(${PCK}_LIBRARY
  NAMES cpptrace
  HINTS
    ${PRAGMA_DEPS_DIR}/cpptrace/lib
)

set(REQ_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR)
if(WIN32)
  find_file(
    ${PCK}_RUNTIME
    NAMES cpptrace.dll
    HINTS
      ${PRAGMA_DEPS_DIR}/cpptrace/bin
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
endif()
