# find-package for PortAudio, provides:
#   PORTAUDIO_FOUND
#   PORTAUDIO_INCLUDE_DIRS
#   PORTAUDIO_LIBRARIES
#   PORTAUDIO_DEFINITIONS

find_package(PkgConfig)
pkg_check_modules(PC_PORTAUDIO QUIET portaudio)

set(PORTAUDIO_DEFINITIONS ${PC_PORTAUDIO_CFLAGS_OTHER})

find_path(PORTAUDIO_INCLUDE_DIR
    NAMES
        "portaudio.h"
    HINTS
        "${PC_PORTAUDIO_INCLUDEDIR}"
        "${PC_PORTAUDIO_INCLUDE_DIRS}"
)

find_library(PORTAUDIO_LIBRARY
    NAMES
        "portaudio"
    HINTS
        "${PC_PORTAUDIO_LIBDIR}"
        "${PC_PORTAUDIO_LIBRARY_DIRS}"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PortAudio DEFAULT_MSG PORTAUDIO_LIBRARY PORTAUDIO_INCLUDE_DIR)

mark_as_advanced(PORTAUDIO_INCLUDE_DIR PORTAUDIO_LIBRARY)

set(PORTAUDIO_INCLUDE_DIRS "${PORTAUDIO_INCLUDE_DIR}")
set(PORTAUDIO_LIBRARIES "${PORTAUDIO_LIBRARY}")
