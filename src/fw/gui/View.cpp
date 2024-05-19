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
, parent_xform()
, box()
{
}

void View::position( Vec2 _position, Space origin)
{
    if ( origin == WORLD_SPACE )
    {
        _position = Vec2::transform(_position, parent_xform.world_matrix());
    }
    box.pos(_position);
}

Vec2 View::position(Space origin) const
{
    if ( origin == WORLD_SPACE )
    {
        return Vec2::transform(box.pos(), parent_xform.model_matrix() );
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
    parent_xform = {};
    parent_xform.pos(ImGuiEx::GetContentRegion( WORLD_SPACE ).Min);

    return onDraw();
}

Rect View::rect(Space space) const
{
    Vec2 half_size = box.size() / 2.0f;
    Vec2 screen_pos = position(space);
    Rect result {
        screen_pos - half_size,
        screen_pos + half_size
    };
    return result;
}
