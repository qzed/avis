#include "audio.hpp"

#include <boost/lockfree/spsc_queue.hpp>

#include <iostream>
#include <vector>
#include <atomic>
#include <thread>


using namespace avis::audio;
using namespace std::literals::chrono_literals;

constexpr ffmpeg::stream_format output_format { 2, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLT, 192000 };


int main(int argc, char const** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    // initialize
    av_register_all();
    auto init_portaudio = portaudio::scope_initializer();

    auto sample_size = ffmpeg::get_pcm_sample_size(output_format);

    boost::lockfree::spsc_queue<uint8_t> queue{1024*1024};
    std::atomic_bool stream_eof{false};
    std::atomic_bool stream_flushed{false};

    // output-stream callback
    auto callback_fn = [&](auto output, auto frame_count, auto time, auto flags) {
        uint8_t* out = reinterpret_cast<uint8_t*>(output);

        unsigned long len_bytes = frame_count * sample_size;
        unsigned long count = queue.pop(out, len_bytes);
        std::fill(out + count, out + len_bytes, 0);

        if (count < len_bytes && stream_eof) {
            stream_flushed = true;
            return paComplete;
        } else {
            return paContinue;
        }
    };

    // set up streams
    auto const pa_output_format = portaudio::make_stream_format(output_format);
    auto ostream = portaudio::output_stream::open_default(pa_output_format, 256, std::move(callback_fn));
    auto istream = ffmpeg::audio_input_stream::open_default(output_format, argv[1]);

    auto buffer_size = 1024;
    auto buffer = std::vector<uint8_t>(sample_size * buffer_size);

    // start streams and read to queue
    ostream.start();
    while (!istream.eof()) {
        int len_samples = istream.read(buffer.data(), buffer_size);
        int len_bytes = len_samples * sample_size;

        int pushed = 0;
        while (pushed != len_bytes)
            pushed += queue.push(buffer.data() + pushed, len_bytes - pushed);
    }

    // wait for playback to finish
    stream_eof = true;
    while (!stream_flushed)
        std::this_thread::sleep_for(1us);

    ostream.stop();
}
