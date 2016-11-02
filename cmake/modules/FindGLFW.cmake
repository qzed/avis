# find-package for glfw, provides:
#   GLFW_FOUND
#   GLFW_INCLUDE_DIRS
#   GLFW_LIBRARIES
#   GLFW_DEFINITIONS

find_package(PkgConfig)
pkg_check_modules(PC_GLFW QUIET glfw)

set(GLFW_DEFINITIONS ${PC_GLFW_CFLAGS_OTHER})

find_path(GLFW_INCLUDE_DIR
    NAMES
        "GLFW/glfw3.h"
    HINTS
        "${PC_GLFW_INCLUDEDIR}"
        "${PC_GLFW_INCLUDE_DIRS}"
)

find_library(GLFW_LIBRARY
    NAMES
        "glfw"
        "glfw3"
    HINTS
        "${PC_GLFW_LIBDIR}"
        "${PC_GLFW_LIBRARY_DIRS}"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW DEFAULT_MSG GLFW_LIBRARY GLFW_INCLUDE_DIR)

mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)

set(GLFW_INCLUDE_DIRS "${GLFW_INCLUDE_DIR}")
set(GLFW_LIBRARIES "${GLFW_LIBRARY}")
