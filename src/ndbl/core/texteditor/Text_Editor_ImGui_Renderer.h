#pragma once
#include "Text_Editor.h"
#include <imgui.h>

namespace ndbl
{

    void text_editor_render(
        Text_Editor& editor, const char* title, const ImVec2& size = ImVec2(),
        bool border = false, bool ignoreImGuiChild = false // TODO: use flags
    );
        
} // namespace ndbl
