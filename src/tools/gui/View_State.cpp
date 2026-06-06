#include "View_State.h"
#include "ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define DEBUG_DRAW 1
#endif

using namespace tools;

View_State::View_State(Flags flags)
: _flags(flags)
{
}
