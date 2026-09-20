#include "zh/video/player.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace zh::video {
namespace {
struct Vertex { float x, y; std::uint32_t color; float u, v; };

std::string number(double value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}
}

PresentationRect fit_aspect(std::uint32_t source_width, std::uint32_t source_height,
    std::uint32_t target_width, std::uint32_t target_height)
{
    if (source_width == 0 || source_height == 0 || target_width == 0 || target_height == 0)
        throw VideoError("video aspect fit requires positive dimensions");
    const double scale = std::min(static_cast<double>(target_width) / source_width,
        static_cast<double>(target_height) / source_height);
    const float width = static_cast<float>(source_width * scale);
    const float height = static_cast<float>(source_height * scale);
    return {(static_cast<float>(target_width) - width) * 0.5F,
        (static_cast<float>(target_height) - height) * 0.5F, width, height};
}

NullVideoAudioSink::NullVideoAudioSink(std::size_t capacity) : capacity_(capacity)
{
    if (capacity == 0) throw VideoError("video audio sink capacity must be positive");
}

bool NullVideoAudioSink::submit(VideoAudioChunk chunk)
{
    if (chunk.sample_rate == 0 || chunk.channels != 2 || chunk.interleaved.empty()
        || chunk.interleaved.size() % chunk.channels != 0 || chunks_.size() >= capacity_) return false;
    chunks_.push_back({static_cast<double>(chunk.interleaved.size() / chunk.channels) / chunk.sample_rate});
    return true;
}

void NullVideoAudioSink::set_paused(bool paused) noexcept { paused_ = paused; }

void NullVideoAudioSink::advance(double seconds) noexcept
{
    if (paused_ || !std::isfinite(seconds) || seconds <= 0.0) return;
    while (seconds > 0.0 && !chunks_.empty()) {
        const double amount = std::min(seconds, chunks_.front().seconds);
        chunks_.front().seconds -= amount;
        seconds -= amount;
        clock_ += amount;
        if (chunks_.front().seconds <= 0.0000001) chunks_.pop_front();
    }
}

void NullVideoAudioSink::reset() noexcept
{
    chunks_.clear();
    clock_ = 0.0;
    paused_ = false;
}

VideoPlayer::VideoPlayer(std::unique_ptr<VideoDecoder> decoder, VideoAudioSink& audio,
    renderer::RecordingGpuDevice& recorder, std::uint32_t target_width, std::uint32_t target_height)
    : decoder_(std::move(decoder)), audio_(audio), recorder_(recorder),
      target_width_(target_width), target_height_(target_height)
{
    if (!decoder_) throw VideoError("video player requires a decoder");
    if (target_width == 0 || target_height == 0 || target_width > 16384 || target_height > 16384)
        throw VideoError("video presentation target must be within 1..16384");
    recorder_.record_marker("video-begin logical=" + decoder_->logical_path());
}

VideoPlayer::~VideoPlayer() { shutdown(); }

void VideoPlayer::set_paused(bool paused) noexcept
{
    if (state_ != PlaybackState::playing && state_ != PlaybackState::paused) return;
    user_paused_ = paused;
    const bool effective = user_paused_ || !focused_;
    state_ = effective ? PlaybackState::paused : PlaybackState::playing;
    audio_.set_paused(effective);
    recorder_.record_marker(effective ? "video-pause" : "video-resume");
}

void VideoPlayer::set_focused(bool focused) noexcept
{
    if (state_ != PlaybackState::playing && state_ != PlaybackState::paused) return;
    focused_ = focused;
    const bool effective = user_paused_ || !focused_;
    state_ = effective ? PlaybackState::paused : PlaybackState::playing;
    audio_.set_paused(effective);
    recorder_.record_marker(focused ? "video-focus-gained" : "video-focus-lost");
}

bool VideoPlayer::fetch_pending()
{
    if (have_pending_) return true;
    try {
        if (!decoder_->read_frame(pending_)) return false;
        for (auto& chunk : decoder_->drain_audio())
            if (!audio_.submit(std::move(chunk))) { fail("video audio adapter queue is full or rejected decoded PCM"); return false; }
        have_pending_ = true;
        return true;
    } catch (const std::exception& error) {
        fail(error.what());
        return false;
    }
}

void VideoPlayer::advance(double seconds)
{
    if (state_ != PlaybackState::playing) return;
    if (!std::isfinite(seconds) || seconds < 0.0) { fail("video advance duration is invalid"); return; }
    media_clock_ += seconds;
    audio_.advance(seconds);
    if (!fetch_pending()) {
        if (state_ == PlaybackState::playing && decoder_->end_of_stream()) finish(PlaybackState::completed, "video-eos return-game-render");
        return;
    }
    const double master = decoder_->metadata().has_audio ? audio_.clock_seconds() : media_clock_;
    while (state_ == PlaybackState::playing && have_pending_ && pending_.timestamp_seconds <= master + 0.000001) {
        if (!present(pending_)) return;
        have_pending_ = false;
        if (!fetch_pending()) {
            if (state_ == PlaybackState::playing && decoder_->end_of_stream()) finish(PlaybackState::completed, "video-eos return-game-render");
            return;
        }
    }
}

