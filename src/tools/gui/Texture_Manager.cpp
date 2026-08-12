#include "Texture_Manager.h"
// #include "tools/gui/GL_Helpers.h"
#include <lodepng.h>
#include <utility>

#include "Texture.h"
#include "tools/core/Log.h"
#include "tools/core/Asserts.h"

#define VERIFY_TEXTURE_MANAGER_IS_INITIALIZED() VERIFY( g_texture_manager != nullptr, "g_texture_manager is not initialized, did you cann texture_manager_init() ?")

// private
namespace tools
{
    static Texture_Manager* g_texture_manager{ nullptr };

    Texture*    _texture_manager_load_png_to_gpu(const Path&);      // Create a texture (loaded to GPU) from a png file path
    int         _texture_manager_load_png(const Path&, Texture*);   // Load a PNG file to Texture (RAM only)
    int         _texture_manager_load_to_gpu(Texture*);             // Load a Texture to GPU
}

tools::Texture_Manager* tools::texture_manager_init()
{
    ASSERT(g_texture_manager == nullptr);
    g_texture_manager = new Texture_Manager();
    return g_texture_manager;
}

tools::Texture_Manager* tools::texture_manager()
{
    VERIFY_TEXTURE_MANAGER_IS_INITIALIZED();
    return g_texture_manager;
}

void tools::texture_manager_shutdown()
{
    VERIFY_TEXTURE_MANAGER_IS_INITIALIZED()
    ASSERT(g_texture_manager != nullptr);
    texture_manager_release_all();
    delete g_texture_manager;
    g_texture_manager = nullptr;
}

tools::Texture* tools::texture_manager_load(const Path& path)
{
    VERIFY_TEXTURE_MANAGER_IS_INITIALIZED();

    // Return if already exists
    auto tex = g_texture_manager->texture_by_absolute_path.find(path.string());
    if (tex != g_texture_manager->texture_by_absolute_path.end() )
        return tex->second;

    return _texture_manager_load_png_to_gpu(path);
}

bool tools::texture_manager_release_all()
{
    VERIFY_TEXTURE_MANAGER_IS_INITIALIZED();

    bool success = true;
    for( const auto& [key, texture] : g_texture_manager->texture_by_absolute_path )
    {
        if( texture->gl_handler ) // is zero when texture is not loaded to GPU
        {
            glDeleteTextures(1, &texture->gl_handler);
            if( GL_NO_ERROR != glGetError())
            {
                success = false;
                TOOLS_LOG(tools::Verbosity_Warning, "Texture_Manager", "Unable to release: %s (code: %i)\n", key.c_str(), glGetError());
            }
            else
            {
                TOOLS_LOG(tools::Verbosity_Diagnostic, "Texture_Manager", "Released %s\n", key.c_str());
            }
        }
        delete texture;
    }
    g_texture_manager->texture_by_absolute_path.clear();
    return success;
}

tools::Texture* tools::_texture_manager_load_png_to_gpu(const Path &path)
{
    VERIFY_TEXTURE_MANAGER_IS_INITIALIZED();
    
    auto* texture = new Texture();

    // 1. Load png file to Texture (RAM only)
    int error = _texture_manager_load_png(path, texture);
    if ( error )
    {
        delete texture;
        TOOLS_LOG(tools::Verbosity_Error, "Texture_Manager", "Unable to load png (code %u): %s\n",  error, path.c_str());
        VERIFY(false, "Unable to load png");
    }

    // 2. Load texture to GPU
    error = _texture_manager_load_to_gpu(texture);
    if ( error )
    {
        delete texture;
        TOOLS_LOG(tools::Verbosity_Error, "Texture_Manager", "Unable to load texture to GPU (code %u): %s\n",  error, path.c_str());
        return nullptr;
    }

    g_texture_manager->texture_by_absolute_path.emplace(path.string(), texture);
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Texture_Manager", "File loaded to GPU: %s\n", path.c_str());

    return texture;
}

int tools::_texture_manager_load_png(const Path& path, Texture* texture)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Texture_Manager", "Loading PNG from disk %s ...\n", path.c_str());
    std::vector<unsigned char> buffer;
    unsigned error = lodepng::load_file(buffer, path.string() ); //load the image file with given filename
    if (error) {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "Texture_Manager", "Error: %i %s\n", error, lodepng_error_text(error) );
        return 1;
    }
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Texture_Manager", "Decoding PNG %s ...\n", path.c_str());
    error = lodepng::decode(texture->buffer, (unsigned&)texture->width, (unsigned&)texture->height, buffer); //decode the png
    if (error) {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "Texture_Manager", "Error: %i %s\n", error, lodepng_error_text(error) );
        return 2;
    }
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Texture_Manager", "PNG read (image: %i x %i px)\n", texture->width, texture->height );
    return 0;
}

int tools::_texture_manager_load_to_gpu(Texture* texture)
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)texture->width, (int)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->buffer.data());

    return glGetError();
}
