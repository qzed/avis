# find-package for FFMPEG, provides:
#   FFMPEG_FOUND
#   FFMPEG_INCLUDE_DIRS as well as separate floags for avformat, avcodec and swresample
#   FFMPEG_LIBRARIES as well as separate flags for avformat, avcodec, avutil and swresample

find_package(PkgConfig)

find_path(FFMPEG_AVFORMAT_INCLUDE_DIR   NAMES "libavformat/avformat.h")
find_path(FFMPEG_AVCODEC_INCLUDE_DIR    NAMES "libavcodec/avcodec.h")
find_path(FFMPEG_SWRESAMPLE_INCLUDE_DIR NAMES "libswresample/swresample.h")

find_library(FFMPEG_AVFORMAT_LIBRARY   NAMES "avformat")
find_library(FFMPEG_AVCODEC_LIBRARY    NAMES "avcodec")
find_library(FFMPEG_AVUTIL_LIBRARY     NAMES "avutil")
find_library(FFMPEG_SWRESAMPLE_LIBRARY NAMES "swresample")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFMPEG DEFAULT_MSG
    FFMPEG_AVFORMAT_INCLUDE_DIR
    FFMPEG_AVCODEC_INCLUDE_DIR
    FFMPEG_SWRESAMPLE_INCLUDE_DIR
    FFMPEG_AVFORMAT_LIBRARY
    FFMPEG_AVCODEC_LIBRARY
    FFMPEG_AVUTIL_LIBRARY
    FFMPEG_SWRESAMPLE_LIBRARY
)

mark_as_advanced(
    FFMPEG_AVFORMAT_INCLUDE_DIR
    FFMPEG_AVCODEC_INCLUDE_DIR
    FFMPEG_SWRESAMPLE_INCLUDE_DIR
    FFMPEG_AVFORMAT_LIBRARY
    FFMPEG_AVCODEC_LIBRARY
    FFMPEG_AVUTIL_LIBRARY
    FFMPEG_SWRESAMPLE_LIBRARY
)

set(FFMPEG_INCLUDE_DIRS
    "${FFMPEG_AVFORMAT_INCLUDE_DIR}"
    "${FFMPEG_AVCODEC_INCLUDE_DIR}"
    "${FFMPEG_SWRESAMPLE_INCLUDE_DIR}"
)

set(FFMPEG_LIBRARIES
    "${FFMPEG_AVFORMAT_LIBRARY}"
    "${FFMPEG_AVCODEC_LIBRARY}"
    "${FFMPEG_AVUTIL_LIBRARY}"
    "${FFMPEG_SWRESAMPLE_LIBRARY}"
)
