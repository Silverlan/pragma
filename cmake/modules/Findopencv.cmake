set(PCK "opencv")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES opencv2/opencv.hpp
  HINTS
    ${PRAGMA_DEPS_DIR}/opencv/include
)

find_library(${PCK}_LIB_CORE      NAMES opencv_core opencv_core4120      HINTS ${PRAGMA_DEPS_DIR}/opencv/lib)
find_library(${PCK}_LIB_IMGCODECS NAMES opencv_imgcodecs opencv_imgcodecs4120 HINTS ${PRAGMA_DEPS_DIR}/opencv/lib)
find_library(${PCK}_LIB_IMGPROC   NAMES opencv_imgproc opencv_imgproc4120   HINTS ${PRAGMA_DEPS_DIR}/opencv/lib)

set(REQ_VARS ${PCK}_LIB_CORE ${PCK}_LIB_IMGCODECS ${PCK}_LIB_IMGPROC ${PCK}_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK} REQUIRED_VARS ${REQ_VARS})

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIB_CORE} ${${PCK}_LIB_IMGCODECS} ${${PCK}_LIB_IMGPROC})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
endif()
