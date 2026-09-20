#include "zh/video/decoder.h"

#include "zh/data/vfs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

namespace zh::video {
namespace {

std::string ffmpeg_error(int code)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE]{};
    av_strerror(code, buffer, sizeof(buffer));
    return buffer;
}

[[noreturn]] void fail(std::string_view path, std::string message)
{
    throw VideoError("video '" + std::string(path) + "': " + std::move(message));
}

double stream_timestamp(const AVFrame* frame, const AVStream* stream)
{
    if (frame->best_effort_timestamp == AV_NOPTS_VALUE) return std::numeric_limits<double>::quiet_NaN();
    return static_cast<double>(frame->best_effort_timestamp) * av_q2d(stream->time_base);
}

bool bink_audio_codec(AVCodecID id)
{
    return id == AV_CODEC_ID_BINKAUDIO_DCT || id == AV_CODEC_ID_BINKAUDIO_RDFT;
}

} // namespace

void validate_video_dimensions(std::uint32_t width, std::uint32_t height, const VideoLimits& limits)
{
    if (width == 0 || height == 0) throw VideoError("video dimensions must be positive");
    if (width > limits.maximum_width || height > limits.maximum_height)
        throw VideoError("video dimensions exceed configured maximum");
    const auto pixels = static_cast<std::uint64_t>(width) * height;
    if (pixels > limits.maximum_pixels) throw VideoError("video pixel count exceeds configured maximum");
    if (pixels > limits.maximum_frame_bytes / 4U) throw VideoError("video RGBA conversion buffer exceeds configured maximum");
}

double normalize_media_timestamp(double candidate, double previous, double step) noexcept
{
    const double safe_step = std::isfinite(step) && step > 0.0 ? step : 1.0 / 30.0;
    if (!std::isfinite(candidate) || candidate < 0.0) return previous < 0.0 ? 0.0 : previous + safe_step;
    if (previous >= 0.0 && candidate <= previous) return previous + safe_step;
    return candidate;
}

class FfmpegVideoDecoder::Impl {
public:
    Impl(const data::VirtualFileSystem& source, std::string path, VideoLimits configured, bool available)
        : logical(std::move(path)), limits(configured), audio_queue(configured.maximum_audio_chunks)
    {
        try {
        if (!available) fail(logical, "Bink decoder is unavailable in the distribution FFmpeg build");
        const auto* resource = source.find(logical);
        if (!resource) fail(logical, "logical resource is missing from the mounted VFS");
        if (resource->size == 0) fail(logical, "resource is empty or truncated");
        if (resource->size > limits.maximum_source_bytes || resource->size > std::numeric_limits<std::size_t>::max())
            fail(logical, "resource exceeds configured source-byte limit");
        bytes = source.read_prefix(logical, static_cast<std::size_t>(resource->size));
        if (bytes.size() != resource->size) fail(logical, "short VFS read");

        constexpr int io_buffer_size = 32768;
        auto* io_buffer = static_cast<unsigned char*>(av_malloc(io_buffer_size));
        if (!io_buffer) fail(logical, "cannot allocate FFmpeg custom-I/O buffer");
        io = avio_alloc_context(io_buffer, io_buffer_size, 0, this, &read_callback, nullptr, &seek_callback);
        if (!io) { av_free(io_buffer); fail(logical, "cannot create FFmpeg custom-I/O context"); }
        format = avformat_alloc_context();
        if (!format) fail(logical, "cannot allocate FFmpeg format context");
        format->pb = io;
        format->flags |= AVFMT_FLAG_CUSTOM_IO;
        int result = avformat_open_input(&format, nullptr, nullptr, nullptr);
        if (result < 0) fail(logical, "cannot open Bink stream: " + ffmpeg_error(result));
        if (!format->iformat || std::string_view(format->iformat->name) != "bink")
            fail(logical, "unsupported container; expected Bink");
        result = avformat_find_stream_info(format, nullptr);
        if (result < 0) fail(logical, "cannot inspect Bink streams: " + ffmpeg_error(result));

        for (unsigned index = 0; index < format->nb_streams; ++index) {
            const auto id = format->streams[index]->codecpar->codec_id;
            if (video_stream < 0 && id == AV_CODEC_ID_BINKVIDEO) video_stream = static_cast<int>(index);
            else if (audio_stream < 0 && bink_audio_codec(id)) audio_stream = static_cast<int>(index);
        }
        if (video_stream < 0) fail(logical, "Bink video stream is missing or unsupported");
        open_video();
        if (audio_stream >= 0) open_audio();
        packet = av_packet_alloc();
        decoded = av_frame_alloc();
            if (!packet || !decoded) fail(logical, "cannot allocate bounded decode state");
        } catch (...) {
            close();
            throw;
        }
    }

