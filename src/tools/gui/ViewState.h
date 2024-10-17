#pragma once

#include "geometry/Box.h"
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

        bool                    begin_draw(); // Call this before to draw your own view
        Rect                    get_content_region(Space = PARENT_SPACE ) const;
        XForm2D*                xform() { return &box.xform; };

        Box                     box; // in PARENT_SPACE
        ViewState*              _parent;
        Rect                    _content_region; // Space available before to draw (in PARENT_SPACE)
        std::vector<ViewState*> _children;

    };
}
