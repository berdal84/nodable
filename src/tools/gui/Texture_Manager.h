#pragma once

#include <map>
#include <string>
#include "tools/core/File_System.h"

namespace tools
{
    // forward declarations
    struct Texture;

    struct Texture_Manager
    {
        std::map<std::string, Texture*> texture_by_absolute_path;
    };

    Texture_Manager* texture_manager_init();
    void             texture_manager_shutdown();
    Texture_Manager* texture_manager();
    Texture*         texture_manager_load(const Path &path); // Get texture from absolute path (resource will be loaded from disk the first call)
    bool             texture_manager_release_all();          // Release all the loaded textures (There is no check if they are still in use)
}