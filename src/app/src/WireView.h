#pragma once

#include "Nodable.h"
#include "View.h" // base class
#include <imgui/imgui.h>
#include <mirror.h>

namespace Nodable
{
    namespace WireView
    {
        void Draw( ImDrawList *draw_list, ImVec2 _from, ImVec2 _to );
    };
}