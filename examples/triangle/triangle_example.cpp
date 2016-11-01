#include "triangle_example.hpp"

#include <avis/utils/fileio.hpp>

#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>


namespace avis {

void triangle_example::cb_create() {
    setup_shader_modules();
    setup_renderpass();
    setup_descriptors();
    setup_pipeline_layout();
    setup_pipeline();
    setup_framebuffers();
    setup_command_pool();
    setup_screenquad();
    setup_texture();
    setup_uniform_buffer();
    setup_command_buffers();
    setup_semaphores();
}

void triangle_example::setup_shader_modules() {
    auto const device = get_device().get_handle();
    auto const vert_shader_src = "./resources/triangle.vert.spv";
    auto const frag_shader_src = "./resources/triangle.frag.spv";

    vert_shader_module_ = vulkan::make_shader_module(device, utils::read_file_to_vector(vert_shader_src)).move_or_throw();
    frag_shader_module_ = vulkan::make_shader_module(device, utils::read_file_to_vector(frag_shader_src)).move_or_throw();
}

void triangle_example::setup_renderpass() {
    auto color_attachment = VkAttachmentDescription{};
    color_attachment.format         = get_swapchain().get_surface_format().format;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    auto color_attachment_ref = VkAttachmentReference{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto subpass = VkSubpassDescription{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment_ref;

    auto dependency = VkSubpassDependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    auto create_info = VkRenderPassCreateInfo{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.pAttachments    = &color_attachment;
    create_info.subpassCount    = 1;
    create_info.pSubpasses      = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies   = &dependency;

    renderpass_ = vulkan::make_render_pass(get_device().get_handle(), create_info).move_or_throw();
}

void triangle_example::setup_descriptors() {
    // setup descriptor layout: texture
    auto sampler_binding = VkDescriptorSetLayoutBinding{};
    sampler_binding.binding            = 0;
    sampler_binding.descriptorCount    = 1;
    sampler_binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler_binding.pImmutableSamplers = nullptr;

    // setup descriptor layout: texture-offset
    auto offset_binding = VkDescriptorSetLayoutBinding{};
    offset_binding.binding            = 1;
    offset_binding.descriptorCount    = 1;
    offset_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    offset_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    offset_binding.pImmutableSamplers = nullptr;

    // setup descriptor layout
    auto bindings = std::array<VkDescriptorSetLayoutBinding, 2>{{ sampler_binding, offset_binding }};

    auto layout_info = VkDescriptorSetLayoutCreateInfo{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = bindings.size();
    layout_info.pBindings    = bindings.data();

    auto layout = vulkan::make_descriptor_set_layout(get_device().get_handle(), layout_info).move_or_throw();

    auto pool_size = std::array<VkDescriptorPoolSize, 2>{};
    pool_size[0].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size[0].descriptorCount = 1;
    pool_size[1].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size[1].descriptorCount = 1;

    // setup descriptor pool
    auto pool_info = VkDescriptorPoolCreateInfo{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = pool_size.size();
    pool_info.pPoolSizes    = pool_size.data();
    pool_info.maxSets       = 1;

    auto pool = vulkan::make_descriptor_pool(get_device().get_handle(), pool_info).move_or_throw();

    // setup descriptor set
    auto layouts = std::array<VkDescriptorSetLayout, 1>{{ layout.get_handle() }};

    auto alloc_info = VkDescriptorSetAllocateInfo{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool     = pool.get_handle();
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts        = layouts.data();

    auto set = VkDescriptorSet{nullptr};
    vulkan::except(vkAllocateDescriptorSets(get_device().get_handle(), &alloc_info, &set));

    descriptor_layout_ = std::move(layout);
    descriptor_pool_   = std::move(pool);
    descriptor_set_    = set;
}

void triangle_example::setup_pipeline_layout() {
    auto layouts = std::array<VkDescriptorSetLayout, 1>{{ descriptor_layout_.get_handle() }};

    auto create_info = VkPipelineLayoutCreateInfo{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount         = layouts.size();
    create_info.pSetLayouts            = layouts.data();
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges    = nullptr;

    pipeline_layout_ = vulkan::make_pipeline_layout(get_device().get_handle(), create_info).move_or_throw();
}

void triangle_example::setup_pipeline() {
    // shader
    auto vert_shader_stage_info = VkPipelineShaderStageCreateInfo{};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module_.get_handle();
    vert_shader_stage_info.pName  = "main";

    auto frag_shader_stage_info = VkPipelineShaderStageCreateInfo{};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module_.get_handle();
    frag_shader_stage_info.pName  = "main";

    auto shader_stages = std::array<VkPipelineShaderStageCreateInfo, 2>{{
        vert_shader_stage_info, frag_shader_stage_info
    }};

    // vertex input
    auto vertex_binding = vulkan::screenquad::vertex::get_binding_description();
    auto vertex_attribs = vulkan::screenquad::vertex::get_attribute_descriptions();

    auto vertex_input_info = VkPipelineVertexInputStateCreateInfo{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount   = 1;
    vertex_input_info.pVertexBindingDescriptions      = &vertex_binding;
    vertex_input_info.vertexAttributeDescriptionCount = vertex_attribs.size();
    vertex_input_info.pVertexAttributeDescriptions    = vertex_attribs.data();

    // input assembly
    auto input_assembly_info = VkPipelineInputAssemblyStateCreateInfo{};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    input_assembly_info.primitiveRestartEnable = true;

    // viewport and scissor
    auto viewport = VkViewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(get_swapchain().get_extent().width);
    viewport.height   = static_cast<float>(get_swapchain().get_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 0.0f;

    auto scissor = VkRect2D{};
    scissor.offset = {0, 0};
    scissor.extent = get_swapchain().get_extent();

    auto viewport_state = VkPipelineViewportStateCreateInfo{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports    = &viewport;
    viewport_state.scissorCount  = 1;
    viewport_state.pScissors     = &scissor;

    // rasterizer
    auto rasterizer = VkPipelineRasterizationStateCreateInfo{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = false;
    rasterizer.rasterizerDiscardEnable = false;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = false;

    // multisampling
    auto multisampling = VkPipelineMultisampleStateCreateInfo{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = true;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = false;
    multisampling.alphaToOneEnable      = false;

    // blending
    auto color_blend_attachment = VkPipelineColorBlendAttachmentState{};
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

    auto color_blending = VkPipelineColorBlendStateCreateInfo{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = false;
    color_blending.logicOp           = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    // create pipeline
    auto create_info = VkGraphicsPipelineCreateInfo{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = shader_stages.size();
    create_info.pStages    = shader_stages.data();

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

    pipeline_ = vulkan::make_pipeline(get_device().get_handle(), nullptr, create_info).move_or_throw();
}

void triangle_example::setup_framebuffers() {
    auto const  device      = get_device().get_handle();
    auto const& image_views = get_swapchain().get_image_views();

    framebuffers_.clear();
    framebuffers_.reserve(image_views.size());

    for (std::size_t i = 0; i < image_views.size(); i++) {
        auto image_view = image_views[i].get_handle();

        auto create_info = VkFramebufferCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass      = renderpass_.get_handle();
        create_info.attachmentCount = 1;
        create_info.pAttachments    = &image_view;
        create_info.width           = get_swapchain().get_extent().width;
        create_info.height          = get_swapchain().get_extent().height;
        create_info.layers          = 1;

        framebuffers_.push_back(vulkan::make_framebuffer(device, create_info).move_or_throw());
    }
}

void triangle_example::setup_command_pool() {
    auto create_info = VkCommandPoolCreateInfo{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = get_device().get_graphics_queue().index();

    command_pool_ = vulkan::make_command_pool(get_device().get_handle(), create_info).move_or_throw();
}

void triangle_example::setup_screenquad() {
    screenquad_ = vulkan::make_screenquad(get_device().get_handle(), get_device().get_physical_device(),
            command_pool_.get_handle(), get_device().get_graphics_queue().handle()).move_or_throw();
}

void triangle_example::setup_texture() {
    auto const device = get_device().get_handle();

    // create staging texture image
    auto staging_image_info = VkImageCreateInfo{};
    staging_image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    staging_image_info.imageType     = VK_IMAGE_TYPE_2D;
    staging_image_info.extent        = texture_extent;
    staging_image_info.mipLevels     = 1;
    staging_image_info.arrayLayers   = 1;
    staging_image_info.format        = VK_FORMAT_R32_SFLOAT;
    staging_image_info.tiling        = VK_IMAGE_TILING_LINEAR;
    staging_image_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    staging_image_info.usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    staging_image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    staging_image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    staging_image_info.flags         = 0;

    auto memory_staging_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto tex_staging_image = vulkan::make_image(device, get_device().get_physical_device(), staging_image_info,
            memory_staging_flags).move_or_throw();

    // create device_local texture image
    auto image_info = VkImageCreateInfo{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.extent        = texture_extent;
    image_info.mipLevels     = 1;
    image_info.arrayLayers   = 1;
    image_info.format        = VK_FORMAT_R32_SFLOAT;
    image_info.tiling        = VK_IMAGE_TILING_LINEAR;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags         = 0;

    auto memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    auto tex_image = vulkan::make_image(device, get_device().get_physical_device(), image_info, memory_flags)
            .move_or_throw();

    // clear staging image
    {
        void* data = tex_staging_image.map_memory(device, 0, texture_bytes, 0).move_or_throw();
        std::fill(static_cast<std::uint8_t*>(data), static_cast<std::uint8_t*>(data) + texture_bytes, 0);
        tex_staging_image.unmap_memory(device);
    }

    // prepare for use: transition image layouts and transfer staging to device-local
    {
        // prepare for use: create memory barriers to transform layouts
        auto staging_barrier = VkImageMemoryBarrier{};
        staging_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        staging_barrier.oldLayout           = VK_IMAGE_LAYOUT_PREINITIALIZED;
        staging_barrier.newLayout           = VK_IMAGE_LAYOUT_GENERAL;
        staging_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        staging_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        staging_barrier.image               = tex_staging_image.get_handle();
        staging_barrier.srcAccessMask       = VK_ACCESS_HOST_WRITE_BIT;
        staging_barrier.dstAccessMask       = VK_ACCESS_HOST_WRITE_BIT;

        staging_barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        staging_barrier.subresourceRange.baseMipLevel   = 0;
        staging_barrier.subresourceRange.levelCount     = 1;
        staging_barrier.subresourceRange.baseArrayLayer = 0;
        staging_barrier.subresourceRange.layerCount     = 1;

        auto barrier = VkImageMemoryBarrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = tex_image.get_handle();
        barrier.srcAccessMask       = 0;
        barrier.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;

        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

        // prepare for use: place barrier
        auto alloc_info = VkCommandBufferAllocateInfo{};
        alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool        = command_pool_.get_handle();
        alloc_info.commandBufferCount = 1;

        auto cmdbufs = vulkan::make_command_buffers(device, alloc_info).move_or_throw();
        auto cmdbuf = cmdbufs[0];

        auto begin_info = VkCommandBufferBeginInfo{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vulkan::except(vkBeginCommandBuffer(cmdbuf, &begin_info));

        vkCmdPipelineBarrier(cmdbuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0, 0, nullptr, 0, nullptr, 1, &staging_barrier);

        vkCmdPipelineBarrier(cmdbuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        auto img_copy = VkImageCopy{};
        img_copy.srcOffset      = {0, 0, 0};
        img_copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        img_copy.dstOffset      = {0, 0, 0};
        img_copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        img_copy.extent         = texture_extent;
        vkCmdCopyImage(cmdbuf, tex_staging_image.get_handle(), VK_IMAGE_LAYOUT_GENERAL,
                tex_image.get_handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &img_copy);

        vulkan::except(vkEndCommandBuffer(cmdbuf));

        auto submit_info = VkSubmitInfo{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers    = &cmdbuf;

        vulkan::except(get_device().get_graphics_queue().submit(1, &submit_info, nullptr));
        vulkan::except(get_device().get_graphics_queue().wait_idle());
    }

    // create image view
    auto view_info = VkImageViewCreateInfo{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image    = tex_image.get_handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format   = VK_FORMAT_R32_SFLOAT;

    view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel   = 0;
    view_info.subresourceRange.levelCount     = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount     = 1;

    auto tex_view = vulkan::make_image_view(device, view_info).move_or_throw();

    // create sampler
    auto sampler_info = VkSamplerCreateInfo{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter               = VK_FILTER_LINEAR;
    sampler_info.minFilter               = VK_FILTER_LINEAR;
    sampler_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.anisotropyEnable        = true;
    sampler_info.maxAnisotropy           = 16;
    sampler_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = true;
    sampler_info.compareEnable           = false;
    sampler_info.compareOp               = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_info.mipLodBias              = 0.0f;
    sampler_info.minLod                  = 0.0f;
    sampler_info.maxLod                  = 0.0f;

    auto tex_sampler = vulkan::make_sampler(device, sampler_info).move_or_throw();

    // sampler uniform binding
    auto descriptor_image_info = VkDescriptorImageInfo{};
    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor_image_info.imageView   = tex_view.get_handle();
    descriptor_image_info.sampler     = tex_sampler.get_handle();

    auto descriptor_write = VkWriteDescriptorSet{};
    descriptor_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet          = descriptor_set_;
    descriptor_write.dstBinding      = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo      = &descriptor_image_info;

    vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);

    // set handles
    texture_staging_image_ = std::move(tex_staging_image);
    texture_image_         = std::move(tex_image);
    texture_view_          = std::move(tex_view);
    texture_sampler_       = std::move(tex_sampler);
}

void triangle_example::setup_uniform_buffer() {
    // create uniform buffers
    auto const logical  = get_device().get_handle();
    auto const physical = get_device().get_physical_device();
    auto const size     = sizeof(std::int32_t);

    auto const staging_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    auto const staging_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    auto const usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    auto const flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    auto staging_buffer = vulkan::make_exclusive_buffer(logical, physical, size, staging_usage, staging_flags)
            .move_or_throw();

    auto buffer = vulkan::make_exclusive_buffer(logical, physical, size, usage, flags)
            .move_or_throw();

    // set up descriptors
    auto desc_info = VkDescriptorBufferInfo{};
    desc_info.buffer = buffer.get_handle();
    desc_info.offset = 0;
    desc_info.range  = sizeof(std::int32_t);

    auto desc_write = VkWriteDescriptorSet{};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.dstSet          = descriptor_set_;
    desc_write.dstBinding      = 1;
    desc_write.dstArrayElement = 0;
    desc_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    desc_write.descriptorCount = 1;
    desc_write.pBufferInfo     = &desc_info;

    vkUpdateDescriptorSets(logical, 1, &desc_write, 0, nullptr);

    uniform_staging_buffer_ = std::move(staging_buffer);
    uniform_buffer_         = std::move(buffer);
}

void triangle_example::setup_transfer_cmdbuffer(std::int32_t offset, std::uint32_t len) {
    // create transfer command buffer
    auto alloc_info = VkCommandBufferAllocateInfo{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool_.get_handle();
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    auto command_buffer = vulkan::make_primary_command_buffer(get_device().get_handle(), command_pool_.get_handle())
            .move_or_throw();

    auto begin_info = VkCommandBufferBeginInfo{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    vulkan::except(vkBeginCommandBuffer(command_buffer.get_handle(), &begin_info));

    // transition image to transfer-src layout
    auto img_barrier_start = VkImageMemoryBarrier{};
    img_barrier_start.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier_start.oldLayout           = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier_start.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier_start.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier_start.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier_start.image               = texture_staging_image_.get_handle();
    img_barrier_start.srcAccessMask       = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier_start.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;

    img_barrier_start.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier_start.subresourceRange.baseMipLevel   = 0;
    img_barrier_start.subresourceRange.levelCount     = 1;
    img_barrier_start.subresourceRange.baseArrayLayer = 0;
    img_barrier_start.subresourceRange.layerCount     = 1;

    vkCmdPipelineBarrier(command_buffer.get_handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr, 0, nullptr, 1, &img_barrier_start);

    auto img_copy = VkImageCopy{};
    img_copy.srcOffset      = {0, offset, 0};
    img_copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    img_copy.dstOffset      = {0, offset, 0};
    img_copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    img_copy.extent         = {texture_extent.width, len, 1};
    vkCmdCopyImage(command_buffer.get_handle(), texture_staging_image_.get_handle(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture_image_.get_handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &img_copy);

    auto img_barrier_stop = VkImageMemoryBarrier{};
    img_barrier_stop.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier_stop.oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier_stop.newLayout           = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier_stop.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier_stop.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier_stop.image               = texture_staging_image_.get_handle();
    img_barrier_stop.srcAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
    img_barrier_stop.dstAccessMask       = VK_ACCESS_HOST_WRITE_BIT;

    img_barrier_stop.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier_stop.subresourceRange.baseMipLevel   = 0;
    img_barrier_stop.subresourceRange.levelCount     = 1;
    img_barrier_stop.subresourceRange.baseArrayLayer = 0;
    img_barrier_stop.subresourceRange.layerCount     = 1;

    vkCmdPipelineBarrier(command_buffer.get_handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
            0, nullptr, 0, nullptr, 1, &img_barrier_stop);

    auto buf_copy = VkBufferCopy{};
    buf_copy.srcOffset = 0;
    buf_copy.dstOffset = 0;
    buf_copy.size = sizeof(std::int32_t);
    vkCmdCopyBuffer(command_buffer.get_handle(), uniform_staging_buffer_.get_handle(), uniform_buffer_.get_handle(), 1, &buf_copy);

    vulkan::except(vkEndCommandBuffer(command_buffer.get_handle()));

    transfer_cmdbuffer_ = std::move(command_buffer);
}

void triangle_example::setup_command_buffers() {
    // create command buffers
    auto alloc_info = VkCommandBufferAllocateInfo{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool_.get_handle();
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>(framebuffers_.size());

    auto command_buffers = vulkan::make_command_buffers(get_device().get_handle(), alloc_info).move_or_throw();

    // fill command buffers
    for (std::size_t i = 0; i < command_buffers.size(); i++) {
        auto const buffer = command_buffers[i];

        auto begin_info = VkCommandBufferBeginInfo{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        begin_info.pInheritanceInfo = nullptr;

        vulkan::except(vkBeginCommandBuffer(buffer, &begin_info));

        // transform texture-layout to be accessed by shader-read
        auto img_barrier_start = VkImageMemoryBarrier{};
        img_barrier_start.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        img_barrier_start.oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        img_barrier_start.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        img_barrier_start.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier_start.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier_start.image               = texture_image_.get_handle();
        img_barrier_start.srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
        img_barrier_start.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;

        img_barrier_start.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        img_barrier_start.subresourceRange.baseMipLevel   = 0;
        img_barrier_start.subresourceRange.levelCount     = 1;
        img_barrier_start.subresourceRange.baseArrayLayer = 0;
        img_barrier_start.subresourceRange.layerCount     = 1;

        vkCmdPipelineBarrier(buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr, 0, nullptr, 1, &img_barrier_start);

        // main draw commands
        auto const clear_color = VkClearValue{{{0.0f, 0.0f, 0.0f, 1.0f}}};

        auto pass_info = VkRenderPassBeginInfo{};
        pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        pass_info.renderPass        = renderpass_.get_handle();
        pass_info.framebuffer       = framebuffers_[i].get_handle();
        pass_info.renderArea.offset = {0, 0};
        pass_info.renderArea.extent = get_swapchain().get_extent();
        pass_info.clearValueCount   = 1;
        pass_info.pClearValues      = &clear_color;

        vkCmdBeginRenderPass(buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get_handle());
        vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_.get_handle(), 0, 1,
                &descriptor_set_, 0, nullptr);

        screenquad_.cmd_draw(buffer);

        vkCmdEndRenderPass(buffer);

        // transform texture-layout to be accessed by host-write
        auto img_barrier_end = VkImageMemoryBarrier{};
        img_barrier_end.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        img_barrier_end.oldLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        img_barrier_end.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        img_barrier_end.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier_end.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barrier_end.image               = texture_image_.get_handle();
        img_barrier_end.srcAccessMask       = VK_ACCESS_SHADER_READ_BIT;
        img_barrier_end.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;

        img_barrier_end.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        img_barrier_end.subresourceRange.baseMipLevel   = 0;
        img_barrier_end.subresourceRange.levelCount     = 1;
        img_barrier_end.subresourceRange.baseArrayLayer = 0;
        img_barrier_end.subresourceRange.layerCount     = 1;

        vkCmdPipelineBarrier(buffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr, 0, nullptr, 1, &img_barrier_end);

        vulkan::except(vkEndCommandBuffer(buffer));
    }

    command_buffers_ = std::move(command_buffers);
}

void triangle_example::setup_semaphores() {
    sem_img_available_ = vulkan::make_semaphore(get_device().get_handle()).move_or_throw();
    sem_img_finished_  = vulkan::make_semaphore(get_device().get_handle()).move_or_throw();

    auto fence_info = VkFenceCreateInfo{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    staging_fence_     = vulkan::make_fence(get_device().get_handle(), fence_info).move_or_throw();
}

void triangle_example::cb_destroy() {
    sem_img_available_.destroy();
    sem_img_finished_.destroy();
    uniform_buffer_.destroy();

    command_buffers_.destroy();
    command_pool_.destroy();

    texture_sampler_.destroy();
    texture_view_.destroy();
    texture_image_.destroy();

    screenquad_.destroy();

    framebuffers_.clear();

    pipeline_.destroy();
    pipeline_layout_.destroy();

    descriptor_set_ = nullptr;
    descriptor_pool_.destroy();
    descriptor_layout_.destroy();

    renderpass_.destroy();

    vert_shader_module_.destroy();
    frag_shader_module_.destroy();
}

void triangle_example::cb_display() {
    using clock = std::chrono::high_resolution_clock;
    auto const start_frame = clock::now();

    // update texture-image
    if (!paused_) {
        auto const device = get_device().get_handle();
        auto const fence  = staging_fence_.get_handle();

        // synchronize for device-access to texture-image
        vulkan::except(vkWaitForFences(device, 1, &fence, true, std::numeric_limits<std::uint64_t>::max()));
        vulkan::except(vkResetFences(device, 1, &fence));

        // update texture image
        {
            auto const chunk_size   = texture_extent.width;
            auto const chunk_bytes  = chunk_size * 4;
            auto const offset_bytes = texture_offset_ * chunk_bytes;

            void* data = texture_staging_image_.map_memory(device, offset_bytes, chunk_bytes, 0).move_or_throw();

            auto rnd_gen  = std::mt19937{std::random_device{}()};
            auto rnd_dist = std::uniform_real_distribution<float>{0.f, 1.f};
            std::generate(static_cast<float*>(data), static_cast<float*>(data) + chunk_size, [&](){
                return rnd_dist(rnd_gen);
            });

            texture_staging_image_.unmap_memory(get_device().get_handle());
        }

        // update uniform buffer
        {
            void* data = uniform_staging_buffer_.map_memory(device, 0, sizeof(std::int32_t), 0).move_or_throw();
            *static_cast<std::int32_t*>(data) = texture_offset_ + 1;
            uniform_staging_buffer_.unmap_memory(device);
        }

        // transfer staging to device-local
        {
            setup_transfer_cmdbuffer(texture_offset_, 1);
            // TODO: extract image-transfer to oneshot command buffer with dynamic range image-copy cmd

            auto command_buffer = transfer_cmdbuffer_.get_handle();

            auto submit_info = VkSubmitInfo{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers    = &command_buffer;

            vulkan::except(get_device().get_graphics_queue().submit(1, &submit_info, fence));
        }

        texture_offset_++;
        if (texture_offset_ >= texture_extent.height)
            texture_offset_ -= texture_extent.height;
    }

    // render texture to screen
    auto const sem_img_available = sem_img_available_.get_handle();
    auto const sem_img_finished  = sem_img_finished_.get_handle();

    std::uint32_t image_index = 0;
    auto status = vkAcquireNextImageKHR(get_device().get_handle(), get_swapchain().get_swapchain(),
            std::numeric_limits<std::uint64_t>::max(), sem_img_available, nullptr, &image_index);

    if (status == VK_ERROR_OUT_OF_DATE_KHR || status == VK_SUBOPTIMAL_KHR)
        return;

    vulkan::except(status);

    VkPipelineStageFlags const wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    auto const command_buffer = command_buffers_[image_index];
    auto const swapchain      = get_swapchain().get_swapchain();

    auto submit_info = VkSubmitInfo{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = &sem_img_available;
    submit_info.pWaitDstStageMask    = wait_stages;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = &sem_img_finished;

    vulkan::except(get_device().get_graphics_queue().submit(1, &submit_info, nullptr));

    auto present_info = VkPresentInfoKHR{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &sem_img_finished;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swapchain;
    present_info.pImageIndices      = &image_index;
    present_info.pResults           = nullptr;

    vulkan::except(get_device().get_present_queue().present_khr(present_info));

    auto const delta_frame = clock::now() - start_frame;
    std::cout << "frame-time: " << std::chrono::duration_cast<std::chrono::microseconds>(delta_frame).count()
            << u8"µs\n";
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
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        paused_ = !paused_;
}

auto triangle_example::select_physical_device(std::vector<VkPhysicalDevice> const& devices) const -> VkPhysicalDevice {
    auto selected = devices[0];

    auto properties = VkPhysicalDeviceProperties{};
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
