#pragma once

#include "Color.h"
#include "geometry/Vec2.h"
#include "geometry/Vec4.h"

#include "Font_Manager_Config.h"
#include "Size.h"

namespace tools
{
    typedef u8_t Debug_Flags;
    enum Debug_Flags_ : int
    {
        Debug_Flags_NONE                        = 0,
        Debug_Flags_DRAW_IMGUIEX_DEBUG_LINES    = 1 << 0,
        Debug_Flags_SHOW_IMGUI_CONFIG_WINDOW    = 1 << 1,
        Debug_Flags_DRAW_LAYOUT_DEBUG_LINES     = 1 << 2,
        Debug_Flags_ALL                         = ~Debug_Flags_NONE
    };

    // Framework configuration
    struct Config
    {
        constexpr static tools::Vec4 COLOR_ERROR {1.f, 0.f, 0.f};

        Config() = default;

        const char*           app_default_title        = "Default App Title";
        Debug_Flags           debug_flags              = Debug_Flags_NONE;
        bool                  fps_limit_on             = true;
        float                 fps_limit                = 60;
        u32_t                 dt_cap                   = 1000 / 60; // in ms
        Color                 background_color         {0,0,0};
        Vec4                  button_activeColor       { 0.98f, 0.73f, 0.29f, 0.95f}; // orange
        Vec4                  button_hoveredColor      { 0.70f, 0.70f, 0.70f, 0.95f}; // light grey
        Vec4                  button_color             {0.50f, 0.50f, 0.50f, 0.63f}; // grey
        const char*           splashscreen_window_label= "##Splashscreen";
        bool                  show_splashscreen_default = true;
        bool                  imgui_demo               = false;
        float                 dockspace_bottom_size    = 120.f;
        float                 dockspace_top_size       = 48.f;
        float                 dockspace_right_ratio    = 0.3f;
        size_t                log_message_display_max_count = 500;
        std::array<float, Size_COUNT>  size_factor= {
            0.5f, // SM
            1.0f,
            1.25f,
            2.0f, // LG
        };
        std::array<Vec4, Verbosity_COUNT> log_color
        {
            Vec4(0.5f, 0.0f, 0.0f, 1.0f), // red
            Vec4(0.5f, 0.0f, 0.5f, 1.0f), // violet
            Vec4(0.5f, 0.5f, 0.5f, 1.0f), // grey
            Vec4(0.0f, 0.5f, 0.0f, 1.0f)  // green
        };
        Font_Manager_Config font_manager
        {
            {{
                    "default",                  // id
                    "fonts/CenturyGothic.ttf",  // path
                    18.0f,                      // size in px.
                    true,                       // include icons?
                    18.0f                       // icons size in px.
            }},
            {
                    "default",// Font_Slot_Paragraph
                    "default",// Font_Slot_Heading
                    "default",// Font_Slot_Code
                    "default" // Font_Slot_ToolBtn
            },
            {
                "FA-solid-900",                  // Icon font name
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

    Config* config_init();  // create a new configuration and set it as current
    void    config_shutdown(); // destroy the current configuration
    Config* config();     // Get the current config, create_config() must be called first.
}