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

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(${TARGET_NAME} PRIVATE "_WIN32_WINNT=0x0A00") # Windows 10
    endif()

    if(UNIX)
        target_link_options(${TARGET_NAME} PRIVATE --no-undefined)
    endif()

    # Due to msvc compiler bugs, we introduce a few macros as temporary workarounds.
    # Once constexpr works with modules under msvc, the macro can be removed.
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(${TARGET_NAME} PRIVATE "CONSTEXPR_COMPAT=inline const")
        target_compile_definitions(${TARGET_NAME} PRIVATE "CONSTEXPR_DLL_COMPAT=__declspec(dllexport) const")
        target_compile_definitions(${TARGET_NAME} PRIVATE "STATIC_CONST_COMPAT=inline const")
        target_compile_definitions(${TARGET_NAME} PRIVATE "STATIC_DLL_COMPAT=extern __declspec(dllexport)")
    else()
        target_compile_definitions(${TARGET_NAME} PRIVATE "CONSTEXPR_COMPAT=constexpr")
        target_compile_definitions(${TARGET_NAME} PRIVATE "CONSTEXPR_DLL_COMPAT=constexpr")
        target_compile_definitions(${TARGET_NAME} PRIVATE "STATIC_CONST_COMPAT=static const")
        target_compile_definitions(${TARGET_NAME} PRIVATE "STATIC_DLL_COMPAT=static")
    endif()

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_23)
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

    if(UNIX)
        set_target_properties(${TARGET_NAME} PROPERTIES
            INSTALL_RPATH "$ORIGIN"
        )
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
        set(_tmp "${CMAKE_MODULE_PATH}")
        list(APPEND _tmp "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
        set(CMAKE_MODULE_PATH "${_tmp}" PARENT_SCOPE)
    endif()
endfunction()

function(pr_add_rpath TARGET_NAME PATH)
    if(UNIX)
        message("[PR] Adding RPATH \"${PATH}\" to target ${TARGET_NAME}")
        set_property(TARGET ${TARGET_NAME} APPEND PROPERTY INSTALL_RPATH "${PATH}")
    endif()
endfunction()

function(pr_add_module TARGET_NAME LIB_TYPE MODULE_INSTALL_PATH)
    pr_add_library(${TARGET_NAME} ${LIB_TYPE})
    set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)

    # Build rpath to main lib directory
    if(UNIX)
        string(REGEX REPLACE "/$" "" _path_stripped "${MODULE_INSTALL_PATH}")
        string(REPLACE "/" ";" _parts "${_path_stripped}")
        list(LENGTH _parts _depth)

        set(_updirs "")
        math(EXPR _count "${_depth}")
        while(_count GREATER 0)
            set(_updirs "${_updirs}../")
            math(EXPR _count "${_count} - 1")
        endwhile()

        set(NEW_RPATH "$ORIGIN/${_updirs}lib")

        pr_add_rpath(${TARGET_NAME} "${NEW_RPATH}")
    endif()
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

    if(UNIX)
        set_target_properties(${TARGET_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
        )

        set_target_properties(${TARGET_NAME} PROPERTIES
            INSTALL_RPATH "$ORIGIN:$ORIGIN/lib"
        )
    endif()

    message("[PR] Adding executable ${TARGET_NAME}")
    pr_setup_default_project_settings(${TARGET_NAME})

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
        set(_tmp "${CMAKE_MODULE_PATH}")
        list(APPEND _tmp "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
        set(CMAKE_MODULE_PATH "${_tmp}" PARENT_SCOPE)
    endif()
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
