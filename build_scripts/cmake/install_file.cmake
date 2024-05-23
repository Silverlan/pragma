if(EXISTS "${InstallSrc}")
    if(UNIX)
        # If the file is a link, we want to copy both the link and the file that it links to
        get_filename_component(INPUT_FILE_REAL_PATH "${InstallSrc}" REALPATH)
        cmake_path(GET INPUT_FILE_REAL_PATH PARENT_PATH INPUT_LIB_DIR)

        execute_process(
                COMMAND find -L "${INPUT_LIB_DIR}" -samefile "${INPUT_FILE_REAL_PATH}"
                WORKING_DIRECTORY "${INPUT_LIB_DIR}"
                OUTPUT_VARIABLE LINKS_RESULT
        )

        unset(INPUT_FILE_REAL_PATH) #Safe to unset now.
        string(REPLACE "\n" ";" FILES_TO_COPY "${LINKS_RESULT}")

        foreach(TARGET_FILE ${FILES_TO_COPY})
            message("Copying ${TARGET_FILE} to ${InstallDst}...")
            file(COPY "${TARGET_FILE}" DESTINATION "${InstallDst}")
        endforeach()
    else()
	    message("Copying \"${InstallSrc}\" to \"${InstallDst}\"...")
	    file(COPY "${InstallSrc}" DESTINATION "${InstallDst}")
    endif()
else()
	if( NOT "${InstallSrc}" STREQUAL "" )
		message("Unable to copy ${InstallSrc}: File not found.")
	endif()
endif()