bool VideoPlayer::ensure_resources(const VideoFrame& frame)
{
    if (pipeline_) return true;
    renderer::TextureDesc target;
    target.width = target_width_; target.height = target_height_; target.render_target = true;
    color_target_ = recorder_.create_texture(target, "video color target");
    target.format = renderer::TextureFormat::depth24_stencil8; target.sampled = false;
    depth_target_ = recorder_.create_texture(target, "video depth target");
    renderer::TextureDesc movie;
    movie.width = frame.width; movie.height = frame.height;
    movie_texture_ = recorder_.create_texture(movie, "video decoded frame");
    sampler_ = recorder_.create_sampler({renderer::Filter::linear, renderer::Filter::linear,
        renderer::Filter::linear, renderer::AddressMode::clamp_edge, renderer::AddressMode::clamp_edge,
        renderer::AddressMode::clamp_edge, 1}, "video sampler");
    vertex_shader_ = recorder_.create_shader({renderer::ShaderStage::vertex, "video.vert", 0, 0}, "video vertex");
    fragment_shader_ = recorder_.create_shader({renderer::ShaderStage::fragment, "video.frag", 0, 1}, "video fragment");
    vertices_ = recorder_.create_buffer({sizeof(Vertex) * 6U, renderer::BufferUsage::vertex, true}, "video quad vertices");
    renderer::PipelineDesc desc;
    desc.vertex_shader = vertex_shader_; desc.fragment_shader = fragment_shader_;
    desc.vertex_layout = renderer::VertexLayout::position_color_uv;
    desc.depth_stencil.depth_test = false; desc.depth_stencil.depth_write = false;
    desc.raster.cull = renderer::CullMode::none;
    pipeline_ = recorder_.create_pipeline(renderer::PipelineKey(desc), "video presentation");
    if (!color_target_ || !depth_target_ || !movie_texture_ || !sampler_ || !vertex_shader_
        || !fragment_shader_ || !vertices_ || !pipeline_) {
        fail("video recorder resource creation failed: " + recorder_.last_error());
        return false;
    }
    return true;
}

bool VideoPlayer::present(const VideoFrame& frame)
{
    if (frame.width == 0 || frame.height == 0
        || frame.rgba.size() != static_cast<std::size_t>(frame.width) * frame.height * 4U) {
        fail("decoded video frame has invalid dimensions or RGBA byte count");
        return false;
    }
    if (!ensure_resources(frame)) return false;
    const auto rect = fit_aspect(frame.width, frame.height, target_width_, target_height_);
    const float left = rect.x + renderer::RendererConventions::ui_half_pixel_offset;
    const float top = rect.y + renderer::RendererConventions::ui_half_pixel_offset;
    const float right = left + rect.width; const float bottom = top + rect.height;
    const std::array<Vertex, 6> vertices{{
        {left, top, 0xffffffffU, 0, 0}, {right, top, 0xffffffffU, 1, 0},
        {right, bottom, 0xffffffffU, 1, 1}, {left, top, 0xffffffffU, 0, 0},
        {right, bottom, 0xffffffffU, 1, 1}, {left, bottom, 0xffffffffU, 0, 1},
    }};
    if (auto result = recorder_.upload({vertices_, sizeof(vertices), 0, sizeof(vertices)}, vertices.data()); !result) {
        fail(result.error); return false;
    }
    recorder_.record_marker("video-texture-upload bytes=" + std::to_string(frame.rgba.size())
        + " pts=" + number(frame.timestamp_seconds));
    recorder_.record_marker("video-aspect rect=" + number(rect.x) + "," + number(rect.y) + ","
        + number(rect.width) + "," + number(rect.height));
    renderer::RenderPassDesc pass;
    pass.color_targets[0] = color_target_; pass.color_target_count = 1; pass.depth_target = depth_target_;
    pass.width = target_width_; pass.height = target_height_;
    if (auto result = recorder_.begin_pass(pass, "video pass"); !result) { fail(result.error); return false; }
    renderer::DrawDesc draw;
    draw.pipeline = pipeline_; draw.vertex_buffer = vertices_; draw.vertex_or_index_count = 6;
    draw.fragment_bindings.textures[0] = movie_texture_;
    draw.fragment_bindings.samplers[0] = sampler_;
    draw.fragment_bindings.texture_count = 1;
    if (auto result = recorder_.draw(draw); !result) { (void)recorder_.end_pass(); fail(result.error); return false; }
    if (auto result = recorder_.end_pass(); !result) { fail(result.error); return false; }
    ++presented_frames_;
    return true;
}

void VideoPlayer::fail(std::string message) noexcept
{
    last_error_ = std::move(message);
    finish(PlaybackState::error, "video-error " + last_error_);
}

void VideoPlayer::skip() noexcept
{
    if (state_ == PlaybackState::playing || state_ == PlaybackState::paused)
        finish(PlaybackState::skipped, "video-skip return-game-render");
}

void VideoPlayer::finish(PlaybackState final_state, std::string_view marker) noexcept
{
    recorder_.record_marker(marker);
    if (decoder_) decoder_->close();
    audio_.reset();
    have_pending_ = false;
    pending_ = {};
    release_resources();
    state_ = final_state;
}

void VideoPlayer::release_resources() noexcept
{
    if (pipeline_) recorder_.destroy(pipeline_);
    if (vertices_) recorder_.destroy(vertices_);
    if (fragment_shader_) recorder_.destroy(fragment_shader_);
    if (vertex_shader_) recorder_.destroy(vertex_shader_);
    if (sampler_) recorder_.destroy(sampler_);
    if (movie_texture_) recorder_.destroy(movie_texture_);
    if (depth_target_) recorder_.destroy(depth_target_);
    if (color_target_) recorder_.destroy(color_target_);
    pipeline_ = {}; vertices_ = {}; fragment_shader_ = {}; vertex_shader_ = {}; sampler_ = {};
    movie_texture_ = {}; depth_target_ = {}; color_target_ = {};
}

void VideoPlayer::shutdown() noexcept
{
    if (state_ == PlaybackState::stopped) return;
    if (state_ == PlaybackState::playing || state_ == PlaybackState::paused || state_ == PlaybackState::idle)
        finish(PlaybackState::stopped, "video-shutdown return-game-render");
    else {
        release_resources();
        state_ = PlaybackState::stopped;
    }
}

} // namespace zh::video
