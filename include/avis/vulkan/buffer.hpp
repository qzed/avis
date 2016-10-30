#pragma once

#include <avis/vulkan/handle.hpp>


namespace avis {
namespace vulkan {

class buffer {
public:
    buffer()
            : buffer_{}
            , memory_{} {}

    buffer(handle<VkBuffer>&& buffer, handle<VkDeviceMemory>&& memory)
            : buffer_{std::forward<handle<VkBuffer>>(buffer)}
            , memory_{std::forward<handle<VkDeviceMemory>>(memory)} {}

    buffer(buffer const& other) = delete;
    buffer(buffer&& other)      = default;

    inline void destroy() noexcept;

    auto operator= (buffer const& other) -> buffer& = delete;
    auto operator= (buffer&& other)      -> buffer& = default;

    inline auto get_handle()        const noexcept -> VkBuffer;
    inline auto get_memory_handle() const noexcept -> VkDeviceMemory;

    inline auto map_memory(VkDevice device, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags)
            const noexcept -> expected<void*>;
    inline void unmap_memory(VkDevice device) const noexcept;

private:
    handle<VkBuffer>       buffer_;
    handle<VkDeviceMemory> memory_;
};

inline auto make_exclusive_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size,
        VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags, VkAllocationCallbacks const* alloc = nullptr)
        noexcept -> expected<buffer>
{
    // create buffer
    auto buffer_info = VkBufferCreateInfo{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = size;
    buffer_info.usage       = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto buffer_handle = VkBuffer{};
    AVIS_VULKAN_EXCEPT_RETURN(vkCreateBuffer(device, &buffer_info, alloc, &buffer_handle));

    auto buffer = make_handle(buffer_handle, alloc, [=](auto... p){
        vkDestroyBuffer(device, p...);
    });
    AVIS_VULKAN_EXCEPT_RETURN(buffer.status());

    // allocate buffer memory
    auto mem_requirements = VkMemoryRequirements{};
    vkGetBufferMemoryRequirements(device, buffer_handle, &mem_requirements);

    auto mem_properties = VkPhysicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

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

    auto memory_handle = VkDeviceMemory{};
    AVIS_VULKAN_EXCEPT_RETURN(vkAllocateMemory(device, &alloc_info, alloc, &memory_handle));

    auto memory = make_handle(memory_handle, alloc, [=](auto... p){
        vkFreeMemory(device, p...);
    });
    AVIS_VULKAN_EXCEPT_RETURN(memory.status());

    // bind memory
    AVIS_VULKAN_EXCEPT_RETURN(vkBindBufferMemory(device, buffer_handle, memory_handle, 0));

    return {{ buffer.move(), memory.move() }};
}


void buffer::destroy() noexcept {
    buffer_.destroy();
    memory_.destroy();
}

auto buffer::get_handle() const noexcept -> VkBuffer {
    return buffer_.get_handle();
}

auto buffer::get_memory_handle() const noexcept -> VkDeviceMemory {
    return memory_.get_handle();
}

auto buffer::map_memory(VkDevice device, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags) const noexcept -> expected<void*> {
    void* data = nullptr;
    AVIS_VULKAN_EXCEPT_RETURN(vkMapMemory(device, memory_.get_handle(), offset, size, flags, &data));
    return data;
}

void buffer::unmap_memory(VkDevice device) const noexcept {
    vkUnmapMemory(device, memory_.get_handle());
}

} /* namespace vulkan */
} /* namespace avis */
