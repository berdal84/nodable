#pragma once
#include <fw/reflection/enum.h>

namespace fw {
    enum FontSlot {
        FontSlot_Paragraph,
        FontSlot_Heading,
        FontSlot_Code,
        FontSlot_ToolBtn,
        FontSlot_COUNT
    };

    R_ENUM(FontSlot)
        R_ENUM_VALUE(FontSlot_Paragraph)
        R_ENUM_VALUE(FontSlot_Heading)
        R_ENUM_VALUE(FontSlot_Code)
        R_ENUM_VALUE(FontSlot_ToolBtn)
    R_ENUM_END
}

