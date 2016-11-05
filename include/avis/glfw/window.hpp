#pragma once

#include <avis/vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <algorithm>
#include <functional>


namespace avis {
namespace glfw {

class window {
public:
    using fn_key_input_callback = std::function<void(int, int, int, int)>;
    using fn_resize_callback    = std::function<void(unsigned int, unsigned int)>;

    window()
            : window_{nullptr}
            , title_{""}
            , width_{800}
            , height_{600}
            , resizable_{true} {}

    window(std::string title, unsigned int widht, unsigned int height, bool resizable = true)
            : window_{nullptr}
            , title_{std::move(title)}
            , width_{widht}
            , height_{height}
            , resizable_{resizable} {}

    window(window&& other)
            : window_{std::exchange(other.window_, nullptr)}
            , title_{std::move(other.title_)}
            , width_{other.width_}
            , height_{other.height_}
            , resizable_{other.resizable_} { glfwSetWindowUserPointer(window_, this); }

    window(window&) = delete;

    virtual ~window() { destroy(); }

    inline auto operator= (window const&) -> window& = delete;
    inline auto operator= (window&& other) -> window&;

    void create();
    void destroy();

    inline void set_key_input_callback(fn_key_input_callback key_input_callback);
    inline void set_resize_callback(fn_resize_callback resize_callback);

    inline auto get_width()  const noexcept -> unsigned int;
    inline auto get_height() const noexcept -> unsigned int;
    inline auto get_title()  const noexcept -> std::string;
    inline auto get_handle() const noexcept -> GLFWwindow*;

    inline auto get_terminate_request() const noexcept -> bool;
    inline void set_terminate_request(bool b) noexcept;

    inline void poll_events() noexcept;

private:
    GLFWwindow*  window_;
    std::string  title_;
    unsigned int width_;
    unsigned int height_;
    bool         resizable_;

    fn_key_input_callback cb_input_key_;
    fn_resize_callback    cb_resize_;
};


auto window::operator= (window&& other) -> window& {
    destroy();

    window_    = std::exchange(other.window_, nullptr);
    title_     = std::move(other.title_);
    width_     = other.width_;
    height_    = other.height_;
    resizable_ = other.resizable_;

    glfwSetWindowUserPointer(window_, this);

    return *this;
}


void window::set_key_input_callback(fn_key_input_callback key_input_callback) {
    cb_input_key_ = std::move(key_input_callback);
}

void window::set_resize_callback(fn_resize_callback resize_callback) {
    cb_resize_ = std::move(resize_callback);
}

auto window::get_width() const noexcept -> unsigned int {
    return width_;
}

auto window::get_height() const noexcept -> unsigned int {
    return height_;
}

auto window::get_title() const noexcept -> std::string {
    return title_;
}

auto window::get_handle() const noexcept -> GLFWwindow* {
    return window_;
}


auto window::get_terminate_request() const noexcept -> bool {
    return glfwWindowShouldClose(window_);
}

void window::set_terminate_request(bool b) noexcept {
    glfwSetWindowShouldClose(window_, b);
}

void window::poll_events() noexcept {
    glfwPollEvents();
}

} /* namespace glfw */
} /* namespace avis */
