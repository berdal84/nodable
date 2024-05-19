#pragma once
#include "FontManager.h"
#include "core/Vec2.h"
#include "core/Vec4.h"
#include "core/Color.h"
#include <imgui.h>
#include <string>

namespace fw
{

    // Common configuration
    struct Config {

        Config() = default;
        Config(const Config&) = delete; // disable copy

        std::string           app_window_label         = "Framework View";
        bool                  vsync                    = false;
        bool                  debug                    = false;
        bool                  show_fps                 = false;
        bool                  delta_time_limit         = true;
        u32_t                 delta_time_min           = 1000 / 60; // in ms
        Color                 background_color         = fw::Color(0.f,0.f,0.f);
        Vec4                  button_activeColor       = fw::Vec4(0.98f, 0.73f, 0.29f, 0.95f); // orange
        Vec4                  button_hoveredColor      = fw::Vec4(0.70f, 0.70f, 0.70f, 0.95f); // light grey
        Vec4                  button_color             = fw::Vec4(0.50f, 0.50f, 0.50f, 0.63f); // grey
        const char*           splashscreen_window_label= "##Splashscreen";
        bool                  splashscreen             = true; // hide/show splashscreen
        bool                  imgui_demo               = false;
        float                 dockspace_bottom_size    = 48.f;
        float                 dockspace_top_size       = 48.f;
        float                 dockspace_right_ratio    = 0.3f;
        size_t                log_tooltip_max_count    = 25;
        std::array<
            Vec4,
            log::Verbosity_COUNT> log_color {
                Vec4(0.5f, 0.0f, 0.0f, 1.0f), // red
                Vec4(0.5f, 0.0f, 0.5f, 1.0f), // violet
                Vec4(0.5f, 0.5f, 0.5f, 1.0f), // grey
                Vec4(0.0f, 0.5f, 0.0f, 1.0f)  // green
            };
        FontManager::Config font_manager{
            {{
                    "default",                  // id
                    "fonts/CenturyGothic.ttf",  // path
                    18.0f,                      // size in px.
                    true,                       // include icons?
                    18.0f                       // icons size in px.
            }},
            {
                    "default",// FontSlot_Paragraph
                    "default",// FontSlot_Heading
                    "default",// FontSlot_Code
                    "default" // FontSlot_ToolBtn
            },
            {
                "FA-solid-900",           // Icon font name
                "fonts/fa-solid-900.ttf"  // Icon font path
            },
            1.0f  // subsampling
        };

