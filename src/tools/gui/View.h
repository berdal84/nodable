#pragma once

#include "tools/core/reflection/reflection"
#include "geometry/Box.h"
#include "geometry/Rect.h"
#include "geometry/Space.h"
#include "ImGuiEx.h" // ImGui with extensions

namespace tools
{
    /**
     * View is an abstract class to provide a GUI for a specific Node.
     * View also implement a small static library to _draw_property_view custom ImGui widgets/graphics.
     */
	class View
	{
	public:
        bool  visible;
        bool  hovered;
        bool  selected;

		explicit View();
		virtual ~View() = default;
        virtual bool  draw();
        tools::Vec2   get_pos(Space = SCREEN_SPACE) const;
        void          set_pos(const Vec2&, Space = SCREEN_SPACE);
        Rect          get_rect(Space = SCREEN_SPACE) const;
        void          set_size(const Vec2&);
        Vec2          get_size() const;
        View*         get_parent() const;
        void          add_child(View* view);
        void          translate(const Vec2& delta);
        const Rect&   get_content_region(Space = SCREEN_SPACE) const;
    private:
        View*         m_parent;
        Rect          m_content_region; // Space available before to draw
        Box           m_screen_box; // stored in SCREEN_SPACE
        std::vector<View*> m_children;

        REFLECT_BASE_CLASS()
    };
}
