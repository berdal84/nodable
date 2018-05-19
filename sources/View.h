#pragma once
#include <imgui.h>

namespace Nodable{
	class View
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
		~View(){}

		/* Draw only shadow for a rectangle */
		static void DrawRectShadow (ImVec2 _topLeftCorner, ImVec2 _bottomRightCorner, float _borderRadius = 0.0f, int _shadowRadius = 10, ImVec2 _shadowOffset = ImVec2(), ImColor _shadowColor = ImColor(0.0f,0.0f,0.0f));
		
		/* Draw a text with a shadow. */
		static void ShadowedText   (ImVec2 _offset, ImColor _color, const char*, ...);

		void        setColor       (ColorType_ ,ImColor);
		ImColor     getColor       (ColorType_);
	private:
		ImColor colors[ColorType_COUNT];
	};
}
