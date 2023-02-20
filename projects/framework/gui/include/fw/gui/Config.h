#pragma once
#include <imgui.h>
#include <string>
#include <fw/gui/FontManager.h>

namespace fw
{

    // NodableView's configuration
    struct Config {

        Config() = default;
        Config(const Config&) = delete; // disable copy

        std::string           app_window_label         = "Framework NodableView";
        float                 min_frame_time           = 1.0f / 60.0f;            // limit to 60fps
        ImColor               background_color         = ImColor(0.f,0.f,0.f);
        ImVec4                button_activeColor       = ImVec4(0.98f, 0.73f, 0.29f, 0.95f); // orange
        ImVec4                button_hoveredColor      = ImVec4(0.70f, 0.70f, 0.70f, 0.95f); // light grey
        ImVec4                button_color             = ImVec4(0.50f, 0.50f, 0.50f, 0.63f); // grey
        const char*           splashscreen_window_label= "##Splashscreen";
        bool                  splashscreen             = true; // hide/show splashscreen
        bool                  imgui_demo               = false;
        float                 dockspace_bottom_size    = 48.f;
        float                 dockspace_top_size       = 48.f;
        float                 dockspace_right_ratio    = 0.3f;
        size_t                log_tooltip_max_count    = 25;
        std::array<
                ImVec4,
                fw::log::Verbosity_COUNT> log_color         {
                        ImVec4(0.5f, 0.0f, 0.0f, 1.0f), // red
                        ImVec4(0.5f, 0.0f, 0.5f, 1.0f), // violet
                        ImVec4(0.5f, 0.5f, 0.5f, 1.0f), // grey
                        ImVec4(0.0f, 0.5f, 0.0f, 1.0f)  // green
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
            colors[ImGuiCol_Text]                   = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_TextDisabled]           = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
            colors[ImGuiCol_WindowBg]               = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
            colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.64f, 0.24f, 0.24f, 1.00f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
            colors[ImGuiCol_PopupBg]                = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
            colors[ImGuiCol_Border]                 = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
            colors[ImGuiCol_BorderShadow]           = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
            colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.90f, 0.80f, 0.80f, 1.00f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.90f, 0.65f, 0.65f, 1.00f);
            colors[ImGuiCol_TitleBg]                = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_TitleBgActive]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.49f, 0.63f, 0.69f, 1.00f);
            colors[ImGuiCol_MenuBarBg]              = ImVec4(0.60f, 0.60f, 0.60f, 0.98f);
            colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
            colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.61f, 0.61f, 0.62f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
            colors[ImGuiCol_CheckMark]              = ImVec4(0.31f, 0.23f, 0.14f, 1.00f);
            colors[ImGuiCol_SliderGrab]             = ImVec4(0.71f, 0.46f, 0.22f, 0.63f);
            colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.71f, 0.46f, 0.22f, 1.00f);
            colors[ImGuiCol_Button]                 = button_color;
            colors[ImGuiCol_ButtonHovered]          = button_hoveredColor;
            colors[ImGuiCol_ButtonActive]           = button_activeColor;
            colors[ImGuiCol_Header]                 = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(0.89f, 0.65f, 0.11f, 0.96f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
            colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.71f, 0.71f, 0.71f, 0.78f);
            colors[ImGuiCol_SeparatorActive]        = ImVec4(1.00f, 0.62f, 0.00f, 1.00f);
            colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
            colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
            colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
            colors[ImGuiCol_Tab]                    = ImVec4(0.58f, 0.54f, 0.50f, 0.86f);
            colors[ImGuiCol_TabHovered]             = ImVec4(1.00f, 0.79f, 0.45f, 1.00f);
            colors[ImGuiCol_TabActive]              = ImVec4(1.00f, 0.73f, 0.25f, 1.00f);
            colors[ImGuiCol_TabUnfocused]           = ImVec4(0.53f, 0.53f, 0.53f, 0.97f);
            colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
            colors[ImGuiCol_DockingPreview]         = ImVec4(1.00f, 0.70f, 0.09f, 0.70f);
            colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
            colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
            colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.55f);

            _style.WindowBorderSize   = 1.0f;
            _style.FrameBorderSize    = 1.0f;
            _style.FrameRounding      = 3.0f;
            _style.ChildRounding      = 3.0f;
            _style.WindowRounding     = 0.0f;
            _style.AntiAliasedFill    = true;
            _style.AntiAliasedLines   = true;
            _style.WindowPadding      = ImVec2(10.0f,10.0f);
        }
    };
}