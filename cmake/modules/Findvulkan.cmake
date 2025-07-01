set(PCK "vulkan")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES vulkan/vulkan.hpp
  HINTS
    ${PRAGMA_DEPS_DIR}/vulkan/include
)

find_library(${PCK}_LIBRARY
  NAMES vulkan vulkan-1
  HINTS
    ${PRAGMA_DEPS_DIR}/vulkan/lib
)

find_path(${PCK}_ROOT_DIR
  NAMES sdk.env
  HINTS
    ${PRAGMA_DEPS_DIR}/vulkan
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR ${PCK}_ROOT_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})

  set(VULKAN_SDK ${${PCK}_ROOT_DIR})
endif()
