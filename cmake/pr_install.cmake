function(pr_install_component NAME)
    message("Registering install component '${NAME}'...")
    set_property(GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENT_PART "${NAME}")

    get_property(_parts GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENTS)
    list(APPEND _parts ${NAME})
    set_property(GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENTS "${_parts}")
endfunction(pr_install_component)
function(pr_append_install_command cmd)
    get_property(install_commands GLOBAL PROPERTY PR_INSTALL_COMMANDS)
    set(install_commands "${install_commands} ${cmd}")
    set_property(GLOBAL PROPERTY PR_INSTALL_COMMANDS ${install_commands})

    if(ARGC GREATER 1)
        set(component "${ARGV1}")
        cmake_language(EVAL CODE "${cmd}")
    endif()
endfunction()
function(pr_set_install_target target)
    get_property(install_targets GLOBAL PROPERTY PR_INSTALL_TARGETS)
    set(install_targets ${install_targets} ${target})
    set_property(GLOBAL PROPERTY PR_INSTALL_TARGETS ${install_targets})
endfunction()
function(pr_install_files)
    set(options)
    set(oneValueArgs INSTALL_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_INSTALL_DIR)
        set(PA_INSTALL_DIR "${BINARY_OUTPUT_DIR}")
    endif()

    get_property(part GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENT_PART)
    foreach(FILE_PATH ${PA_UNPARSED_ARGUMENTS})
        string(REPLACE "\\" "/" FILE_PATH ${FILE_PATH})
        if (NOT FILE_PATH MATCHES ".*\\.a$") # Skip static Linux libraries
            message("Adding install rule for \"${FILE_PATH}\" to \"${PA_INSTALL_DIR}\"...")

            # Check if the file is a symlink and get the real file path
            get_filename_component(REAL_FILE_PATH "${FILE_PATH}" REALPATH)

            if(FILE_PATH STREQUAL REAL_FILE_PATH)
                # If the file is not a symlink, just install it
                pr_append_install_command(
                    "install(
                        FILES \"${FILE_PATH}\"
                        DESTINATION \"${PA_INSTALL_DIR}\"
                        COMPONENT \${component}
                    )\n"
                    ${part}
                )
            else()
                # on Linux/UNIX: gather the link and all its intermediate targets
                set(_to_install_list "${FILE_PATH}")
                set(_current        "${FILE_PATH}")

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
                    message(STATUS "Installing file \"${_f}\" to \"${PA_INSTALL_DIR}\"...")
                    pr_append_install_command(
                        "install(
                            FILES \"${_f}\"
                            DESTINATION \"${PA_INSTALL_DIR}\"
                            COMPONENT \${component}
                        )\n"
                        ${part}
                    )
                endforeach()
            endif()
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

    get_property(part GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENT_PART)
    foreach(TARGET ${PA_UNPARSED_ARGUMENTS})
        pr_get_normalized_identifier_name(${TARGET})
        set(FILE_PATH "${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY}")
        string(REPLACE "\\" "/" FILE_PATH ${FILE_PATH})
        message("Adding install rule for library \"${TARGET}\" (\"${FILE_PATH}\") to \"${PA_INSTALL_DIR}\"...")
        pr_append_install_command(
            "install(
                FILES \"${FILE_PATH}\"
                DESTINATION \"${PA_INSTALL_DIR}\"
                COMPONENT \${component})\n"
            ${part}
        )
        if(UNIX)
            pr_append_install_command(
                "install(
                    TARGETS \"${TARGET}\"
                    RUNTIME DESTINATION \"${PA_INSTALL_DIR}\"
                    LIBRARY DESTINATION \"${PA_INSTALL_DIR}\"
                    COMPONENT \${component})\n"
                ${part}
            )
        endif()
    endforeach()
endfunction(pr_install_libraries)

