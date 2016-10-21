#include "triangle_example.hpp"

#include <avis/vulkan/shader.hpp>
#include <avis/utils/fileio.hpp>
#include <iostream>


namespace avis {
namespace {

inline auto load_shader_module(VkDevice device, std::string const& file) -> vulkan::handle<VkShaderModule> {
    return vulkan::create_shader_module(device, fileio::read_file_to_vector(file));
}

auto create_semaphore(VkDevice device, VkAllocationCallbacks const* alloc) -> vulkan::expected<vulkan::handle<VkSemaphore>> {
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore handle = nullptr;
    VkResult status = vkCreateSemaphore(device, &create_info, nullptr, &handle);
    if (status != VK_SUCCESS) return vulkan::to_result(status);

    return vulkan::make_handle(handle, nullptr, [=](auto h, auto a){
        vkDestroySemaphore(device, h, a);
    });
}

} /* namespace */


void triangle_example::cb_create() {
    setup_shader_modules();
    setup_renderpass();
    setup_pipeline_layout();
    setup_pipeline();
    setup_framebuffers();
    setup_command_pool();
    setup_command_buffers();
    setup_semaphores();
}

void triangle_example::setup_shader_modules() {
    vert_shader_module_ = load_shader_module(get_device().get_handle(), "./resources/triangle.vert.spv");
    frag_shader_module_ = load_shader_module(get_device().get_handle(), "./resources/triangle.frag.spv");
}

void triangle_example::setup_renderpass() {
    VkDevice device = get_device().get_handle();

    VkAttachmentDescription color_attachment = {};
    color_attachment.format         = get_swapchain().get_surface_format().format;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.pAttachments    = &color_attachment;
    create_info.subpassCount    = 1;
    create_info.pSubpasses      = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies   = &dependency;

    VkRenderPass renderpass = nullptr;
    VkResult status = vkCreateRenderPass(device, &create_info, nullptr, &renderpass);
    if (status != VK_SUCCESS)
        throw vulkan::exception(vulkan::to_result(status));

    renderpass_ = vulkan::make_handle(renderpass, nullptr, [=](auto h, auto a){
        vkDestroyRenderPass(device, h, a);
    }).move_or_throw();
}

void triangle_example::setup_pipeline_layout() {
    VkDevice device = get_device().get_handle();

    VkPipelineLayoutCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount         = 0;
    create_info.pSetLayouts            = nullptr;
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges    = nullptr;

    VkPipelineLayout handle = nullptr;
    VkResult status = vkCreatePipelineLayout(device, &create_info, nullptr, &handle);
    if (status != VK_SUCCESS) throw vulkan::exception(vulkan::to_result(status));

    pipeline_layout_ = vulkan::make_handle(handle, nullptr, [=](auto h, auto a){
        vkDestroyPipelineLayout(device, h, a);
    }).move_or_throw();
}

void triangle_example::setup_pipeline() {
    VkDevice device = get_device().get_handle();

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module_.get_handle();
    vert_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module_.get_handle();
    frag_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount   = 0;
    vertex_input_info.pVertexBindingDescriptions      = nullptr;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions    = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = false;

    VkViewport viewport = {};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(get_swapchain().get_extent().width);
    viewport.height   = static_cast<float>(get_swapchain().get_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 0.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = get_swapchain().get_extent();

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports    = &viewport;
    viewport_state.scissorCount  = 1;
    viewport_state.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = false;
    rasterizer.rasterizerDiscardEnable = false;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = false;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = true;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = false;
    multisampling.alphaToOneEnable      = false;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask
            = VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable         = false;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = false;
    color_blending.logicOp           = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    // vk::DynamicState dynamic_states[] = {vk::DynamicState::eViewport, vk::DynamicState::eLineWidth};
    // vk::PipelineDynamicStateCreateInfo dynamic_state{};
    // dynamic_state.sType = ...
    // dynamic_state.dynamicStateCount = 2;
    // dynamic_state.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = 2;
    create_info.pStages    = shader_stages;

    create_info.pVertexInputState   = &vertex_input_info;
    create_info.pInputAssemblyState = &input_assembly_info;
    create_info.pViewportState      = &viewport_state;
    create_info.pRasterizationState = &rasterizer;
    create_info.pMultisampleState   = &multisampling;
    create_info.pDepthStencilState  = nullptr;
    create_info.pColorBlendState    = &color_blending;
    create_info.pDynamicState       = nullptr;

    create_info.layout     = pipeline_layout_.get_handle();
    create_info.renderPass = renderpass_.get_handle();
    create_info.subpass    = 0;

    create_info.basePipelineHandle = nullptr;
    create_info.basePipelineIndex  = -1;

    VkPipeline handle = nullptr;
    VkResult status = vkCreateGraphicsPipelines(device, nullptr, 1, &create_info, nullptr, &handle);
    if (status != VK_SUCCESS) throw vulkan::exception(vulkan::to_result(status));

    pipeline_ = vulkan::make_handle(handle, nullptr, [=](auto h, auto a){
        vkDestroyPipeline(device, h, a);
    }).move_or_throw();
}

void triangle_example::setup_framebuffers() {
    VkDevice const device = get_device().get_handle();

    auto const& image_views = get_swapchain().get_image_views();

    framebuffers_.clear();
    framebuffers_.reserve(image_views.size());

    for (std::size_t i = 0; i < image_views.size(); i++) {
        auto image_view = image_views[i].get_handle();

        VkFramebufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass      = renderpass_.get_handle();
        create_info.attachmentCount = 1;
        create_info.pAttachments    = &image_view;
        create_info.width           = get_swapchain().get_extent().width;
        create_info.height          = get_swapchain().get_extent().height;
        create_info.layers          = 1;

        VkFramebuffer handle = nullptr;
        VkResult status = vkCreateFramebuffer(device, &create_info, nullptr, &handle);
        if (status != VK_SUCCESS) throw vulkan::exception(vulkan::to_result(status));

        framebuffers_.push_back(vulkan::make_handle(handle, nullptr, [=](auto h, auto a) {
            vkDestroyFramebuffer(device, h, a);
        }).move_or_throw());
    }
}

void triangle_example::setup_command_pool() {
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = get_device().get_graphics_queue_index();

    VkDevice device = get_device().get_handle();
    VkCommandPool handle = nullptr;
    VkResult status = vkCreateCommandPool(device, &create_info, nullptr, &handle);
    if (status != VK_SUCCESS) throw vulkan::exception(vulkan::to_result(status));

    command_pool_ = vulkan::make_handle(handle, nullptr, [=](auto h, auto a){
        vkDestroyCommandPool(device, h, a);
    }).move_or_throw();
}

void triangle_example::setup_command_buffers() {
    // create command buffers
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool_.get_handle();
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>(framebuffers_.size());

    auto command_buffers = vulkan::command_buffers::create(get_device().get_handle(), alloc_info).move_or_throw();

    // fill command buffers
    for (std::size_t i = 0; i < command_buffers.size(); i++) {
        auto const buffer = command_buffers[i];

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        begin_info.pInheritanceInfo = nullptr;

        VkResult status = vkBeginCommandBuffer(buffer, &begin_info);
        if (status != VK_SUCCESS) throw vulkan::exception(vulkan::to_result(status));

        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

        VkRenderPassBeginInfo pass_info = {};
        pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        pass_info.renderPass        = renderpass_.get_handle();
        pass_info.framebuffer       = framebuffers_[i].get_handle();
        pass_info.renderArea.offset = {0, 0};
        pass_info.renderArea.extent = get_swapchain().get_extent();
        pass_info.clearValueCount   = 1;
        pass_info.pClearValues      = &clear_color;

        vkCmdBeginRenderPass(buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get_handle());
        vkCmdDraw(buffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(buffer);

        status = vkEndCommandBuffer(buffer);
        if (status != VK_SUCCESS) throw vulkan::exception(vulkan::to_result(status));
    }

    command_buffers_ = std::move(command_buffers);
}

void triangle_example::setup_semaphores() {
    sem_img_available_ = create_semaphore(get_device().get_handle(), nullptr).move_or_throw();
    sem_img_finished_  = create_semaphore(get_device().get_handle(), nullptr).move_or_throw();
}


void triangle_example::cb_destroy() {
    sem_img_available_.destroy();
    sem_img_finished_.destroy();
    command_buffers_.destroy();
    command_pool_.destroy();
    framebuffers_.clear();
    pipeline_.destroy();
    pipeline_layout_.destroy();
    renderpass_.destroy();

    vert_shader_module_.destroy();
    frag_shader_module_.destroy();
}

void triangle_example::cb_display() {
    VkResult status = VK_SUCCESS;

    std::uint32_t image_index = 0;
    status = vkAcquireNextImageKHR(get_device().get_handle(), get_swapchain().get_swapchain(),
            std::numeric_limits<std::uint64_t>::max(), sem_img_available_.get_handle(), nullptr, &image_index);

    if (status == VK_ERROR_OUT_OF_DATE_KHR || status == VK_SUBOPTIMAL_KHR)
        return;
    if (status != VK_SUCCESS)
        throw vulkan::exception(vulkan::to_result(status));

    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSemaphore sem_img_available = sem_img_available_.get_handle();
    VkSemaphore sem_img_finished  = sem_img_finished_.get_handle();

    VkCommandBuffer command_buffer = command_buffers_[image_index];

    VkSwapchainKHR swapchain = get_swapchain().get_swapchain();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = &sem_img_available;
    submit_info.pWaitDstStageMask    = wait_stages;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = &sem_img_finished;

    status = vkQueueSubmit(get_device().get_graphics_queue(), 1, &submit_info, nullptr);
    if (status != VK_SUCCESS) throw vulkan::exception(vulkan::to_result(status));

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &sem_img_finished;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swapchain;
    present_info.pImageIndices      = &image_index;
    present_info.pResults           = nullptr;

    status = vkQueuePresentKHR(get_device().get_present_queue(), &present_info);
    if (status != VK_SUCCESS) throw vulkan::exception(vulkan::to_result(status));
}

void triangle_example::cb_resize(unsigned int width, unsigned int height) noexcept {
    setup_renderpass();     // NOTE: format-check could avoid re-creation in certain cases
    setup_pipeline();       // NOTE: could in some cases be avoided by using dynamic state, depends on renderpass
    setup_framebuffers();
    setup_command_buffers();
}

void triangle_example::cb_input_key(int key, int scancode, int action, int mods) noexcept {
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
        window().set_terminate_request(true);
}

auto triangle_example::select_physical_device(std::vector<VkPhysicalDevice> const& devices) const -> VkPhysicalDevice {
    VkPhysicalDevice selected = devices[0];

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(selected, &properties);

    char const* device_type = [&](){
        switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "CPU";
        default:                                     return "Unknown";
        }
    }();

    std::cout << "Selected Device:\n"
              << "    Vendor ID:      " << properties.vendorID                        << "\n"
              << "    Device Name:    " << properties.deviceName                      << "\n"
              << "    Device ID:      " << properties.deviceID                        << "\n"
              << "    Device Type:    " << device_type                                << "\n"
              << "    Driver Version: " << VK_VERSION_MAJOR(properties.driverVersion) << "."
                                        << VK_VERSION_MINOR(properties.driverVersion) << "."
                                        << VK_VERSION_PATCH(properties.driverVersion) << "\n"
              << "    Vulkan Version: " << VK_VERSION_MAJOR(properties.apiVersion)    << "."
                                        << VK_VERSION_MINOR(properties.apiVersion)    << "."
                                        << VK_VERSION_PATCH(properties.apiVersion)    << "\n";

    return selected;
}

} /* namespace avis */
