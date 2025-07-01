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
  NAMES luajit-p luajit
  HINTS
    ${PRAGMA_DEPS_DIR}/luajit/lib
)

set(LIBS ${${PCK}_LIBRARY})
set(REQ_VARS ${PCK}_LIBRARY ${PCK}_INCLUDE_DIR)
if(WIN32)
  find_library(${PCK}_LUA_LIBRARY
    NAMES lua51
    HINTS
      ${PRAGMA_DEPS_DIR}/luajit/lib
  )

  find_file(
    ${PCK}_RUNTIME
    NAMES lua51.dll
    HINTS ${PRAGMA_DEPS_DIR}/luajit/bin
  )

  set(REQ_VARS ${REQ_VARS} ${PCK}_LUA_LIBRARY ${PCK}_RUNTIME)
  set(LIBS ${LIBS} ${${PCK}_LUA_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${REQ_VARS}
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${LIBS})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})

  set(LUA_LIBRARIES   ${LIBS})
  set(LUA_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
  set(LUA_FOUND TRUE)
endif()
