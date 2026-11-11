#pragma once

#include "bdc/String.hpp"
#include "bdc/Hash_Map.hpp"

namespace ndbl
{
    // forward declarations
    struct Texture;
    struct Path;

    struct Texture_Manager
    {
        bdc::Hash_Map<bdc::String, Texture> texture_by_absolute_path;
    };

    Texture_Manager* texture_manager_init();
    void             texture_manager_shutdown();
    Texture_Manager* texture_manager();
    Texture*         texture_manager_get_or_load(const Path &path); // Get texture from absolute path (resource will be loaded from disk the first call)
    bool             texture_manager_release_all();          // Release all the loaded textures (There is no check if they are still in use)
}