// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <chrono>
#include <thread>
#include <vrm/sdl/math.hpp>
#include <vrm/sdl/common.hpp>
#include <vrm/sdl/resource.hpp>
#include <vrm/sdl/elements.hpp>
#include <vrm/sdl/context/context.hpp>

VRM_SDL_NAMESPACE
{
    namespace impl
    {
        template <typename TSettings>
        auto& interpolated_engine<TSettings>::update_fn() noexcept
        {
            return _update_fn;
        }

        template <typename TSettings>
        auto& interpolated_engine<TSettings>::draw_fn() noexcept
        {
            return _draw_fn;
        }

        template <typename TSettings>
        auto& interpolated_engine<TSettings>::interpolate_fn() noexcept
        {
            return _interpolate_fn;
        }

        template <typename TSettings>
        void interpolated_engine<TSettings>::run_update(ft step)
        {
            _prev_state = _current_state;
            this->update_fn()(_current_state, step);
        }

        template <typename TSettings>
        void interpolated_engine<TSettings>::run_draw(float interp_t)
        {
            this->_interpolate_fn(
                _interpolated_state, _prev_state, _current_state, interp_t);

            draw_fn()(_interpolated_state);
        }



        template <typename TSettings>
        auto& VRM_CORE_CONST_FN // .
            non_interpolated_engine<TSettings>::update_fn() noexcept
        {
            return _update_fn;
        }

        template <typename TSettings>
        auto& VRM_CORE_CONST_FN // .
            non_interpolated_engine<TSettings>::draw_fn() noexcept
        {
            return _draw_fn;
        }

        template <typename TSettings>
        auto& VRM_CORE_CONST_FN // .
            non_interpolated_engine<TSettings>::interpolate_fn() noexcept
        {
            return _interpolate_fn;
        }

        template <typename TSettings>
        void non_interpolated_engine<TSettings>::run_update(ft step)
        {
            this->update_fn()(_current_state, step);
        }

        template <typename TSettings>
        void non_interpolated_engine<TSettings>::run_draw(float)
        {
            draw_fn()(_current_state);
        }



        template <typename TSettings>
        auto& VRM_CORE_CONST_FN context<TSettings>::on_key_up() noexcept
        {
            return _on_key_up;
        }

        template <typename TSettings>
        auto& VRM_CORE_CONST_FN context<TSettings>::on_key_down() noexcept
        {
            return _on_key_down;
        }

        template <typename TSettings>
        auto& VRM_CORE_CONST_FN context<TSettings>::on_btn_up() noexcept
        {
            return _on_btn_up;
        }

        template <typename TSettings>
        auto& VRM_CORE_CONST_FN context<TSettings>::on_btn_down() noexcept
        {
            return _on_btn_down;
        }



        template <typename TSettings>
        context<TSettings>::context(
            timer_type& timer, engine_type& engine, class window& window)
            : _timer{timer}, _engine{engine}, _window{window}
        {
            if(TTF_Init() != 0)
            {
                log_sdl_error("ttf_init");
                std::terminate();
            }

            on_key_down() = [this](auto k)
            {
                _input_state.key(k, true);
            };

            on_key_up() = [this](auto k)
            {
                _input_state.key(k, false);
            };

            on_btn_down() = [this](auto b)
            {
                _input_state.btn(b, true);
            };

            on_btn_up() = [this](auto b)
            {
                _input_state.btn(b, false);
            };

            bind_events();
        }

        template <typename TSettings>
        context<TSettings>::~context()
        {
            unbind_events();
        }

        template <typename TSettings>
        const auto& VRM_CORE_CONST_FN // .
            context<TSettings>::update_duration() const noexcept
        {
            return _update_duration;
        }

        template <typename TSettings>
        const auto& VRM_CORE_CONST_FN // .
            context<TSettings>::draw_duration() const noexcept
        {
            return _draw_duration;
        }

        template <typename TSettings>
        const auto& VRM_CORE_CONST_FN // .
            context<TSettings>::total_duration() const noexcept
        {
            return _total_duration;
        }

        template <typename TSettings>
        const auto& VRM_CORE_CONST_FN // .
            context<TSettings>::real_duration() const noexcept
        {
            return _real_duration;
        }


        template <typename TSettings>
        template <typename T>
        auto context<TSettings>::ms_from_duration(const T& duration) const
            noexcept
        {
            return std::chrono::duration_cast<ms_double_duration>(duration)
                .count();
        }

        template <typename TSettings>
        auto VRM_CORE_PURE_FN context<TSettings>::update_ms() const noexcept
        {
            return ms_from_duration(update_duration());
        }

        template <typename TSettings>
        auto VRM_CORE_PURE_FN context<TSettings>::draw_ms() const noexcept
        {
            return ms_from_duration(draw_duration());
        }

        template <typename TSettings>
        auto VRM_CORE_PURE_FN context<TSettings>::total_ms() const noexcept
        {
            return ms_from_duration(total_duration());
        }

        template <typename TSettings>
        auto VRM_CORE_PURE_FN context<TSettings>::real_ms() const noexcept
        {
            return ms_from_duration(real_duration());
        }

        template <typename TSettings>
        auto& context<TSettings>::timer() noexcept
        {
            return _timer;
        }

        template <typename TSettings>
        const auto& context<TSettings>::timer() const noexcept
        {
            return _timer;
        }

        template <typename TSettings>
        void context<TSettings>::run()
        {
            auto time_dur([](auto&& f)
                {
                    auto ms_start(hr_clock::now());
                    f();

                    return hr_clock::now() - ms_start;
                });

            _real_duration = time_dur([&, this]
                {
                    _total_duration = time_dur([&, this]
                        {
                            _update_duration = time_dur([&, this]
                                {
                                    timer().run(real_ms(), [&, this](auto step)
                                        {
                                            _engine.run_update(step);
                                        });
                                });

                            _draw_duration = time_dur([this]
                                {
                                    // clear(vec4f{0.f, 0.f, 0.f, 1.f});
                                    _engine.run_draw(_timer.interp_t());

                                    window().display();
                                });
                        });

                    limit_fps_if_necessary();
                });
        }

        template <typename TSettings>
        void context<TSettings>::clear(const vec4f& color) noexcept
        {
            window().clear(color);
        }


        template <typename TSettings>
        void context<TSettings>::limit_fps_if_necessary() const noexcept
        {
            if(total_ms() >= ms_limit()) return;

            auto delay_ms(ms_limit() - total_ms());
            SDL_Delay(std::round(delay_ms));
        }

        template <typename TSettings>
        auto VRM_CORE_PURE_FN context<TSettings>::fps() const noexcept
        {
            // constexpr float seconds_ft_ratio{60.f};
            // return seconds_ft_ratio / total_ms();
            // return total_duration().count();

            return vrmc::to_int(1000.f / real_ms());
        }

        template <typename TSettings>
        auto context<TSettings>::mouse_x() const noexcept
        {
            return _input_state.mouse_x();
        }

        template <typename TSettings>
        auto context<TSettings>::mouse_y() const noexcept
        {
            return _input_state.mouse_y();
        }

        template <typename TSettings>
        auto context<TSettings>::mouse_pos() const noexcept
        {
            return _input_state.mouse_pos();
        }

        template <typename TSettings>
        auto VRM_CORE_PURE_FN context<TSettings>::key(kkey k) const noexcept
        {
            return _input_state.key(k);
        }

        template <typename TSettings>
        auto context<TSettings>::btn(mbtn b) const noexcept
        {
            return _input_state.btn(b);
        }

        template <typename TSettings>
        template <typename... Ts>
        auto context<TSettings>::make_surface(Ts&&... xs) noexcept
        {
            return unique_surface(FWD(xs)...);
        }

        template <typename TSettings>
        void context<TSettings>::title(const std::string& s) noexcept
        {
            window().title(s);
        }
    }
}
VRM_SDL_NAMESPACE_END
