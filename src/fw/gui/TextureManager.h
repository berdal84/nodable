
#pragma once

#include "gl3w/GL/gl3w.h"
#include "gl3w/GL/glcorearb.h"
#include "lodepng/lodepng.h"
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "core/log.h"
#include "core/types.h"

namespace fw
{
    struct Texture  //  Simple data structure to store OpenGL texture information
    {
        Texture();
        Texture(std::vector<unsigned char> _buffer, int _width, int _height, GLuint _image = 0 );

        GLuint gl_handler;                 // OpenGL handler to the texture in the GPU
        std::vector<unsigned char> buffer; // Image buffer loaded into memory
        u32_t width;                       // in pixels
        u32_t height;                      // in pixels
    };

    class TextureManager
    {
    public:
        TextureManager() {}
        TextureManager(const TextureManager&) = delete;
        Texture*   load(const std::string &path);           // Get texture from absolute path (resource will be loaded from disk the first call)
        bool       release_all();                           // Release all the loaded textures (There is no check if they are still in use)
    private:
        Texture*   load_png_to_gpu(const std::string&);     // Create a texture (loaded to GPU) from a png file path
        static int load_png(const std::string&, Texture*);  // Load a PNG file to Texture (RAM only)
        static int load_to_gpu(Texture*);                   // Load a Texture to GPU
        std::map<std::string, Texture*> m_register;         // Texture storage (absolute path => Texture*)
    };

}