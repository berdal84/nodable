
#pragma once
#include <iostream>
#include <vector>
#include <lodepng.h>
#include <string>
#include <gl3w/GL/glcorearb.h>
#include <gl3w/GL/gl3w.h>
#include <nodable/Log.h>

namespace Nodable
{
    /**
     * @brief Simple data structure to store OpenGL texture information
     */
    struct Texture
    {
        Texture(GLuint& _image, int& _width, int& _height): image(_image), width(_width), height(_height){}
        ~Texture() {}
        GLuint image;
        int width;
        int height;
    };

    /**
     * @brief Texture manager, could also be called texture resource manager.
     */
    class TextureManager
    {
    public:
        TextureManager() = default;
        ~TextureManager() = default;

        /**
         * Get a texture from file (first time) or from static map (next times).
         *
         * @param path
         * @return
         */
        Texture *get_or_create(const std::string& path)
        {
            // Return if already exists
            auto tex = m_register.find(path );
            if (tex != m_register.end() )
                return &tex->second;

            return create_texture_from_file_path(path);
        }

        void release_resources()
        {
            for( const auto& eachTxt : m_register )
            {
                glDeleteTextures(1, &eachTxt.second.image);
                LOG_MESSAGE("Texture", "Texture %s released.\n", eachTxt.first.c_str())
            }
            m_register.clear();
        }

    private:


        Texture* create_texture_from_file_path(const std::string& path)
        {
            // Try to load a PNG
            std::vector<unsigned char> image;

            GLuint texture = 0;
            int width = 0;
            int height = 0;

            auto isLoaded = load_png(path, image, &texture, &width, &height);
            if ( isLoaded )
            {
                LOG_MESSAGE("Texture", "Texture %s loaded.\n", path.c_str())

                // Store for later use
                auto res = m_register.insert({path, {texture, width, height }});

                return &res.first->second;
            }
            else
            {
                LOG_ERROR("Texture", "Texture %s NOT loaded !\n", path.c_str())
                return nullptr;
            }
        }

        /**
         * Loads a PNG image into a RGBA vector
         * @param filename
         * @param image
         * @return
         */
        int load_png(
                const std::string& filename,
                std::vector<unsigned char>& image,
                GLuint* out_texture,
                int* out_width,
                int* out_height)
        {
            std::cout << "Loading " << filename << "..." << std::endl;
            std::vector<unsigned char> buffer;
            lodepng::load_file(buffer, filename); //load the image file with given filename
            unsigned w, h;
            unsigned error = lodepng::decode(image, w, h, buffer); //decode the png

            //stop if there is an error
            if (error) {
                std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
                return 0;
            }

            std::cout << "decoder read an image: " << w << " by " << h << " px" << std::endl;

            // Create a OpenGL texture identifier
            GLuint image_texture;
            glGenTextures(1, &image_texture);
            glBindTexture(GL_TEXTURE_2D, image_texture);

            // Setup filtering parameters for display
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

            *out_texture = image_texture;
            *out_width = w;
            *out_height = h;

            return true;
        }

        std::map<std::string, Texture> m_register;
    };

}