#include "Texture_Manager.h"
#include <lodepng.h>
#include <utility>

#include "bdc/Allocators.hpp"
#include "Texture.h"
#include "Log.h"
#include "Asserts.h"
#include "File_System.h"

namespace ndbl
{

static Texture_Manager* g_texture_manager{ nullptr };

Texture*    _texture_load_png_from_disk_to_gpu(const Path&);
int         _texture_load_from_memory_to_gpu(Texture*);

Texture_Manager* texture_manager_init()
{
    NDBL_ASSERT( g_texture_manager == nullptr, "g_texture_manager is already initialized, did you forgot to call texture_manager_shutdown() ?");
    g_texture_manager = bdc::memory_new<Texture_Manager>();
    return g_texture_manager;
}

Texture_Manager* texture_manager()
{
    NDBL_ASSERT( g_texture_manager != nullptr, "g_texture_manager is not initialized, did you forgot to call texture_manager_init() ?");
    return g_texture_manager;
}

void texture_manager_shutdown()
{
    NDBL_ASSERT( g_texture_manager != nullptr, "g_texture_manager is not initialized, you might have called this twice or didn't call texture_manager_ini() at all.");
    texture_manager_release_all();
    bdc::memory_delete( g_texture_manager );
    g_texture_manager = nullptr;
}

Texture* texture_manager_get_or_load(const Path& path)
{
    Texture_Manager* state = texture_manager();

    // Return if already exists..
    Result found = hashmap_find( state->texture_by_absolute_path, path.tstring() );
    if ( found )
    {
        return found.data;
    }
    // ... or load
    return _texture_load_png_from_disk_to_gpu(path);
}

bool texture_manager_release_all()
{
    Texture_Manager* state = texture_manager();

    bool success = true;
    FOR_EACH_HASHMAP_VALUE( texture, state->texture_by_absolute_path )
    {
        if( texture.gl_handler ) // is zero when texture is not loaded to GPU
        {
            glDeleteTextures(1, &texture.gl_handler);
            if( GL_NO_ERROR != glGetError())
            {
                success = false;
                NDBL_LOG(Verbosity_Warning, "Texture_Manager", "Unable to release: " STRING_FMT " (code: %i)\n", STRING_FMT_ARGS(texture.name), glGetError());
            }
            else
            {
                NDBL_LOG(Verbosity_Diagnostic, "Texture_Manager", "Released " STRING_FMT "\n", STRING_FMT_ARGS(texture.name));
            }
        }
        texture_release(texture);
    }
    hashmap_release(state->texture_by_absolute_path);
    return success;
}

Texture* _texture_load_png_from_disk_to_gpu(const Path &path)
{
    Texture_Manager* state = texture_manager();   
    
    NDBL_LOG(Verbosity_Diagnostic, "Texture_Manager", "Loading file from disk to GPU: %s\n", path.c_str());

    // 1. Load png file to Texture (RAM only)
    Image_Load_Result loaded = image_load(path);
    if ( !loaded.ok )
    {
        NDBL_LOG(Verbosity_Error, "Texture_Manager", "Unable to load png, error: %s\n", loaded.error.c_str());
        NDBL_ASSERT(false, "Unable to load png");
    }

    // 2. Load texture to GPU
    Texture texture;    
    texture_init(texture, path.tstring(), loaded.image);

    u16_t error = _texture_load_from_memory_to_gpu(&texture);
    if ( error )
    {
        texture_release(texture);
        NDBL_LOG(Verbosity_Error, "Texture_Manager", "Unable to load texture to GPU (code %u): %s\n",  error, path.c_str());
        return nullptr;
    }

    Result result = hashmap_add(state->texture_by_absolute_path, texture.name, texture);
    NDBL_LOG(Verbosity_Diagnostic, "Texture_Manager", "-- File loaded to GPU\n");

    return &result.data->value;
}

int _texture_load_from_memory_to_gpu(Texture* texture)
{
    // Create a OpenGL texture identifier
    glGenTextures(1, &texture->gl_handler);
    glBindTexture(GL_TEXTURE_2D, texture->gl_handler);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Load image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->image.width, texture->image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->image.data);

    return glGetError();
}

} // namespace ndbl