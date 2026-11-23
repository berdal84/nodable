#pragma once

#include <cassert>
#include "geometry/Vec2.h"
#include "bdc/Array.hpp"

namespace ndbl
{
    //forward declarations
    struct Path;
    
    // Note: this is more like a placeholder, since I only load PNG RGBA 8 bits/channel right now.
    //       But, I use it to mark this constrain in asserts where I need. Any future change would break on purpose.
    enum Image_Type
    {
        Image_Type_NULL       = 0,
        Image_Type_RGBA_8Bits = 1
    };

    struct Image // non-resizable
    {
        unsigned char*    data;
        u32_t             width;
        u32_t             height;
        Image_Type        type;
    };

    Image&  image_init(Image&, u32_t width, u32_t height, Image_Type type = Image_Type_RGBA_8Bits);
    void    image_release(Image&);

    struct Image_Load_Result
    {
        bool        ok;
        Image       image;
        bdc::String error;
    };

    struct Image_Write_Result
    {
        bool        ok;
        bdc::String error;
    };

    Image_Load_Result  image_load(const Path&);
    Image_Write_Result image_write(Image&, const Path&);
    void               image_vflip(Image&);
}