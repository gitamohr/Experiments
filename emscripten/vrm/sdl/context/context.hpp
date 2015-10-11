// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <chrono>
#include <vrm/sdl/math.hpp>
#include <vrm/sdl/common.hpp>
#include <vrm/sdl/utils.hpp>
#include <vrm/sdl/resource.hpp>
#include <vrm/sdl/elements.hpp>
#include <vrm/sdl/context/unique_sdl_resources.hpp>

namespace vrm
{
    namespace sdl
    {
        namespace impl
        {
            class context
            {
            private:
                const std::size_t _width;
                const std::size_t _height;

                impl::unique_window _window;
                impl::unique_glcontext _glcontext;
                // impl::unique_renderer _renderer;
                // impl::unique_texture _texture;

                SDL_Event _event;

                impl::key_event_handler _on_key_down{
                    impl::null_key_event_handler()};
                impl::key_event_handler _on_key_up{
                    impl::null_key_event_handler()};

                impl::btn_event_handler _on_btn_down{
                    impl::null_btn_event_handler()};
                impl::btn_event_handler _on_btn_up{
                    impl::null_btn_event_handler()};

                impl::update_fn _update_fn{impl::null_update_fn()};
                impl::draw_fn _draw_fn{impl::null_draw_fn()};

                hr_duration _update_duration;
                hr_duration _draw_duration;

                impl::input_state _input_state;

                auto& on_key_up() noexcept;
                auto& on_key_down() noexcept;

                auto& on_btn_up() noexcept;
                auto& on_btn_down() noexcept;

            public:
                auto& update_fn() noexcept;
                auto& draw_fn() noexcept;

            private:
                void run_events();
                void run_update();
                void run_draw();

            public:
                context(const std::string& title, std::size_t width,
                    std::size_t height);

                context(const context&) = delete;
                context& operator=(const context&) = delete;

                context(context&&) = default;
                context& operator=(context&&) = default;

                void run();

                const auto& update_duration() const noexcept;
                const auto& draw_duration() const noexcept;
                auto total_duration() const noexcept;

                auto update_ms() const noexcept;
                auto draw_ms() const noexcept;
                auto total_ms() const noexcept;
                auto fps() const noexcept;

                auto mouse_x() const noexcept;
                auto mouse_y() const noexcept;
                auto mouse_pos() const noexcept;

                auto key(kkey k) const noexcept;
                auto btn(mbtn b) const noexcept;

                template <typename... Ts>
                auto make_surface(Ts&&... xs) noexcept;

                template <typename... Ts>
                auto make_texture(Ts&&... xs) noexcept;

                // auto make_sprite() noexcept;
                // auto make_sprite(texture& t) noexcept;

                // auto make_ttffont(const std::string& path, sz_t font_size);

                // auto make_ttftext_texture(
                //    ttffont& f, const std::string& s, SDL_Color color);

                // void draw(texture& t, const vec2f& pos) noexcept;
                // void draw(sprite& s) noexcept;

                void title(const std::string& s) noexcept;
            };
        }
    }
}