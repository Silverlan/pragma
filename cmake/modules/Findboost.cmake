set(PCK "boost")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES boost/config.hpp
  HINTS
    ${PRAGMA_DEPS_DIR}/boost/include
)

find_library(${PCK}_LIB_DATE_TIME     NAMES boost_date_time "boost_date_time-vc143-mt-x64-1_89" "libboost_date_time-clang21-mt-x64-1_89"      HINTS ${PRAGMA_DEPS_DIR}/boost/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK} REQUIRED_VARS ${PCK}_LIB_DATE_TIME ${PCK}_INCLUDE_DIR)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIB_DATE_TIME})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
