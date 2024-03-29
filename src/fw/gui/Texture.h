#pragma once

#include "core/types.h"
#include "gl3w/GL/gl3w.h"

namespace fw
{
    struct Texture  //  Simple data structure to store OpenGL texture information
    {
        Texture(): Texture({}, 0, 0)
        {}

        Texture(std::vector<unsigned char> _buffer, int _width, int _height, GLuint _image = 0)
            : buffer(std::move(_buffer))
            , gl_handler(_image)
            , width(_width)
            , height(_height)
        {}

        GLuint gl_handler;                 // OpenGL handler to the texture in the GPU
        std::vector<unsigned char> buffer; // Image buffer loaded into memory
        u32_t width;                       // in pixels
        u32_t height;                      // in pixels
    };
}
