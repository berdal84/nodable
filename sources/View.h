#pragma once
#include <imgui.h>

namespace Nodable{
	class View
	{
	public:
		View(){}
		~View(){}
		static void DrawRectShadow(ImVec2 _from, ImVec2 _to, float _rectInitialRadius = 0.0f, int _radius = 10, ImVec2 _offset = ImVec2());

		void    setColor(ImColor _color){color = _color;}
		ImColor getColor()const{return color;}

		void    setBorderColor(ImColor _color){borderColor = _color;}
		ImColor getBorderColor()const{return borderColor;}
	private:
		ImColor color        = ImColor(1.0f, 1.0f, 1.0f);	
		ImColor borderColor  = ImColor(0.2f, 0.2f, 0.2f);
	};
}
