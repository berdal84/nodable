
#pragma once
#include <iostream>
#include <vector>
#include <lodepng.h>
#include <filesystem>
#include <gl3w/GL/glcorearb.h>
#include <gl3w/GL/gl3w.h>
#include <Core/Log.h>

namespace Nodable
{
    class Texture
    {
    private:
        static std::map<std::filesystem::path, Texture*> s_textures;

    public:

        Texture(
                GLuint& _image,
                int& _width,
                int& _height)
        :
            width(_width),
            height(_height),
            image(_image)
        {}

        ~Texture()
        {
            glDeleteTextures(1, &image);
        }

        int width;
        int height;
        GLuint image;

        /**
         * Get a texture from file (first time) or from static map (next times).
         *
         * @param path
         * @return
         */
        static Texture *GetWithPath(std::filesystem::path& path)
        {
            // Return if already exists
            auto tex = Texture::s_textures.find( path.string() );
            if ( tex != s_textures.end() )
                return tex->second;

            return CreateTextureFromFile(path);
        }

    private:

        static Texture* CreateTextureFromFile(std::filesystem::path& path)
        {
            // Try to load a PNG
            std::vector<unsigned char> image;

            GLuint texture = 0;
            int width = 0;
            int height = 0;

            auto isLoaded = Texture::LoadPNG(path, image, &texture, &width, &height);
            if ( isLoaded )
            {
                LOG_MESSAGE(Log::Verbosity::Default, "Texture %s loaded.\n", path.c_str());
                auto newTexture = new Texture(texture, width, height);

                // Store for later use
                s_textures.insert( { path, newTexture });

                return newTexture;
            }
            else
            {
                LOG_ERROR(Log::Verbosity::Default, "Texture %s NOT loaded !\n", path.c_str());
                return nullptr;
            }
        }

        /**
         * Loads a PNG image into a RGBA vector
         * @param filename
         * @param image
         * @return
         */
        static int LoadPNG(
                std::filesystem::path& filename,
                std::vector<unsigned char>& image,
                GLuint* out_texture,
                int* out_width,
                int* out_height)
        {
            std::cout << "Loading " << filename << "..." << std::endl;
            std::vector<unsigned char> buffer;
            lodepng::load_file(buffer, filename.string()); //load the image file with given filename
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
    };

    std::map<std::filesystem::path, Texture*> Texture::s_textures;
}