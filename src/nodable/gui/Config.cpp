#include "Config.h"
#include "build_info.h"
#include "types.h"

using namespace ndbl;
using namespace fw;

ndbl::Config::Config()
{
    reset_default();
}

void ndbl::Config::reset_default()
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
    ui_node_borderWidth                   = 1.0f;
    ui_node_instructionBorderRatio        = 2.0f;
    ui_node_padding                       = { 8.0f, 4.0f, 4.0f, 4.0f };
    ui_node_propertyslot_radius           = 4.0f;
    ui_node_invokableColor                = Color(255, 199, 115);            // light orange
    ui_node_variableColor                 = Color( 171, 190, 255);           // blue
    ui_node_instructionColor              = Vec4(0.7f, 0.9f, 0.7f, 1.0f);    // green
    ui_node_literalColor                  = Vec4(0.75f, 0.75f, 0.75f, 1.0f); // light grey
    ui_node_condStructColor               = Vec4(1.f, 1.f, 1.f, 1.0f);       // white
    ui_node_fillColor                     = Vec4(0.7f, 0.9f, 0.7f, 1.0f);    // green
    ui_node_highlightedColor              = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ui_node_slot_border_color             = Vec4(0.2f, 0.2f, 0.2f, 1.0f);
    ui_node_borderColor                   = Vec4(1.0f, 1.0f, 1.0f, 0.8f);
    ui_node_borderHighlightedColor        = Vec4(1.0f, 1.0f, 1.0f, 0.8f);
    ui_node_shadowColor                   = Vec4(0.0f, 0.0f, 0.0f, 0.2f);
    ui_node_slot_hovered_color            = Color(200, 200, 200);
    ui_node_slot_color                    = Color(127, 127, 127);
    ui_node_spacing                       = 30.0f;
    ui_node_speed                         = 20.0f;
    ui_node_animation_subsample_count     = 4;  // 60fps * 4 gives virtually 240Fps for the animations
    ui_node_slot_size                     = {10.f, 10.f};
    ui_node_slot_gap                      = 4.0f;
    ui_node_slot_border_radius            = 0.1f;

    // wires
    ui_wire_bezier_roundness              = {0.25f, 2.0f};
    ui_wire_bezier_thickness              = 2.0f;
    ui_wire_bezier_fade_length_minmax     = {300.0f, 1000.0f};
    ui_wire_color                         = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ui_wire_shadowColor                   = ui_node_shadowColor;

    // code flow
    ui_codeflow_color                     = Color(150, 170, 140); // slightly green
    ui_codeflow_shadowColor               = Color(0, 0, 0, 64);
    ui_codeflow_thickness_ratio           = 0.45f; // relative to ui_node_slot_size.x

    // buttons
    ui_toolButton_size                    = Vec2(0.0f, 25.0f);

    // history
    ui_history_btn_spacing                = 1.f;
    ui_history_btn_height                 = 10.f;
    ui_history_btn_width_max              = 20.f;

    // overlay
    ui_overlay_margin                     = 10.0f;
    ui_overlay_indent                     = 5.0f;
    ui_overlay_window_bg_golor            = Vec4(0.9f,0.9f,0.9f,0.2f);
    ui_overlay_border_color               = Vec4(0,0,0,0);
    ui_overlay_text_color                 = Vec4(0,0,0,0.5f);

    // Window names
    ui_file_info_window_label             = "File";
    ui_help_window_label                  = "Help";
    ui_imgui_config_window_label          = "ImGui";
    ui_node_properties_window_label       = "Node";
    ui_config_window_label                = "Settings";
    ui_startup_window_label               = "Startup";
    ui_toolbar_window_label               = "Toolbar";
    ui_virtual_machine_window_label       = "VM";

    // Graph
    ui_graph_grid_color_major             = Color(0, 0, 0, 42);
    ui_graph_grid_color_minor             = Color(0, 0, 0, 17);
    ui_graph_grid_subdivs                 = 4;
    ui_graph_grid_size                    = 100.0f;

    // Misc.
    experimental_graph_autocompletion     = false;
    experimental_hybrid_history           = false;
    isolate_selection                     = false;
    graph_unfold_dt                       = 1.5f;
    graph_unfold_iterations               = 100;

    // NodableView
    common.dockspace_right_ratio       = 0.25f;
    common.dockspace_top_size          = 36.f;
    common.dockspace_bottom_size       = 100.f;

    const char *k_paragraph = "Paragraph";
    const char *k_heading   = "Heading 1";
    const char *k_code      = "Code";
    const char *k_tool      = "Tool Button";

    common.font_manager.text = {
                                      // id          , font_path                           , size , icons? , icons size
                                      { k_paragraph  , "fonts/JetBrainsMono-Regular.ttf"   , 16.0f, true   , 16.0f      },
                                      { k_heading    , "fonts/JetBrainsMono-Bold.ttf"      , 20.0f, true   , 20.0f      },
                                      { k_code       , "fonts/JetBrainsMono-Regular.ttf"   , 16.0f, true   , 16.0f      },
                                      { k_tool       , "fonts/JetBrainsMono-Medium.ttf"    , 16.0f, true   , 16.0f      }
    };

    common.font_manager.defaults[FontSlot_Paragraph] = k_paragraph;
    common.font_manager.defaults[FontSlot_Heading]   = k_heading;
    common.font_manager.defaults[FontSlot_Code]      = k_code;
    common.font_manager.defaults[FontSlot_ToolBtn]   = k_tool;
    common.font_manager.subsamples                   = 1.0f;
    common.font_manager.icon                         = {"Icons", "fonts/fa-solid-900.ttf" };
    common.app_window_label                          = BuildInfo::version_extended;
}