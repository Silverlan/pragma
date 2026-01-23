function(pr_git_checkout directory url)
    # optional arguments: commitId, branch
    if(ARGC GREATER 2)
        set(commitId "${ARGV2}")
    endif()

    if(ARGC GREATER 3)
        set(branch "${ARGV3}")
    else()
        set(branch "") # Explicitly clear it so it doesn't leak from parent
    endif()

    message(STATUS "Updating submodule '${directory}'...")

    # Directory exists but is not a git directory
    if(IS_DIRECTORY "${directory}" AND NOT IS_DIRECTORY "${directory}/.git")
        message(STATUS "Submodule directory already exists, but is not a git directory. Skipping update...")
        return()
    endif()

    get_filename_component(absDir "${directory}" ABSOLUTE)

    # Clone if directory doesn't exist or is empty
    if(NOT IS_DIRECTORY "${absDir}")
        message(STATUS "Cloning ${url}...")
        set(clone_args clone "${url}" "${directory}")
        if(branch)
            list(APPEND clone_args "-b" "${branch}")
        endif()

        execute_process(
            COMMAND git ${clone_args}
            RESULT_VARIABLE clone_res
        )
        if(NOT clone_res EQUAL 0)
            message(FATAL_ERROR "Failed to clone repository.")
        endif()
    endif()

    # Reset to commit ID
    if(NOT "${commitId}" STREQUAL "")
        # Get current HEAD SHA
        execute_process(
            COMMAND git rev-parse HEAD
            WORKING_DIRECTORY "${absDir}"
            OUTPUT_VARIABLE full_sha
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        # Check if current commit matches the requested commitId
        string(LENGTH "${commitId}" commit_len)
        string(SUBSTRING "${full_sha}" 0 ${commit_len} short_current)

        if("${short_current}" STREQUAL "${commitId}")
            message(STATUS "Already up-to-date at commit '${commitId}'.")
        else()
            message(STATUS "Resetting to commit '${commitId}'...")
            execute_process(
                COMMAND git reset --hard "${commitId}"
                WORKING_DIRECTORY "${absDir}"
                RESULT_VARIABLE reset_res
            )
            if(NOT reset_res EQUAL 0)
                message(FATAL_ERROR "Failed to reset to commit ${commitId}")
            endif()
        endif()
    else()
        # No commitId provided, just pull all changes
        execute_process(
            COMMAND git pull
            WORKING_DIRECTORY "${absDir}"
            RESULT_VARIABLE pull_res
        )
    endif()

    # Update submodules
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${absDir}"
        RESULT_VARIABLE sub_res
    )
    
    if(NOT sub_res EQUAL 0)
        message(FATAL_ERROR "Failed to update submodules recursively.")
    endif()
endfunction()

function(pr_checkout_submodule IDENTIFIER GIT_URL GIT_SHA CHECKOUT_PATH)
    pr_git_checkout("${CMAKE_SOURCE_DIR}/${CHECKOUT_PATH}" "${GIT_URL}" "${GIT_SHA}")
endfunction()
