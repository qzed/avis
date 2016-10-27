#pragma once

#include "screenquad.hpp"

#include <avis/application_base.hpp>
#include <avis/vulkan/render_pass.hpp>
#include <avis/vulkan/shader_module.hpp>
#include <avis/vulkan/command_buffers.hpp>
#include <avis/vulkan/semaphore.hpp>

#include <array>


namespace avis {

constexpr auto texture_extent = VkExtent3D{4096, 1024, 1};
constexpr auto texture_bytes  = texture_extent.width * texture_extent.height * texture_extent.depth * 4;


class triangle_example final : private application_base {
public:
    triangle_example(application_info const& appinfo)
            : application_base(appinfo)
            , paused_{false}
            , texture_offset_{0} {}

    using application_base::create;
    using application_base::destroy;
    using application_base::run;

private:
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
    void setup_vertex_buffer();
    void setup_texture();
    void setup_uniform_buffer();
    void setup_command_buffers();
    void setup_semaphores();

private:
    bool         paused_;
    std::int32_t texture_offset_;

    vulkan::shader_module                        vert_shader_module_;
    vulkan::shader_module                        frag_shader_module_;

    vulkan::render_pass                          renderpass_;
    vulkan::handle<VkDescriptorSetLayout>        descriptor_layout_;
    vulkan::handle<VkDescriptorPool>             descriptor_pool_;
    VkDescriptorSet                              descriptor_set_;
    vulkan::handle<VkPipelineLayout>             pipeline_layout_;
    vulkan::handle<VkPipeline>                   pipeline_;
    std::vector<vulkan::handle<VkFramebuffer>>   framebuffers_;
    vulkan::handle<VkCommandPool>                command_pool_;
    vulkan::handle<VkDeviceMemory>               vertex_buffer_memory_;
    vulkan::handle<VkBuffer>                     vertex_buffer_;
    vulkan::handle<VkDeviceMemory>               texture_memory_;
    vulkan::handle<VkImage>                      texture_image_;
    vulkan::handle<VkImageView>                  texture_view_;
    vulkan::handle<VkSampler>                    texture_sampler_;
    vulkan::handle<VkBuffer>                     ubo_buffer_;
    vulkan::handle<VkDeviceMemory>               ubo_buffer_memory_;
    vulkan::command_buffers                      command_buffers_;
    vulkan::semaphore                            sem_img_available_;
    vulkan::semaphore                            sem_img_finished_;
};

} /* namespace avis */
