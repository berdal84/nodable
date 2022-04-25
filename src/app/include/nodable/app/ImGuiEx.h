#pragma once

#include <imgui/imgui.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <imgui/imgui_internal.h>
#include <nodable/app/types.h>

namespace ndbl
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
                vec2 _topLeftCorner,
                vec2 _bottomRightCorner,
                float _borderRadius = 0.0f,
                int _shadowRadius = 10,
                vec2 _shadowOffset = vec2(),
                ImColor _shadowColor = ImColor(0.0f, 0.0f, 0.0f));

        static void ShadowedText(
                vec2 _offset,
                ImColor _shadowColor,
                const char *_format,
                ...);

        static void ColoredShadowedText(
                vec2 _offset,
                ImColor _textColor,
                ImColor _shadowColor,
                const char *_format,
                ...);

        static vec2 CursorPosToScreenPos(vec2 _cursorPosition);

        static vec2 ToScreenPosOffset();

        static void DrawVerticalWire(
                ImDrawList *draw_list,
                vec2 pos0,
                vec2 pos1,
                ImColor color,
                ImColor shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        static void DrawHorizontalWire(
                ImDrawList *draw_list,
                vec2 pos0,
                vec2 pos1,
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

    private:
        static bool    s_is_in_a_frame;
        static bool    s_is_any_tooltip_open;
        static float   s_tooltip_delay_default;
        static float   s_tooltip_duration_default;
        static float   s_tooltip_delay_elapsed;

    };
}