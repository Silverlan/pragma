function(pr_add_module_reference TARGET_NAME IDENTIFIER)
    if(WIN32)
        set(options PRIVATE PUBLIC)
        set(oneValueArgs)
        set(multiValueArgs)
        cmake_parse_arguments(PARSE_ARGV 3 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

        set(VISIBILITY PRIVATE)
        if(PA_PUBLIC)
            set(VISIBILITY PUBLIC)
        endif()

        pr_get_normalized_identifier_name(${IDENTIFIER})

		set(DEFAULT_MODULE "")
		set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_MODULE ${DEFAULT_MODULE} CACHE PATH "Path to ${PRETTY_IDENTIFIER} module file.")

		message("[PR] Adding module reference \"${DEPENDENCY_${NORMALIZED_IDENTIFIER}_MODULE}\" (${NORMALIZED_IDENTIFIER}) to target ${TARGET_NAME}")
        # Note: There must *not* be a space between "/reference" and the path, otherwise only one /reference will be recoginized.
        target_compile_options(${PROJ_NAME} ${VISIBILITY} /reference${DEPENDENCY_${NORMALIZED_IDENTIFIER}_MODULE})
	endif()
endfunction()

function(pr_get_module_file_path IDENTIFIER BIN_PATH)
    if(WIN32)
        set(options)
        set(oneValueArgs TARGET_NAME)
        set(multiValueArgs)
        cmake_parse_arguments(PARSE_ARGV 2 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

        set(MODULE_TARGET_NAME "${IDENTIFIER}")
        if(DEFINED PA_TARGET_NAME)
            set(MODULE_TARGET_NAME "${PA_TARGET_NAME}")
        endif()

        set(LIB_SUB_DIR "")
        set(LIB_FILE_NAME "${IDENTIFIER}.cppm.ifc")
        set(LIB_SUB_DIR "/${MODULE_TARGET_NAME}.dir/$<CONFIG>")
        set(MODULE_FILE_PATH "${BIN_PATH}${LIB_SUB_DIR}/${LIB_FILE_NAME}")
        return(PROPAGATE MODULE_FILE_PATH)
    endif()
endfunction()

function(pr_reference_module TARGET_NAME MODULE_NAME BIN_DIR)
    if(WIN32)
        set(options PRIVATE PUBLIC)
        set(oneValueArgs TARGET_NAME)
        set(multiValueArgs)
        cmake_parse_arguments(PARSE_ARGV 3 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

        set(MODULE_TARGET_NAME "${MODULE_NAME}")
        if(DEFINED PA_TARGET_NAME)
            set(MODULE_TARGET_NAME "${PA_TARGET_NAME}")
        endif()

        set(VISIBILITY PRIVATE)
        if(PA_PUBLIC)
            set(VISIBILITY PUBLIC)
        endif()

        pr_get_module_file_path(${MODULE_NAME} ${BIN_DIR} TARGET_NAME "${MODULE_TARGET_NAME}")

        pr_get_normalized_identifier_name(${MODULE_NAME})
        set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_MODULE "${MODULE_FILE_PATH}" CACHE PATH "Path to ${PRETTY_IDENTIFIER} module file.")

        pr_add_module_reference(${TARGET_NAME} ${MODULE_NAME} ${VISIBILITY})
    endif()
endfunction()
