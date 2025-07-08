set(PCK "swiftshader")

if (${PCK}_FOUND)
  return()
endif()

include(FindPackageHandleStandardArgs)
if(UNIX)
  find_library(${PCK}_LIBRARY
    NAMES libvulkan.so.1
    HINTS
      ${PRAGMA_DEPS_DIR}/swiftshader/lib
  )
  find_package_handle_standard_args(${PCK}
    REQUIRED_VARS ${PCK}_LIBRARY
  )
  if(${PCK}_FOUND)
    set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  endif()
else()
  find_file(
    ${PCK}_RUNTIME
    NAMES vulkan-1.dll
    HINTS
      ${PRAGMA_DEPS_DIR}/swiftshader/bin
  )
  find_package_handle_standard_args(${PCK}
    REQUIRED_VARS ${PCK}_RUNTIME
  )
endif()
