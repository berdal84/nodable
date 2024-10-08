#pragma once

#include <map>
#include <string>
#include <utility>
#include "tools/core/FileSystem.h"

namespace tools
{
    // forward declarations
    struct Texture;

    class TextureManager
    {
    public:
        TextureManager() = default;
        TextureManager(const TextureManager&) = delete;
        Texture*   load(const Path &path); // Get texture from absolute path (resource will be loaded from disk the first call)
        bool       release_all();          // Release all the loaded textures (There is no check if they are still in use)
    private:
        Texture*   load_png_to_gpu(const Path&);     // Create a texture (loaded to GPU) from a png file path
        static int load_png(const Path&, Texture*);  // Load a PNG file to Texture (RAM only)
        static int load_to_gpu(Texture*);                   // Load a Texture to GPU
        std::map<std::string, Texture*> m_register;         // Texture storage (absolute path => Texture*)
    };

    [[nodiscard]]
    TextureManager* init_texture_manager(); // note: pointer must be stored since it is required to shut it down
    TextureManager* get_texture_manager();
    void            shutdown_texture_manager(TextureManager*);
}