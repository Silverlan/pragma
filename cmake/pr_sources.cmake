function(pr_add_header_list TARGET_NAME HEADER_LIST)
    set(options)
    set(oneValueArgs TARGET_NAME HEADER_LIST)
    set(multiValueArgs BASE_DIRS)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")
    if(NOT DEFINED PA_BASE_DIRS)
        set(PA_BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    foreach(source IN LISTS HEADER_LIST)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" source_path_relative "${source_path}")
        string(REPLACE "/" "\\" source_path_msvc "${source_path_relative}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()

    target_sources(
        ${TARGET_NAME}
        PRIVATE FILE_SET
                headers
                TYPE
                HEADERS
                BASE_DIRS
                ${PA_BASE_DIRS}
                FILES
                ${HEADER_LIST})
endfunction()

function(pr_add_headers TARGET_NAME HEADER_LOCATION)
    set(options LINK_ONLY PRIVATE PUBLIC)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 3 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(VISIBILITY PUBLIC)
    if(PA_PRIVATE)
        set(VISIBILITY PRIVATE)
    endif()

    message("[PR] Adding include directory \"${HEADER_LOCATION}\" to target ${TARGET_NAME} with visibility ${VISIBILITY}")
    target_include_directories(${TARGET_NAME} ${VISIBILITY} "${HEADER_LOCATION}")

    set(options)
    set(oneValueArgs TARGET_NAME HEADER_LOCATION)
    set(multiValueArgs BASE_DIRS)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")
    if(NOT DEFINED PA_BASE_DIRS)
        set(PA_BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    message("[PR] Adding headers in location directory \"${CMAKE_SOURCE_DIR}/${HEADER_LOCATION}\"...")
    file(GLOB_RECURSE HEADER_LIST "${HEADER_LOCATION}/*.h" "${HEADER_LOCATION}/*.hpp")
    pr_add_header_list(${TARGET_NAME} "${HEADER_LIST}" "${BASE_DIRS}" BASE_DIRS "${PA_BASE_DIRS}")
endfunction()

function(pr_add_module_list TARGET_NAME FILE_SET_NAME)
    set(options LINK_ONLY PRIVATE PUBLIC)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 3 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(VISIBILITY PUBLIC)
    if(PA_PRIVATE)
        set(VISIBILITY PRIVATE)
    endif()

    target_sources(${TARGET_NAME} ${VISIBILITY} FILE_SET ${FILE_SET_NAME} TYPE CXX_MODULES FILES ${PA_UNPARSED_ARGUMENTS})
    source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${PA_UNPARSED_ARGUMENTS})
endfunction()

function(pr_add_modules TARGET_NAME MODULE_LOCATION)
    if(NOT "${MODULE_LOCATION}" MATCHES "/$")
        # Append a trailing slash if not present
        set(MODULE_LOCATION "${MODULE_LOCATION}/")
    endif()

    set(options)
    set(oneValueArgs)
    set(multiValueArgs EXCLUDE)
    cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_EXCLUDE)
        set(PA_EXCLUDE "")
    endif()

    message("[PR] Adding modules in location directory \"${CMAKE_SOURCE_DIR}/${MODULE_LOCATION}\"...")

    set(EXCLUSION_MODULE_LIST "")
    foreach(EXCLUSION_FILTER ${PA_EXCLUDE})
        file(GLOB_RECURSE EXCLUSION_FILTER_LIST "${EXCLUSION_FILTER}")
        list(APPEND EXCLUSION_MODULE_LIST ${EXCLUSION_FILTER_LIST})
    endforeach()

    # 1. Get all .cppm files in ${MODULE_LOCATION}implementation recursively and add as PRIVATE
    file(GLOB_RECURSE IMPLEMENTATION_MODULE_LIST "${MODULE_LOCATION}implementation/*.cppm")
    if (IMPLEMENTATION_MODULE_LIST)
        foreach(EXCLUDED_FILE ${EXCLUSION_MODULE_LIST})
            list(REMOVE_ITEM IMPLEMENTATION_MODULE_LIST "${EXCLUDED_FILE}")
        endforeach()
        pr_add_module_list(${TARGET_NAME} cxx_modules_impl PRIVATE "${IMPLEMENTATION_MODULE_LIST}")
    endif()

    # 2. Get all .cppm files in ${MODULE_LOCATION} recursively excluding the ones in ${MODULE_LOCATION}implementation/
    file(GLOB_RECURSE ALL_MODULE_LIST "${MODULE_LOCATION}*.cppm")

    # Filter out the implementation files from ALL_MODULE_LIST
    set(PUBLIC_MODULE_LIST "")
    foreach(MODULE_FILE ${ALL_MODULE_LIST})
        string(FIND ${MODULE_FILE} "/implementation/" FOUND)
        if (FOUND EQUAL -1) # If "/implementation/" is not found in the path
            list(APPEND PUBLIC_MODULE_LIST ${MODULE_FILE})
        endif()
    endforeach()

    # Add the PUBLIC modules to the target
    if (PUBLIC_MODULE_LIST)
        foreach(EXCLUDED_FILE ${EXCLUSION_MODULE_LIST})
            list(REMOVE_ITEM PUBLIC_MODULE_LIST "${EXCLUDED_FILE}")
        endforeach()
        pr_add_module_list(${TARGET_NAME} cxx_modules PUBLIC ${PUBLIC_MODULE_LIST})
    endif()
endfunction()


function(pr_add_source_list TARGET_NAME SOURCE_LIST)
    foreach(source IN LISTS SOURCE_LIST)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" source_path_relative "${source_path}")
        string(REPLACE "/" "\\" source_path_msvc "${source_path_relative}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()

    target_sources(${TARGET_NAME} PRIVATE ${SOURCE_LIST})
endfunction()

function(pr_add_sources TARGET_NAME SOURCE_LOCATION)
    if(NOT "${SOURCE_LOCATION}" MATCHES "/$")
        # Append a trailing slash if not present
        set(SOURCE_LOCATION "${SOURCE_LOCATION}/")
    endif()

    set(options)
    set(oneValueArgs)
    set(multiValueArgs EXCLUDE FILTER)
    cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT DEFINED PA_EXCLUDE)
        set(PA_EXCLUDE "")
    endif()

    set(VISIBILITY PRIVATE)
    message("[PR] Adding include directory \"${SOURCE_LOCATION}\" to target ${TARGET_NAME} with visibility ${VISIBILITY}")
    target_include_directories(${TARGET_NAME} ${VISIBILITY} "${SOURCE_LOCATION}")

    message("[PR] Adding sources in location directory \"${CMAKE_SOURCE_DIR}/${SOURCE_LOCATION}\"...")

    set(EXCLUSION_MODULE_LIST "")
    foreach(EXCLUSION_FILTER ${PA_EXCLUDE})
        file(GLOB_RECURSE EXCLUSION_FILTER_LIST "${EXCLUSION_FILTER}")
        list(APPEND EXCLUSION_MODULE_LIST ${EXCLUSION_FILTER_LIST})
    endforeach()

	if(DEFINED PA_FILTER)
		file(GLOB_RECURSE SOURCE_LIST ${PA_FILTER})
	else()
		file(GLOB_RECURSE SOURCE_LIST "${SOURCE_LOCATION}*.c" "${SOURCE_LOCATION}*.cpp")
	endif()
    foreach(EXCLUDED_FILE ${EXCLUSION_MODULE_LIST})
        list(REMOVE_ITEM SOURCE_LIST "${EXCLUDED_FILE}")
    endforeach()
    pr_add_source_list(${TARGET_NAME} "${SOURCE_LIST}")
endfunction()

function(pr_add_compile_definitions TARGET_NAME)
    set(options PRIVATE PUBLIC)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 1 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(VISIBILITY PRIVATE)
    if(PA_PUBLIC)
        set(VISIBILITY PUBLIC)
    endif()

    message("[PR] Adding compile definitions ${PA_UNPARSED_ARGUMENTS} to target ${TARGET_NAME} with visibility ${VISIBILITY}")
    target_compile_definitions(${TARGET_NAME} ${VISIBILITY} ${PA_UNPARSED_ARGUMENTS})
endfunction()
