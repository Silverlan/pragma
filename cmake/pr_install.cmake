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
        install(
            FILES "${FILE_PATH}"
            DESTINATION "${PA_INSTALL_DIR}"
            OPTIONAL
            COMPONENT ${PRAGMA_INSTALL_COMPONENT})
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
        set(FILE_PATH "$<TARGET_FILE:${TARGET}>")
        string(REPLACE "\\" "/" FILE_PATH ${FILE_PATH})
        message("Adding install rule for target \"${TARGET}\" (\"${FILE_PATH}\") to \"${PA_INSTALL_DIR}\"...")
        install(
            FILES "${FILE_PATH}"
            DESTINATION "${PA_INSTALL_DIR}"
            OPTIONAL
            COMPONENT ${PRAGMA_INSTALL_COMPONENT})
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
