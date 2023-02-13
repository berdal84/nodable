#pragma once

#include "ImGuiEx.h"
#include <fw/core/reflection/reflection>

namespace fw
{
    /**
     * View is an abstract class to provide a GUI for a specific Node.
     * View also implement a small static library to draw custom ImGui widgets/graphics.
     *
     * TODO: split "interface" and "ImGui implementation"
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

		virtual bool         draw() = 0;
		bool                 draw_as_child(const char* _name, const ImVec2 & _size, bool border = false, ImGuiWindowFlags flags = 0);
		void                 set_color(ColorType, const ImVec4 * );
		ImColor              get_color(ColorType) const;
		inline void          set_visible(bool _visibility){ m_is_visible = _visibility;}
        inline bool          is_visible()const {return m_is_visible;}
		inline bool          is_hovered()const {return m_is_hovered;}
		inline ImRect        get_visible_rect() const { return m_visible_rect; }

	protected:
        bool     m_is_visible;
		ImRect   m_visible_rect;
		ImRect   m_visible_screen_rect;
		bool     m_is_hovered;
    private:
        std::map<ColorType, const ImVec4 *> m_colors;

		REFLECT_BASE_CLASS()
    };
}