function(pr_install_targets)
    set(options)
    set(oneValueArgs INSTALL_DIR RENAME)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_INSTALL_DIR)
        set(PA_INSTALL_DIR "${BINARY_OUTPUT_DIR}")
    endif()

    get_property(part GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENT_PART)
    set(additional_args "")
    if(DEFINED PA_RENAME)
        set(additional_args RENAME "${PA_RENAME}")
    endif()

    foreach(TARGET ${PA_UNPARSED_ARGUMENTS})
        get_target_property(_type ${TARGET} TYPE)
        if(UNIX AND _type STREQUAL "EXECUTABLE")
            # On UNIX, we need to install the executable as a program to ensure it is executable
            pr_append_install_command(
                "install(
                    PROGRAMS
                    $<TARGET_FILE:${TARGET}>
                    OPTIONAL
                    DESTINATION \"${PA_INSTALL_DIR}\"
                    ${additional_args}
                    COMPONENT \${component}
                )\n"
                ${part}
            )
        else()
            set(FILE_PATH "$<TARGET_FILE:${TARGET}>")
            string(REPLACE "\\" "/" FILE_PATH ${FILE_PATH})
            message("Adding install rule for target \"${TARGET}\" (\"${FILE_PATH}\") to \"${PA_INSTALL_DIR}\"...")
            pr_append_install_command(
                "install(
                    FILES \"${FILE_PATH}\"
                    OPTIONAL
                    DESTINATION \"${PA_INSTALL_DIR}\"
                    ${additional_args}
                    COMPONENT \${component})\n"
                ${part}
            )
            if(UNIX)
                pr_append_install_command(
                    "install(
                        TARGETS \"${TARGET}\"
                        RUNTIME DESTINATION \"${PA_INSTALL_DIR}\"
                        LIBRARY DESTINATION \"${PA_INSTALL_DIR}\"
                        ${additional_args}
                        OPTIONAL
                        COMPONENT \${component})\n"
                    ${part}
                )
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

    # if the caller passed any PATTERN or EXCLUDE, prepend FILES_MATCHING
    list(FIND PA_UNPARSED_ARGUMENTS "PATTERN" _hasPattern)
    list(FIND PA_UNPARSED_ARGUMENTS "EXCLUDE" _hasExclude)
    if(_hasPattern GREATER -1 OR _hasExclude GREATER -1)
        list(INSERT PA_UNPARSED_ARGUMENTS 0 FILES_MATCHING)
    endif()

    get_property(part GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENT_PART)
    pr_append_install_command(
        "install(
            DIRECTORY \"${FILE_PATH}\"
            DESTINATION \"${PA_INSTALL_DIR}\"
            USE_SOURCE_PERMISSIONS
            COMPONENT \${component} ${PA_UNPARSED_ARGUMENTS})\n"
        ${part}
    )
endfunction(pr_install_directory)

function(pr_install_create_directory DIR_NAME)
    add_custom_command(
        TARGET pragma-install-base
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/${DIR_NAME})
endfunction(pr_install_create_directory)

function(pr_include_install PATH)
    if (EXISTS "${PATH}/cmake/modules")
        list(APPEND CMAKE_MODULE_PATH "${PATH}/cmake/modules")
    endif()
    include("${PATH}/Install.cmake" OPTIONAL)
endfunction()

function(pr_include_cmake_modules PATH)
    if (EXISTS "${PATH}/cmake/modules")
        list(APPEND CMAKE_MODULE_PATH "${PATH}/cmake/modules")
    endif()
endfunction()

function(pr_install_binaries IDENTIFIER)
    # Ensure IDENTIFIER is provided
    if(NOT IDENTIFIER)
        message(FATAL_ERROR "pr_install_binaries: <IDENTIFIER> argument is required.")
    endif()

    set(PA_IDENTIFIER "${IDENTIFIER}")

    # Parse optional INSTALL_DIR
    set(options)
    set(oneValueArgs INSTALL_DIR PACKAGE)
    set(multiValueArgs)
    # PARSE_ARGV 1: start parsing from the second argument onward
    cmake_parse_arguments(PA "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT PA_PACKAGE)
        set(PA_PACKAGE "${IDENTIFIER}")
    endif()
    find_package("${PA_PACKAGE}" REQUIRED)

    # Default install dir if not set
    if(NOT PA_INSTALL_DIR)
        set(PA_INSTALL_DIR "${BINARY_OUTPUT_DIR}")
    endif()

    # Select files based on platform
    if(UNIX)
        set(files_to_install "${${PA_IDENTIFIER}_LIBRARIES}")
        if(NOT files_to_install)
            message(FATAL_ERROR "pr_install_binaries: No files to install, ${PA_IDENTIFIER}_LIBRARIES is empty.")
        endif()
    else()
        set(files_to_install "${${PA_IDENTIFIER}_RUNTIME}")
        if(NOT files_to_install)
            message(FATAL_ERROR "pr_install_binaries: No files to install, ${PA_IDENTIFIER}_RUNTIME is empty.")
        endif()
    endif()

    function(_install_list file_list)
        foreach(_f IN LISTS file_list)
            message(STATUS "Installing binary '\"${_f}\' to '${PA_INSTALL_DIR}'...")
            pr_install_files(
                "${_f}"
                INSTALL_DIR "${PA_INSTALL_DIR}"
            )
        endforeach()
    endfunction()

    # Install symlinks and targets on Linux (non-Apple)
    if(UNIX AND NOT APPLE)
        set(_to_install ${files_to_install})
        foreach(_cur IN LISTS files_to_install)
            set(_current "${_cur}")
            while(IS_SYMLINK "${_current}")
                file(READ_SYMLINK "${_current}" _dest)
                get_filename_component(_dir "${_current}" DIRECTORY)
                get_filename_component(_next "${_dir}/${_dest}" ABSOLUTE)
                list(APPEND _to_install "${_next}")
                set(_current "${_next}")
            endwhile()
        endforeach()
        _install_list("${_to_install}")
    else()
        _install_list("${files_to_install}")
    endif()
endfunction()

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
