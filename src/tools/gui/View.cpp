#include "View.h"
#include "ImGuiEx.h"

using namespace tools;

REGISTER
{
    registration::push_class<View>("View");
}

View::View()
: is_hovered(false)
, is_visible(true)
, parent_content_region()
, box()
{
}

void View::position( Vec2 _position, Space origin)
{
    if ( origin == PARENT_SPACE )
    {
        Vec2 parent_space_pos = Vec2::transform(_position, parent_content_region.world_matrix());
        return box.pos(parent_space_pos);
    }
    box.pos(_position);
}

Vec2 View::position(Space origin) const
{
    if ( origin == PARENT_SPACE )
    {
        return Vec2::transform(box.pos(), parent_content_region.model_matrix() );
    }
    return box.pos();
}

void View::translate( Vec2 _delta)
{
    Vec2 new_pos = position( WORLD_SPACE ) + _delta;
    position( new_pos, WORLD_SPACE );
}

bool View::draw()
{
    // Get the content region's top left corner
    // This will be used later to convert coordinates between WORLD_SPACE and PARENT_SPACE
    parent_content_region = ImGuiEx::GetContentRegion( WORLD_SPACE );

    return onDraw();
}

Rect View::rect(Space space) const
{
    Rect r =  box.rect();
    if( space == PARENT_SPACE)
    {
        assert(false);
    }
    return r;
}
