#pragma once

#include <string>

#include "tools/core/Color.h"
#include "tools/core/geometry/Vec2.h"
#include "tools/core/geometry/Vec4.h"

#include "FontManagerConfig.h"

namespace tools
{
    // Framework configuration
    struct Config
    {
        Config() = default;
        Config(const Config&) = delete; // disable copy

        const char*           app_default_title        = "Default App Title";
        bool                  vsync                    = false;
        bool                  runtime_debug            = false;
        bool                  delta_time_limit         = true;
        u32_t                 delta_time_min           = 1000 / 60; // in ms
        Color                 background_color         {0,0,0};
        Vec4                  button_activeColor       { 0.98f, 0.73f, 0.29f, 0.95f}; // orange
        Vec4                  button_hoveredColor      { 0.70f, 0.70f, 0.70f, 0.95f}; // light grey
        Vec4                  button_color             {0.50f, 0.50f, 0.50f, 0.63f}; // grey
        const char*           splashscreen_window_label= "##Splashscreen";
        bool                  show_splashscreen_default = true;
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
        FontManagerConfig font_manager{
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
        Vec2   padding        {10.0f,10.0f};
        bool   antialiased    {true};
        float  window_rounding{0.f};
        float  frame_rounding {3.f};
        float  border_size    {1.f};
    };

    bool    has_config();
    Config* get_config();     // Get the current config, create_config() must be called first.
    Config* init_config();  // create a new configuration and set it as current
    void shutdown_config(); // destroy the current configuration
}