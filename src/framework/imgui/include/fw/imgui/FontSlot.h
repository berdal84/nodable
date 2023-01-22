#pragma once

typedef int FontSlot;      // to allow bitwise operations, we use this type in all signatures
enum FontSlot_             // Enum to identify each font slots
{
    FontSlot_Paragraph,
    FontSlot_Heading,
    FontSlot_Code,
    FontSlot_ToolBtn,
    FontSlot_COUNT
};