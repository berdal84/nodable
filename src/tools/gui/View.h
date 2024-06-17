#pragma once

#include "tools/core/geometry/Box2D.h"
#include "tools/core/geometry/Rect.h"
#include "tools/core/reflection/reflection"

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
        tools::Vec2   get_pos(Space) const;
        void          set_pos(const Vec2&, Space);
        Rect          get_rect(Space) const;
        void          set_size(const Vec2&);
        Vec2          get_size() const;
        View*         get_parent() const;
        void          add_child(View* view);
        void          translate(const Vec2& delta);
        const Rect&   get_content_region(Space) const;
    private:
        View* m_parent;
        Rect  m_content_region; // Space available before to draw
        Box2D m_screen_box; // stored in SCREEN_SPACE
        std::vector<View*> m_children;

        REFLECT_BASE_CLASS()
    };
}
