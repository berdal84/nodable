#pragma once

#include <imgui/imgui.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <fw/gui/EventManager.h>
#include <fw/gui/types.h>
#include <imgui/imgui_internal.h>

namespace fw
{
    class ImGuiEx
    {
    public:
        virtual ~ImGuiEx() = 0;

        template<typename ...Args>
        static void DrawHelperEx(float _alpha, const char* _format, Args... args)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, _alpha);
            ImGui::Text(ICON_FA_QUESTION_CIRCLE);
            ImGui::PopStyleVar();

            if( BeginTooltip() )
            {
                ImGui::Text(_format, args...);
                EndTooltip();
            }

        }

        template<typename ...Args>
        static void DrawHelper(const char* _format, Args... args) { DrawHelperEx(0.25f, _format, args...); } // simple "?" test with a tooltip.

        /**
         * Draw a rounded-rectangle shadow
         * TODO: use a low cost method, this one is drawing several rectangle with modulated opacity.
        */
        static void DrawRectShadow(
                ImVec2 _topLeftCorner,
                ImVec2 _bottomRightCorner,
                float _borderRadius = 0.0f,
                int _shadowRadius = 10,
                ImVec2 _shadowOffset = ImVec2(),
                ImColor _shadowColor = ImColor(0.0f, 0.0f, 0.0f));

        static void ShadowedText(
                ImVec2 _offset,
                ImColor _shadowColor,
                const char *_format,
                ...);

        static void ColoredShadowedText(
                ImVec2 _offset,
                ImColor _textColor,
                ImColor _shadowColor,
                const char *_format,
                ...);

        static ImVec2 CursorPosToScreenPos(ImVec2 _cursorPosition);

        static ImVec2 ToScreenPosOffset();

        static void DrawVerticalWire(
                ImDrawList *draw_list,
                ImVec2 pos0,
                ImVec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        static void DrawHorizontalWire(
                ImDrawList *draw_list,
                ImVec2 pos0,
                ImVec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        static void     BeginFrame();
        static void     EndFrame();
        static bool     BeginTooltip(float _delay = s_tooltip_delay_default
                                   , float _duration = s_tooltip_duration_default);
        static void     EndTooltip();
        static ImRect&  EnlargeToInclude(ImRect& _rect, ImRect _other);

        static void     MenuItemBindedToEvent(uint16_t type, bool selected = false, bool enable = true);
        static void     BulletTextWrapped(const char*);
    private:
        static bool    s_is_in_a_frame;
        static bool    s_is_any_tooltip_open;
        static float   s_tooltip_delay_default;
        static float   s_tooltip_duration_default;
        static float   s_tooltip_delay_elapsed;

    };
}