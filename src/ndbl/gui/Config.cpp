#include "Config.h"
#include "IconsFontAwesome5.h"
#include "ndbl/core/Event.h"
#include "ndbl/core/Node.h"
#include "ndbl/gui/Event.h"
#include "ndbl/gui/File_View.h"
#include "ndbl/gui/Layout.h"

namespace ndbl
{

static Config* g_config = {};

#define VERIFY_NDBLCONFIG_IS_INITIALIZED() VERIFY(g_config != nullptr, "Config is not initialized, did you call config_init() ?")

Config* config_init()
{
    ASSERT(g_config == nullptr);
    
    g_config = bdc::memory_new<Config>();
    config_reset();

    // (to create functions/operators from the API)
    // TODO: add a list of preset to create operators/functions
    // action_manager_add_action( label.c_str(), Shortcut{}, EventPayload_CreateNode{Event_Type_FUNCTION, invokable->get_sig() } );
    return g_config;
}

void config_reset()
{
    g_config->app_default_title         = "Default App Title";
    g_config->debug_flags               = Debug_Flags_NONE;
    g_config->fps_limit_on              = true;
    g_config->fps_limit                 = 60;
    g_config->dt_cap                    = 1000 / 60; // in ms
    g_config->background_color          = {0,0,0};
    g_config->button_activeColor        = { 0.98f, 0.73f, 0.29f, 0.95f}; // orange
    g_config->button_hoveredColor       = { 0.70f, 0.70f, 0.70f, 0.95f}; // light grey
    g_config->button_color              = {0.50f, 0.50f, 0.50f, 0.63f}; // grey
    g_config->splashscreen_window_label = "##Splashscreen";
    g_config->show_splashscreen_default = true;
    g_config->imgui_demo                = false;
    g_config->dockspace_bottom_size     = 120.f;
    g_config->dockspace_top_size        = 48.f;
    g_config->dockspace_right_ratio     = 0.3f;
    g_config->log_message_display_max_count = 500;

    g_config->size_factor = {
        0.5f, // SM
        1.0f,
        1.25f,
        2.0f, // LG
    };

    g_config->log_color = {
        Vec4(0.5f, 0.0f, 0.0f, 1.0f), // red
        Vec4(0.5f, 0.0f, 0.5f, 1.0f), // violet
        Vec4(0.5f, 0.5f, 0.5f, 1.0f), // grey
        Vec4(0.0f, 0.5f, 0.0f, 1.0f)  // green
    };

    g_config->font_manager = {
        {{
            "default",                  // id
            "fonts/CenturyGothic.ttf",  // path
            18.0f,                      // size in px.
            true,                       // include icons?
            18.0f                       // icons size in px.
        }},
        {
            "default", // Font_Slot_Paragraph
            "default", // Font_Slot_Heading
            "default", // Font_Slot_Code
            "default"  // Font_Slot_ToolBtn
        },
        {
            "FA-solid-900",           // Icon font name
            "fonts/fa-solid-900.ttf"  // Icon font path
        },
        1.0f  // subsampling
    };
    g_config->padding         = { 10.0f, 10.0f };
    g_config->antialiased     = true;
    g_config->window_rounding = 0.f;
    g_config->frame_rounding  = 3.f;
    g_config->border_size     = 1.f;

    g_config->ui_splashscreen_imagePath       = "images/nodable-logo-xs.png";
    g_config->ui_text_textEditorPalette       = {
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
    g_config->ui_node_borderWidth                   = 1.0f;
    g_config->ui_node_instructionBorderRatio        = 2.0f;
    g_config->ui_node_padding                       = Vec4{ 3.0f, 5.0f, 10.0f, 5.0f };
    g_config->ui_slot_circle_radius_base            = 4.0f;

    g_config->ui_node_highlightedColor              = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    g_config->ui_node_borderColor                   = Vec4(1.0f, 1.0f, 1.0f, 0.8f);
    g_config->ui_node_borderHighlightedColor        = Vec4(1.0f, 1.0f, 1.0f, 0.8f);
    g_config->ui_node_shadowColor                   = Vec4(0.0f, 0.0f, 0.0f, 0.2f);

    g_config->ui_node_fill_color = {};
    for (auto& col : g_config->ui_node_fill_color)
        col = Vec4(0.7f, 0.9f, 0.7f, 1.0f); // green;

    g_config->ui_node_fill_color[Node_Type_VARIABLE]     =  Color(171, 190, 255);
    g_config->ui_node_fill_color[Node_Type_VARIABLE_REF] =  g_config->ui_node_fill_color[Node_Type_VARIABLE];
    g_config->ui_node_fill_color[Node_Type_LITERAL]      =  Color(200, 200, 200);
    g_config->ui_node_fill_color[Node_Type_FUNCTION]     =  Color(255, 199, 115);
    g_config->ui_node_fill_color[Node_Type_OPERATOR]     =  g_config->ui_node_fill_color[Node_Type_FUNCTION];

    g_config->ui_slot_border_color                  = Vec4(0.2f, 0.2f, 0.2f, 1.0f);
    g_config->ui_slot_hovered_color                 = Color(200, 200, 200);
    g_config->ui_slot_color_light                   = Color(255, 255, 255);
    g_config->ui_slot_color_dark                    = Color(127, 127, 127);
    g_config->ui_node_gap_base                      = Vec2(40.0f, 40.f);
    g_config->ui_node_speed                         = 20.0f;
    g_config->ui_node_physics_frequency             = 120.f;
    g_config->ui_node_detail                        = View_Detail_NORMAL;
    g_config->ui_node_selected_rectangle_offset     = 4.f;
    g_config->ui_node_border_radius                 = 5.f;
    g_config->ui_slot_rectangle_size                = Vec2{10.f, 10.f};
    g_config->ui_slot_gap                           = 4.0f;
    g_config->ui_slot_border_radius                 = 0.1f;
    g_config->ui_slot_invisible_btn_expand_size     = 4.f; // +4px

    // wires
    g_config->ui_wire_bezier_roundness              = Vec2{0.25f, 2.0f};
    g_config->ui_wire_bezier_thickness              = 2.0f;
    g_config->ui_wire_bezier_fade_lensqr_range      = {300.0f*300.f, 1000.0f*1000.0f};
    g_config->ui_wire_color                         = Color(255, 255, 255);
    g_config->ui_wire_shadowColor                   = g_config->ui_node_shadowColor;

    // code flow
    g_config->ui_codeflow_color                     = Color(150, 170, 140); // slightly green
    g_config->ui_codeflow_shadowColor               = Color(0, 0, 0, 64);
    g_config->ui_codeflow_thickness_ratio           = 0.45f; // relative to ui_slot_rectangle_size.x

    // buttons
    g_config->ui_toolButton_size                    = Vec2(0.0f, 25.0f);

    // history
    g_config->ui_history_btn_spacing                = 1.f;
    g_config->ui_history_btn_height                 = 10.f;
    g_config->ui_history_btn_width_max              = 20.f;
    g_config->ui_history_size_max                   = 200;

    // overlay
    g_config->ui_textview_padding                     = 10.0f;
    g_config->ui_overlay_indent                     = 5.0f;
    g_config->ui_overlay_window_bg_golor            = Vec4(0.9f,0.9f,0.9f,0.2f);
    g_config->ui_overlay_border_color               = Vec4(0,0,0,0);
    g_config->ui_overlay_text_color                 = Vec4(0,0,0,0.5f);

    // Window names
    g_config->ui_file_info_window_label             = "File";
    g_config->ui_help_window_label                  = "Help";
    g_config->ui_imgui_config_window_label          = "ImGui";
    g_config->ui_node_properties_window_label       = "Node";
    g_config->ui_config_window_label                = "Settings";
    g_config->ui_startup_window_label               = "Startup";
    g_config->ui_toolbar_window_label               = "Toolbar";
    g_config->ui_interpreter_window_label           = "VM";

    // Scopes
    g_config->ui_scope_padding                      = padding(10.f);
    g_config->ui_scope_border_radius                = 7.f;
    g_config->ui_scope_border_thickness             = 3.f;
    g_config->ui_scope_gap_base                     = 10.f;
    g_config->ui_scope_fill_col_light               = Color(100, 100, 100);
    g_config->ui_scope_fill_col_dark                = Color(70,70,70);
    g_config->ui_scope_border_col                   = Color(255,255,255,40);

    // Graph
    g_config->ui_graph_grid_color_major             = Color(0, 0, 0, 42);
    g_config->ui_graph_grid_color_minor             = Color(0, 0, 0, 17);
    g_config->ui_grid_subdiv_count                  = 4;
    g_config->ui_grid_size                          = 100.0f;

    // Misc.
    g_config->flags                                 = Config_Flag_EXPERIMENTAL_MULTI_SELECTION;                                          
    g_config->graph_view_unfold_duration            = 1.0f; // in sec.

    // NodableView
    g_config->dockspace_right_ratio       = 0.25f;
    g_config->dockspace_top_size          = 36.f;
    g_config->dockspace_bottom_size       = 110.f;

    const char *k_paragraph = "Paragraph";
    const char *k_heading   = "Heading 1";
    const char *k_code      = "Code";
    const char *k_tool      = "Tool Button";

    g_config->font_manager.text = {
        // id          , font_path                           , size , icons? , icons size
        { k_paragraph  , "fonts/JetBrainsMono-Regular.ttf"   , 16.0f, true   , 16.0f      },
        { k_heading    , "fonts/JetBrainsMono-Bold.ttf"      , 20.0f, true   , 20.0f      },
        { k_code       , "fonts/JetBrainsMono-Regular.ttf"   , 16.0f, true   , 16.0f      },
        { k_tool       , "fonts/JetBrainsMono-Medium.ttf"    , 16.0f, true   , 16.0f      }
    };

    g_config->font_manager.defaults[Font_Slot_Paragraph] = k_paragraph;
    g_config->font_manager.defaults[Font_Slot_Heading]   = k_heading;
    g_config->font_manager.defaults[Font_Slot_Code]      = k_code;
    g_config->font_manager.defaults[Font_Slot_ToolBtn]   = k_tool;
    g_config->font_manager.subsamples                   = 1.0f;
    g_config->font_manager.icon                         = {"Icons", "fonts/fa-solid-900.ttf" };
    g_config->app_default_title = NDBL_APP_NAME " " NDBL_BUILD_REF " - Built " __DATE__ " at " __TIME__;

    // Actions
    g_config->actions = {
        {{ Event_Type_FILE_SAVE         }, ICON_FA_SAVE " Save"         , {SDLK_s, KMOD_CTRL }},
        {{ Event_Type_FILE_SAVE_AS      }, ICON_FA_SAVE " Save as"      , {SDLK_s, KMOD_CTRL }},
        {{ Event_Type_FILE_CLOSE        }, ICON_FA_TIMES "  Close"      , {SDLK_w, KMOD_CTRL }},
        {{ Event_Type_FILE_BROWSE       }, ICON_FA_FOLDER_OPEN " Open"  , {SDLK_o, KMOD_CTRL }},
        {{ Event_Type_FILE_NEW          }, ICON_FA_FILE " New"          , {SDLK_n, KMOD_CTRL }},
        {{ Event_Type_REQUEST_EXIT      }, ICON_FA_SIGN_OUT_ALT " Exit" , {SDLK_F4, KMOD_ALT }},
        {{ Event_Type_UNDO              }, "Undo"                       , {SDLK_z, KMOD_CTRL }},
        {{ Event_Type_REDO              }, "Redo"                       , {SDLK_y, KMOD_CTRL }},
        {{ Event_Type_RESET_GRAPH_VIEW  }, "Reset Graph"                , {SDLK_F5, KMOD_NONE }},
        {{ Event_Type_DELETE            }, "Delete Selection"           , {SDLK_DELETE, KMOD_NONE }},
        {{ Event_Type_RESET_LAYOUT              }, "Arrange Selection"  , {SDLK_a, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR},
        {{ Event_Type_TOGGLE_FOLDING            }, "Fold Selection"     , {SDLK_x, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR},
        {{ Event_Type_SELECT_NEXT               }, "Next"               , {SDLK_n, KMOD_NONE }},
        {{ Event_Type_TOGGLE_ISOLATION_FLAGS    }, "Isolation"          , {SDLK_i, KMOD_CTRL }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_TEXT_EDITOR},
        {{ Event_Type_MOVE                      }, "Drag whole "        , {SDLK_SPACE, KMOD_NONE, "Space + Drag" }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR},
        {{ Event_Type_FRAME_SELECTION           }, "Frame Selection"    , {SDLK_f, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR},
        {{ Event_Type_FRAME_SELECTION           }, "Frame All"          , {SDLK_f, KMOD_LCTRL }},
        {{ Event_Type_TOGGLE_HELP               }, "Splashscreen"       , {SDLK_F1 }},    
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_RETURN      }, ICON_FA_CODE " Return Statement"        },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_IF_ELSE     }, ICON_FA_CODE " Condition"               },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_FOR_LOOP    }, ICON_FA_CODE " For Loop"                },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_WHILE_LOOP  }, ICON_FA_CODE " While Loop"              },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_SCOPE       }, ICON_FA_CODE " Scope"                   },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_ROOT        }, ICON_FA_CODE " Entry Point"             },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_VARIABLE    }, ICON_FA_DATABASE " Boolean Variable"    },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_VARIABLE    }, ICON_FA_DATABASE " Double Variable"     },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_VARIABLE    }, ICON_FA_DATABASE " Integer Variable"    },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_VARIABLE    }, ICON_FA_DATABASE " String Variable"     },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_LITERAL     }, ICON_FA_FILE " Boolean Literal"         },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_LITERAL     }, ICON_FA_FILE " Double Literal"          },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_LITERAL     }, ICON_FA_FILE " Integer Literal"         },
        {{ .type = Event_Type_NEW_NODE, .code = Node_Type_LITERAL     }, ICON_FA_FILE " String Literal"          }
    };
}

void config_shutdown()
{
    VERIFY_NDBLCONFIG_IS_INITIALIZED();
    bdc::memory_delete(g_config);
    g_config = nullptr;
}

Config* config()
{
    VERIFY_NDBLCONFIG_IS_INITIALIZED();
    return g_config;
}

float Config::ui_codeflow_thickness() const
{
    return ui_slot_rectangle_size.x * ui_codeflow_thickness_ratio;
}

Vec2 Config::ui_node_gap(Size size) const
{
    return ui_node_gap_base * g_config->size_factor[size];
}

float Config::ui_slot_circle_radius(Size size) const
{
    return ui_slot_circle_radius_base * g_config->size_factor[size];
}

Vec4& Config::ui_slot_color(Node_Slot::Flags slot_flags)
{
    if ( (slot_flags & Node_Slot::Flag_INPUT) == Node_Slot::Flag_INPUT )
        return ui_slot_color_light;

    return ui_slot_color_dark;
}

float Config::ui_scope_gap(Size size) const
{
    return ui_scope_gap_base * g_config->size_factor[size];
}

} // namespace ndbl