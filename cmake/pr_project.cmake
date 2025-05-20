function(pr_setup_default_project_settings TARGET_NAME)
    if(MSVC)
        target_compile_options(
            ${TARGET_NAME}
            PUBLIC /wd4251 # needs to have dll-interface to be used by clients of class
                   /wd4996
                   /wd4244 # conversion from 'float' to 'unsigned __int64'
                   /wd4267 # conversion from 'size_t' to 'uint32_t'
                   /wd4305 # truncation from 'double' to 'float'
                   /wd4275 # non dll-interface class '*' used as base for dll-interface class '*'
        )

        target_compile_options(${TARGET_NAME} PRIVATE /bigobj)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-c++11-narrowing)
        target_compile_options(${TARGET_NAME} PUBLIC -Wno-missing-template-arg-list-after-template-kw)
        target_compile_options(${TARGET_NAME} PUBLIC -Wno-macro-redefined -Wno-potentially-evaluated-expression -Wno-switch)
    endif()

    if(UNIX)
        target_link_options(${TARGET_NAME} PRIVATE --no-undefined)
    endif()

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
    set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(${TARGET_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()

function(pr_project TARGET_NAME)
    project(${TARGET_NAME} CXX)
    message("[PR] ---------------------- Start Of Project \"${TARGET_NAME}\" ----------------------")
endfunction()

function(pr_precompile_headers TARGET_NAME PRECOMPILED_HEADER)
    message("Using precompiled header src/${PRECOMPILED_HEADER} for project ${TARGET_NAME}")
    target_precompile_headers(${TARGET_NAME} PRIVATE "src/${PRECOMPILED_HEADER}")

    # Disable precompiled headers for c-files
    file(GLOB_RECURSE SRC_C_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.c")
    set_source_files_properties("${SRC_C_FILES}" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endfunction()

function(pr_add_library TARGET_NAME LIB_TYPE)
    pr_project(${TARGET_NAME})
    add_library(${TARGET_NAME} ${LIB_TYPE})
    message("[PR] Adding library ${TARGET_NAME}")
    pr_setup_default_project_settings(${TARGET_NAME})
endfunction()

function(pr_add_executable TARGET_NAME)
    pr_project(${TARGET_NAME})

    if(WIN32)
        set(options CONSOLE)
        set(oneValueArgs APP_ICON_WIN APP_ICON_LIN DEBUGGER_LAUNCH_ARGS)
        set(multiValueArgs)
        cmake_parse_arguments(PARSE_ARGV 1 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")
        add_executable(${TARGET_NAME} WIN32 ${PA_APP_ICON_WIN})

        if(PA_CONSOLE)
            set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
        endif()
        if(DEFINED PA_DEBUGGER_LAUNCH_ARGS)
            set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${PA_DEBUGGER_LAUNCH_ARGS}")
        endif()
    else()
        # TODO: Apply icon using .desktop file
        add_executable(${TARGET_NAME})
    endif()

    message("[PR] Adding executable ${TARGET_NAME}")
    pr_setup_default_project_settings(${TARGET_NAME})
endfunction()

function(pr_finalize TARGET_NAME)
    set(options)
    set(oneValueArgs FOLDER)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 1 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(DEFINED PA_FOLDER)
        set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "${PA_FOLDER}")
        set_property(GLOBAL PROPERTY PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER 1)
    endif()

    message("[PR] ---------------------- End Of Project \"${TARGET_NAME}\" ----------------------")
endfunction()
