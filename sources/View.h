#pragma once
#include <imgui/imgui.h>
#include "Component.h"

namespace Nodable{
	class View : public Component
	{
	public:
		enum ColorType_
		{
			ColorType_Fill,
			ColorType_Border,
			ColorType_BorderHighlights,
			ColorType_Shadow,
			ColorType_COUNT
		};

		View();
		
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

		void        setColor       (ColorType_ ,ImColor);
		ImColor     getColor       (ColorType_);

		/* Return true if this view is hovered by mouse cursor, false if not */
		bool        isHovered      ()const{return hovered;}

	protected:
		bool    hovered = false;
	private:
		ImColor colors[ColorType_COUNT];
		
	};
}
