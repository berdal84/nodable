#pragma once

#include <map>
#include <string>
#include <utility>

namespace fw
{
    // forward declarations
    struct Texture;

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