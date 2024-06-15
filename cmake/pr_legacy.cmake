function(set_target_folder TARGET FOLDER)
    if(TARGET ${TARGET})
        set_target_properties(${TARGET} PROPERTIES FOLDER ${FOLDER})
    endif()
endfunction(set_target_folder)

function(resolve_links IDS)
    foreach(ID IN LISTS IDS)
        pr_get_normalized_identifier_name(${ID})
        get_filename_component(TMP_DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY "${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY}" REALPATH)
        set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY
            ${TMP_DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY}
            CACHE PATH "Path to library." FORCE)
    endforeach()
endfunction()

function(register_third_party_library LIB_NAME)
    message("Processing third-party library '${LIB_NAME}'...")
    set(extra_macro_args ${ARGN})

    list(LENGTH extra_macro_args num_extra_args)
    if(${num_extra_args} GREATER 0)
        list(GET extra_macro_args 0 optional_arg)
        add_subdirectory(third_party_libs/${LIB_NAME} third_party_libs/${optional_arg} EXCLUDE_FROM_ALL)
        return()
    endif()
    add_subdirectory(third_party_libs/${LIB_NAME} EXCLUDE_FROM_ALL)
endfunction(register_third_party_library)

function(pr_include_third_party_library)
    set(options)
    set(oneValueArgs DIR TARGET INC LINK_ONLY)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(SHOULD_INCLUDE_FOLDER 0)
    if(NOT ${PA_UNPARSED_ARGUMENTS} STREQUAL "")
        set(IDENTIFIER "${PA_UNPARSED_ARGUMENTS}")
        set(SHOULD_INCLUDE_FOLDER 1)
    else()
        set(IDENTIFIER "${PA_TARGET}")
    endif()
    pr_get_normalized_identifier_name(${IDENTIFIER})

    if(NOT DEFINED PA_DIR)
        string(TOLOWER "${NORMALIZED_IDENTIFIER}" LOWER_NORMALIZED_IDENTIFIER)
        set(PA_DIR "${LOWER_NORMALIZED_IDENTIFIER}")
    endif()

    if(NOT DEFINED PA_TARGET)
        set(PA_TARGET "${PA_DIR}")
    endif()

    if(NOT DEFINED PA_INC)
        set(PA_INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/${PA_TARGET}/include")
    endif()

    set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY "$<TARGET_LINKER_FILE:${PA_TARGET}>" CACHE STRING "Path to library." FORCE)
    if(NOT PA_LINK_ONLY)
        pr_set_include_path(${NORMALIZED_IDENTIFIER} "${PA_INC}")
    endif()

    if(SHOULD_INCLUDE_FOLDER)
        register_third_party_library("${PA_DIR}")
    endif()
    set_target_folder("${PA_TARGET}" third_party_libs)

    pragma_install_lib("${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY}" "${BINARY_OUTPUT_DIR}")
endfunction()
