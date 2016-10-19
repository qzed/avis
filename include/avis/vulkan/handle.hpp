#pragma once

// TODO: remove

#include <avis/vulkan/vulkan.hpp>
#include <avis/vulkan/expected.hpp>
#include <algorithm>
#include <functional>


namespace avis {
namespace vulkan {

template <class T>
class handle {
public:
    using handle_type = T;
    using destroy_fn = std::function<void(handle_type, VkAllocationCallbacks const*)>;

    handle()
        : handle_{nullptr}
        , alloc_{nullptr}
        , destructor_{} {}

    handle(handle_type handle, VkAllocationCallbacks const* alloc, destroy_fn&& destructor)
        : handle_{handle}
        , alloc_{alloc}
        , destructor_{destructor} {}

    handle(handle&& other)
        : handle_{std::exchange(other.handle_, nullptr)}
        , alloc_{std::exchange(other.alloc_, nullptr)}
        , destructor_{std::move(other.destructor_)} {}

    handle(handle const& other) = delete;

    ~handle() { destroy(); }

    inline void destroy() noexcept;
    inline void destroy(VkAllocationCallbacks const* alloc) noexcept;

    inline auto operator= (handle const& rhs) noexcept -> handle& = delete;
    inline auto operator= (handle&& rhs) noexcept -> handle&;

    inline auto get_handle() const noexcept -> handle_type;
    inline auto release()    noexcept -> handle_type;

    inline auto allocator() noexcept -> VkAllocationCallbacks const* &;
    inline auto allocator() const noexcept -> VkAllocationCallbacks const* const&;

private:
    handle_type                  handle_;
    VkAllocationCallbacks const* alloc_;
    destroy_fn                   destructor_;
};


template <class T, class DestructorFn>
auto make_handle(T hndl, VkAllocationCallbacks const* alloc, DestructorFn&& destr) noexcept -> expected<handle<T>>
try {
    return handle<T>{hndl, alloc, std::forward<DestructorFn>(destr)};
} catch (...) {
    return result::error_out_of_host_memory;
}


template <class T>
void handle<T>::destroy() noexcept {
    destroy(alloc_);
}

template <class T>
void handle<T>::destroy(VkAllocationCallbacks const* alloc) noexcept {
    if (!handle_) return;

    destructor_(handle_, alloc);

    destructor_ = {};
    handle_ = nullptr;
    alloc_ = nullptr;
}

template <class T>
auto handle<T>::operator= (handle&& rhs) noexcept -> handle& {
    destroy();
    handle_     = std::exchange(rhs.handle_, nullptr);
    alloc_      = std::exchange(rhs.alloc_, nullptr);
    destructor_ = std::move(rhs.destructor_);
    return *this;
}

template <class T>
auto handle<T>::get_handle() const noexcept -> handle_type {
    return handle_;
}

template <class T>
auto handle<T>::release() noexcept -> handle_type  {
    alloc_      = nullptr;
    destructor_ = {};
    return std::exchange(handle_, nullptr);
}

template <class T>
auto handle<T>::allocator() noexcept -> VkAllocationCallbacks const* & {
    return alloc_;
}

template <class T>
auto handle<T>::allocator() const noexcept -> VkAllocationCallbacks const* const& {
    return alloc_;
}

} /* namespace vulkan */
} /* namespace avis */
