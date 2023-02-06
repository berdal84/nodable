#include <fw/gui/View.h>

using namespace fw;

REGISTER
{
    registration::push_class<View>("View");
}

constexpr vec4 NULL_COLOR(0.5f, 0.5f, 0.5f, 1);

View::View():
    m_is_hovered(false),
    m_is_visible(true),
    m_visible_rect(0.0f, 512.0f) // need to be != 0.0f (cf GraphNodeView frame_all)
{
    set_color(ColorType_Fill, &NULL_COLOR);
    set_color(ColorType_Border, &NULL_COLOR);
    set_color(ColorType_BorderHighlights, &NULL_COLOR);
    set_color(ColorType_Highlighted, &NULL_COLOR);
    set_color(ColorType_Shadow, &NULL_COLOR);
}

void View::set_color(ColorType _type, const vec4* _color)
{
	m_colors.insert_or_assign(_type, _color);
}

ImColor View::get_color(ColorType _type) const
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
    {
        auto innerCursorScreenPos = ImGui::GetCursorScreenPos();
        changed = draw();
        m_visible_rect = m_visible_screen_rect;
        m_visible_rect.Translate(innerCursorScreenPos * -1.0f);
    }
	ImGui::EndChild();

	return changed;
}
