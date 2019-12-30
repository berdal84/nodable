#pragma once

// All the Views uses ImGui
#include <imgui/imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
// I also import internal for maths operators
#include <imgui/imgui_internal.h>

#include "Component.h"

namespace Nodable{
	class View : public Component
	{
	public:
		enum ColorType_
		{
			ColorType_Fill,
			ColorType_Highlighted,
			ColorType_Border,
			ColorType_BorderHighlights,
			ColorType_Shadow,
			ColorType_COUNT
		};

		View();
		virtual ~View(){}
		
		virtual bool draw()=0;

		/* Draw only shadow for a rectangle */
		static void DrawRectShadow      (	ImVec2 	_topLeftCorner, 
											ImVec2 	_bottomRightCorner, 
											float 	_borderRadius 	= 0.0f, 
											int 	_shadowRadius 	= 10, 
											ImVec2 	_shadowOffset 	= ImVec2(), 
											ImColor _shadowColor 	= ImColor(0.0f,0.0f,0.0f));
		
		/* Draw a text (default colored) with a shadow. */
		static void ShadowedText        (ImVec2 _offset, ImColor _shadowColor, const char*, ...);

		/* Draw a colored text with a shadow. */
		static void ColoredShadowedText (ImVec2 _offset, ImColor _textColor, ImColor _shadowColor, const char*, ...);

		/* */
		static ImVec2 ConvertCursorPositionToScreenPosition(ImVec2);

		void        setColor       (ColorType_ ,ImColor);
		ImColor     getColor       (ColorType_);

		void        setVisible(bool _b) { visible = _b; }
		bool        isVisible() { return visible; }

		/* Return true if this view is hovered by mouse cursor, false if not */
		bool        isHovered      ()const{return hovered;}

	protected:
		bool    hovered   = false;
	private:
		bool    visible = true;
		ImColor colors[ColorType_COUNT];
		
	};
}
