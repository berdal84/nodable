#include <nodable/View.h>
#include <nodable/Settings.h>

using namespace Nodable;

View::View():
	hovered(false),
	visible(true),
	visibleRect()
{
    Settings* settings = Settings::GetCurrent();

	// set default colors
	colors.insert({ColorType_Fill,              &settings->ui.node.fillColor});
	colors.insert({ ColorType_Highlighted,      &settings->ui.node.highlightedColor});
	colors.insert({ ColorType_Border,           &settings->ui.node.borderColor});
	colors.insert({ ColorType_BorderHighlights, &settings->ui.node.borderHighlightedColor});
	colors.insert({ ColorType_Shadow,           &settings->ui.node.shadowColor});
}

void View::setColor(ColorType_ _type, ImVec4* _color)
{
	colors.insert_or_assign(_type, _color);
}

ImColor View::getColor(ColorType_ _type) const
{
	return  ImColor(*colors.at(_type));
}

bool View::drawAsChild(const char* _name, const ImVec2& _size, bool border, ImGuiWindowFlags flags)
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
