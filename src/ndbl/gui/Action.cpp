#include "Action.h"
#include "SDL_keyboard.h"
#include "bdc/String_Builder.hpp"

namespace ndbl
{
    using namespace bdc;

    String Shortcut::to_string() const
    {
        String_Builder sb;
        string_builder_init(sb);

        if( mod & KMOD_CTRL )       string_builder_append( sb, "Ctrl + ");
        if( mod & KMOD_ALT )        string_builder_append( sb, "Alt + ");
        if( key )                   string_builder_append( sb, SDL_GetKeyName(key));
        if( !description.empty() )  string_builder_append( sb, description);

        return string_builder_build_tstring(sb);
    }
} // namespace ndbl
