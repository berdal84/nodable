#pragma once

#include "ImGuiEx.h"
#include "../core/reflection/reflection"

namespace fw
{
    /**
     * View is an abstract class to provide a GUI for a specific Node.
     * View also implement a small static library to _draw_property_view custom ImGui widgets/graphics.
     */
	class View
	{
	public:

	    /**
	     * Enum to define some color types
	     */
		enum ColorType
		{
			Color_FILL,
			Color_HIGHLIGH,
			Color_BORDER,
			Color_BORDER_HIGHLIGHT,
			Color_SHADOW,
			Color_COUNT
		};

		View();
		virtual ~View() = default;
        virtual bool  draw() = 0;
		void          set_color(ColorType, const ImVec4* );          // set color of a given type
		ImColor       get_color(ColorType) const;                    // get color of a given type
		void          set_visible(bool _visibility);                 // show/hide view
        bool          is_visible()const;                             // check if visible
		bool          is_hovered()const;                             // check if hovered
	protected:
        static void   use_available_region(View* , ImRect rect = ImRect());  // update m_xxx_space_content_region with available space or given rectangle
        bool     m_is_visible;
		bool     m_is_hovered;
        ImRect   m_screen_space_content_region;  // view rectangle in screen space coordinates
        ImRect   m_local_space_content_region;  // view rectangle in window space coordinates
    private:
        std::array<const ImVec4*, Color_COUNT> m_colors;

		REFLECT_BASE_CLASS()
    };
}
