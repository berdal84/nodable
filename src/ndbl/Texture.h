#pragma once

#include "Image.h"
#include "GL_Helpers.h"
#include "bdc/String.hpp"

namespace ndbl
{
    struct Texture
    {
        bdc::String name;
        Image       image;
        GLuint      gl_handler;
    };

    inline void texture_init(Texture& texture, const bdc::String& name, Image image = {})
    {
        texture.name  = string_copy(name);
        texture.image = image;
    }

    inline void texture_release(Texture& texture)
    {
        string_release(texture.name);
        image_release(texture.image);
    }
}
