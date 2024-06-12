function(pr_setup_default_project_settings TARGET_NAME)
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /wd4251)
        target_compile_options(${TARGET_NAME} PRIVATE /wd4996)

        target_compile_options(${TARGET_NAME} PRIVATE /bigobj)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-c++11-narrowing)
    endif()

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
    set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(${TARGET_NAME} PROPERTIES POSITION_INDEPENDENT_CODE
                                                    ON)
endfunction()

function(pr_project TARGET_NAME)
    project(${TARGET_NAME} CXX)
    message(
        "[PR] ---------------------- Start Of Project \"${TARGET_NAME}\" ----------------------"
    )
endfunction()

function(pr_add_library TARGET_NAME LIB_TYPE)
    pr_project(${TARGET_NAME})
    add_library(${TARGET_NAME} ${LIB_TYPE})
    message("[PR] Adding library ${TARGET_NAME}")
    pr_setup_default_project_settings(${TARGET_NAME})
endfunction()

function(pr_add_executable TARGET_NAME)
    pr_project(${TARGET_NAME})
    add_executable(${TARGET_NAME})
    message("[PR] Adding executable ${TARGET_NAME}")
    pr_setup_default_project_settings(${TARGET_NAME})
endfunction()

function(pr_finalize TARGET_NAME)
    set(options)
    set(oneValueArgs FOLDER)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 1 PA "${options}" "${oneValueArgs}"
                          "${multiValueArgs}")

    if(DEFINED PA_FOLDER)
        set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "${PA_FOLDER}")
        set_property(GLOBAL PROPERTY PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER
                                     1)
    endif()

    message(
        "[PR] ---------------------- End Of Project \"${TARGET_NAME}\" ----------------------"
    )
endfunction()
