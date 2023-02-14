#include <fw/gui/View.h>

using namespace fw;

REGISTER
{
    registration::push_class<View>("View");
}

constexpr ImVec4 NULL_COLOR{0.5f, 0.5f, 0.5f, 1.0f};

View::View()
    : m_is_hovered(false)
    , m_colors({&NULL_COLOR,&NULL_COLOR,&NULL_COLOR,&NULL_COLOR,&NULL_COLOR})
    , m_is_visible(true)
    , m_visible_rect(0.0f, 512.0f) // need to be != 0.0f (cf GraphNodeView frame_all)
{
}

void View::set_color(ColorType _type, const ImVec4 * _color)
{
	m_colors[_type] = _color;
}

ImColor View::get_color(ColorType _type) const
{
	return  ImColor(*m_colors[_type]);
}

bool View::draw_as_child(const char* _name, const ImVec2 & _size, bool border, ImGuiWindowFlags flags)
{
	bool changed;

	/* Compute visible rect in screen position*/
	auto outerCursorScreenPos = ImGui::GetCursorScreenPos();
    m_visible_screen_rect.Min = outerCursorScreenPos;
    m_visible_screen_rect.Max = outerCursorScreenPos + _size;

	ImGui::BeginChild(_name, _size, border, flags);
    {
        auto innerCursorScreenPos = ImGui::GetCursorScreenPos();
        changed = on_draw();
        m_visible_rect = m_visible_screen_rect;
        m_visible_rect.Translate(innerCursorScreenPos * -1.0f);
    }
	ImGui::EndChild();

	return changed;
}

void View::set_visible(bool _visibility)
{
    m_is_visible = _visibility;
}

bool View::is_visible() const
{
    return m_is_visible;
}

bool View::is_hovered() const
{
    return m_is_hovered;
}

ImRect View::get_visible_rect() const
{
    return m_visible_rect;
}
