set(PCK "7z")

if (${PCK}_FOUND)
  return()
endif()

include(FindPackageHandleStandardArgs)
if(UNIX)
  find_library(${PCK}_LIBRARY
    NAMES 7z.so
    HINTS
      ${PRAGMA_DEPS_DIR}/7z/lib
  )
  find_package_handle_standard_args(${PCK} REQUIRED_VARS ${PCK}_LIBRARY)
  if(${PCK}_FOUND)
    set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  endif()
else()
  find_file(
    ${PCK}_RUNTIME
    NAMES 7z.dll
    HINTS
      ${PRAGMA_DEPS_DIR}/7z/bin
  )
  find_package_handle_standard_args(${PCK} REQUIRED_VARS ${PCK}_RUNTIME)
endif()
