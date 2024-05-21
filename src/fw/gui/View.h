#pragma once

#include "fw/core/geometry/Box2D.h"
#include "fw/core/geometry/Rect.h"
#include "fw/core/reflection/reflection"

#include "ImGuiEx.h" // ImGui with extensions

namespace fw
{
    /**
     * View is an abstract class to provide a GUI for a specific Node.
     * View also implement a small static library to _draw_property_view custom ImGui widgets/graphics.
     */
	class View
	{
	public:
        bool is_visible;
        bool is_hovered;

		View();
		virtual ~View() = default;
        bool          draw();
        virtual bool  onDraw() = 0;
        fw::Vec2      position(Space) const; // Get position in a given Space
        void          position(Vec2 _delta, Space ); // Set position in a given Space
        Rect          rect(Space) const; // Get rectangle in a given Space
        void          translate(Vec2 _delta);
	protected:
        Box2D parent_content_region;
        Box2D box; // Screen space Box2D
		REFLECT_BASE_CLASS()
    };
}
