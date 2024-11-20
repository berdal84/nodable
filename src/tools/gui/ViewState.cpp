#include "ViewState.h"
#include "ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define DEBUG_DRAW 1
#endif

using namespace tools;

ViewState::ViewState()
: ViewState(1.f, 1.f)
{
}

ViewState::ViewState(float width, float height)
: hovered(false)
, visible(true)
, selected(false)
, _shape()
{
    _shape.set_size({width, height});
}
