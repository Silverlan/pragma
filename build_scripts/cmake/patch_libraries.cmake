execute_process(
    COMMAND bash "${CMAKE_CURRENT_LIST_DIR}/patch_libraries.sh" "${INSTALL_LOCATION}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/"
)
