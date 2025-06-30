set(PCK "eigen")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES Eigen/Eigen
  HINTS
    ${CMAKE_SOURCE_DIR}/third_party_libs/eigen
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
