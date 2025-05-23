function(pr_install_files)
    set(options)
    set(oneValueArgs INSTALL_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_INSTALL_DIR)
        set(PA_INSTALL_DIR "${BINARY_OUTPUT_DIR}")
    endif()

    foreach(FILE_PATH ${PA_UNPARSED_ARGUMENTS})
        string(REPLACE "\\" "/" FILE_PATH ${FILE_PATH})
        message("Adding install rule for \"${FILE_PATH}\" to \"${PA_INSTALL_DIR}\"...")

        # Check if the file is a symlink and get the real file path
        get_filename_component(REAL_FILE_PATH "${FILE_PATH}" REALPATH)

        if(FILE_PATH STREQUAL REAL_FILE_PATH)
            # If the file is not a symlink, just install it
            install(
                FILES "${FILE_PATH}"
                DESTINATION "${PA_INSTALL_DIR}"
                OPTIONAL
                COMPONENT ${PRAGMA_INSTALL_COMPONENT}
            )
        else()
            # If the file is a symlink, install both the symlink and the real file
            message("File \"${FILE_PATH}\" is a symlink to \"${REAL_FILE_PATH}\"")
            
            # Install the real file (the target of the symlink)
            install(
                FILES "${REAL_FILE_PATH}"
                DESTINATION "${PA_INSTALL_DIR}"
                OPTIONAL
                COMPONENT ${PRAGMA_INSTALL_COMPONENT}
            )

            # Install the symlink itself
            install(
                FILES "${FILE_PATH}"
                DESTINATION "${PA_INSTALL_DIR}"
                OPTIONAL
                COMPONENT ${PRAGMA_INSTALL_COMPONENT}
            )
        endif()
    endforeach()
endfunction(pr_install_files)

function(pr_install_libraries)
    set(options)
    set(oneValueArgs INSTALL_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_INSTALL_DIR)
        set(PA_INSTALL_DIR "${BINARY_OUTPUT_DIR}")
    endif()

    foreach(TARGET ${PA_UNPARSED_ARGUMENTS})
        pr_get_normalized_identifier_name(${TARGET})
        set(FILE_PATH "${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY}")
        string(REPLACE "\\" "/" FILE_PATH ${FILE_PATH})
        message("Adding install rule for library \"${TARGET}\" (\"${FILE_PATH}\") to \"${PA_INSTALL_DIR}\"...")
        install(
            FILES "${FILE_PATH}"
            DESTINATION "${PA_INSTALL_DIR}"
            OPTIONAL
            COMPONENT ${PRAGMA_INSTALL_COMPONENT})
        if(UNIX)
            install(
                TARGETS "${TARGET}"
                RUNTIME DESTINATION "${PA_INSTALL_DIR}"
                LIBRARY DESTINATION "${PA_INSTALL_DIR}"
                OPTIONAL
                COMPONENT ${PRAGMA_INSTALL_COMPONENT})
        endif()
    endforeach()
endfunction(pr_install_libraries)

function(pr_install_targets)
    set(options)
    set(oneValueArgs INSTALL_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_INSTALL_DIR)
        set(PA_INSTALL_DIR "${BINARY_OUTPUT_DIR}")
    endif()

    foreach(TARGET ${PA_UNPARSED_ARGUMENTS})
        get_target_property(_type ${TARGET} TYPE)
        if(UNIX AND _type STREQUAL "EXECUTABLE")
            # On UNIX, we need to install the executable as a program to ensure it is executable
            install(
                PROGRAMS
                $<TARGET_FILE:${TARGET}>
                DESTINATION "${PA_INSTALL_DIR}"
                OPTIONAL
                COMPONENT ${PRAGMA_INSTALL_COMPONENT}
            )
        else()
            set(FILE_PATH "$<TARGET_FILE:${TARGET}>")
            string(REPLACE "\\" "/" FILE_PATH ${FILE_PATH})
            message("Adding install rule for target \"${TARGET}\" (\"${FILE_PATH}\") to \"${PA_INSTALL_DIR}\"...")
            install(
                FILES "${FILE_PATH}"
                DESTINATION "${PA_INSTALL_DIR}"
                OPTIONAL
                COMPONENT ${PRAGMA_INSTALL_COMPONENT})
            if(UNIX)
                install(
                    TARGETS "${TARGET}"
                    RUNTIME DESTINATION "${PA_INSTALL_DIR}"
                    LIBRARY DESTINATION "${PA_INSTALL_DIR}"
                    OPTIONAL
                    COMPONENT ${PRAGMA_INSTALL_COMPONENT})
            endif()
        endif()
    endforeach()
