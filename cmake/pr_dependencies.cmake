function(pr_set_include_path IDENTIFIER PATH)
    set(options FORCE)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    pr_get_normalized_identifier_name(${IDENTIFIER})

    set(ADDITIONAL_ARGS "")
    if(PA_FORCE)
        set(ADDITIONAL_ARGS "FORCE")
    endif()

    set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_INCLUDE
        ${PATH}
        CACHE PATH "Path to ${IDENTIFIER} include directory." ${ADDITIONAL_ARGS})
endfunction()

function(pr_set_raw_library_path IDENTIFIER PATH)
    set(options FORCE)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    pr_get_normalized_identifier_name(${IDENTIFIER})

    set(ADDITIONAL_ARGS "")
    if(PA_FORCE)
        set(ADDITIONAL_ARGS "FORCE")
    endif()

    set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY
        "${PATH}"
        CACHE FILEPATH "Path to ${IDENTIFIER} library." ${ADDITIONAL_ARGS})
endfunction()

function(pr_set_library_path IDENTIFIER)
    set(options FORCE)
    set(oneValueArgs TARGET)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_TARGET)
        set(PA_TARGET "${IDENTIFIER}")
    endif()

    pr_get_normalized_identifier_name(${IDENTIFIER})

    set(ADDITIONAL_ARGS "")
    if(PA_FORCE)
        set(ADDITIONAL_ARGS "FORCE")
    endif()

    set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY
        "$<TARGET_LINKER_FILE:${PA_TARGET}>"
        CACHE FILEPATH "Path to ${PRETTY_IDENTIFIER} library." ${ADDITIONAL_ARGS})
endfunction()

function(pr_add_include_dir TARGET_NAME IDENTIFIER)
    set(options PRIVATE PUBLIC)
    set(oneValueArgs)
    set(multiValueArgs LIB_NAME)
    cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(VISIBILITY PRIVATE)
    if(PA_PUBLIC)
        set(VISIBILITY PUBLIC)
    endif()

    pr_get_normalized_identifier_name(${IDENTIFIER})
    pr_set_include_path(${IDENTIFIER} "")

    message(
        "[PR] Adding include directory \"${DEPENDENCY_${NORMALIZED_IDENTIFIER}_INCLUDE}\" (${NORMALIZED_IDENTIFIER}) to target ${TARGET_NAME} with visibility ${VISIBILITY}"
    )
    target_include_directories(${TARGET_NAME} ${VISIBILITY} ${DEPENDENCY_${NORMALIZED_IDENTIFIER}_INCLUDE})
endfunction()

function(pr_link_library TARGET_NAME IDENTIFIER)
    set(options PRIVATE PUBLIC)
    set(oneValueArgs)
    set(multiValueArgs LIB_NAME)
    cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(VISIBILITY PRIVATE)
    if(PA_PUBLIC)
        set(VISIBILITY PUBLIC)
    endif()

    pr_get_normalized_identifier_name(${IDENTIFIER})
    pr_set_library_path(${IDENTIFIER} "")

    message(
        "[PR] Linking library \"${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY}\" (${NORMALIZED_IDENTIFIER}) to target ${TARGET_NAME} with visibility ${VISIBILITY}"
    )
    target_link_libraries(${TARGET_NAME} ${VISIBILITY} ${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY})
endfunction()

function(pr_include_and_link_library TARGET_NAME IDENTIFIER)
    set(options PRIVATE PUBLIC)
    set(oneValueArgs)
    set(multiValueArgs LIB_NAME)
    cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(VISIBILITY PRIVATE)
    if(PA_PUBLIC)
        set(VISIBILITY PUBLIC)
    endif()

    pr_get_normalized_identifier_name(${IDENTIFIER})

    pr_set_include_path(${IDENTIFIER} "")
    message(
        "[PR] Adding include directory \"${DEPENDENCY_${NORMALIZED_IDENTIFIER}_INCLUDE}\" (${NORMALIZED_IDENTIFIER}) to target ${TARGET_NAME} with visibility ${VISIBILITY}"
    )
    target_include_directories(${TARGET_NAME} ${VISIBILITY} ${DEPENDENCY_${NORMALIZED_IDENTIFIER}_INCLUDE})

    pr_set_library_path(${IDENTIFIER} "")
    message(
        "[PR] Linking library \"${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY}\" (${NORMALIZED_IDENTIFIER}) to target ${TARGET_NAME} with visibility ${VISIBILITY}"
    )
    target_link_libraries(${TARGET_NAME} ${VISIBILITY} ${DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY})
endfunction()

