# ManifestInstall.cmake
# Variables: SRC_DIR, DST_DIR, MANIFEST_FILE

if(NOT EXISTS "${DST_DIR}")
    file(MAKE_DIRECTORY "${DST_DIR}")
endif()

# 1. Load existing manifest
set(KNOWN_FILES "")
if(EXISTS "${MANIFEST_FILE}")
    file(STRINGS "${MANIFEST_FILE}" MANIFEST_LINES)
    foreach(LINE ${MANIFEST_LINES})
        string(REPLACE "|" ";" LIST_LINE "${LINE}")
        list(GET LIST_LINE 0 REL_PATH)
        list(GET LIST_LINE 1 FILE_HASH)
        set(HASH_${REL_PATH} "${FILE_HASH}")
        list(APPEND KNOWN_FILES "${REL_PATH}")
    endforeach()
endif()

# 2. Scan Source Files
file(GLOB_RECURSE ALL_SRC_FILES RELATIVE "${SRC_DIR}" "${SRC_DIR}/*")
set(NEW_MANIFEST_CONTENT "")

foreach(FILE ${ALL_SRC_FILES})
    if(FILE MATCHES "^\\.git/" OR FILE MATCHES "/\\.git/")
        continue()
    endif()

    set(S_PATH "${SRC_DIR}/${FILE}")
    set(D_PATH "${DST_DIR}/${FILE}")
    get_filename_component(REL_DIR "${FILE}" DIRECTORY)
    set(D_SUBDIR "${DST_DIR}/${REL_DIR}")

    file(SHA256 "${S_PATH}" S_HASH)
    set(SHOULD_COPY FALSE)
    set(MANIFEST_HASH_TO_SAVE "${S_HASH}") # Default: save the new source hash

    if(NOT EXISTS "${D_PATH}")
        message(STATUS "[ManifestInstall] New file: ${FILE}")
        set(SHOULD_COPY TRUE)
    else()
        file(SHA256 "${D_PATH}" D_HASH)
        
        # Check if user modified the destination
        if(DEFINED HASH_${FILE} AND NOT "${D_HASH}" STREQUAL "${HASH_${FILE}}")
            # If file was modified by user, we'll skip the file and keep the old manifest hash.
            # This is to avoid overwriting user changes.
            message(STATUS "[ManifestInstall] PRESERVING USER CHANGES: ${FILE}")
            set(MANIFEST_HASH_TO_SAVE "${HASH_${FILE}}")
            set(SHOULD_COPY FALSE)
        elseif(NOT "${S_HASH}" STREQUAL "${D_HASH}")
            # File is 'clean' (matches manifest or no manifest entry) but differs from source
            message(STATUS "[ManifestInstall] Updating: ${FILE}")
            set(SHOULD_COPY TRUE)
        endif()
    endif()

    if(SHOULD_COPY)
        file(MAKE_DIRECTORY "${D_SUBDIR}")
        file(COPY "${S_PATH}" DESTINATION "${D_SUBDIR}")
    endif()

    string(APPEND NEW_MANIFEST_CONTENT "${FILE}|${MANIFEST_HASH_TO_SAVE}\n")
    list(REMOVE_ITEM KNOWN_FILES "${FILE}")
endforeach()

# 3. Handle Deletions
foreach(OLD_FILE ${KNOWN_FILES})
    set(D_PATH "${DST_DIR}/${OLD_FILE}")
    if(EXISTS "${D_PATH}")
        file(SHA256 "${D_PATH}" D_HASH)
        if("${D_HASH}" STREQUAL "${HASH_${OLD_FILE}}")
            message(STATUS "[ManifestInstall] Removing: ${OLD_FILE}")
            file(REMOVE "${D_PATH}")
        else()
            message(STATUS "[ManifestInstall] Preserving deleted asset (modified by user): ${OLD_FILE}")
            string(APPEND NEW_MANIFEST_CONTENT "${OLD_FILE}|${HASH_${OLD_FILE}}\n")
        endif()
    endif()
endforeach()

file(WRITE "${MANIFEST_FILE}" "${NEW_MANIFEST_CONTENT}")
