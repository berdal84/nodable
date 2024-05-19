#pragma once

#include "../core/reflection/reflection"
#include "ImGuiEx.h"
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
		View();
		virtual ~View() = default;
        virtual bool  draw() = 0;
		void          set_visible(bool _visibility);                 // show/hide view
        bool          is_visible()const;                             // check if visible
		bool          is_hovered()const;                             // check if hovered
	protected:
        static void   use_available_region(View* , Rect = {});  // update m_xxx_space_content_region with available space or given rectangle
        bool     m_is_visible;
		bool     m_is_hovered;
        Rect m_screen_space_content_region;  // view rectangle in screen space coordinates
        Rect m_local_space_content_region;  // view rectangle in window space coordinates
		REFLECT_BASE_CLASS()
    };
}
