extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

#include <algorithm>
#include <string>
#include <vector>
#include <system_error>
#include <cinttypes>
#include <iostream>


namespace avis {
namespace audio {
namespace ffmpeg {

inline auto get_message(int errc) -> std::string {
    char buffer[AV_ERROR_MAX_STRING_SIZE];
    char* str = av_make_error_string(buffer, AV_ERROR_MAX_STRING_SIZE, errc);

    if (str != nullptr)
        return {buffer};
    else
        return "Unknown error";
}

inline auto to_string(int errc) -> std::string {
    switch (errc) {
    case AVERROR_BSF_NOT_FOUND:         return "AVERROR_BSF_NOT_FOUND";
    case AVERROR_BUG:                   return "AVERROR_BUG";
    case AVERROR_BUFFER_TOO_SMALL:      return "AVERROR_BUFFER_TOO_SMALL";
    case AVERROR_DECODER_NOT_FOUND:     return "AVERROR_DECODER_NOT_FOUND";
    case AVERROR_DEMUXER_NOT_FOUND:     return "AVERROR_DEMUXER_NOT_FOUND";
    case AVERROR_ENCODER_NOT_FOUND:     return "AVERROR_ENCODER_NOT_FOUND";
    case AVERROR_EOF:                   return "AVERROR_EOF";
    case AVERROR_EXIT:                  return "AVERROR_EXIT";
    case AVERROR_EXTERNAL:              return "AVERROR_EXTERNAL";
    case AVERROR_FILTER_NOT_FOUND:      return "AVERROR_FILTER_NOT_FOUND";
    case AVERROR_INVALIDDATA:           return "AVERROR_INVALIDDATA";
    case AVERROR_MUXER_NOT_FOUND:       return "AVERROR_MUXER_NOT_FOUND";
    case AVERROR_OPTION_NOT_FOUND:      return "AVERROR_OPTION_NOT_FOUND";
    case AVERROR_PATCHWELCOME:          return "AVERROR_PATCHWELCOME";
    case AVERROR_PROTOCOL_NOT_FOUND:    return "AVERROR_PROTOCOL_NOT_FOUND";
    case AVERROR_STREAM_NOT_FOUND:      return "AVERROR_STREAM_NOT_FOUND";
    case AVERROR_BUG2:                  return "AVERROR_BUG2";
    case AVERROR_UNKNOWN:               return "AVERROR_UNKNOWN";
    case AVERROR_EXPERIMENTAL:          return "AVERROR_EXPERIMENTAL";
    case AVERROR_INPUT_CHANGED:         return "AVERROR_INPUT_CHANGED";
    case AVERROR_OUTPUT_CHANGED:        return "AVERROR_OUTPUT_CHANGED";
    case AVERROR_HTTP_BAD_REQUEST:      return "AVERROR_HTTP_BAD_REQUEST";
    case AVERROR_HTTP_UNAUTHORIZED:     return "AVERROR_HTTP_UNAUTHORIZED";
    case AVERROR_HTTP_FORBIDDEN:        return "AVERROR_HTTP_FORBIDDEN";
    case AVERROR_HTTP_NOT_FOUND:        return "AVERROR_HTTP_NOT_FOUND";
    case AVERROR_HTTP_OTHER_4XX:        return "AVERROR_HTTP_OTHER_4XX";
    case AVERROR_HTTP_SERVER_ERROR:     return "AVERROR_HTTP_SERVER_ERROR ";
    case AVERROR(ENOMEM):               return "AVERROR(ENOMEM)";
    case AVERROR(EINVAL):               return "AVERROR(EINVAL)";
    case AVERROR(EAGAIN):               return "AVERROR(EAGAIN)";
    case AVERROR(ENOENT):               return "AVERROR(ENOENT)";
    default:                            return "<unknown>";
    }
}


namespace detail {

class ffmpeg_error_category : public std::error_category {
    virtual const char* name()                 const noexcept override;
    virtual std::string message(int condition) const noexcept override;
};

auto ffmpeg_error_category::name() const noexcept -> const char* {
    return "portaudio";
}

auto ffmpeg_error_category::message(int condition) const noexcept -> std::string {
    return get_message(condition) + ". (" + to_string(condition) + ")";
}

} /* namespace detail */


inline auto ffmpeg_category() noexcept -> const std::error_category& {
    static detail::ffmpeg_error_category category;
    return category;
}

inline auto make_error_code(int from) noexcept -> std::error_code {
    return std::error_code(from, ffmpeg_category());
}

inline auto make_error_condition(int from) noexcept -> std::error_condition {
    return std::error_condition(from, ffmpeg_category());
}


class exception : public std::system_error {
public:
    exception(int errc) : std::system_error{errc, ffmpeg_category()} {}
    exception(int errc, std::string const& what) : std::system_error{errc, ffmpeg_category(), what} {}
};


constexpr auto except(int errc) -> int {
    if (errc < 0)
        throw exception(errc);
    else
        return errc;
}


namespace detail {

template <class T, class DestructorFn>
class handle {
public:
    using handle_type = T;

