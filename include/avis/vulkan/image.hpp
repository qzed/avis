#pragma once

#include <avis/vulkan/handle.hpp>


namespace avis {
namespace vulkan {

class image {
public:
    image()
            : image_{}
            , memory_{} {}

    image(handle<VkImage>&& image, handle<VkDeviceMemory>&& memory)
            : image_{std::forward<handle<VkImage>>(image)}
            , memory_{std::forward<handle<VkDeviceMemory>>(memory)} {}

    image(image const& other) = delete;
    image(image&& other)      = default;

    inline void destroy() noexcept;

    auto operator= (image const& other) -> image& = delete;
    auto operator= (image&& other)      -> image& = default;

    inline auto get_handle()        const noexcept -> VkImage;
    inline auto get_memory_handle() const noexcept -> VkDeviceMemory;

    inline auto map_memory(VkDevice device, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags)
            const noexcept -> expected<void*>;
    inline void unmap_memory(VkDevice device) const noexcept;

private:
    handle<VkImage>        image_;
    handle<VkDeviceMemory> memory_;
};

inline auto make_image(VkDevice device, VkPhysicalDevice physical_device, VkImageCreateInfo const& image_info,
        VkMemoryPropertyFlags memory_flags, VkAllocationCallbacks const* alloc = nullptr) noexcept
        -> expected<image>
{
    // create image
    auto image_handle = VkImage{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateImage(device, &image_info, alloc, &image_handle));

    auto image = vulkan::make_handle(image_handle, nullptr, [=](auto... p) {
        vkDestroyImage(device, p...);
    });
    AVIS_VULKAN_EXCEPT_RETURN(image.status());

    // create texture memory
    auto mem_properties = VkPhysicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    auto mem_requirements = VkMemoryRequirements{};
    vkGetImageMemoryRequirements(device, image_handle, &mem_requirements);

    std::uint32_t mem_index;
    {
    bool mem_index_found = false;
        for (int i = 0; i < mem_properties.memoryTypeCount && !mem_index_found; i++) {
            if ((mem_requirements.memoryTypeBits & (1u << i))
                        && ((mem_properties.memoryTypes[i].propertyFlags & memory_flags) == memory_flags)) {
                mem_index = i;
                mem_index_found = true;
            }
        }

        if (!mem_index_found)
            return result::error_feature_not_present;
    }

    auto alloc_info = VkMemoryAllocateInfo{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize  = mem_requirements.size;
    alloc_info.memoryTypeIndex = mem_index;

    auto memory_handle = VkDeviceMemory{nullptr};
    AVIS_VULKAN_EXCEPT_RETURN(vkAllocateMemory(device, &alloc_info, nullptr, &memory_handle));

    auto memory = vulkan::make_handle(memory_handle, nullptr, [=](auto... p){
        vkFreeMemory(device, p...);
    });
    AVIS_VULKAN_EXCEPT_RETURN(memory.status());

    // bind memory
    AVIS_VULKAN_EXCEPT_RETURN(vkBindImageMemory(device, image_handle, memory_handle, 0));

    return {{ image.move(), memory.move() }};
}


void image::destroy() noexcept {
    image_.destroy();
    memory_.destroy();
}

auto image::get_handle() const noexcept -> VkImage {
    return image_.get_handle();
}

auto image::get_memory_handle() const noexcept -> VkDeviceMemory {
    return memory_.get_handle();
}

auto image::map_memory(VkDevice device, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags) const noexcept -> expected<void*> {
    void* data = nullptr;
    AVIS_VULKAN_EXCEPT_RETURN(vkMapMemory(device, memory_.get_handle(), offset, size, flags, &data));
    return data;
}

void image::unmap_memory(VkDevice device) const noexcept {
    vkUnmapMemory(device, memory_.get_handle());
}

} /* namespace vulkan */
} /* namespace avis */
