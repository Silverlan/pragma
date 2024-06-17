cmake_minimum_required(VERSION 3.12)

project(external_libs)

function(register_external_lib SUB_PROJ_NAME)
	get_property(IS_REGISTERED GLOBAL PROPERTY PR_REG_LIB_${SUB_PROJ_NAME})
	if(IS_REGISTERED)
		# Don't register library if it has already been registered
		return()
	endif()

	message("Processing external library '${SUB_PROJ_NAME}'...")
	add_subdirectory(${SUB_PROJ_NAME})

	if(SUB_PROJ_NAME MATCHES "util_*")
		set_target_properties(${SUB_PROJ_NAME} PROPERTIES FOLDER external_libs/util)
	else()
		set_target_properties(${SUB_PROJ_NAME} PROPERTIES FOLDER external_libs)
	endif()

	set_property(GLOBAL PROPERTY PR_REG_LIB_${SUB_PROJ_NAME} TRUE)
endfunction(register_external_lib)

# Note: Module-based libraries have to be included before
# any other libraries that they are being referenced by.
register_external_lib("mathutil")
register_external_lib("sharedutils")
register_external_lib("sharedutils-static")
register_external_lib("util_udm")
register_external_lib("util_pragma_doc")
register_external_lib("cppbezierfit")
register_external_lib("alsoundsystem")
register_external_lib("datasystem")
register_external_lib("iglfw")
register_external_lib("luasystem")
register_external_lib("materialsystem")
register_external_lib("networkmanager")
register_external_lib("panima")
register_external_lib("prosper")
register_external_lib("util_bsp")
register_external_lib("util_formatted_text")
register_external_lib("util_image")
register_external_lib("util_pad")
register_external_lib("util_smdmodel")
register_external_lib("util_sound")
register_external_lib("util_source_script")
register_external_lib("util_source2")
register_external_lib("util_timeline_scene")
register_external_lib("util_unicode")
register_external_lib("util_versioned_archive")
register_external_lib("util_vmf")
register_external_lib("util_zip")
register_external_lib("vfilesystem")
register_external_lib("wgui")

# There may be additional external libraries (e.g. by modules).
# We'll do a search add them last and assume they don't have any dependencies between each other.
file(GLOB LIB_LIST
    "${CMAKE_CURRENT_LIST_DIR}/*"
)
foreach(LIB_PATH ${LIB_LIST})
	if(IS_DIRECTORY ${LIB_PATH})
		get_filename_component(LIB_NAME ${LIB_PATH} NAME)
		register_external_lib(${LIB_NAME})
	endif()
endforeach()

set(CMAKE_CXX_STANDARD 20)

set_target_properties(networkmanager PROPERTIES FOLDER external_libs/networkmanager)
set_target_properties(clientmanager PROPERTIES FOLDER external_libs/networkmanager)
set_target_properties(servermanager PROPERTIES FOLDER external_libs/networkmanager)

set_target_properties(materialsystem PROPERTIES FOLDER external_libs/materialsystem)
set_target_properties(cmaterialsystem PROPERTIES FOLDER external_libs/materialsystem)

set(CMAKE_CXX_STANDARD 20)
