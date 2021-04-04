#include "View.h"
#include "Log.h"
#include "Settings.h"

using namespace Nodable;

View::View():
	Component(),
	hovered(false),
	visible(true),
	visibleRect()
{
    Settings* settings = Settings::GetCurrent();

	// set default colors
	colors.insert({ColorType_Fill,              &settings->ui.nodes.fillColor});
	colors.insert({ ColorType_Highlighted,      &settings->ui.nodes.highlightedColor});
	colors.insert({ ColorType_Border,           &settings->ui.nodes.borderColor});
	colors.insert({ ColorType_BorderHighlights, &settings->ui.nodes.borderHighlightedColor});
	colors.insert({ ColorType_Shadow,           &settings->ui.nodes.shadowColor});
}

ImVec2 Nodable::View::CursorPosToScreenPos(ImVec2 _position)
{
	const ImVec2 offset = ImGui::GetCursorScreenPos() - ImGui::GetCursorPos();
	return _position + offset;
}

void View::setColor(ColorType_ _type, ImVec4* _color)
{
	colors.insert_or_assign(_type, _color);
}

ImColor View::getColor(ColorType_ _type)
{
    ImVec4 c = *colors[_type];
	return  ImColor(c);
}

bool Nodable::View::drawAsChild(const char* _name, const ImVec2& _size, bool border, ImGuiWindowFlags flags)
{
	bool result;

	/* Compute visible rect in screen position*/
	auto outerCursorScreenPos = ImGui::GetCursorScreenPos();
	visibleScreenRect.Min = outerCursorScreenPos;
	visibleScreenRect.Max = outerCursorScreenPos + _size;

	ImGui::BeginChild(_name, _size, border, flags);
	auto innerCursorScreenPos = ImGui::GetCursorScreenPos();		
	result = this->draw();
	ImGui::EndChild();

	visibleRect = visibleScreenRect;
	visibleRect.Translate(innerCursorScreenPos * -1.0f);

	return result;
}

void View::DrawRectShadow (ImVec2 _topLeftCorner, ImVec2 _bottomRightCorner, float _borderRadius, int _shadowRadius, ImVec2 _shadowOffset, ImColor _shadowColor)
{
	ImVec2 itemRectMin(_topLeftCorner.x + _shadowOffset.x, _topLeftCorner.y + _shadowOffset.y);
	ImVec2 itemRectMax(_bottomRightCorner.x + _shadowOffset.x, _bottomRightCorner.y + _shadowOffset.y);
	ImVec4 color       = _shadowColor;
	color.w /= _shadowRadius;
	auto borderRadius  = _borderRadius;

	// draw N concentric rectangles.
	for(int i = 0; i < _shadowRadius; i++)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		draw_list->AddRectFilled(itemRectMin, itemRectMax, ImColor(color), borderRadius);

		itemRectMin.x -= 1.0f;
		itemRectMin.y -= 1.0f;

		itemRectMax.x += 1.0f;
		itemRectMax.y += 1.0f;

		borderRadius += 1.0f;
	}
}

void View::ShadowedText(ImVec2 _offset, ImColor _shadowColor, const char* _format, ...)
{
	// draw first the shadow
	auto p = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(p.x + _offset.x, p.y + _offset.y));	

	va_list args;
    va_start(args, _format);
    ImGui::TextColored(_shadowColor, _format, args);
	ImGui::SetCursorPos(p);
    ImGui::Text(_format, args);
    va_end(args);
}

void View::ColoredShadowedText(ImVec2 _offset, ImColor _textColor, ImColor _shadowColor, const char* _format, ...)
{
	// draw first the shadow
	auto p = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(p.x + _offset.x, p.y + _offset.y));	

	va_list args;
    va_start(args, _format);
    ImGui::TextColored(_shadowColor, _format, args);
	ImGui::SetCursorPos(p);
    ImGui::TextColored(_textColor, _format, args);
    va_end(args);
}