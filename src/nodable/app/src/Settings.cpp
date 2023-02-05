#include <nodable/app/Settings.h>
#include <fw/imgui/types.h>

using namespace ndbl;
using namespace fw;

Settings::Settings()
{
    ui_splashscreen_imagePath       = "images/nodable-logo-xs.png";
    ui_text_textEditorPalette       = {
            0xffffffff, // None
            0xffd69c56, // Keyword
            0xff00ff00, // Number
            0xff7070e0, // String
            0xff70a0e0, // Char literal
            0xffffffff, // Punctuation
            0xff409090, // Preprocessor
            0xffdddddd, // Identifier
            0xff9bc64d, // Known identifier
            0xffc040a0, // Preproc identifier
            0xff909090, // Comment (single line)
            0xff909090, // Comment (multi line)
            0x30000000, // Background
            0xffe0e0e0, // Cursor
            0x20ffffff, // Selection
            0x800020ff, // ErrorMarker
            0x40f08000, // Breakpoint
            0x88909090, // Line number
            0x40000000, // Current line fill
            0x40808080, // Current line fill (inactive)
            0x40a0a0a0, // Current line edge
    };

    // nodes
    ui_node_padding                = 6.0f;
    ui_node_propertyConnectorRadius  = 5.0f;
    ui_node_invokableColor         = ImColor(255, 199, 115);          // light orange
    ui_node_variableColor          = ImColor( 171, 190, 255);         // blue
    ui_node_instructionColor       = vec4(0.7f, 0.9f, 0.7f, 1.0f);    // green
    ui_node_literalColor           = vec4(0.75f, 0.75f, 0.75f, 1.0f); // light grey
    ui_node_fillColor              = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ui_node_highlightedColor       = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ui_node_borderColor            = vec4(0.2f, 0.2f, 0.2f, 1.0f);
    ui_node_borderHighlightedColor = vec4(1.0f, 1.0f, 1.0f, 0.8f);
    ui_node_shadowColor            = vec4(0.0f, 0.0f, 0.0f, 0.2f);
    ui_node_nodeConnectorHoveredColor = ImColor(200, 200, 200);
    ui_node_nodeConnectorColor     = ImColor(127, 127, 127);
    ui_node_spacing                = 30.0f;
    ui_node_speed                  = 8.0f;
    ui_node_animation_subsample_count = 4;  // 60fps * 4 gives virtually 240Fps for the animations
    ui_node_connector_height       = 20.0f;
    ui_node_connector_padding      = 2.0f;
    ui_node_connector_width        = ui_node_connector_height;

    // wires
    ui_wire_bezier_roundness        = 0.5f;
    ui_wire_bezier_thickness        = 2.0f;
    ui_wire_displayArrows           = false;
    ui_wire_fillColor               = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ui_wire_shadowColor             = ui_node_shadowColor;

    // code flow
    ui_codeFlow_lineColor           = ImColor(200, 255, 200, 50);
    ui_codeFlow_lineShadowColor     = ImColor(0, 0, 0, 64);

    // buttons
    ui_button_color                 = vec4(0.50f, 0.50f, 0.50f, 0.63f);
    ui_button_hoveredColor          = vec4(0.70f, 0.70f, 0.70f, 0.95f);
    ui_button_activeColor           = vec4(0.98f, 0.73f, 0.29f, 0.95f);
    ui_toolButton_size              = vec2(0.0f, 25.0f);

    // history
    ui_history_btn_spacing            = 1.f;
    ui_history_btn_height             = 10.f;
    ui_history_btn_width_max          = 20.f;

    // overlay
    ui_overlay_margin                = 10.0f;
    ui_overlay_indent                 = 5.0f;
    ui_overlay_window_bg_golor        = vec4(0.9f,0.9f,0.9f,0.2f);
    ui_overlay_border_color           = vec4(0,0,0,0);
    ui_overlay_text_color             = vec4(0,0,0,0.5f);

    // Misc.
    experimental_graph_autocompletion = false;
    experimental_hybrid_history       = false;
    isolate_selection                 = false;

    // AppView
    fw_app_view.dockspace_right_ratio = 0.25f;
    fw_app_view.dockspace_down_size   = 15.f * 2.f; // for 2 lines
    {
        constexpr const char *k_paragraph = "Paragraph";
        constexpr const char *k_heading   = "Heading 1";
        constexpr const char *k_code      = "Code";
        constexpr const char *k_tool      = "Tool Button";

        fw_app_view.fonts = {
                // id          , font_path                          , size , icons? , icons size
                { k_paragraph  , "fonts/JetBrainsMono-Medium.ttf"   , 18.0f, true   , 18.0f      },
                { k_heading    , "fonts/JetBrainsMono-Bold.ttf"     , 25.0f, true   , 18.0f      },
                { k_code       , "fonts/JetBrainsMono-Regular.ttf"  , 18.0f, true   , 18.0f      },
                { k_tool       , "fonts/JetBrainsMono-Bold.ttf"     , 16.0f, true   , 14.0f      }
        };

        fw_app_view.fonts_default[FontSlot_Paragraph] = k_paragraph;
        fw_app_view.fonts_default[FontSlot_Heading]   = k_heading;
        fw_app_view.fonts_default[FontSlot_Code]      = k_code;
        fw_app_view.fonts_default[FontSlot_ToolBtn]   = k_tool;
    }
    fw_app_view.icon_font = {"Icons", "fonts/fa-solid-900.ttf" };
}

