#include <avis/audio/io/portaudio.hpp>
#include <avis/audio/io/ffmpeg.hpp>

namespace avis {
namespace audio {
namespace portaudio {

constexpr auto to_sample_format(AVSampleFormat sample_format) -> PaSampleFormat {
    switch (sample_format) {
    case AV_SAMPLE_FMT_FLT:  return paFloat32;
    case AV_SAMPLE_FMT_U8:   return paUInt8;
    case AV_SAMPLE_FMT_S16:  return paInt16;
    case AV_SAMPLE_FMT_S32:  return paInt32;
    default: throw portaudio::exception(paSampleFormatNotSupported);
    }
}

constexpr auto make_stream_format(ffmpeg::stream_format const& format) -> portaudio::stream_format {
    return {format.channels, to_sample_format(format.sample_format), static_cast<double>(format.sample_rate)};
}

} /* namespace portaudio */
} /* namespace audio */
} /* namespace avis */
