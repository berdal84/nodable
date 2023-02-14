#pragma once

#include "ImGuiEx.h"
#include <fw/core/reflection/reflection>

namespace fw
{
    /**
     * View is an abstract class to provide a GUI for a specific Node.
     * View also implement a small static library to draw custom ImGui widgets/graphics.
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

		virtual bool  on_draw() = 0;                                 // custom draw to implement
		bool          draw_as_child( const char* _name,
                                     const ImVec2 & _size,
                                     bool border = false,
                                     ImGuiWindowFlags flags = 0);    // draw within a sized child
		void          set_color(ColorType, const ImVec4* );          // set color of a given type
		ImColor       get_color(ColorType) const;                    // get color of a given type
		void          set_visible(bool _visibility);                 // show/hide view
        bool          is_visible()const;                             // check if visible
		bool          is_hovered()const;                             // check if hovered
		ImRect        get_visible_rect()const;                       // get visible rectangle (relative to view local coordinates)

	protected:
        bool     m_is_visible;
		ImRect   m_visible_rect;
		ImRect   m_visible_screen_rect;
		bool     m_is_hovered;
    private:
        std::array<const ImVec4*, ColorType_COUNT> m_colors;

		REFLECT_BASE_CLASS()
    };
}