        void patch_imgui_style(ImGuiStyle& _style) const // Apply the configuration to an existing ImGuiStyle
        {
            ImVec4 * colors = _style.Colors;
            colors[ImGuiCol_Text]                   = Vec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_TextDisabled]           = Vec4(0.21f, 0.21f, 0.21f, 1.00f);
            colors[ImGuiCol_WindowBg]               = Vec4(0.76f, 0.76f, 0.76f, 1.00f);
            colors[ImGuiCol_DockingEmptyBg]         = Vec4(0.64f, 0.24f, 0.24f, 1.00f);
            colors[ImGuiCol_ChildBg]                = Vec4(0.69f, 0.69f, 0.69f, 1.00f);
            colors[ImGuiCol_PopupBg]                = Vec4(0.66f, 0.66f, 0.66f, 1.00f);
            colors[ImGuiCol_Border]                 = Vec4(0.70f, 0.70f, 0.70f, 1.00f);
            colors[ImGuiCol_BorderShadow]           = Vec4(0.30f, 0.30f, 0.30f, 0.50f);
            colors[ImGuiCol_FrameBg]                = Vec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_FrameBgHovered]         = Vec4(0.90f, 0.80f, 0.80f, 1.00f);
            colors[ImGuiCol_FrameBgActive]          = Vec4(0.90f, 0.65f, 0.65f, 1.00f);
            colors[ImGuiCol_TitleBg]                = Vec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_TitleBgActive]          = Vec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]       = Vec4(0.49f, 0.63f, 0.69f, 1.00f);
            colors[ImGuiCol_MenuBarBg]              = Vec4(0.60f, 0.60f, 0.60f, 0.98f);
            colors[ImGuiCol_ScrollbarBg]            = Vec4(0.40f, 0.40f, 0.40f, 1.00f);
            colors[ImGuiCol_ScrollbarGrab]          = Vec4(0.61f, 0.61f, 0.62f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = Vec4(0.70f, 0.70f, 0.70f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive]    = Vec4(0.80f, 0.80f, 0.80f, 1.00f);
            colors[ImGuiCol_CheckMark]              = Vec4(0.31f, 0.23f, 0.14f, 1.00f);
            colors[ImGuiCol_SliderGrab]             = Vec4(0.71f, 0.46f, 0.22f, 0.63f);
            colors[ImGuiCol_SliderGrabActive]       = Vec4(0.71f, 0.46f, 0.22f, 1.00f);
            colors[ImGuiCol_Button]                 = (ImVec4)button_color;
            colors[ImGuiCol_ButtonHovered]          = (ImVec4)button_hoveredColor;
            colors[ImGuiCol_ButtonActive]           = (ImVec4)button_activeColor;
            colors[ImGuiCol_Header]                 = Vec4(0.70f, 0.70f, 0.70f, 1.00f);
            colors[ImGuiCol_HeaderHovered]          = Vec4(0.89f, 0.65f, 0.11f, 0.96f);
            colors[ImGuiCol_HeaderActive]           = Vec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_Separator]              = Vec4(0.43f, 0.43f, 0.50f, 0.50f);
            colors[ImGuiCol_SeparatorHovered]       = Vec4(0.71f, 0.71f, 0.71f, 0.78f);
            colors[ImGuiCol_SeparatorActive]        = Vec4(1.00f, 0.62f, 0.00f, 1.00f);
            colors[ImGuiCol_ResizeGrip]             = Vec4(1.00f, 1.00f, 1.00f, 0.30f);
            colors[ImGuiCol_ResizeGripHovered]      = Vec4(1.00f, 1.00f, 1.00f, 0.60f);
            colors[ImGuiCol_ResizeGripActive]       = Vec4(1.00f, 1.00f, 1.00f, 0.90f);
            colors[ImGuiCol_Tab]                    = Vec4(0.58f, 0.54f, 0.50f, 0.86f);
            colors[ImGuiCol_TabHovered]             = Vec4(1.00f, 0.79f, 0.45f, 1.00f);
            colors[ImGuiCol_TabActive]              = Vec4(1.00f, 0.73f, 0.25f, 1.00f);
            colors[ImGuiCol_TabUnfocused]           = Vec4(0.53f, 0.53f, 0.53f, 0.97f);
            colors[ImGuiCol_TabUnfocusedActive]     = Vec4(0.79f, 0.79f, 0.79f, 1.00f);
            colors[ImGuiCol_DockingPreview]         = Vec4(1.00f, 0.70f, 0.09f, 0.70f);
            colors[ImGuiCol_DockingEmptyBg]         = Vec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_PlotLines]              = Vec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered]       = Vec4(0.90f, 0.70f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogram]          = Vec4(0.90f, 0.70f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered]   = Vec4(1.00f, 0.60f, 0.00f, 1.00f);
            colors[ImGuiCol_TextSelectedBg]         = Vec4(0.00f, 0.00f, 1.00f, 0.35f);
            colors[ImGuiCol_DragDropTarget]         = Vec4(1.00f, 1.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight]           = Vec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight]  = Vec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg]      = Vec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg]       = Vec4(0.20f, 0.20f, 0.20f, 0.55f);
            colors[ImGuiCol_TableBorderLight]       = Vec4(0.20f, 0.20f, 0.20f, 0.80f);
            colors[ImGuiCol_TableBorderStrong]      = Vec4(0.20f, 0.20f, 0.20f, 0.90f);
            colors[ImGuiCol_TableHeaderBg]          = Vec4(0.20f, 0.20f, 0.20f, 0.60f);
            colors[ImGuiCol_TableRowBg]             = Vec4(0.20f, 0.20f, 0.20f, 0.40f);
            colors[ImGuiCol_TableRowBgAlt]          = Vec4(0.20f, 0.20f, 0.20f, 0.20f);

            _style.WindowBorderSize   = 1.0f;
            _style.FrameBorderSize    = 1.0f;
            _style.FrameRounding      = 3.0f;
            _style.ChildRounding      = 3.0f;
            _style.WindowRounding     = 0.0f;
            _style.AntiAliasedFill    = true;
            _style.AntiAliasedLines   = true;
            _style.WindowPadding      = Vec2(10.0f,10.0f);
        }
    };
}