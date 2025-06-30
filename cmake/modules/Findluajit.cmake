set(PCK "luajit")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES lua.hpp
  HINTS
    ${PRAGMA_DEPS_DIR}/luajit/include
)

find_library(${PCK}_LIBRARY
  NAMES luajit-p
  HINTS
    ${PRAGMA_DEPS_DIR}/luajit/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})

  set(LUA_LIBRARIES   ${${PCK}_LIBRARY})
  set(LUA_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
  set(LUA_FOUND TRUE)
endif()
