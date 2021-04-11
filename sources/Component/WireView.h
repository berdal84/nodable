#pragma once

#include "Nodable.h"
#include "View.h" // base class
#include <imgui/imgui.h>
#include <mirror.h>

namespace Nodable
{
    // forward
    class MemberView;
    class NodeView;

    namespace WireView
    {
        void Draw(
                ImDrawList *draw_list,
                ImVec2 _from, ImVec2 _to,
                const NodeView *_fromNode, const NodeView *_toNode);

        void DrawVerticalWire(
                ImDrawList *draw_list,
                ImVec2 pos0,
                ImVec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        void DrawHorizontalWire(
                ImDrawList *draw_list,
                ImVec2 pos0,
                ImVec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);
    };
}