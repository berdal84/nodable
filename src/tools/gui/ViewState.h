#pragma once

#include "geometry/BoxShape2D.h"
#include "geometry/Rect.h"
#include "geometry/Space.h"
#include "ImGuiEx.h" // ImGui with extensions

namespace tools
{
    /**
     * This class is not supposed to be used as-is to draw a view, it has to be wrapped.
     * See examples in NodeView or SlotView
     */
	struct ViewState
	{
        bool  visible;
        bool  hovered;
        bool  selected;

		ViewState();
		ViewState(float width, float height);

        SpatialNode2D* xform() { return &box.xform; };
        BoxShape2D      box; // in PARENT_SPACE
    };
}
