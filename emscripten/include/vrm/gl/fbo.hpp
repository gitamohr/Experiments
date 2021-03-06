// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <vrm/gl/common.hpp>
#include <vrm/gl/check.hpp>

VRM_SDL_NAMESPACE
{
    namespace impl
    {
        class fbo
        {
        private:
            GLuint _id, _n;

            bool bound() const noexcept
            {
                GLint result;
                VRM_SDL_GLCHECK(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &result));
                return vrmc::to_num<GLuint>(result) == _id;
            }

        public:
            // TODO:

            fbo() = default;
            fbo(GLuint n) noexcept : _n{n} {}

            void generate() noexcept
            {
                VRM_CORE_ASSERT(_n == 1);
                VRM_SDL_GLCHECK(glGenFramebuffers(_n, &_id));
            }

            void deleteFBO() noexcept
            {
                VRM_SDL_GLCHECK(glDeleteFramebuffers(_n, &_id));
            }

            void bind() noexcept
            {
                VRM_SDL_GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
            }

            void unbind() noexcept
            {
                VRM_CORE_ASSERT(bound());
                VRM_SDL_GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
            }

            template <typename TF>
            void with(TF&& f) noexcept
            {
                bind();
                f();
                unbind();
            }
        };

        struct gl_fbo_deleter
        {
            template <typename T>
            void operator()(T& fbo) noexcept
            {
                fbo.deleteFBO();
            }
        };

        using unique_fbo = unique_resource<impl::fbo, gl_fbo_deleter>;
    }

    auto make_fbo(GLuint n = 1) noexcept
    {
        VRM_CORE_ASSERT(n > 0);

        impl::fbo f{n};
        f.generate();

        return impl::unique_fbo{f};
    }
}
VRM_SDL_NAMESPACE_END