#pragma once

#include <nodable/core/reflection/reflection>

#include <nodable/app/ImGuiEx.h>
#include <nodable/core/Component.h>

namespace Nodable
{
    // forward decl
    class IAppCtx;

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
		enum Color
		{
			Color_Fill,
			Color_Highlighted,
			Color_Border,
			Color_BorderHighlights,
			Color_Shadow,
			Color_COUNT
		};

		View(IAppCtx& _ctx);
		virtual ~View() = default;

		virtual bool         draw() = 0;
		bool                 draw_as_child(const char* _name, const vec2& _size, bool border = false, ImGuiWindowFlags flags = 0);
		void                 set_color(Color _type, vec4* _color );
		ImColor              get_color(Color) const;
		inline void          set_visible(bool _visibility){ m_is_visible = _visibility;}
        inline bool          is_visible()const{return m_is_visible;}
		inline bool          is_hovered()const{return m_is_hovered;}
		// inline void          set_visible_rect(ImRect _rect) { m_visible_rect = _rect; }
		inline ImRect        get_visible_rect() const { return m_visible_rect; }

	protected:
        bool     m_is_visible;
		ImRect   m_visible_rect;
		ImRect   m_visible_screen_rect;
		bool     m_is_hovered;
        IAppCtx& m_ctx;

    private:
        std::map<Color, vec4*> m_colors;

		REFLECT_ENABLE()
    };
}
