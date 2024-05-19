#include "View.h"
#include "ImGuiEx.h"

using namespace fw;

REGISTER
{
    registration::push_class<View>("View");
}

View::View()
    : is_hovered(false)
    , is_visible(true)
    , m_screen_space_content_region(0.0f, 512.0f) // need to be != 0.0f (cf GraphNodeView frame_all)
{}

void View::use_available_region(View* view, const Rect& rect)
{
    if( rect.has_area() )
    {
        view->m_screen_space_content_region = rect;
    }
    else
    {
        view->m_screen_space_content_region = ImGuiEx::GetContentRegion(Space_Screen);
        view->m_local_space_content_region  = ImGuiEx::GetContentRegion(Space_Local);
    }
}
