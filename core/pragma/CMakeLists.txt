include("../../CMakeShared.txt")

set(PRECOMPILED_HEADER "stdafx_weave")
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
def_project(pragma exe ${APP_ICON_RESOURCE_WINDOWS})

add_custom_target(build_core ALL DEPENDS shared server client)
add_dependencies(pragma pragma_server)
add_dependencies(build_core pragma)
if(UNIX)
	target_link_libraries(pragma "dl")
	target_link_libraries(pragma "pthread")

	function(get_soname LIB_FILE OUT_VAR_NAME)
		execute_process(
			COMMAND bash "-c" "objdump -p ${LIB_FILE} | grep SONAME"
		   OUTPUT_VARIABLE LIB_SONAME
		)
		string(FIND ${LIB_SONAME} "SONAME" LIB_SONAME_POS)
		MATH(EXPR LIB_SONAME_POS "${LIB_SONAME_POS}+6")
		string(LENGTH ${LIB_SONAME} LIB_SONAME_LENGTH)
		string(SUBSTRING ${LIB_SONAME} ${LIB_SONAME_POS} ${LIB_SONAME_LENGTH} LIB_SONAME)
		string(STRIP "${LIB_SONAME}" LIB_SONAME)
		set (${OUT_VAR_NAME} ${LIB_SONAME} PARENT_SCOPE)
	endfunction()

	function(patch_library LIBRARY_NAME OUTPUT_DIR)
		get_filename_component(LIBRARY_FILENAME ${LIBRARY_NAME} NAME)
		add_custom_command(
		  TARGET build_core
		  POST_BUILD
		  COMMAND sh patch_library.sh "${CMAKE_CURRENT_BINARY_DIR}/../../output/${OUTPUT_DIR}/${LIBRARY_FILENAME}" '$$ORIGIN'
		  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
		  COMMENT "Updating library runpath for library ${LIBRARY_FILENAME}..."
		)

		add_custom_command(
		  TARGET build_core
		  POST_BUILD
		  COMMAND sh normalize_library_name.sh "${CMAKE_CURRENT_BINARY_DIR}/../../output/${OUTPUT_DIR}/${LIBRARY_FILENAME}"
		  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
		  COMMENT ""
		)
	endfunction()
	# file(GLOB_RECURSE LIB_FILES
	#    "${CMAKE_CURRENT_BINARY_DIR}/../../output/lib/*.so"
	# )
	set(LIB_FILES
		libalsoundsystem.so
		libassimp.so.4.1.0
		libboost_date_time.so.1.69.0
		libboost_system.so.1.69.0
		libbz2.so.1
		libcengine.so
		libclient.so
		libcmaterialsystem.so
		libdatasystem.so
		libengine.so
		libfmod.so.10.5
		libfmodstudio.so.10.5
		libfreetype.so.6.16.0
		libglfw.so.3.3
		libgtengine.so.3.24
		libiglfw.so
		libluabind.so
		libluajit.so
		libluasystem.so
		libmaterialsystem.so
		libmathutil.so
		libogg.so.0.8.3
		libphonon.so
		libpng16.so.16.37.0
		libprosper.so
		libserver.so
		libshared.so
		libsharedutils.so
		libvfilesystem.so
		libvorbis.so.0.4.8
		libvorbisfile.so.3.3.7
		libwgui.so
		libzip.so.5.0
	)
	foreach(LIBNAME ${LIB_FILES})
		patch_library("${CMAKE_CURRENT_BINARY_DIR}/../../output/lib/${LIBNAME}" lib)
	endforeach()
	patch_library("${CMAKE_CURRENT_BINARY_DIR}/../../output/modules/mount_external/libhl.so.2.3.0" modules/mount_external)
	patch_library("${CMAKE_CURRENT_BINARY_DIR}/../../output/modules/mount_external/libpr_mount_external.so" modules/mount_external)
	patch_library("${CMAKE_CURRENT_BINARY_DIR}/../../output/modules/mount_external/libutil_archive.so" modules/mount_external)
	patch_library("${CMAKE_CURRENT_BINARY_DIR}/../../output/modules/steamworks/libpr_steamworks.so" modules/steamworks)
	patch_library("${CMAKE_CURRENT_BINARY_DIR}/../../output/modules/steamworks/libsteam_api.so" modules/steamworks)
	patch_library("${CMAKE_CURRENT_BINARY_DIR}/../../output/modules/pr_dmx/libpr_dmx.so" modules)

	add_custom_command(
	  TARGET build_core
	  POST_BUILD
	  COMMAND sh patch_library.sh "${CMAKE_CURRENT_BINARY_DIR}/../../output/pragma" '$$ORIGIN/lib'
	  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
	  COMMENT "Updating pragma runpath..."
	)

	add_custom_command(
	  TARGET build_core
	  POST_BUILD
	  COMMAND sh patch_library.sh "${CMAKE_CURRENT_BINARY_DIR}/../../output/pragma_server" '$$ORIGIN/lib'
	  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
	  COMMENT "Updating pragma_server runpath..."
	)
else()
	set_target_properties(pragma PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")
endif()
