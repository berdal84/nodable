#pragma once

#include <mirror.h>
#include <imgui/imgui.h>

#include <nodable/Nodable.h>
#include <nodable/View.h> // base class

namespace Nodable
{
    namespace WireView
    {
        void Draw( ImDrawList *draw_list, ImVec2 _from, ImVec2 _to );
    };
}