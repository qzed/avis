#pragma once

#include <avis/application_base.hpp>
#include <avis/vulkan/command_buffers.hpp>
#include <array>


namespace avis {

struct vertex {
    float pos[2];
    float color[3];

    static auto get_binding_description()    -> VkVertexInputBindingDescription;
    static auto get_attribute_descriptions() -> std::array<VkVertexInputAttributeDescription, 2>;
};

const std::vector<vertex> vertices = {
    {{-0.75f, -0.75f}, {0.4784f, 0.5882f, 0.2431f}},
    {{ 0.75f, -0.75f}, {0.4784f, 0.5882f, 0.2431f}},
    {{ 0.75f,  0.75f}, {0.1725f, 0.2509f, 0.3137f}},
    {{-0.75f,  0.75f}, {0.1725f, 0.2509f, 0.3137f}}
};

const std::vector<uint16_t> indices = {
    1, 2, 0, 3
};


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
    void setup_vertex_buffer();
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
    vulkan::handle<VkDeviceMemory>               vertex_buffer_memory_;
    vulkan::handle<VkBuffer>                     vertex_buffer_;
    vulkan::command_buffers                      command_buffers_;
    vulkan::handle<VkSemaphore>                  sem_img_available_;
    vulkan::handle<VkSemaphore>                  sem_img_finished_;

};

} /* namespace avis */
