#pragma once

#include <avis/application_base.hpp>
#include <avis/vulkan/buffer.hpp>
#include <avis/vulkan/image.hpp>
#include <avis/vulkan/command_buffers.hpp>
#include <avis/vulkan/screenquad.hpp>
#include <avis/vulkan/handles.hpp>
#include <avis/audio/io.hpp>

#include <boost/lockfree/spsc_queue.hpp>

#include <array>


namespace avis {

constexpr auto texture_extent = VkExtent3D{4096, 1024, 1};
constexpr auto texture_bytes  = texture_extent.width * texture_extent.height * texture_extent.depth * 4;


class application final : private application_base {
public:
    application(application_info const& appinfo)
            : application_base(appinfo)
            , paused_{true}
            , texture_offset_{0} {}

    using application_base::create;
    using application_base::destroy;

    void play(std::string const& file, audio::ffmpeg::stream_format const& fmt = {2, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLT, 192000});

private:
    void frame_update();
    void frame_draw();

    int cb_audio(void* outbuf, unsigned long framecount, PaStreamCallbackTimeInfo const* time, unsigned long flags);

    void cb_create() override;
    void cb_destroy() override;
    void cb_display() override;
    void cb_resize(unsigned int width, unsigned int height) noexcept override;
    void cb_input_key(int key, int scancode, int action, int mods) noexcept override;

    auto select_physical_device(std::vector<VkPhysicalDevice> const& devices) const -> VkPhysicalDevice override;

    void setup_renderpass();
    void setup_shader_modules();
    void setup_descriptors();
    void setup_pipeline_layout();
    void setup_pipeline();
    void setup_framebuffers();
    void setup_command_pool();
    void setup_screenquad();
    void setup_texture();
    void setup_uniform_buffer();
    void setup_command_buffers();
    void setup_semaphores();

    void setup_transfer_cmdbuffer(std::int32_t offset, std::uint32_t len);

private:
    std::atomic_bool paused_;
    std::int32_t     texture_offset_;

    vulkan::shader_module            vert_shader_module_;
    vulkan::shader_module            frag_shader_module_;

    vulkan::render_pass              renderpass_;
    vulkan::descriptor_set_layout    descriptor_layout_;
    vulkan::descriptor_pool          descriptor_pool_;
    VkDescriptorSet                  descriptor_set_;
    vulkan::pipeline_layout          pipeline_layout_;
    vulkan::pipeline                 pipeline_;
    std::vector<vulkan::framebuffer> framebuffers_;
    vulkan::screenquad               screenquad_;
    vulkan::image                    texture_staging_image_;
    vulkan::image                    texture_image_;
    vulkan::image_view               texture_view_;
    vulkan::sampler                  texture_sampler_;
    vulkan::buffer                   uniform_staging_buffer_;
    vulkan::buffer                   uniform_buffer_;
    vulkan::command_pool             command_pool_;
    vulkan::command_buffer           transfer_cmdbuffer_;
    vulkan::command_buffers          command_buffers_;

    vulkan::semaphore                sem_img_available_;
    vulkan::semaphore                sem_img_finished_;
    vulkan::fence                    staging_fence_;

    audio::ffmpeg::audio_input_stream audio_in_;
    audio::portaudio::output_stream   audio_out_;
    int                               audio_out_sample_size_;
    std::vector<uint8_t>              audio_rdbuf_;
    std::unique_ptr<boost::lockfree::spsc_queue<std::uint8_t>> audio_queue_;
    std::atomic_bool                  audio_eof_;
    std::atomic_bool                  audio_flushed_;
};

} /* namespace avis */
