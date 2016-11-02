# find-package for vulkan, provides:
#   VULKAN_FOUND
#   VULKAN_INCLUDE_DIRS
#   VULKAN_LIBRARIES
#   VULKAN_DEFINITIONS

find_package(PkgConfig)
pkg_check_modules(PC_VULKAN QUIET vulkan)

set(VULKAN_DEFINITIONS ${PC_VULKAN_CFLAGS_OTHER})

find_path(VULKAN_INCLUDE_DIR
    NAMES
        "vulkan/vulkan.h"
    HINTS
        "${PC_VULKAN_INCLUDEDIR}"
        "${PC_VULKAN_INCLUDE_DIRS}"
        "$ENV{VULKAN_SDK}/include"
        "$ENV{VK_SDK_PATH}/include"
        "$ENV{VULKAN_SDK}/Include"
        "$ENV{VK_SDK_PATH}/Include"
)

find_library(VULKAN_LIBRARY
    NAMES
        "vulkan"
        "vulkan-1"
    HINTS
        "${PC_VULKAN_LIBDIR}"
        "${PC_VULKAN_LIBRARY_DIRS}"
        "$ENV{VULKAN_SDK}/lib"
        "$ENV{VK_SDK_PATH}/lib"
        "$ENV{VULKAN_SDK}/Bin"
        "$ENV{VK_SDK_PATH}/Bin"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan DEFAULT_MSG VULKAN_LIBRARY VULKAN_INCLUDE_DIR)

mark_as_advanced(VULKAN_INCLUDE_DIR VULKAN_LIBRARY)

set(VULKAN_INCLUDE_DIRS "${VULKAN_INCLUDE_DIR}")
set(VULKAN_LIBRARIES "${VULKAN_LIBRARY}")
