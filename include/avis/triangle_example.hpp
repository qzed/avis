#pragma once

#include <avis/application_base.hpp>
#include <avis/vulkan/command_buffers.hpp>


namespace avis {

class triangle_example final : private application_base {
public:
    triangle_example(application_info const& appinfo)
            : application_base(appinfo) {}

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
    void setup_pipeline_layout();
    void setup_pipeline();
    void setup_framebuffers();
    void setup_command_pool();
    void setup_command_buffers();
    void setup_semaphores();

private:
    vulkan::handle<VkShaderModule>               vert_shader_module_;
    vulkan::handle<VkShaderModule>               frag_shader_module_;

    vulkan::handle<VkRenderPass>                 renderpass_;
    vulkan::handle<VkPipelineLayout>             pipeline_layout_;
    vulkan::handle<VkPipeline>                   pipeline_;
    std::vector<vulkan::handle<VkFramebuffer>>   framebuffers_;
    vulkan::handle<VkCommandPool>                command_pool_;
    vulkan::command_buffers                      command_buffers_;
    vulkan::handle<VkSemaphore>                  sem_img_available_;
    vulkan::handle<VkSemaphore>                  sem_img_finished_;
};

} /* namespace avis */