    handle(handle_type handle, DestructorFn&& destructor)
            : handle_(handle)
            , destructor_{std::forward<DestructorFn>(destructor)} {}

    ~handle() { if (handle_ != nullptr) destructor_(handle_); }

    auto get()       noexcept -> handle_type&;
    auto get() const noexcept -> handle_type const&;

    auto release() noexcept -> handle_type;

    auto operator->() const noexcept -> handle_type;

private:
    handle_type  handle_;
    DestructorFn destructor_;
};

template <class T, class D>
auto handle<T, D>::get() noexcept -> handle_type& {
    return handle_;
}

template <class T, class D>
auto handle<T, D>::get() const noexcept -> handle_type const& {
    return handle_;
}

template <class T, class D>
auto handle<T, D>::release() noexcept -> handle_type {
    return std::exchange(handle_, nullptr);
}

template <class T, class D>
auto handle<T, D>::operator->() const noexcept -> handle_type {
    return handle_;
}


template <class T, class DestructorFn>
auto make_handle(T hndl, DestructorFn&& destructor) -> handle<T, DestructorFn> {
    return {hndl, std::forward<DestructorFn>(destructor)};
}


template <class Fn>
class on_scope_exit_impl {
public:
    on_scope_exit_impl(Fn&& fn) : fn_{std::forward<Fn>(fn)} {}
    ~on_scope_exit_impl() { fn_(); }

private:
    Fn fn_;
};

template <class Fn>
auto on_scope_exit(Fn&& fn) -> on_scope_exit_impl<Fn> {
    return std::forward<Fn>(fn);
}

} /* namespace detail */


struct stream_format {
    int            channels;
    std::int64_t   channel_layout;
    AVSampleFormat sample_format;
    int            sample_rate;

    inline auto operator== (stream_format const& rhs) -> bool;
    inline auto operator!= (stream_format const& rhs) -> bool;
};

auto make_stream_format(AVFrame const& frame) -> stream_format {
    int channels = av_frame_get_channels(&frame);
    std::int64_t channel_layout = av_frame_get_channel_layout(&frame);

    if (channels == 0 && channel_layout != 0)
        channels = av_get_channel_layout_nb_channels(channel_layout);
    else if (channels != 0 && channel_layout == 0)
        channel_layout = av_get_default_channel_layout(channels);

    return {
        channels,
        channel_layout,
        static_cast<AVSampleFormat>(frame.format),
        av_frame_get_sample_rate(&frame)
    };
}

auto stream_format::operator== (stream_format const& rhs) -> bool {
    return channels == rhs.channels
            && channel_layout == rhs.channel_layout
            && sample_format == rhs.sample_format
            && sample_rate == rhs.sample_rate;
}

auto stream_format::operator!= (stream_format const& rhs) -> bool {
    return !(*this == rhs);
}

auto get_pcm_sample_size(stream_format const& fmt) -> int {
    return except(av_samples_get_buffer_size(nullptr, fmt.channels, 1, fmt.sample_format, 1));
}


class audio_input_stream {
public:
    static auto open_default(stream_format const& format, std::string const& filename) -> audio_input_stream;

    audio_input_stream()
            : output_format_{}
            , input_format_{}
            , format_ctx_{nullptr}
            , codec_ctx_{nullptr}
            , codec_{nullptr}
            , swr_ctx_{nullptr}
            , frame_{nullptr}
            , buffer_{}
            , buffer_offset_{0}
            , eof_{false} {}

    audio_input_stream(AVFormatContext* format_ctx, AVCodecContext* codec_ctx, AVCodec* codec, stream_format const& format)
            : output_format_{format}
            , input_format_{}
            , format_ctx_{format_ctx}
            , codec_ctx_{codec_ctx}
            , codec_{codec}
            , swr_ctx_{nullptr}
            , frame_{av_frame_alloc()}
            , buffer_{}
            , buffer_offset_{0}
            , eof_{false}
            { if (frame_ == nullptr) throw exception(AVERROR(ENOMEM)); }

    audio_input_stream(audio_input_stream const& other) = delete;

