set(PRAGMA_INSTALL_COMPONENT "pragma")
set(PRAGMA_INSTALL_COMPONENT_FULL "pragma-full")
function(pragma_install TARGET_NAME DESTINATION_PATH)
    # Depending if target is library or executable we should change type of target install Win32: all runtime otherwise: pragma and pragma_server runtime,
    # otherwise library
    set(TARGET_INSTALL_TYPE "")
    if(WIN32)
        set(TARGET_INSTALL_TYPE "RUNTIME")
    else()
        get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
        if(TARGET_TYPE STREQUAL "EXECUTABLE")
            set(TARGET_INSTALL_TYPE "RUNTIME")
            # Process executable target
        elseif(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
            set(TARGET_INSTALL_TYPE "LIBRARY")
        else()
            # this is not usable to us. bail.
            return()
        endif()
    endif()
    message("Adding install rule for target \"${TARGET_NAME}\" to \"${DESTINATION_PATH}\"...")
    install(
        TARGETS ${TARGET_NAME} ${TARGET_INSTALL_TYPE}
        DESTINATION "${DESTINATION_PATH}"
        COMPONENT ${PRAGMA_INSTALL_COMPONENT})
endfunction(pragma_install)

function(pragma_install_lib_c INPUT_LIBRARY_FILENAME LIB_NAME DESTINATION_PATH)
    if("${INPUT_LIBRARY_FILENAME}" STREQUAL "")
        return()
    endif()

    string(LENGTH "${INPUT_LIBRARY_FILENAME}" "INPUT_LIBRARY_FILENAME_LENGTH")
    if(${INPUT_LIBRARY_FILENAME_LENGTH} EQUAL 0)
        set(INPUT_LIBRARY_FILENAME_LAST_CHAR "")
    else()
        math(EXPR INPUT_LIBRARY_FILENAME_LENGTH "${INPUT_LIBRARY_FILENAME_LENGTH}-1")
        string(SUBSTRING "${INPUT_LIBRARY_FILENAME}" ${INPUT_LIBRARY_FILENAME_LENGTH} 1 "INPUT_LIBRARY_FILENAME_LAST_CHAR")
    endif()

    if("${INPUT_LIBRARY_FILENAME_LAST_CHAR}" STREQUAL "/" OR "${INPUT_LIBRARY_FILENAME_LAST_CHAR}" STREQUAL "\\")
        set(INPUT_LIBRARY_PATH "${INPUT_LIBRARY_FILENAME}")
    else()
        get_filename_component(INPUT_LIBRARY_PATH "${INPUT_LIBRARY_FILENAME}" DIRECTORY REALPATH)
    endif()

    if(WIN32)
        set(INPUT_LIBRARY_PATH_BIN "${INPUT_LIBRARY_PATH}/${LIB_NAME}.dll")
    else()
        set(INPUT_LIBRARY_PATH_BIN "${INPUT_LIBRARY_PATH}/lib${LIB_NAME}.so")
    endif()

    set(InstallSrc "${INPUT_LIBRARY_PATH_BIN}")
    set(InstallDst "${CMAKE_INSTALL_PREFIX}/${DESTINATION_PATH}")
    message("Adding install rule for \"${InstallSrc}\" to \"${InstallDst}\"...")
    add_custom_command(
        TARGET pragma-install
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DInstallSrc=${InstallSrc} -DInstallDst=${InstallDst} -P ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/cmake/install_file.cmake
        COMMENT "Copying ${InstallSrc} to ${InstallDst}...")
endfunction(pragma_install_lib_c)

function(pragma_install_lib INPUT_LIBRARY_PATH DESTINATION_PATH)
    if("${INPUT_LIBRARY_PATH}" STREQUAL "")
        return()
    endif()
    set(LIB_FILES_TO_COPY "") # Four unix systems
    if(WIN32)
        string(REGEX REPLACE "\\.[^.]*$" "" INPUT_LIBRARY_PATH_NO_EXT ${INPUT_LIBRARY_PATH})

        set(INPUT_LIBRARY_PATH_BIN "${INPUT_LIBRARY_PATH_NO_EXT}.dll")
    else()
        set(INPUT_LIBRARY_PATH_BIN "${INPUT_LIBRARY_PATH}")
    endif()

    if(UNIX)
        cmake_path(GET INPUT_LIBRARY_PATH_BIN EXTENSION LIB_EXTENSION)
        if(LIB_EXTENSION STREQUAL ".a")
            return()
        endif()
    endif()

    set(InstallSrc "${INPUT_LIBRARY_PATH_BIN}")
    set(InstallDst "${CMAKE_INSTALL_PREFIX}/${DESTINATION_PATH}")
    message("Adding install rule for \"${InstallSrc}\" to \"${InstallDst}\"...")
    add_custom_command(
        TARGET pragma-install
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DInstallSrc=${InstallSrc} -DInstallDst=${InstallDst} -P ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/cmake/install_file.cmake
        COMMENT "Copying ${InstallSrc} to ${InstallDst}...")
endfunction(pragma_install_lib)

function(pragma_install_create_directory DIR_NAME)
    add_custom_command(
        TARGET pragma-install
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/${DIR_NAME})
endfunction(pragma_install_create_directory)
