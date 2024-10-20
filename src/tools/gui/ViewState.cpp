#include "ViewState.h"
#include "ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define DEBUG_DRAW 1
#endif

using namespace tools;

ViewState::ViewState()
: ViewState(100.f, 100.f)
{
}

ViewState::ViewState(float width, float height)
: hovered(false)
, visible(true)
, selected(false)
, box()
{
    box.set_size({width, height});
}
