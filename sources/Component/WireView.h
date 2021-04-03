#pragma once

#include "Nodable.h"
#include "View.h" // base class
#include <imgui/imgui.h>
#include <mirror.h>

namespace Nodable{
	class WireView : public View
	{
	public:
		bool update()override {return true;}
		bool draw()override;
		MIRROR_CLASS(WireView)(
			MIRROR_PARENT(View));

        static void DrawVerticalWire(
                ImDrawList *draw_list,
                ImVec2 pos0,
                ImVec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        static void DrawHorizontalWire(
                ImDrawList *draw_list,
                ImVec2 pos0,
                ImVec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);
    };
}