#include <SDL_keyboard.h>
#include <nodable/app/Shortcut.h>

using namespace ndbl;

std::string Shortcut::to_string() const
{
    std::string result;

    if (mod & KMOD_CTRL) result += "Ctrl + ";
    if (mod & KMOD_ALT) result += "Alt + ";
    result += SDL_GetKeyName(key);

    return result;
}
