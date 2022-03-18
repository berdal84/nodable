#include <nodable/app/View.h>
#include <nodable/app/Settings.h>
#include <nodable/app/AppContext.h>

using namespace Nodable;

View::View(AppContext* _ctx):
    m_context(_ctx),
	hovered(false),
	visible(true),
	visibleRect()
{
    Settings* settings = m_context->settings;
	// set default colors
	colors.insert({ Color_Fill,              &settings->ui_node_fillColor});
	colors.insert({ Color_Highlighted,      &settings->ui_node_highlightedColor});
	colors.insert({ Color_Border,           &settings->ui_node_borderColor});
	colors.insert({ Color_BorderHighlights, &settings->ui_node_borderHighlightedColor});
	colors.insert({ Color_Shadow,           &settings->ui_node_shadowColor});
}

void View::setColor(Color _type, vec4* _color)
{
	colors.insert_or_assign(_type, _color);
}

ImColor View::getColor(Color _type) const
{
	return  ImColor(*colors.at(_type));
}

bool View::drawAsChild(const char* _name, const vec2& _size, bool border, ImGuiWindowFlags flags)
{
	bool changed;

	/* Compute visible rect in screen position*/
	auto outerCursorScreenPos = ImGui::GetCursorScreenPos();
	visibleScreenRect.Min = outerCursorScreenPos;
	visibleScreenRect.Max = outerCursorScreenPos + _size;

	ImGui::BeginChild(_name, _size, border, flags);
	auto innerCursorScreenPos = ImGui::GetCursorScreenPos();		
	changed = draw();
	ImGui::EndChild();

	visibleRect = visibleScreenRect;
	visibleRect.Translate(innerCursorScreenPos * -1.0f);

	return changed;
}
