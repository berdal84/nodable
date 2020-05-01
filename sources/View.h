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
		bool         drawAsChild(const char* _name, const ImVec2& _size, bool border = false, ImGuiWindowFlags flags = 0);
		void         setColor(ColorType_ ,ImColor);
		ImColor      getColor(ColorType_);
		void         setVisible(bool _b){ visible = _b; }
		bool         isVisible()const{ return visible; }
		bool         isHovered()const{return hovered;}

		ImRect  visibleRect;
		ImRect  visibleScreenRect;

	protected:
		bool    hovered;

	private:
		bool    visible;
		ImColor colors[ColorType_COUNT];

	public:
		static void DrawRectShadow(
			ImVec2 	_topLeftCorner,
			ImVec2 	_bottomRightCorner,
			float 	_borderRadius = 0.0f,
			int 	_shadowRadius = 10,
			ImVec2 	_shadowOffset = ImVec2(),
			ImColor _shadowColor = ImColor(0.0f, 0.0f, 0.0f));

		static void ShadowedText(ImVec2 _offset, ImColor _shadowColor, const char*, ...);
		static void ColoredShadowedText(ImVec2 _offset, ImColor _textColor, ImColor _shadowColor, const char*, ...);
		static ImVec2 CursorPosToScreenPos(ImVec2);
		
	};
}
