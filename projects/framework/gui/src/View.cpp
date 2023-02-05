#include <fw/gui/View.h>

using namespace fw;

REGISTER
{
    registration::push_class<View>("View");
}

View::View():
    m_is_hovered(false),
    m_is_visible(true),
    m_visible_rect(0.0f, 512.0f) // need to be != 0.0f (cf GraphNodeView frame_all)
{}

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
    {
        auto innerCursorScreenPos = ImGui::GetCursorScreenPos();
        changed = draw();
        m_visible_rect = m_visible_screen_rect;
        m_visible_rect.Translate(innerCursorScreenPos * -1.0f);
    }
	ImGui::EndChild();

	return changed;
}