    audio_input_stream(audio_input_stream&& other)
            : output_format_{std::move(other.output_format_)}
            , input_format_{std::move(other.input_format_)}
            , format_ctx_{std::exchange(other.format_ctx_, nullptr)}
            , codec_ctx_{std::exchange(other.codec_ctx_, nullptr)}
            , codec_{std::exchange(other.codec_, nullptr)}
            , swr_ctx_{std::exchange(other.swr_ctx_, nullptr)}
            , frame_{std::exchange(other.frame_, nullptr)}
            , buffer_{std::move(other.buffer_)}
            , buffer_offset_{other.buffer_offset_}
            , eof_{other.eof_} {}

    ~audio_input_stream() { close(); }


    auto operator= (audio_input_stream const& rhs) -> audio_input_stream& = delete;
    auto operator= (audio_input_stream&& rhs)      -> audio_input_stream&;

    inline void close();

    inline auto read(unsigned char* buffer, int) -> int;
    inline auto eof() const noexcept -> bool;

    inline auto play()  const noexcept -> int;
    inline auto pause() const noexcept -> int;

    inline auto get_format() const noexcept -> stream_format const&;

    inline auto get_av_format_context() const noexcept -> AVFormatContext*;
    inline auto get_av_codec_context()  const noexcept -> AVCodecContext*;
    inline auto get_av_codec()          const noexcept -> AVCodec*;

private:
    stream_format        output_format_;
    stream_format        input_format_;

    AVFormatContext*     format_ctx_;
    AVCodecContext*      codec_ctx_;
    AVCodec*             codec_;

