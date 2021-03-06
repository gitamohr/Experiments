// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <vrm/gl/common.hpp>

VRM_SDL_NAMESPACE
{
    enum class index_type : GLenum
    {
        ui_byte = GL_UNSIGNED_BYTE,
        ui_short = GL_UNSIGNED_SHORT,
        ui_int = GL_UNSIGNED_INT
    };
}
VRM_SDL_NAMESPACE_END