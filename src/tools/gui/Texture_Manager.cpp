#include "Texture_Manager.h"
#include "tools/gui/GL_Helpers.h"
#include <lodepng.h>

#include "Texture.h"
#include "tools/core/Log.h"
#include "tools/core/Asserts.h"

using namespace tools;

static Texture_Manager* g_texture_manager{ nullptr };

Texture_Manager* tools::init_texture_manager()
{
    ASSERT(g_texture_manager == nullptr);
    g_texture_manager = new Texture_Manager();
    return g_texture_manager;
}

Texture_Manager* tools::get_texture_manager()
{
    return g_texture_manager;
}

void tools::shutdown_texture_manager(Texture_Manager* texture_manager)
{
    ASSERT(g_texture_manager == texture_manager);
    ASSERT(g_texture_manager != nullptr);
    g_texture_manager->release_all();
    delete g_texture_manager;
    g_texture_manager = nullptr;
}

Texture* Texture_Manager::load(const Path& path)
{
    // Return if already exists
    auto tex = m_register.find(path.string());
    if (tex != m_register.end() )
        return tex->second;

    return load_png_to_gpu(path);
}

bool Texture_Manager::release_all()
{
    bool success = true;
    for( const auto& [key, texture] : m_register )
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
    m_register.clear();
    return success;
}
Texture *Texture_Manager::load_png_to_gpu(const Path &path)
{
    auto* texture = new Texture();

    // 1. Load png file to Texture (RAM only)
    int error = load_png(path, texture);
    if ( error )
    {
        delete texture;
        TOOLS_LOG(tools::Verbosity_Error, "Texture_Manager", "Unable to load png (code %u): %s\n",  error, path.c_str());
        VERIFY(false, "Unable to load png");
    }

    // 2. Load texture to GPU
    error = load_to_gpu(texture);
    if ( error )
    {
        delete texture;
        TOOLS_LOG(tools::Verbosity_Error, "Texture_Manager", "Unable to load texture to GPU (code %u): %s\n",  error, path.c_str());
        return nullptr;
    }

    m_register.emplace(path.string(), texture);
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Texture_Manager", "File loaded to GPU: %s\n", path.c_str());

    return texture;
}

int Texture_Manager::load_png(const Path& path, Texture* texture)
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

int Texture_Manager::load_to_gpu(Texture* texture)
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
