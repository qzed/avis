#pragma once

#include <portaudio.h>

#include <system_error>
#include <functional>
#include <memory>


namespace avis {
namespace audio {
namespace portaudio {

inline auto get_message(PaError errc) -> std::string {
    return Pa_GetErrorText(errc);
}

inline auto to_string(PaError errc) -> std::string {
    switch (errc) {
    case paNoError:                                 return "paNoError";
    case paNotInitialized:                          return "paNotInitialized";
    case paUnanticipatedHostError:                  return "paUnanticipatedHostError";
    case paInvalidChannelCount:                     return "paInvalidChannelCount";
    case paInvalidSampleRate:                       return "paInvalidSampleRate";
    case paInvalidDevice:                           return "paInvalidDevice";
    case paInvalidFlag:                             return "paInvalidFlag";
    case paSampleFormatNotSupported:                return "paSampleFormatNotSupported";
    case paBadIODeviceCombination:                  return "paBadIODeviceCombination";
    case paInsufficientMemory:                      return "paInsufficientMemory";
    case paBufferTooBig:                            return "paBufferTooBig";
    case paBufferTooSmall:                          return "paBufferTooSmall";
    case paNullCallback:                            return "paNullCallback";
    case paBadStreamPtr:                            return "paBadStreamPtr";
    case paTimedOut:                                return "paTimedOut";
    case paInternalError:                           return "paInternalError";
    case paDeviceUnavailable:                       return "paDeviceUnavailable";
    case paIncompatibleHostApiSpecificStreamInfo:   return "paIncompatibleHostApiSpecificStreamInfo";
    case paStreamIsStopped:                         return "paStreamIsStopped";
    case paStreamIsNotStopped:                      return "paStreamIsNotStopped";
    case paInputOverflowed:                         return "paInputOverflowed";
    case paOutputUnderflowed:                       return "paOutputUnderflowed";
    case paHostApiNotFound:                         return "paHostApiNotFound";
    case paInvalidHostApi:                          return "paInvalidHostApi";
    case paCanNotReadFromACallbackStream:           return "paCanNotReadFromACallbackStream";
    case paCanNotWriteToACallbackStream:            return "paCanNotWriteToACallbackStream";
    case paCanNotReadFromAnOutputOnlyStream:        return "paCanNotReadFromAnOutputOnlyStream";
    case paCanNotWriteToAnInputOnlyStream:          return "paCanNotWriteToAnInputOnlyStream";
    case paIncompatibleStreamHostApi:               return "paIncompatibleStreamHostApi";
    case paBadBufferPtr:                            return "paBadBufferPtr";
    default:                                        return "<unknown>";
    }
}


namespace detail {

class portaudio_error_category : public std::error_category {
    inline virtual const char* name()                 const noexcept override;
    inline virtual std::string message(int condition) const noexcept override;
};

auto portaudio_error_category::name() const noexcept -> const char* {
    return "portaudio";
}

auto portaudio_error_category::message(int condition) const noexcept -> std::string {
    auto r = static_cast<PaError>(condition);
    return get_message(r) + ". (" + to_string(r) + ")";
}

} /* namespace detail */


inline auto portaudio_category() noexcept -> const std::error_category& {
    static detail::portaudio_error_category category;
    return category;
}

inline auto make_error_code(PaError from) noexcept -> std::error_code {
    return std::error_code(static_cast<int>(from), portaudio_category());
}

inline auto make_error_condition(PaError from) noexcept -> std::error_condition {
    return std::error_condition(static_cast<int>(from), portaudio_category());
}


class exception : public std::system_error {
public:
    exception(PaError errc) : std::system_error{errc, portaudio_category()} {}
    exception(PaError errc, std::string const& what) : std::system_error{errc, portaudio_category(), what} {}
};


constexpr auto except(PaError errc) -> int {
    if (errc < 0)
        throw exception(errc);
    else
        return errc;
}


class scope_initializer {
public:
    inline scope_initializer();
    inline ~scope_initializer();
};

scope_initializer::scope_initializer()  { except(Pa_Initialize()); }
scope_initializer::~scope_initializer() { except(Pa_Terminate()); }


struct stream_format {
    int            channels;
    PaSampleFormat sample_format;
    double         sample_rate;
};

class stream_base {
public:
    stream_base() : stream_{nullptr} {}
    stream_base(PaStream* stream) : stream_{stream} {}
    stream_base(stream_base const& other) = delete;
    stream_base(stream_base&& other) : stream_{std::exchange(other.stream_, nullptr)} {}