endfunction(pr_install_targets)

function(pr_install_directory FILE_PATH)
    set(options)
    set(oneValueArgs INSTALL_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 1 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_INSTALL_DIR)
        set(PA_INSTALL_DIR "${BINARY_OUTPUT_DIR}")
    endif()

    string(REPLACE "\\" "/" FILE_PATH ${FILE_PATH})
    message("Adding install rule for \"${FILE_PATH}\" to \"${PA_INSTALL_DIR}\"...")
    install(
        DIRECTORY "${FILE_PATH}"
        DESTINATION "${PA_INSTALL_DIR}"
        OPTIONAL
        COMPONENT ${PRAGMA_INSTALL_COMPONENT} ${PA_UNPARSED_ARGUMENTS})
endfunction(pr_install_directory)

function(pr_install_create_directory DIR_NAME)
    add_custom_command(
        TARGET pragma-install
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/${DIR_NAME})
endfunction(pr_install_create_directory)

function(pr_install_binary)
    set(options)
    set(oneValueArgs BIN_DIR WIN LIN INSTALL_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_INSTALL_DIR)
        set(PA_INSTALL_DIR "${BINARY_OUTPUT_DIR}")
    endif()

    if(NOT DEFINED PA_BIN_DIR)
        set(IDENTIFIER "${PA_UNPARSED_ARGUMENTS}")
        pr_get_normalized_identifier_name(${IDENTIFIER})
        get_filename_component(DIR_PATH "${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY}" DIRECTORY)
    else()
        set(DIR_PATH "${PA_BIN_DIR}")
    endif()
    if(WIN32)
        file(TO_NATIVE_PATH "${DIR_PATH}/${PA_WIN}" PA_BIN_DIR)
    else()
        file(TO_NATIVE_PATH "${DIR_PATH}/${PA_LIN}" PA_BIN_DIR)
    endif()

    if(UNIX AND NOT APPLE)
        # on Linux/UNIX: gather the link and all its intermediate targets
        set(_to_install_list "${PA_BIN_DIR}")
        set(_current        "${PA_BIN_DIR}")

        # walk the symlink chain *without* collapsing it
        while(IS_SYMLINK "${_current}")
            file(READ_SYMLINK "${_current}" _link_dest)
            # figure out the next path *absolutely*, but keep it as a symlink
            get_filename_component(_dir  "${_current}" DIRECTORY)
            get_filename_component(_next "${_dir}/${_link_dest}" ABSOLUTE)
            list(APPEND _to_install_list "${_next}")
            set(_current "${_next}")
        endwhile()

        # now install each file (will include the original symlink + each real target)
        foreach(_f IN LISTS _to_install_list)
            message(STATUS "Installing binary \"${_f}\" to \"${PA_INSTALL_DIR}\"...")
            pr_install_files(
                "${_f}"
                INSTALL_DIR "${PA_INSTALL_DIR}"
            )
        endforeach()
    else()
        message(STATUS "Installing binary \"${PA_BIN_DIR}\" to \"${PA_INSTALL_DIR}\"...")
        pr_install_files(
            "${PA_BIN_DIR}"
            INSTALL_DIR "${PA_INSTALL_DIR}"
        )
    endif()
endfunction()
