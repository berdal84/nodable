#pragma once

#include "ImGuiEx.h"
#include <fw/core/reflection/reflection>

namespace fw
{
    /**
     * View is an abstract class to provide a GUI for a specific Node.
     * View also implement a small static library to draw_property custom ImGui widgets/graphics.
     */
	class View
	{
	public:

	    /**
	     * Enum to define some color types
	     */
		enum ColorType
		{
			ColorType_Fill,
			ColorType_Highlighted,
			ColorType_Border,
			ColorType_BorderHighlights,
			ColorType_Shadow,
			ColorType_COUNT
		};

		View();
		virtual ~View() = default;
        virtual bool  draw(ImRect rect = ImRect());
		void          set_color(ColorType, const ImVec4* );          // set color of a given type
		ImColor       get_color(ColorType) const;                    // get color of a given type
		void          set_visible(bool _visibility);                 // show/hide view
        bool          is_visible()const;                             // check if visible
		bool          is_hovered()const;                             // check if hovered
	protected:
        virtual bool  draw_implem() = 0;                             // custom draw to implement
        bool     m_is_visible;
		bool     m_is_hovered;
        ImRect   m_screen_space_content_region;  // view rectangle in screen space coordinates
        ImRect   m_local_space_content_region;  // view rectangle in window space coordinates
    private:
        std::array<const ImVec4*, ColorType_COUNT> m_colors;

		REFLECT_BASE_CLASS()
    };
}
