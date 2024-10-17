#pragma once

#include "tools/core/reflection/reflection"
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
	class ViewState
	{
	public:
        bool  visible;
        bool  hovered;
        bool  selected;

		explicit ViewState();
		~ViewState() = default;
        bool          begin_draw(); // Call this before to draw your own view
        tools::Vec2   get_pos(Space = SCREEN_SPACE) const;
        void          set_pos(const Vec2&, Space = SCREEN_SPACE);
        Rect          get_rect(Space = SCREEN_SPACE) const;
        void          set_size(const Vec2&);
        Vec2          get_size() const;
        ViewState*    get_parent() const;
        void          add_child(ViewState* view);
        void          translate(const Vec2& delta);
        Rect          get_content_region(Space = SCREEN_SPACE) const;

    private:
        ViewState*    m_parent;
        Rect          m_content_region; // Space available before to draw (in PARENT_SPACE)
        Box           m_box; // in PARENT_SPACE
        std::vector<ViewState*> m_children;
        Vec2          m_window_pos; // in SCREEN_SPACE
        REFLECT_BASE_CLASS()
    };
}
