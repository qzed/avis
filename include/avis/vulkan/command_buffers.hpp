#pragma once

#include <avis/vulkan/handle.hpp>
#include <avis/vulkan/expected.hpp>
#include <vector>
#include <algorithm>


namespace avis {
namespace vulkan {

using command_buffer = handle<VkCommandBuffer>;

class command_buffers {
public:
    command_buffers()
            : device_{nullptr}
            , pool_{nullptr}
            , handles_{} {}

    command_buffers(VkDevice device, VkCommandPool pool, std::vector<VkCommandBuffer>&& handles)
            : device_{device}
            , pool_{pool}
            , handles_{std::forward<std::vector<VkCommandBuffer>>(handles)} {}

    command_buffers(command_buffers&& other)
            : device_{std::exchange(other.device_, nullptr)}
            , pool_{std::exchange(other.pool_, nullptr)}
            , handles_{std::move(other.handles_)} {}

    command_buffers(command_buffers const& other) = delete;

    ~command_buffers() { destroy(); }

    inline auto operator= (command_buffers const& rhs) -> command_buffers& = delete;
    inline auto operator= (command_buffers&& rhs) noexcept -> command_buffers&;

    inline void destroy() noexcept;

    inline auto empty()   const noexcept;
    inline auto size()    const noexcept;

    inline auto operator[] (std::uint32_t index) const noexcept -> VkCommandBuffer;
    inline auto at(std::uint32_t index) const -> VkCommandBuffer;

    inline auto cbegin()  const noexcept;
    inline auto cend()    const noexcept;

    inline auto crbegin() const noexcept;
    inline auto crend()   const noexcept;

private:
    VkDevice                     device_;
    VkCommandPool                pool_;
    std::vector<VkCommandBuffer> handles_;
};


inline auto make_command_buffers(VkDevice device, VkCommandBufferAllocateInfo const& alloc_info)
        noexcept -> expected<command_buffers>
try {
    auto handles = std::vector<VkCommandBuffer>(alloc_info.commandBufferCount, nullptr);
    AVIS_VULKAN_EXCEPT_RETURN(vkAllocateCommandBuffers(device, &alloc_info, handles.data()));

    return {{device, alloc_info.commandPool, std::move(handles)}};
} catch (...) {
    return result::error_out_of_host_memory;
}

inline auto make_primary_command_buffer(VkDevice device, VkCommandPool pool) noexcept -> expected<command_buffer> {
    auto alloc_info = VkCommandBufferAllocateInfo{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = pool;
    alloc_info.commandBufferCount = 1;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    auto handle = VkCommandBuffer{};
    AVIS_VULKAN_EXCEPT_RETURN(vkAllocateCommandBuffers(device, &alloc_info, &handle));

    return make_handle(handle, nullptr, [=](auto h, auto a){
        vkFreeCommandBuffers(device, pool, 1, &h);
    });
}


void command_buffers::destroy() noexcept {
    if (device_ == nullptr) return;

    vkFreeCommandBuffers(device_, pool_, handles_.size(), handles_.data());

    handles_.clear();
    pool_   = nullptr;
    device_ = nullptr;
}

auto command_buffers::operator= (command_buffers&& rhs) noexcept -> command_buffers& {
    destroy();

    device_  = std::exchange(rhs.device_, nullptr);
    pool_    = std::exchange(rhs.pool_, nullptr);
    handles_ = std::move(rhs.handles_);

    return *this;
}

auto command_buffers::empty() const noexcept {
    return handles_.empty();
}

auto command_buffers::size() const noexcept {
    return handles_.size();
}

auto command_buffers::operator[] (std::uint32_t index) const noexcept -> VkCommandBuffer {
    return handles_[index];
}

auto command_buffers::at(std::uint32_t index) const -> VkCommandBuffer {
    return handles_.at(index);
}

auto command_buffers::cbegin() const noexcept {
    return handles_.cbegin();
}

auto command_buffers::cend() const noexcept {
    return handles_.cend();
}

auto command_buffers::crbegin() const noexcept {
    return handles_.crbegin();
}

auto command_buffers::crend() const noexcept {
    return handles_.crend();
}

} /* namespace vulkan */
} /* namespace avis */
