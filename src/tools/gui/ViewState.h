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

        BoxShape2D&          shape() { return _shape; }
        const BoxShape2D&    shape() const { return _shape; }
        SpatialNode2D&       spatial_node() { return _shape.spatial_node(); };
        const SpatialNode2D& spatial_node() const { return _shape.spatial_node(); };

    private:
        BoxShape2D _shape; // in PARENT_SPACE
    };
}
