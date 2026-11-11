#include "Image.h"

#include "File_System.h"
#include "lodepng.h" // for io operations
#include "Log.h"

namespace ndbl
{

Image& image_init(Image& image, u32_t width, u32_t height, Image_Type type)
{
    assert(type == Image_Type_RGBA_8Bits); // the only supported :)

    u32_t bytes_per_pixel = 4;
    image.data   = static_cast<unsigned char*>( bdc::memory_malloc( bytes_per_pixel * width * height ) );
    image.width  = width;
    image.height = height;
    image.type   = type;

    return image;
}

void image_release(Image& image)
{
    bdc::memory_free(image.data);
}

Image_Load_Result image_load(const Path& path)
{
    NDBL_LOG(Verbosity_Diagnostic, "Texture_Manager", "Loading PNG from disk " STRING_FMT " ...\n", STRING_FMT_ARGS( path.tstring() ) );
    Image image;
    u16_t error = lodepng_decode32_file(&image.data, &image.width, &image.height, path.c_str()); //decode the png
    if (error)
    {
        image_release(image);
        NDBL_LOG(Verbosity_Diagnostic, "Texture_Manager", "Error: %i %s\n", error, lodepng_error_text(error) );
        return { .ok = false, .error = string_tprintf("lodepng_decode32_file failed: %s\n", lodepng_error_text(error) )};
    }
    NDBL_LOG(Verbosity_Diagnostic, "Texture_Manager", "PNG read (image: %u x %u px)\n", image.width, image.height );
    return { .ok = true, .image = image };
}

Image_Write_Result image_write(Image& image, const Path& path)
{
    NDBL_LOG(Verbosity_Diagnostic, "Texture_Manager", "Writting PNG to disk " STRING_FMT " ...\n", STRING_FMT_ARGS( path.tstring() ) );
    u16_t error = lodepng_encode32_file(path.c_str(), image.data, image.width, image.height);
    if(error)
    {
        NDBL_LOG(Verbosity_Diagnostic, "Texture_Manager", "Error: %i %s\n", error, lodepng_error_text(error) );
        return { .ok = false, .error = string_tprintf("lodepng_encode32_file failed: %s\n", lodepng_error_text(error) )};
    }
    return { .ok = true };
}

void image_vflip(Image& image)
{
    assert(image.data != nullptr);
    assert(image.type == Image_Type_RGBA_8Bits); // we operates on u32_t and therefore expects RGBA 8 bits/channel
    
    auto data = reinterpret_cast<u32_t*>( image.data ); // allocates a temporary row to take advantage of memcpy() rather than copying data bytes by bytes.
    
    for (u32_t y = 0u; y < image.height / 2; ++y)
    {
        std::swap_ranges( // Note: swap_ranges uses register and therefore does not allocates a temporary row
            data + y * image.width, data + (y + 1) * image.width, // row y
            data + (image.height - y - 1) * image.width);         // row height-y-1
    }
}

}