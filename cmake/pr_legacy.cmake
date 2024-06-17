function(pr_set_target_folder TARGET FOLDER)
    if(TARGET ${TARGET})
        set_target_properties(${TARGET} PROPERTIES FOLDER ${FOLDER})
    endif()
endfunction(pr_set_target_folder)

function(register_third_party_library LIB_NAME)
    message("Processing third-party library '${LIB_NAME}'...")
    set(extra_macro_args ${ARGN})

    list(LENGTH extra_macro_args num_extra_args)
    if(${num_extra_args} GREATER 0)
        list(GET extra_macro_args 0 optional_arg)
        add_subdirectory(third_party_libs/${LIB_NAME} third_party_libs/${optional_arg})
        return()
    endif()
    add_subdirectory(third_party_libs/${LIB_NAME})
endfunction(register_third_party_library)

function(pr_include_third_party_library)
    set(options)
    set(oneValueArgs DIR TARGET INC LINK_ONLY NAME)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(SHOULD_INCLUDE_FOLDER 0)
    if(NOT ${PA_UNPARSED_ARGUMENTS} STREQUAL "")
        set(DIR_NAME "${PA_UNPARSED_ARGUMENTS}")
        set(SHOULD_INCLUDE_FOLDER 1)
    else()
        set(DIR_NAME "${PA_TARGET}")
    endif()
    if(DEFINED PA_NAME)
        set(IDENTIFIER "${PA_NAME}")
    else()
        set(IDENTIFIER "${DIR_NAME}")
    endif()
    pr_get_normalized_identifier_name(${IDENTIFIER})

    if(NOT DEFINED PA_DIR)
        set(PA_DIR "${DIR_NAME}")
    endif()

    if(NOT DEFINED PA_TARGET)
        set(PA_TARGET "${PA_DIR}")
    endif()

    if(NOT DEFINED PA_INC)
        set(PA_INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/${PA_TARGET}/include")
    endif()

    message("Including third-party library \"${IDENTIFIER}\" with target \"${PA_TARGET}\" and directory \"${PA_DIR}\"...")

    set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY "$<TARGET_LINKER_FILE:${PA_TARGET}>" CACHE STRING "Path to library." FORCE)
    if(NOT PA_LINK_ONLY)
        pr_set_include_path(${NORMALIZED_IDENTIFIER} "${PA_INC}")
    endif()

    if(SHOULD_INCLUDE_FOLDER)
        register_third_party_library("${PA_DIR}")
    endif()
    if(NOT PA_LINK_ONLY)
		# Some third-party libraries might be using older CMake functions that don't
		# make the include directory public, so we force it here.
		target_include_directories("${PA_TARGET}" INTERFACE "$<BUILD_INTERFACE:${PA_INC}>")
    endif()
    pr_set_target_folder("${PA_TARGET}" third_party_libs)
endfunction()

function(pr_find_library IDENTIFIER)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs NAMES PATHS)
    cmake_parse_arguments(PARSE_ARGV 1 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    pr_get_normalized_identifier_name(${IDENTIFIER})

    find_library(DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY NAMES ${PA_NAMES} PATHS ${PA_PATHS} NO_DEFAULT_PATH)
endfunction()
