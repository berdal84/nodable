#include "View.h"

using namespace Nodable;

void View::DrawRectShadow(ImVec2 _from, ImVec2 _to, float _rectInitialRadius, int _radius, ImVec2 _offset)
{
	ImVec2 itemRectMin(_from.x + _offset.x, _from.y + _offset.y);
	ImVec2 itemRectMax(_to.x + _offset.x, _to.y + _offset.y);
	auto color       = ImColor(0.0f,0.0f,0.0f, 0.3f / float(_radius));
	auto rectRadius  = _rectInitialRadius;


	for(int i = 0; i < _radius; i++)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		draw_list->AddRectFilled(itemRectMin, itemRectMax, color, rectRadius);

		itemRectMin.x -= 1.0f;
		itemRectMin.y -= 1.0f;

		itemRectMax.x += 1.0f;
		itemRectMax.y += 1.0f;

		rectRadius += 1.0f;
	}
}

void View::ShadowedText(ImVec2 _offset, ImColor _color, const char* _format, ...)
{
	// draw first the shadow
	auto p = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(p.x + _offset.x, p.y + _offset.y));
	

	va_list args;
    va_start(args, _format);
    ImGui::TextColored(_color, _format, args);
	ImGui::SetCursorPos(p);
    ImGui::Text(_format, args);
    va_end(args);
}