void Settings::patch_imgui_style(ImGuiStyle& _style)
{
    vec4* colors = _style.Colors;
    colors[ImGuiCol_Text]                   = vec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = vec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_WindowBg]               = vec4(0.76f, 0.76f, 0.76f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg]         = vec4(0.64f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ChildBg]                = vec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_PopupBg]                = vec4(0.66f, 0.66f, 0.66f, 1.00f);
    colors[ImGuiCol_Border]                 = vec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = vec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_FrameBg]                = vec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = vec4(0.90f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = vec4(0.90f, 0.65f, 0.65f, 1.00f);
    colors[ImGuiCol_TitleBg]                = vec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = vec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = vec4(0.49f, 0.63f, 0.69f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = vec4(0.60f, 0.60f, 0.60f, 0.98f);
    colors[ImGuiCol_ScrollbarBg]            = vec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = vec4(0.61f, 0.61f, 0.62f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = vec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = vec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_CheckMark]              = vec4(0.31f, 0.23f, 0.14f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = vec4(0.71f, 0.46f, 0.22f, 0.63f);
    colors[ImGuiCol_SliderGrabActive]       = vec4(0.71f, 0.46f, 0.22f, 1.00f);
    colors[ImGuiCol_Button]                 = ui_button_color;
    colors[ImGuiCol_ButtonHovered]          = ui_button_hoveredColor;
    colors[ImGuiCol_ButtonActive]           = ui_button_activeColor;
    colors[ImGuiCol_Header]                 = vec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = vec4(0.89f, 0.65f, 0.11f, 0.96f);
    colors[ImGuiCol_HeaderActive]           = vec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Separator]              = vec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = vec4(0.71f, 0.71f, 0.71f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = vec4(1.00f, 0.62f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = vec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered]      = vec4(1.00f, 1.00f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = vec4(1.00f, 1.00f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab]                    = vec4(0.58f, 0.54f, 0.50f, 0.86f);
    colors[ImGuiCol_TabHovered]             = vec4(1.00f, 0.79f, 0.45f, 1.00f);
    colors[ImGuiCol_TabActive]              = vec4(1.00f, 0.73f, 0.25f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = vec4(0.53f, 0.53f, 0.53f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive]     = vec4(0.79f, 0.79f, 0.79f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = vec4(1.00f, 0.70f, 0.09f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]         = vec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = vec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = vec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = vec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = vec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = vec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = vec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = vec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = vec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = vec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = vec4(0.20f, 0.20f, 0.20f, 0.55f);

    _style.WindowBorderSize   = 1.0f;
    _style.FrameBorderSize    = 1.0f;
    _style.FrameRounding      = 3.0f;
    _style.ChildRounding      = 3.0f;
    _style.WindowRounding     = 0.0f;
    _style.AntiAliasedFill    = true;
    _style.AntiAliasedLines   = true;
    _style.WindowPadding      = vec2(10.0f,10.0f);
}

Settings& Settings::get_instance()
{
    static Settings instance;
    return instance;
}