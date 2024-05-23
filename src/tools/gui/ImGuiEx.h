#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "tools/core/geometry/Rect.h"
#include "tools/core/geometry/Vec2.h"
#include "tools/core/geometry/Vec4.h"
#include "tools/core/geometry/Space.h"
#include "tools/core/types.h"

#include "ActionManager.h"
#include "EventManager.h"

namespace tools
{
    // forward declarations
    struct Texture;

    namespace ImGuiEx
    {
        constexpr float TOOLTIP_DURATION_DEFAULT = 0.2f;
        constexpr float TOOLTIP_DELAY_DEFAULT    = 0.5f;

        static bool  debug = false; // when true, all DebugXXX commands actually draw, otherwise it is skipped
        static bool  is_in_a_frame = false;
        static bool  is_any_tooltip_open = false;
        static float tooltip_delay_elapsed = 0.0f;

        /**
         * Draw a rounded-rectangle shadow
         * TODO: use a low cost method, this one is drawing several rectangle with modulated opacity.
        */
        extern void DrawRectShadow(
                Vec2 _topLeftCorner,
                Vec2 _bottomRightCorner,
                float _borderRadius = 0.0f,
                int _shadowRadius = 10,
                Vec2 _shadowOffset = Vec2(),
                Vec4 _shadowColor = Vec4(0.0f, 0.0f, 0.0f, 1.f));

        extern void ShadowedText(
                Vec2 _offset,
                Vec4 _shadowColor,
                const char *_format,
                ...);

        extern void ColoredShadowedText(
                Vec2 _offset,
                Vec4 _textColor,
                Vec4 _shadowColor,
                const char *_format,
                ...);

        extern void DrawWire(
                ImDrawList *draw_list,
                Vec2 pos0,
                Vec2 pos1,
                Vec2 norm0,
                Vec2 norm1,
                Vec4 color,
                Vec4 shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        extern void DrawVerticalWire(
                ImDrawList *draw_list,
                Vec2 pos0,
                Vec2 pos1,
                Vec4 color,
                Vec4 shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        extern void DrawHorizontalWire(
                ImDrawList *draw_list,
                Vec2 pos0,
                Vec2 pos1,
                Vec4 color,
                Vec4 shadowColor,
                float thickness = 1.0f,
                float roundness = 0.5f);

        extern void Tooltip_EndFrame();
        extern void Tooltip_NewFrame();
        extern bool     BeginTooltip(float _delay = TOOLTIP_DELAY_DEFAULT, float _duration = TOOLTIP_DURATION_DEFAULT );
        extern void     EndTooltip();
        extern Rect&    EnlargeToInclude(Rect& _rect, Rect _other);
        extern void     BulletTextWrapped(const char*);
        extern Rect     GetContentRegion(Space);
        extern void     DebugRect(const Vec2& p_min, const Vec2& p_max, ImU32 col, float rounding = 0.f, ImDrawFlags flags = 0, float thickness = 1.f);
        extern void     DebugCircle(const Vec2& center, float radius, ImU32 col, int num_segments = 0, float thickness = 1.0f);
        extern void     DebugLine(const Vec2& p1, const Vec2& p2, ImU32 col, float thickness = 1.0f);
        extern void     Image(Texture*);

        template<class EventT>
        static void MenuItem(bool selected = false, bool enable = true) // Shorthand to get a given action from the manager and draw a MenuItem from it.
        {
            const IAction* action = ActionManager::get_instance().get_action_with_id(EventT::id);

            if (ImGui::MenuItem( action->label.c_str(), action->shortcut.to_string().c_str(), selected, enable))
            {
                action->trigger();
            }
        };

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
        static void DrawHelper(const char* _format, Args... args)
        { DrawHelperEx(0.25f, _format, args...); } // simple "?" test with a tooltip.

        inline ImRect toImGui(Rect r) { return { r.min, r.max }; };
        inline ImVec2 toImGui(Vec2 v) { return { v.x, v.y }; };
        inline ImVec4 toImGui(Vec4 v) { return { v.x, v.y, v.z, v.w }; };
    };
}