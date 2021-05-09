
#include <imgui/imgui.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui/imgui_internal.h>

/**
 * @brief Namespace to gather all custom ImGui-like functions.
 */
namespace ImGuiEx {
    /**
     * Draw a rounded-rectangle shadow
     * TODO: use a low cost method, this one is drawing several rectangle with modulated opacity.
    */
    void DrawRectShadow(
            ImVec2 _topLeftCorner,
            ImVec2 _bottomRightCorner,
            float _borderRadius = 0.0f,
            int _shadowRadius = 10,
            ImVec2 _shadowOffset = ImVec2(),
            ImColor _shadowColor = ImColor(0.0f, 0.0f, 0.0f));
    void ShadowedText(
            ImVec2 _offset,
            ImColor _shadowColor,
            const char *_format,
            ...);
    void ColoredShadowedText(
            ImVec2 _offset,
            ImColor _textColor,
            ImColor _shadowColor,
            const char *_format,
            ...);
    ImVec2 CursorPosToScreenPos(ImVec2 _cursorPosition);
    ImVec2 ToScreenPosOffset();
    void DrawVerticalWire(
            ImDrawList *draw_list,
            ImVec2 pos0,
            ImVec2 pos1,
            ImColor color,
            ImColor shadowColor,
            float thickness = 1.0f,
            float roundness = 0.5f);
    void DrawHorizontalWire(
            ImDrawList *draw_list,
            ImVec2 pos0,
            ImVec2 pos1,
            ImColor color,
            ImColor shadowColor,
            float thickness = 1.0f,
            float roundness = 0.5f);
}