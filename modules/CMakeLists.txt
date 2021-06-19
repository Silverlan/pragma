cmake_minimum_required(VERSION 3.12)

project(modules)

function(register_module MODULE_NAME)
	message("Processing module '${MODULE_NAME}'...")
	add_subdirectory(${MODULE_NAME})
endfunction(register_module)

file(GLOB MODULE_LIST
    "${CMAKE_CURRENT_LIST_DIR}/*"
)
foreach(MODULE_PATH ${MODULE_LIST})
	if(IS_DIRECTORY ${MODULE_PATH})
		get_filename_component(MODULE_NAME ${MODULE_PATH} NAME)
		register_module(${MODULE_NAME})

		if("${MODULE_NAME}" STREQUAL "pr_steam_networking")
			set_target_properties(pr_game_networking_server PROPERTIES FOLDER modules)
			set_target_properties(pr_game_networking_client PROPERTIES FOLDER modules)
			set_target_properties(pr_steam_networking_server PROPERTIES FOLDER modules)
			set_target_properties(pr_steam_networking_client PROPERTIES FOLDER modules)
		else()
			if(NOT "${MODULE_NAME}" STREQUAL "interfaces")
				set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules)
			endif()
		endif()
	endif()
endforeach()

# option(CONFIG_ENABLE_BULLET_PHYSICS "Enable Bullet physics engine" ON)
# if(CONFIG_ENABLE_BULLET_PHYSICS)
# 	register_module(pr_bullet)
# 	set_target_properties(pr_bullet PROPERTIES FOLDER modules)
# endif()

# option(CONFIG_ENABLE_PHYSX_PHYSICS "Enable PhysX physics engine" ON)
# if(CONFIG_ENABLE_PHYSX_PHYSICS)
# 	register_module(pr_physx)
# 	set_target_properties(pr_physx PROPERTIES FOLDER modules)
# endif()

set(CMAKE_CXX_STANDARD 20)