    ~Impl() { close(); }

    void open_video()
    {
        auto* stream = format->streams[video_stream];
        const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec) fail(logical, "Bink video decoder is unavailable");
        video = avcodec_alloc_context3(codec);
        if (!video) fail(logical, "cannot allocate Bink video decoder");
        int result = avcodec_parameters_to_context(video, stream->codecpar);
        if (result < 0) fail(logical, "cannot configure Bink video decoder: " + ffmpeg_error(result));
        validate_video_dimensions(static_cast<std::uint32_t>(video->width), static_cast<std::uint32_t>(video->height), limits);
        result = avcodec_open2(video, codec, nullptr);
        if (result < 0) fail(logical, "cannot open Bink video decoder: " + ffmpeg_error(result));
        metadata.width = static_cast<std::uint32_t>(video->width);
        metadata.height = static_cast<std::uint32_t>(video->height);
        metadata.video_codec = codec->name;
        const AVRational rate = av_guess_frame_rate(format, stream, nullptr);
        metadata.frame_rate = rate.num > 0 && rate.den > 0 ? av_q2d(rate) : 30.0;
        scaler = sws_getContext(video->width, video->height, video->pix_fmt,
            video->width, video->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
        if (!scaler) fail(logical, "cannot create bounded RGBA conversion context");
    }

