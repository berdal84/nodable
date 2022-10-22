#include <nodable/app/Shortcut.h>
#include <SDL_keyboard.h>

using namespace ndbl;

std::string Shortcut::to_string( size_t label_max_length ) const {
    std::string result = label.substr(0, label_max_length);

    while ( result.length() < label_max_length + 1 ) result.push_back(' ');

    if( mod & KMOD_CTRL) result += "Ctrl + ";
    if( mod & KMOD_ALT)  result += "Alt + ";
    result += SDL_GetKeyName(key);

    return result;
}

std::vector<Shortcut> ShortcutManager::s_shortcuts = {
        {
                SDLK_DELETE,
                KMOD_NONE,
                EventType::delete_node_action_triggered,
                Condition_HAS_SELELECTION,
                "Delete"
        },
        {
                SDLK_a,
                KMOD_NONE,
                EventType::arrange_node_action_triggered,
                Condition_HAS_SELELECTION,
                "Arrange"
        },
        {
                SDLK_x,
                KMOD_NONE,
                EventType::toggle_folding_selected_node_action_triggered,
                Condition_HAS_SELELECTION,
                "Fold/Unfold"
        },
        {
                SDLK_n,
                KMOD_NONE,
                EventType::select_successor_node_action_triggered,
                Condition_HAS_SELELECTION,
                "Select next"
        },
        {
                SDLK_s,
                KMOD_CTRL,
                EventType::save_file,
                Condition_ALWAYS,
                "Save"
        }
};