    virtual ~stream_base() { if (stream_) close(); }

    inline auto operator= (stream_base const& rhs) -> stream_base& = delete;
    inline auto operator= (stream_base&& rhs)      -> stream_base&;

    inline void close();

    inline void start() const;
    inline void stop()  const;
    inline void abort() const;

    inline bool is_active()  const;
    inline bool is_stopped() const;

    inline auto get_time() const noexcept -> double;

    inline auto get_handle() const noexcept -> PaStream*;

private:
    PaStream* stream_;
};

auto stream_base::operator= (stream_base&& rhs) -> stream_base& {
    stream_ = std::exchange(rhs.stream_, nullptr);
    return *this;
}

void stream_base::start() const {
    except(Pa_StartStream(stream_));
}

void stream_base::stop() const{
    except(Pa_StopStream(stream_));
}

void stream_base::abort() const {
    except(Pa_AbortStream(stream_));
}

void stream_base::close() {
    except(Pa_CloseStream(stream_));
    stream_ = nullptr;
}

bool stream_base::is_active() const {
    return except(Pa_IsStreamActive(stream_));
}

bool stream_base::is_stopped() const {
    return except(Pa_IsStreamStopped(stream_));
}

auto stream_base::get_time() const noexcept -> double {
    return Pa_GetStreamTime(stream_);
}

auto stream_base::get_handle() const noexcept -> PaStream* {
    return stream_;
}


class output_stream : public stream_base {
public:
    using callback_fn = std::function<int(void*, unsigned long, PaStreamCallbackTimeInfo const*, PaStreamCallbackFlags)>;

public:
    template <class... Args>
    static inline auto open_default(stream_format const& format, unsigned long frames_per_buffer,
            Args&&... callback_args) -> output_stream;

    output_stream()
            : stream_base()
            , callback_{nullptr} {}

    output_stream(PaStream* stream, std::unique_ptr<callback_fn>&& callback)
            : stream_base(stream)
            , callback_{std::forward<std::unique_ptr<callback_fn>>(callback)} {}

    output_stream(output_stream&&) = default;
    inline auto operator= (output_stream&& rhs) -> output_stream& = default;

private:
    // NOTE: callback needs to be on heap, could not move class otherwise
    std::unique_ptr<callback_fn> callback_;

    static inline auto cb(void const* input, void* output, unsigned long frame_count,
            PaStreamCallbackTimeInfo const* time,  PaStreamCallbackFlags flags, void* user_data) noexcept -> int;
};


template <class... Args>
inline auto output_stream::open_default(stream_format const& format, unsigned long frames_per_buffer,
        Args&&... callback_args) -> output_stream
{
    auto callback = std::make_unique<callback_fn>(std::forward<Args>(callback_args)...);

    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 0, format.channels, format.sample_format, format.sample_rate, frames_per_buffer,
            output_stream::cb, callback.get());

    return {stream, std::move(callback)};
}


auto output_stream::cb(void const* input, void* output, unsigned long frame_count,
        PaStreamCallbackTimeInfo const* time,  PaStreamCallbackFlags flags, void* user_data) noexcept -> int
{
    return (*static_cast<callback_fn*>(user_data))(output, frame_count, time, flags);
}

} /* namespace portaudio */
} /* namespace audio */
} /* namespace avis */
