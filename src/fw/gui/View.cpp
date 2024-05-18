#include "View.h"

using namespace fw;

REGISTER
{
    registration::push_class<View>("View");
}

View::View()
    : m_is_hovered(false)
    , m_is_visible(true)
    , m_screen_space_content_region(0.0f, 512.0f) // need to be != 0.0f (cf GraphNodeView frame_all)
{
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

void View::use_available_region(View* view, fw::rect rect)
{
    if( rect.GetHeight() == 0 || rect.GetWidth() == 0) {
        view->m_screen_space_content_region = fw::ImGuiEx::GetContentRegion(fw::Space_Screen);
        view->m_local_space_content_region  = fw::ImGuiEx::GetContentRegion(fw::Space_Local);
    } else {
        view->m_screen_space_content_region = rect;
    }
}
