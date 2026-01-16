set_property(GLOBAL PROPERTY PRAGMA_MODULE_NAMES "")
set_property(GLOBAL PROPERTY PRAGMA_MODULE_TARGETS "")

function(pr_init_module_properties IDENTIFIER)
    get_property(global_names GLOBAL PROPERTY PRAGMA_MODULE_NAMES)
    set(global_names ${global_names} ${IDENTIFIER})
    set_property(GLOBAL PROPERTY PRAGMA_MODULE_NAMES ${global_names})

    set_property(GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_TARGETS "")
    set_property(GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_SKIP_TARGET_PROPERTY_FOLDER 0)
    set_property(GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_SKIP_INSTALL 0)
endfunction()

function(pr_update_module_target_properties IDENTIFIER)
    get_property(PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_SKIP_TARGET_PROPERTY_FOLDER)
    get_property(_targets GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_TARGETS)

    if (NOT _targets)
        set(_targets ${IDENTIFIER})
    endif()

    set_property(GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_TARGETS ${_targets})

    get_property(global_targets GLOBAL PROPERTY PRAGMA_MODULE_TARGETS)
    set(global_targets ${global_targets} ${_targets})
    set_property(GLOBAL PROPERTY PRAGMA_MODULE_TARGETS ${global_targets})
    
    set(MODULE_TARGETS ${MODULE_TARGETS} ${_targets} PARENT_SCOPE)

    if (PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER EQUAL 0)
        if(TARGET ${IDENTIFIER})
            set_target_properties(${IDENTIFIER} PROPERTIES FOLDER modules)
        endif()
    endif()
endfunction()

function(pr_include_module IDENTIFIER)
    pr_init_module_properties(${IDENTIFIER})
    add_subdirectory("modules/${IDENTIFIER}")
    pr_update_module_target_properties(${IDENTIFIER})
endfunction()

include("cmake/fetch_modules.cmake")

foreach(IDENTIFIER ${PRAGMA_FETCHED_MODULES})
    pr_include_module(${IDENTIFIER})
endforeach()
