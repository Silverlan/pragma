set(PCK "opencv")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES opencv2/opencv.hpp
  HINTS
    ${PRAGMA_DEPS_DIR}/opencv/include
)

find_library(${PCK}_LIB_CORE      NAMES opencv_core      HINTS ${PRAGMA_DEPS_DIR}/opencv/lib)
find_library(${PCK}_LIB_IMGCODECS NAMES opencv_imgcodecs HINTS ${PRAGMA_DEPS_DIR}/opencv/lib)
find_library(${PCK}_LIB_IMGPROC   NAMES opencv_imgproc   HINTS ${PRAGMA_DEPS_DIR}/opencv/lib)

set(REQ_VARS ${PCK}_LIB_CORE ${PCK}_LIB_IMGCODECS ${PCK}_LIB_IMGPROC ${PCK}_INCLUDE_DIR)

if(WIN32)
  find_file(${PCK}_RT_CORE NAMES opencv_core470.dll HINTS ${PRAGMA_DEPS_DIR}/opencv/bin)
  find_file(${PCK}_RT_IMGCODECS NAMES opencv_imgcodecs470.dll HINTS ${PRAGMA_DEPS_DIR}/opencv/bin)
  find_file(${PCK}_RT_IMGPROC NAMES opencv_imgproc470.dll HINTS ${PRAGMA_DEPS_DIR}/opencv/bin)

  set(REQ_VARS ${REQ_VARS} ${PCK}_RT_CORE ${PCK}_RT_IMGCODECS ${PCK}_RT_IMGPROC)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK} REQUIRED_VARS ${REQ_VARS})

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIB_CORE} ${${PCK}_LIB_IMGCODECS} ${${PCK}_LIB_IMGPROC})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})

  if(WIN32)
    mark_as_advanced(${PCK}_RT_CORE ${PCK}_RT_IMGCODECS ${PCK}_RT_IMGPROC)
    set(${PCK}_RUNTIME   ${${PCK}_RT_CORE} ${${PCK}_RT_IMGCODECS} ${${PCK}_RT_IMGPROC})
  endif()
endif()
