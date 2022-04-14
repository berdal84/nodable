#include <nodable/app/View.h>
#include <nodable/app/Settings.h>
#include <nodable/app/IAppCtx.h>

using namespace Nodable;

REGISTER
{
    registration::push_class<View>("View");
}

View::View(IAppCtx& _ctx):
        m_ctx(_ctx),
        m_is_hovered(false),
        m_is_visible(true),
        m_visible_rect()
{
    Settings& settings = _ctx.settings();
	// set default colors
	m_colors.insert({Color_Fill, &settings.ui_node_fillColor});
	m_colors.insert({Color_Highlighted, &settings.ui_node_highlightedColor});
	m_colors.insert({Color_Border, &settings.ui_node_borderColor});
	m_colors.insert({Color_BorderHighlights, &settings.ui_node_borderHighlightedColor});
	m_colors.insert({Color_Shadow, &settings.ui_node_shadowColor});
}

void View::set_color(Color _type, vec4* _color)
{
	m_colors.insert_or_assign(_type, _color);
}

ImColor View::get_color(Color _type) const
{
	return  ImColor(*m_colors.at(_type));
}

bool View::draw_as_child(const char* _name, const vec2& _size, bool border, ImGuiWindowFlags flags)
{
	bool changed;

	/* Compute visible rect in screen position*/
	auto outerCursorScreenPos = ImGui::GetCursorScreenPos();
    m_visible_screen_rect.Min = outerCursorScreenPos;
    m_visible_screen_rect.Max = outerCursorScreenPos + _size;

	ImGui::BeginChild(_name, _size, border, flags);
	auto innerCursorScreenPos = ImGui::GetCursorScreenPos();		
	changed = draw();
	ImGui::EndChild();

    m_visible_rect = m_visible_screen_rect;
	m_visible_rect.Translate(innerCursorScreenPos * -1.0f);

	return changed;
}
