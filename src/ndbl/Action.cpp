#include "Action.h"

#include "SDL_keyboard.h"
#include "bdc/String_Builder.hpp"

namespace ndbl
{
    
using namespace bdc;

String shortcut_to_tstring(const Shortcut& shortcut)
{
    String_Builder sb;
    string_builder_init(sb);

    if( shortcut.mod & KMOD_CTRL )       string_builder_append( sb, "Ctrl +");
    if( shortcut.mod & KMOD_ALT )        string_builder_append( sb, "Alt +");
    if( shortcut.key )                   string_builder_append( sb, SDL_GetKeyName(shortcut.key));
    if( !shortcut.description.empty() )  string_builder_append( sb, shortcut.description);

    return string_builder_build_tstring(sb, " ");
}

} // namespace ndbl
