#pragma once

#include <avis/vulkan/handle.hpp>
#include <avis/vulkan/result.hpp>
#include <avis/vulkan/expected.hpp>

#include <vector>


namespace avis {
namespace vulkan {

class device {
public:
    struct queue {
        std::uint32_t index;
        VkQueue       handle;
    };


public:
    static auto create(VkSurfaceKHR surface, VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures const& features,
            std::vector<char const*> const& extensions, std::vector<char const*> const& layers,
            VkAllocationCallbacks const* alloc = nullptr) noexcept -> expected<device>;

    device()
            : physical_device_{nullptr}
            , logical_device_{}
            , graphics_queue_{0, nullptr}
            , present_queue_{0, nullptr} {}

    device(VkPhysicalDevice physical_device, handle<VkDevice>&& logical_device, queue const& graphics_queue,
           queue const& present_queue)
            : physical_device_{physical_device}
            , logical_device_{std::forward<handle<VkDevice>>(logical_device)}
            , graphics_queue_{graphics_queue}
            , present_queue_{present_queue} {}

    device(device&& other)
            : physical_device_{std::exchange(other.physical_device_, nullptr)}
            , logical_device_{std::move(other.logical_device_)}
            , graphics_queue_{std::exchange(other.graphics_queue_, {0, nullptr})}
            , present_queue_{std::exchange(other.present_queue_, {0, nullptr})} {}

    device(device const& other) = delete;

    inline auto operator= (device const& rhs) noexcept -> device& = delete;
    inline auto operator= (device&& rhs) noexcept -> device&;

    inline void destroy() noexcept;
    inline void destroy(VkAllocationCallbacks const* alloc) noexcept;

    inline auto get_handle() const noexcept -> VkDevice;
    inline auto get_physical_device() const noexcept -> VkPhysicalDevice;

    inline auto get_graphics_queue() const noexcept -> VkQueue;
    inline auto get_graphics_queue_index() const noexcept -> std::uint32_t;

    inline auto get_present_queue() const noexcept -> VkQueue;
    inline auto get_present_queue_index() const noexcept -> std::uint32_t;

    inline auto allocator() noexcept -> VkAllocationCallbacks const* &;
    inline auto allocator() const noexcept -> VkAllocationCallbacks const* const&;

    // device functions
    inline auto wait_until_idle() const noexcept -> result;

    VkPhysicalDevice physical_device_;
    handle<VkDevice> logical_device_;
    queue            graphics_queue_;
    queue            present_queue_;
};


auto device::operator= (device&& rhs) noexcept -> device& {
    physical_device_ = std::exchange(rhs.physical_device_, nullptr);
    logical_device_  = std::move(rhs.logical_device_);
    graphics_queue_  = std::exchange(rhs.graphics_queue_, {0, nullptr});
    present_queue_   = std::exchange(rhs.present_queue_, {0, nullptr});
    return *this;
}

void device::destroy() noexcept {
    logical_device_.destroy();
    physical_device_ = nullptr;
    graphics_queue_ = {0, nullptr};
    present_queue_ = {0, nullptr};
}

void device::destroy(VkAllocationCallbacks const* alloc) noexcept {
    logical_device_.destroy(alloc);
    physical_device_ = nullptr;
    graphics_queue_ = {0, nullptr};
    present_queue_ = {0, nullptr};
}

auto device::get_handle() const noexcept -> VkDevice {
    return logical_device_.get_handle();
}

auto device::get_physical_device() const noexcept -> VkPhysicalDevice {
    return physical_device_;
}

auto device::get_graphics_queue() const noexcept -> VkQueue {
    return graphics_queue_.handle;
}

auto device::get_graphics_queue_index() const noexcept -> std::uint32_t {
    return graphics_queue_.index;
}

auto device::get_present_queue() const noexcept -> VkQueue {
    return present_queue_.handle;
}

auto device::get_present_queue_index() const noexcept -> std::uint32_t {
    return graphics_queue_.index;
}

auto device::allocator() noexcept -> VkAllocationCallbacks const* & {
    return logical_device_.allocator();
}

auto device::allocator() const noexcept -> VkAllocationCallbacks const* const& {
    return logical_device_.allocator();
}

auto device::wait_until_idle() const noexcept -> result {
    return to_result(vkDeviceWaitIdle(logical_device_.get_handle()));
}

} /* namespace vulkan */
} /* namespace avis */
