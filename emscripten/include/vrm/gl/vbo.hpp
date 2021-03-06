// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <vrm/gl/common.hpp>
#include <vrm/gl/check.hpp>

VRM_SDL_NAMESPACE
{
    enum class buffer_target : GLenum
    {
        array = GL_ARRAY_BUFFER,
        element_array = GL_ELEMENT_ARRAY_BUFFER
    };

    enum class buffer_usage : GLenum
    {
        stream_draw = GL_STREAM_DRAW,
        static_draw = GL_STATIC_DRAW,
        dynamic_draw = GL_DYNAMIC_DRAW
    };



    namespace impl
    {
        template <buffer_target TTarget>
        class vbo
        {
        private:
            static constexpr GLenum target_value{(GLenum)(TTarget)};

            // TODO: BUG: GCC: CLANG: gcc segfault
            // static constexpr GLenum
            // target_value{vrm::core::from_enum(TTarget)};

            GLuint _id, _n;

            bool bound() const noexcept
            {
                GLint result;

                if(TTarget == buffer_target::array)
                {
                    VRM_SDL_GLCHECK(
                        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &result));
                }

                if(TTarget == buffer_target::element_array)
                {
                    VRM_SDL_GLCHECK(glGetIntegerv(
                        GL_ELEMENT_ARRAY_BUFFER_BINDING, &result));
                }

                return vrmc::to_num<GLuint>(result) == _id;
            }

        public:
            vbo() = default;
            vbo(GLuint n) noexcept : _n{n} {}

            void generate() noexcept
            {
                VRM_CORE_ASSERT(_n == 1);
                VRM_SDL_GLCHECK(glGenBuffers(_n, &_id));
            }

            void deleteVBO() noexcept
            {
                VRM_SDL_GLCHECK(glDeleteBuffers(_n, &_id));
            }

            void bind() noexcept
            {
                VRM_SDL_GLCHECK(glBindBuffer(target_value, _id));
            }

            void unbind() noexcept
            {
                VRM_CORE_ASSERT(bound());
                VRM_SDL_GLCHECK(glBindBuffer(target_value, 0));
            }



            void sub_buffer_data(const void* data_ptr, GLsizeiptr byte_count,
                GLintptr vbo_byte_offset = 0) noexcept
            {
                VRM_CORE_ASSERT(byte_count >= 0);
                VRM_CORE_ASSERT(vbo_byte_offset >= 0);
                VRM_CORE_ASSERT(data_ptr != nullptr);
                VRM_CORE_ASSERT(bound());

                VRM_SDL_GLCHECK(glBufferSubData(
                    target_value, vbo_byte_offset, byte_count, data_ptr));
            }

            void sub_buffer_data_bytes(const void* data_ptr, sz_t byte_count,
                sz_t vbo_byte_offset = 0) noexcept
            {
                sub_buffer_data(data_ptr, byte_count, vbo_byte_offset);
            }

            template <typename T>
            void sub_buffer_data_items(const T* data_ptr, sz_t item_count,
                sz_t vbo_byte_offset = 0) noexcept
            {
                sub_buffer_data_bytes(vrmc::to_void_ptr(data_ptr),
                    item_count * sizeof(T), vbo_byte_offset);
            }

            template <typename T>
            void sub_buffer_data_items(const std::vector<T>& vec,
                sz_t item_count_offset, sz_t item_count,
                sz_t vbo_byte_offset = 0) noexcept
            {
                VRM_CORE_ASSERT(vec.size() - item_count_offset >= item_count);

                sub_buffer_data_items<T>(vec.data() + item_count_offset,
                    item_count, vbo_byte_offset);
            }

            template <buffer_usage TUsage>
            void buffer_data(
                const void* data_ptr, GLsizeiptr byte_count) noexcept
            {
                VRM_CORE_ASSERT(bound());

                VRM_SDL_GLCHECK(glBufferData(target_value, byte_count, data_ptr,
                    vrmc::from_enum(TUsage)));
            }

            template <buffer_usage TUsage>
            void buffer_data_bytes(
                const void* data_ptr, sz_t byte_count) noexcept
            {
                buffer_data<TUsage>(data_ptr, byte_count);
            }

            template <buffer_usage TUsage, typename T>
            void buffer_data_items(const T* data_ptr, sz_t item_count) noexcept
            {
                buffer_data_bytes<TUsage>(
                    vrmc::to_void_ptr(data_ptr), sizeof(T) * item_count);
            }

            template <buffer_usage TUsage, typename T, sz_t TN>
            void buffer_data_items(T(&arr)[TN]) noexcept
            {
                buffer_data_items<TUsage, T>(&arr[0], TN);
            }

            template <buffer_usage TUsage, typename T>
            void buffer_data_items(const std::vector<T>& vec, sz_t item_count,
                sz_t item_count_offset = 0) noexcept
            {
                VRM_CORE_ASSERT(vec.size() - item_count_offset >= item_count);

                buffer_data_items<TUsage, T>(
                    vec.data() + item_count_offset, item_count);
            }

            template <buffer_usage TUsage, typename T>
            void buffer_data_items(const std::vector<T>& vec) noexcept
            {
                buffer_data_items<TUsage, T>(vec, vec.size());
            }

            template <buffer_usage TUsage>
            void allocate_buffer_bytes(sz_t byte_count) noexcept
            {
                buffer_data<TUsage>(nullptr, byte_count);
            }

            template <buffer_usage TUsage, typename T>
            void allocate_buffer_items(sz_t item_count) noexcept
            {
                allocate_buffer_bytes<TUsage>(sizeof(T) * item_count);
            }

            template <typename TF>
            void with(TF&& f) noexcept
            {
                bind();
                f();
                unbind();
            }
        };

        struct gl_vbo_deleter
        {
            template <typename T>
            void operator()(T& vbo) noexcept
            {
                // std::cout << "vbo deleted\n";
                vbo.deleteVBO();
            }
        };

        template <buffer_target TTarget>
        using unique_vbo = unique_resource<impl::vbo<TTarget>, gl_vbo_deleter>;
    }

    template <buffer_target TTarget>
    auto make_vbo(GLuint n = 1) noexcept
    {
        VRM_CORE_ASSERT(n > 0);

        impl::vbo<TTarget> v{n};
        v.generate();

        return impl::unique_vbo<TTarget>{v};
    }
}
VRM_SDL_NAMESPACE_END
