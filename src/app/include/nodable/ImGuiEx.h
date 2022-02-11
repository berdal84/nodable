
#include <imgui/imgui.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui/imgui_internal.h>
#include <nodable/types.h>

namespace Nodable
{
    /**
     * @brief Namespace to gather all custom ImGui-like functions.
     */
    namespace ImGuiEx {
        /**
         * Draw a rounded-rectangle shadow
         * TODO: use a low cost method, this one is drawing several rectangle with modulated opacity.
        */
        void DrawRectShadow(
                vec2 _topLeftCorner,
                vec2 _bottomRightCorner,
                float _borderRadius = 0.0f,
                int _shadowRadius = 10,
                vec2 _shadowOffset = vec2(),
                ImColor _shadowColor = ImColor(0.0f, 0.0f, 0.0f));

        void ShadowedText(
                vec2 _offset,
                ImColor _shadowColor,
                const char *_format,
                ...);

        void ColoredShadowedText(
                vec2 _offset,
                ImColor _textColor,
                ImColor _shadowColor,
                const char *_format,
                ...);

        vec2 CursorPosToScreenPos(vec2 _cursorPosition);

        vec2 ToScreenPosOffset();

        void DrawVerticalWire(
                ImDrawList *draw_list,
                vec2 pos0,
                vec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        void DrawHorizontalWire(
                ImDrawList *draw_list,
                vec2 pos0,
                vec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);
    }
}