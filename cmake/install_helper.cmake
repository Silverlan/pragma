function(check_content_version BASE_PATH CONTENTS FILENAME RESULT_VAR)
    set(TARGET_FILE "${BASE_PATH}/${FILENAME}")
    set(${RESULT_VAR} FALSE PARENT_SCOPE)

    if(NOT EXISTS "${TARGET_FILE}")
        if(EXISTS "${BASE_PATH}")
            # if the target path already exists, but there is no version file,
            # we'll assume it's already populated by other means (e.g. build manually)
            # and we'll leave it alone.
            set(${RESULT_VAR} TRUE PARENT_SCOPE)
        endif()
        return()
    endif()

    file(READ "${TARGET_FILE}" RAW_JSON)

    # Extract version
    string(JSON CURRENT_VERSION GET "${RAW_JSON}" version)

    if(NOT "${CURRENT_VERSION}" STREQUAL "${CONTENTS}")      
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(clear_content BASE_PATH)
    if(EXISTS "${BASE_PATH}")
        message(STATUS "Removing directory '${BASE_PATH}'...")
        file(REMOVE_RECURSE "${BASE_PATH}")
    endif()
endfunction()

function(update_content_version BASE_PATH COMMIT_ID FILENAME)
    message(STATUS "Creating directory '${BASE_PATH}'...")
    file(MAKE_DIRECTORY "${BASE_PATH}")

    set(TARGET_FILE "${BASE_PATH}/${FILENAME}")
    message(STATUS "Writing content version to '${TARGET_FILE}'...")

    # Construct minimal JSON string
    set(JSON_PAYLOAD "{\n  \"version\": \"${COMMIT_ID}\"\n}")
    file(WRITE "${TARGET_FILE}" "${JSON_PAYLOAD}")
endfunction()

function(pr_http_extract url dest_dir)
    set(options STRIP_TOP_DIR)
    cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

    get_filename_component(fname "${url}" NAME)
    set(zip_path "${CMAKE_BINARY_DIR}/_downloads/${fname}")
    
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/_downloads")
    message(STATUS "Downloading ${url}...")
    file(DOWNLOAD "${url}" "${zip_path}" SHOW_PROGRESS STATUS dl_status)
    
    list(GET dl_status 0 dl_code)
    if(NOT dl_code EQUAL 0)
        message(FATAL_ERROR "Failed to download ${url}: ${dl_status}")
    endif()

    message(STATUS "Extracting...")
    if(ARG_STRIP_TOP_DIR)
        set(tmp_dir "${dest_dir}_tmp")
        file(REMOVE_RECURSE "${tmp_dir}")
        file(ARCHIVE_EXTRACT INPUT "${zip_path}" DESTINATION "${tmp_dir}")
        
        # Find the single top-level directory
        file(GLOB subdirs RELATIVE "${tmp_dir}" "${tmp_dir}/*")
        list(LENGTH subdirs count)
        
        if(NOT count EQUAL 1)
            message(FATAL_ERROR "STRIP_TOP_DIR failed: archive contains ${count} items at root instead of 1.")
        endif()

        file(RENAME "${tmp_dir}/${subdirs}" "${dest_dir}")
        file(REMOVE_RECURSE "${tmp_dir}")
    else()
        file(MAKE_DIRECTORY "${dest_dir}")
        file(ARCHIVE_EXTRACT INPUT "${zip_path}" DESTINATION "${dest_dir}")
    endif()
endfunction()

function(pr_fetch_prebuilt_binaries base_path base_url version)
    set(options)
    set(oneValueArgs TOOLSET PRIMARY_DIR)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 3 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(DEFINED PA_TOOLSET AND NOT "${PA_TOOLSET}" STREQUAL "")
        set(effective_toolset "${PA_TOOLSET}")
    else()
        set(effective_toolset "${TOOLSET}")
    endif()

    if(PA_PRIMARY_DIR)
        set(check_path "${PA_PRIMARY_DIR}")
    else()
        set(check_path "${base_path}")
    endif()

    check_content_version("${check_path}" "${version}" "version.json" IS_VALID)
    if(NOT IS_VALID)
        # TODO: Ideally all directories that have previously been extracted should be removed
        clear_content("${check_path}")
        update_content_version("${check_path}" "${version}" "version.json")
        
        if(UNIX)
            set(platform_name "linux")
            set(prebuilt_archive_format "tar.gz")
        else()
            set(platform_name "windows")
            set(prebuilt_archive_format "zip")
        endif()

        set(prebuilt_archive_name "binaries-${platform_name}-x64-${effective_toolset}.${prebuilt_archive_format}")
        
        pr_http_extract(
            "${base_url}/${version}/${prebuilt_archive_name}"
            "${base_path}"
        )
    endif()
endfunction()
