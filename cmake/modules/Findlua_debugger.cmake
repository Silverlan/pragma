set(PCK "lua_debugger")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_RESOURCE_DIR
  NAMES lua/modules/mobdebug.lua
  HINTS
    ${PRAGMA_DEPS_DIR}/lua_debugger/resources
)

set(REQ_VARS ${PCK}_RESOURCE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${REQ_VARS}
)