function(pr_set_default_include_path IDENTIFIER PATH)
    pr_get_normalized_identifier_name(${IDENTIFIER})
    if(NOT DEFINED DEPENDENCY_${NORMALIZED_IDENTIFIER}_INCLUDE)
        pr_set_include_path(${IDENTIFIER} "${PATH}")
    endif()
endfunction()

function(pr_set_default_library_file_path IDENTIFIER LIB_PATH LIB_TYPE)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs LIB_NAME)
    cmake_parse_arguments(PARSE_ARGV 3 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(LIB_BASE_FILE_NAME "${IDENTIFIER}")
    if(DEFINED PA_LIB_NAME)
        set(LIB_BASE_FILE_NAME "${PA_LIB_NAME}")
    endif()

    pr_get_normalized_identifier_name(${IDENTIFIER})
    if(NOT DEFINED DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY)
        set(LIB_SUB_DIR "")
        if(WIN32)
            set(LIB_FILE_NAME "${LIB_BASE_FILE_NAME}.lib")
            set(LIB_SUB_DIR "/$<CONFIG>")
        else()
            set(LIB_PREFIX "")
            if(DEPENDENCY_TYPE STREQUAL "STATIC")
                set(FILE_EXTENSION ".a")
                set(LIB_PREFIX "lib")
            else()
                set(FILE_EXTENSION ".so")
            endif()

            set(LIB_FILE_NAME "${LIB_PREFIX}${LIB_BASE_FILE_NAME}${FILE_EXTENSION}")
        endif()
        pr_set_library_path(${IDENTIFIER} "${LIB_PATH}${LIB_SUB_DIR}/${LIB_FILE_NAME}")
    endif()
endfunction()

function(pr_add_external_dependency TARGET_NAME IDENTIFIER DEPENDENCY_TYPE)
    set(options LINK_ONLY PRIVATE PUBLIC)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 3 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(VISIBILITY PRIVATE)
    if(PA_PUBLIC)
        set(VISIBILITY PUBLIC)
    endif()

    if(NOT PA_LINK_ONLY)
        pr_add_include_dir(${TARGET_NAME} ${IDENTIFIER} ${VISIBILITY})
    endif()

    if(DEPENDENCY_TYPE STREQUAL "LIBRARY")
        if(DEFINED PA_UNPARSED_ARGUMENTS AND NOT PA_UNPARSED_ARGUMENTS STREQUAL "")
            set(DEPENDENCY_TARGET_NAME ${PA_UNPARSED_ARGUMENTS})
        else()
            set(DEPENDENCY_TARGET_NAME ${TARGET_NAME})
        endif()

        pr_link_library(${TARGET_NAME} ${IDENTIFIER} ${VISIBILITY})

        add_dependencies(${TARGET_NAME} ${DEPENDENCY_TARGET_NAME})
    endif()
endfunction()

function(pr_add_dependency TARGET_NAME DEPENDENCY_TARGET_NAME DEPENDENCY_TYPE)
    set(options LINK_ONLY PRIVATE PUBLIC)
    set(oneValueArgs BIN_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 3 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(VISIBILITY PRIVATE)
    if(PA_PUBLIC)
        set(VISIBILITY PUBLIC)
    endif()

    if(NOT PA_LINK_ONLY AND NOT DEPENDENCY_TYPE STREQUAL "TARGET")
        pr_add_include_dir(${TARGET_NAME} ${DEPENDENCY_TARGET_NAME} ${VISIBILITY})
    endif()

    if(DEPENDENCY_TYPE STREQUAL "TARGET" OR DEPENDENCY_TYPE STREQUAL "MODULE")
        message("[PR] Linking target \"${DEPENDENCY_TARGET_NAME}\" to target ${TARGET_NAME} with visibility ${VISIBILITY}")
        target_link_libraries(${TARGET_NAME} ${VISIBILITY} "${DEPENDENCY_TARGET_NAME}")

        add_dependencies(${TARGET_NAME} ${DEPENDENCY_TARGET_NAME})
    endif()

    if(DEPENDENCY_TYPE STREQUAL "LIBRARY")
        message("[PR] Linking library \"${DEPENDENCY_TARGET_NAME}\" to target ${TARGET_NAME} with visibility ${VISIBILITY}")
        pr_link_library(${TARGET_NAME} ${DEPENDENCY_TARGET_NAME} ${VISIBILITY})
    endif()

    if(WIN32 AND DEPENDENCY_TYPE STREQUAL "MODULE")
        set(BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        if(DEFINED PA_BIN_DIR)
            set(BIN_DIR "${PA_BIN_DIR}")
        endif()
        pr_reference_module(${TARGET_NAME} ${DEPENDENCY_TARGET_NAME} "${BIN_DIR}" IGNORE_PATH)
    endif()
endfunction()
