#pragma once

#include "core/reflection/reflection"
#include "core/Rect.h"

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
        virtual bool  draw() = 0;
	protected:
        static void use_available_region(View* , const Rect& = {});  // update m_xxx_space_content_region with available space or given rectangle
        Rect m_screen_space_content_region;  // view rectangle in screen space coordinates
        Rect m_local_space_content_region;  // view rectangle in window space coordinates
		REFLECT_BASE_CLASS()
    };
}