    void open_audio()
    {
        auto* stream = format->streams[audio_stream];
        const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec) fail(logical, "Bink audio decoder is unavailable");
        audio = avcodec_alloc_context3(codec);
        if (!audio) fail(logical, "cannot allocate Bink audio decoder");
        int result = avcodec_parameters_to_context(audio, stream->codecpar);
        if (result < 0) fail(logical, "cannot configure Bink audio decoder: " + ffmpeg_error(result));
        result = avcodec_open2(audio, codec, nullptr);
        if (result < 0) fail(logical, "cannot open Bink audio decoder: " + ffmpeg_error(result));
        AVChannelLayout output_layout = AV_CHANNEL_LAYOUT_STEREO;
        result = swr_alloc_set_opts2(&resampler, &output_layout, AV_SAMPLE_FMT_FLT, 48000,
            &audio->ch_layout, audio->sample_fmt, audio->sample_rate, 0, nullptr);
        if (result < 0 || !resampler) fail(logical, "cannot configure Bink audio conversion: " + ffmpeg_error(result));
        result = swr_init(resampler);
        if (result < 0) fail(logical, "cannot initialize Bink audio conversion: " + ffmpeg_error(result));
        metadata.has_audio = true;
        metadata.audio_sample_rate = static_cast<std::uint32_t>(audio->sample_rate);
        metadata.audio_channels = static_cast<std::uint32_t>(audio->ch_layout.nb_channels);
        metadata.audio_codec = codec->name;
    }

    bool read_frame(VideoFrame& output)
    {
        if (closed || eos) return false;
        for (;;) {
            int result = avcodec_receive_frame(video, decoded);
            if (result == 0) {
                convert_video(output);
                av_frame_unref(decoded);
                return true;
            }
            if (result != AVERROR(EAGAIN) && result != AVERROR_EOF)
                fail(logical, "Bink video decode failed: " + ffmpeg_error(result));

            result = av_read_frame(format, packet);
            if (result == AVERROR_EOF) {
                if (!flushing) {
                    flushing = true;
                    avcodec_send_packet(video, nullptr);
                    if (audio) avcodec_send_packet(audio, nullptr);
                    drain_audio_decoder();
                    continue;
                }
                eos = true;
                return false;
            }
            if (result < 0) fail(logical, "Bink packet read failed: " + ffmpeg_error(result));
            if (packet->size < 0 || static_cast<std::size_t>(packet->size) > limits.maximum_packet_bytes) {
                av_packet_unref(packet);
                fail(logical, "packet exceeds configured byte limit");
            }
            if (packet->stream_index == video_stream) {
                result = avcodec_send_packet(video, packet);
                av_packet_unref(packet);
                if (result < 0) fail(logical, "Bink video packet rejected: " + ffmpeg_error(result));
            } else if (packet->stream_index == audio_stream && audio) {
                result = avcodec_send_packet(audio, packet);
                av_packet_unref(packet);
                if (result < 0) fail(logical, "Bink audio packet rejected: " + ffmpeg_error(result));
                drain_audio_decoder();
            } else {
                av_packet_unref(packet);
            }
        }
    }

    void convert_video(VideoFrame& output)
    {
        validate_video_dimensions(static_cast<std::uint32_t>(decoded->width), static_cast<std::uint32_t>(decoded->height), limits);
        output.width = static_cast<std::uint32_t>(decoded->width);
        output.height = static_cast<std::uint32_t>(decoded->height);
        const auto bytes_needed = static_cast<std::size_t>(output.width) * output.height * 4U;
        if (bytes_needed > limits.maximum_frame_bytes) fail(logical, "decoded frame exceeds configured conversion limit");
        output.rgba.assign(bytes_needed, 0);
        std::uint8_t* destinations[]{output.rgba.data(), nullptr, nullptr, nullptr};
        int strides[]{static_cast<int>(output.width * 4U), 0, 0, 0};
        const int rows = sws_scale(scaler, decoded->data, decoded->linesize, 0, decoded->height, destinations, strides);
        if (rows != decoded->height) fail(logical, "incomplete RGBA frame conversion");
        const double step = 1.0 / std::max(1.0, metadata.frame_rate);
        output.timestamp_seconds = normalize_media_timestamp(
            stream_timestamp(decoded, format->streams[video_stream]), previous_video_timestamp, step);
        previous_video_timestamp = output.timestamp_seconds;
    }

    void drain_audio_decoder()
    {
        for (;;) {
            int result = avcodec_receive_frame(audio, decoded);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) return;
            if (result < 0) fail(logical, "Bink audio decode failed: " + ffmpeg_error(result));
            const auto delay = swr_get_delay(resampler, audio->sample_rate);
            const auto frames = av_rescale_rnd(delay + decoded->nb_samples, 48000, audio->sample_rate, AV_ROUND_UP);
            if (frames <= 0 || static_cast<std::size_t>(frames) * 2U > limits.maximum_audio_samples_per_chunk) {
                av_frame_unref(decoded);
                fail(logical, "decoded audio chunk exceeds configured sample limit");
            }
            VideoAudioChunk chunk;
            chunk.interleaved.resize(static_cast<std::size_t>(frames) * 2U);
            std::uint8_t* output[]{reinterpret_cast<std::uint8_t*>(chunk.interleaved.data())};
            const int converted = swr_convert(resampler, output, static_cast<int>(frames),
                const_cast<const std::uint8_t**>(decoded->extended_data), decoded->nb_samples);
            if (converted < 0) { av_frame_unref(decoded); fail(logical, "Bink audio conversion failed: " + ffmpeg_error(converted)); }
            chunk.interleaved.resize(static_cast<std::size_t>(converted) * 2U);
            const double step = static_cast<double>(converted) / 48000.0;
            chunk.timestamp_seconds = normalize_media_timestamp(
                stream_timestamp(decoded, format->streams[audio_stream]), previous_audio_timestamp, step);
            previous_audio_timestamp = chunk.timestamp_seconds;
            av_frame_unref(decoded);
            if (!audio_queue.push(std::move(chunk))) fail(logical, "bounded audio queue is full");
        }
    }

    std::vector<VideoAudioChunk> drain_audio()
    {
        std::vector<VideoAudioChunk> output;
        output.reserve(audio_queue.size());
        VideoAudioChunk chunk;
        while (audio_queue.pop(chunk)) output.push_back(std::move(chunk));
        return output;
    }

    void close() noexcept
    {
        if (closed) return;
        closed = true;
        audio_queue.clear();
        av_frame_free(&decoded);
        av_packet_free(&packet);
        swr_free(&resampler);
        sws_freeContext(scaler); scaler = nullptr;
        avcodec_free_context(&audio);
        avcodec_free_context(&video);
        avformat_close_input(&format);
        if (io) {
            av_freep(&io->buffer);
            avio_context_free(&io);
        }
        bytes.clear();
    }

    static int read_callback(void* opaque, std::uint8_t* destination, int requested)
    {
        auto& self = *static_cast<Impl*>(opaque);
        if (requested <= 0) return 0;
        if (self.position >= self.bytes.size()) return AVERROR_EOF;
        const auto count = std::min<std::size_t>(static_cast<std::size_t>(requested), self.bytes.size() - self.position);
        std::memcpy(destination, self.bytes.data() + self.position, count);
        self.position += count;
        return static_cast<int>(count);
    }

    static std::int64_t seek_callback(void* opaque, std::int64_t offset, int whence)
    {
        auto& self = *static_cast<Impl*>(opaque);
        if (whence == AVSEEK_SIZE) return static_cast<std::int64_t>(self.bytes.size());
        const int base_mode = whence & ~AVSEEK_FORCE;
        std::int64_t base = 0;
        if (base_mode == SEEK_SET) base = 0;
        else if (base_mode == SEEK_CUR) base = static_cast<std::int64_t>(self.position);
        else if (base_mode == SEEK_END) base = static_cast<std::int64_t>(self.bytes.size());
        else return AVERROR(EINVAL);
        if ((offset < 0 && base < -offset) || (offset > 0 && base > std::numeric_limits<std::int64_t>::max() - offset))
            return AVERROR(EINVAL);
        const auto target = base + offset;
        if (target < 0 || static_cast<std::uint64_t>(target) > self.bytes.size()) return AVERROR(EINVAL);
        self.position = static_cast<std::size_t>(target);
        return target;
    }

    std::string logical;
    VideoLimits limits;
    VideoMetadata metadata;
    std::vector<std::uint8_t> bytes;
    std::size_t position = 0;
    AVIOContext* io = nullptr;
    AVFormatContext* format = nullptr;
    AVCodecContext* video = nullptr;
    AVCodecContext* audio = nullptr;
    SwsContext* scaler = nullptr;
    SwrContext* resampler = nullptr;
    AVPacket* packet = nullptr;
    AVFrame* decoded = nullptr;
    int video_stream = -1;
    int audio_stream = -1;
    bool flushing = false;
    bool eos = false;
    bool closed = false;
    double previous_video_timestamp = -1.0;
    double previous_audio_timestamp = -1.0;
    BoundedMediaQueue<VideoAudioChunk> audio_queue;
};

FfmpegVideoDecoder::FfmpegVideoDecoder(const data::VirtualFileSystem& vfs, std::string logical_path,
    VideoLimits limits, bool decoder_available)
    : impl_(std::make_unique<Impl>(vfs, std::move(logical_path), limits, decoder_available)) {}
FfmpegVideoDecoder::~FfmpegVideoDecoder() = default;
FfmpegVideoDecoder::FfmpegVideoDecoder(FfmpegVideoDecoder&&) noexcept = default;
FfmpegVideoDecoder& FfmpegVideoDecoder::operator=(FfmpegVideoDecoder&&) noexcept = default;
const VideoMetadata& FfmpegVideoDecoder::metadata() const noexcept { return impl_->metadata; }
const std::string& FfmpegVideoDecoder::logical_path() const noexcept { return impl_->logical; }
bool FfmpegVideoDecoder::read_frame(VideoFrame& frame) { return impl_->read_frame(frame); }
std::vector<VideoAudioChunk> FfmpegVideoDecoder::drain_audio() { return impl_->drain_audio(); }
bool FfmpegVideoDecoder::end_of_stream() const noexcept { return impl_->eos; }
void FfmpegVideoDecoder::close() noexcept { impl_->close(); }

} // namespace zh::video