    SwrContext*          swr_ctx_;
    AVFrame*             frame_;
    std::vector<uint8_t> buffer_;
    std::size_t          buffer_offset_;
    bool                 eof_;
};


auto audio_input_stream::open_default(stream_format const& format, std::string const& filename) -> audio_input_stream {
    auto format_ctx = detail::make_handle<AVFormatContext*>(nullptr, [](auto h){ avformat_close_input(&h); });
    auto codec_ctx = detail::make_handle<AVCodecContext*>(nullptr, [](auto h){ avcodec_free_context(&h); });
    AVCodec* codec = nullptr;

    ffmpeg::except(avformat_open_input(&format_ctx.get(), filename.c_str(), nullptr, nullptr));
    ffmpeg::except(avformat_find_stream_info(format_ctx.get(), nullptr));

    auto stream = ffmpeg::except(av_find_best_stream(format_ctx.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0));

    codec_ctx.get() = avcodec_alloc_context3(nullptr);
    if (codec_ctx.get() == nullptr)
        throw exception(AVERROR(ENOMEM));

    ffmpeg::except(avcodec_parameters_to_context(codec_ctx.get(), format_ctx->streams[stream]->codecpar));
    av_codec_set_pkt_timebase(codec_ctx.get(), format_ctx->streams[stream]->time_base);

    codec = avcodec_find_decoder(codec_ctx->codec_id);
    if (codec == nullptr)
        throw exception(AVERROR_DECODER_NOT_FOUND);

    ffmpeg::except(avcodec_open2(codec_ctx.get(), codec, nullptr));

    return {format_ctx.release(), codec_ctx.release(), codec, format};
}

auto audio_input_stream::operator=(audio_input_stream&& rhs) -> audio_input_stream& {
    close();

    output_format_ = std::move(rhs.output_format_);
    input_format_  = std::move(rhs.input_format_);
    format_ctx_    = std::exchange(rhs.format_ctx_, nullptr);
    codec_ctx_     = std::exchange(rhs.codec_ctx_, nullptr);
    codec_         = std::exchange(rhs.codec_, nullptr);
    swr_ctx_       = std::exchange(rhs.swr_ctx_, nullptr);
    frame_         = std::exchange(rhs.frame_, nullptr);
    buffer_        = std::move(rhs.buffer_);
    buffer_offset_ = rhs.buffer_offset_;
    eof_           = rhs.eof_;

    return *this;
}

void audio_input_stream::close() {
    if (format_ctx_ != nullptr)
        avformat_close_input(&format_ctx_);

    if (codec_ctx_ != nullptr)
        avcodec_free_context(&codec_ctx_);

    if (swr_ctx_ != nullptr)
        swr_free(&swr_ctx_);

    if (frame_ != nullptr)
        av_frame_free(&frame_);

    codec_ = nullptr;
    buffer_ = {};
}

auto audio_input_stream::read(std::uint8_t* buffer, int samples) -> int {
    std::int64_t const output_sample_size = get_pcm_sample_size(output_format_);

    int read = 0;

    // if samples are bufferd, get them
    if (buffer_offset_ < buffer_.size()) {
        int const num_buffered_samples = (buffer_.size() - buffer_offset_) / output_sample_size;
        int const num_transfer_samples = std::min(num_buffered_samples, samples);
        auto const num_transfer_bytes = num_transfer_samples * output_sample_size;

        auto const copy_start = buffer_.begin() + buffer_offset_;
        auto const copy_end   = buffer_.begin() + buffer_offset_ + num_transfer_bytes;

        std::copy(copy_start, copy_end, buffer);
        buffer_offset_ += num_transfer_bytes;

        if (samples == num_transfer_samples)
            return samples;

        buffer += num_transfer_bytes;
        read   += num_transfer_samples;
    }

    // read, decode and convert next packet
    auto packet = AVPacket{};
    av_init_packet(&packet);

    while (read < samples && !eof_) {
        // try to get next frame from decoder
        int err = avcodec_receive_frame(codec_ctx_, frame_);

        // if no frame received, send next package to decoder
        if (err == AVERROR(EAGAIN)) {
            auto packet_guard = detail::on_scope_exit([&](){ av_packet_unref(&packet); });

            int err = av_read_frame(format_ctx_, &packet);
            if (err == AVERROR_EOF) {
                packet.size = 0;        // explicitly flush decoder
                packet.data = nullptr;
            } else {
                ffmpeg::except(err);
            }

            // NOTE: this should not return AVERROR(EAGAIN) since we are draining the decoder as much as possible
            ffmpeg::except(avcodec_send_packet(codec_ctx_, &packet));

        // if decoder signals EOF, decoder has been flushed, try to flush converter now
        } else if (err == AVERROR_EOF) {
            int len = swr_convert(swr_ctx_, &buffer, samples - read, nullptr, 0);
            if (len > 0) {
                read   += len;
                buffer += len * output_sample_size;
            } else if (len == 0 || len == AVERROR_EOF) {
                eof_ = true;
            } else throw exception(len);

        // else check for error
        } else if (err < 0) {
            throw exception(err);

        // if no error: we got a frame, convert it
        } else {
            auto new_input_format = make_stream_format(*frame_);

            // re-create resample-context if input format has changed
            if (input_format_ != new_input_format) {
                swr_ctx_ = swr_alloc_set_opts(swr_ctx_,
                        output_format_.channel_layout, output_format_.sample_format, output_format_.sample_rate,
                        new_input_format.channel_layout, new_input_format.sample_format, new_input_format.sample_rate,
                        0, nullptr);

                except(swr_init(swr_ctx_));
                input_format_ = new_input_format;
            }

            auto in_ptr = const_cast<const uint8_t**>(frame_->extended_data);

            // if dst-buffer has enough space, copy directly
            auto out_samples = swr_get_out_samples(swr_ctx_, frame_->nb_samples);
            if (out_samples <= samples - read) {
                int const len = except(swr_convert(swr_ctx_, &buffer, samples - read, in_ptr, frame_->nb_samples));
                read   += len;
                buffer += len * output_sample_size;

            // else copy to intermediate-buffer, return as much as possible
            } else {
                // convert and store in intermediate buffer
                buffer_.resize(out_samples * output_sample_size);
                auto out_ptr = buffer_.data();
                int const len = except(swr_convert(swr_ctx_, &out_ptr, out_samples, in_ptr, frame_->nb_samples));
                buffer_.resize(len * output_sample_size);

                // (partial) copy to output-buffer
                int const num_transfer_samples = std::min(len, samples - read);
                auto const num_transfer_bytes = num_transfer_samples * output_sample_size;

                std::copy(buffer_.begin(), buffer_.begin() + num_transfer_bytes, buffer);

                read   += num_transfer_samples;
                buffer += num_transfer_bytes;

                buffer_offset_ = num_transfer_bytes;
            }
        }
    }

    return read;
}

auto audio_input_stream::eof() const noexcept -> bool {
    return eof_;
}

auto audio_input_stream::play() const noexcept -> int {
    return av_read_play(format_ctx_);
}

auto audio_input_stream::pause() const noexcept -> int {
    return av_read_pause(format_ctx_);
}

auto audio_input_stream::get_format() const noexcept -> stream_format const& {
    return output_format_;
}

auto audio_input_stream::get_av_format_context() const noexcept -> AVFormatContext* {
    return format_ctx_;
}

auto audio_input_stream::get_av_codec_context() const noexcept -> AVCodecContext* {
    return codec_ctx_;
}

auto audio_input_stream::get_av_codec() const noexcept -> AVCodec* {
    return codec_;
}

} /* namespae ffmpeg */
} /* namespae audio */
} /* namespae avis */
