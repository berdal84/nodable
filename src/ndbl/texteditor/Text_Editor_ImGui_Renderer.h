#pragma once

#include <imgui.h>

#include "Text_Editor.h"

namespace ndbl
{
    using Coordinates   = Text_Editor::Coordinates;
    using PaletteIndex  = Text_Editor::PaletteIndex;

	typedef std::array<ImColor, (unsigned)PaletteIndex::Max> Palette;

    // text editor API

    bool            text_editor_render(Text_Editor& editor, const char* title, const Palette& palette, const ImVec2& size = ImVec2(), bool border = false, bool ignoreImGuiChild = false);
    void            text_editor_handle_mouse_inputs(Text_Editor& editor);
    void            text_editor_handle_keyboard_inputs(Text_Editor& editor);
    Coordinates     text_editor_screen_pos_to_coordinates(const Text_Editor& editor, const Vec2& aPosition);
    const Palette&  text_editor_get_dark_palette();
	const Palette&  text_editor_get_light_palette();
	const Palette&  text_editor_get_retro_blue_palette();

} // namespace ndbl
