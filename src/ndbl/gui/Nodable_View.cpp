#include "Nodable_View.h"
#include "core/Event.h"
#include "core/Graph.h"
#include "gui/Action_Manager.h"
#include "gui/Action_Manager_View.h"
#include "gui/App_View.h"
#include "gui/Event.h"
#include "gui/Nodable.h"
#include "gui/Scope_View.h"
#include "gui/Texture_Manager.h"
#include "gui/View.h"
#include "tools/core/Event_Manager.h"
#include "tools/core/System.h"
#include "tools/gui/Config.h"
#include "tools/gui/Font_Manager.h"
#include "tools/gui/ImGuiEx.h"
#include "ndbl/gui/File.h"
#include "ndbl/gui/Graph_View.h"

namespace ndbl
{
    // private
    void            _nodableview_on_draw_splashscreen_content(App_View_State*);
    void            _nodableview_on_reset_layout(App_View_State*);
}

void ndbl::nodableview_init(App_View_State* view, App_State* app)
{
     // Init base (tools::App_View)
    tools::appview_init(&view->base, &app->base);

    // Connects to the base class signals
    view->base.signal_reset_layout.connect<_nodableview_on_reset_layout>(view);
    view->base.signal_draw_splashscreen_content.connect<_nodableview_on_draw_splashscreen_content>(view);

    // Load splashscreen image
    Config* cfg         = get_config();
    tools::Path path    = tools::Path::get_asset_path(cfg->ui_splashscreen_imagePath );
    view->logo          = tools::get_texture_manager()->load(path);
}

void ndbl::nodableview_deinit(App_View_State* view)
{
    // Disconnects from the base class signals
    view->base.signal_reset_layout.disconnect();
    view->base.signal_draw_splashscreen_content.disconnect();

    // Deinit base (tools::App_View)
    tools::appview_deinit(&view->base); // will release all textures
}

void ndbl::nodableview_update(App_View_State* view)
{
    File* current_file = view->app()->current_file;

    if( current_file != nullptr )
    {
        fileview_update(&current_file->view, view->base.dt_in_s);
    }
}

void ndbl::nodableview_draw(App_View_State* view)
{
    using namespace tools;

    VERIFY(view->logo != nullptr, "Logo is nullptr, did you call init_ex() ?");

    const float dt = view->base.dt_in_s;

    // note: we draw this view nested in base view's begin/end (similar to ImGui API).
    tools::appview_begin(&view->base);

    Event_Manager*  event_manager   = event_manager_get();
    Config*         cfg             = get_config();
    tools::Config*  tools_cfg       = tools::get_config();
    bool            redock_all      = true;
    File*           current_file    = view->app()->current_file;

    //----------------------------------------------------------------------------------------
    // Draw menu bar
    //----------------------------------------------------------------------------------------

    if (ImGui::BeginMenuBar())
    {
        History* current_file_history = current_file ? &current_file->history : nullptr;
        View_Selection selection;
        
        if ( current_file != nullptr )
        {
            auto* graph_view = componentbag_get<Graph_View>(&current_file->graph->component_bag);
            selection = graph_view->selection;
        }

        if (ImGui::BeginMenu("File"))
        {
            bool has_file = current_file != nullptr;
            bool is_current_file_content_dirty = current_file != nullptr && current_file->has_flags(File_Flag_NEEDS_TO_BE_SAVED);

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_NEW))
                event_manager_dispatch( event_manager, action->event);

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_BROWSE))
                event_manager_dispatch( event_manager, action->event);

            ImGui::Separator();

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_SAVE_AS, false, has_file))
                event_manager_dispatch( event_manager, action->event);

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_SAVE, false, has_file && is_current_file_content_dirty))
                event_manager_dispatch( event_manager, action->event);
            ImGui::Separator();

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_CLOSE, false, has_file))
                event_manager_dispatch( event_manager, action->event);

            auto auto_paste = has_file && current_file->view.experimental_clipboard_auto_paste;

            if (ImGui::MenuItem(ICON_FA_COPY "  Auto-paste clipboard", "", auto_paste, has_file ) && has_file )
            {
                fileview_set_experimental_clipboard_auto_paste(&current_file->view, !auto_paste);
            }
            
            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_REQUEST_EXIT))
                event_manager_dispatch( event_manager, action->event);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (current_file_history)
            {
                if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_UNDO))
                    event_manager_dispatch( event_manager, action->event);

                if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_REDO))
                    event_manager_dispatch( event_manager, action->event);

                ImGui::Separator();
            }
            if (ImGui::MenuItem("Delete", "Del.", false, !selection.empty() ))
            {
                auto user_data = new Event_Data__Selection();
                user_data->selected_items = selection;
                
                Event event{Event_Type_USER};
                event.user.code  = Event_Code_DELETE;
                event.user.data1 = user_data;
                event_manager_dispatch( event_manager, event );
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            //auto frame = ImGui::MenuItem("Frame All", "F");
            redock_all |= ImGui::MenuItem("Redock documents");

            ImGui::Separator();

            auto menu_item_node_view_detail = [current_file, cfg](View_Detail _detail, const char *_label) {
                if (ImGui::MenuItem(_label, "", cfg->ui_node_detail == _detail))
                {
                    cfg->ui_node_detail = _detail;
                    if (current_file != nullptr)
                    {
                        auto* graph_view = componentbag_get<Graph_View>(&current_file->graph->component_bag);
                        graphview_reset_all_properties(graph_view);
                    }
                }
            };

            ImGui::Text("View Detail:");
            ImGui::Indent();
            menu_item_node_view_detail(View_Detail_COMPACT  , "Compact");
            menu_item_node_view_detail(View_Detail_NORMAL   , "Normal");
            ImGui::Unindent();

            ImGui::Separator();
            view->show_properties_editor = ImGui::MenuItem(ICON_FA_COGS " Show Properties", "", view->show_properties_editor);
            view->show_imgui_demo = ImGui::MenuItem("Show ImGui Demo", "", view->show_imgui_demo);

            ImGui::Separator();

            const bool is_fullscreen = appview_is_fullscreen(&view->base);
            if (ImGui::MenuItem("Fullscreen", "", is_fullscreen ))
            {
                appview_set_fullscreen(&view->base, !is_fullscreen);
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Reset Layout", ""))
            {
                view->base.should_reset_layout = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Code"))
        {
            if(const Action* action = ImGuiEx::MenuItem_for_event_user_code(Event_Code_TOGGLE_ISOLATION_FLAGS, cfg->has_flags(Config_Flag_ISOLATION_ON)))
            {
                event_manager_dispatch( event_manager, action->event);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Graph"))
        {

            if(const Action* action = ImGuiEx::MenuItem_for_event_user_code( Event_Code_RESET_GRAPH_VIEW) )
            {
                event_manager_dispatch(event_manager, action->event);
            }

            if(const Action* action = ImGuiEx::MenuItem_for_event_user_code(Event_Code_ARRANGE_SELECTION,false, !selection.empty() ) )
            {
                event_manager_dispatch( event_manager, action->event);
            }

            if(const Action* action = ImGuiEx::MenuItem_for_event_user_code(Event_Code_TOGGLE_FOLDING, false, !selection.empty() ) )
            {
                event_manager_dispatch( event_manager, action->event);
            }

            if (ImGui::MenuItem("Expand/Collapse recursive", nullptr, false, !selection.empty() ))
            {
                if(const Action* action = ImGuiEx::MenuItem_for_event_user_code(Event_Code_TOGGLE_FOLDING,false, !selection.empty() ))
                {
                    event_manager_dispatch( event_manager, action->event);
                }
            }

            ImGui::Separator();
            {
                if(const Action* action = ImGuiEx::MenuItem_for_event_user_code(Event_Code_TOGGLE_ISOLATION_FLAGS, cfg->has_flags(Config_Flag_ISOLATION_ON)))
                {
                    event_manager_dispatch( event_manager, action->event);
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Developer"))
        {
            bool debug = cfg->flags & Config_Flag_DRAW_DEBUG_LINES;
            if ( ImGui::MenuItem("Debug Mode", "", debug ) )
            {
                cfg->tools_cfg->runtime_debug = !debug;
                cfg->clear_flags( Config_Flag_DRAW_DEBUG_LINES );
                cfg->set_flags( !debug * Config_Flag_DRAW_DEBUG_LINES);
                ImGuiEx::set_debug( !debug );
            }

            if ( ImGui::MenuItem("Limit FPS", "", tools_cfg->fps_limit_on ) )
            {
                tools_cfg->fps_limit_on = !tools_cfg->fps_limit_on;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Verbosity"))
            {
                auto menu_item_verbosity = [](Verbosity verbosity, const char* label)
                {
                    if (ImGui::MenuItem(label, "", get_log_verbosity() == verbosity))
                    {
                        set_log_verbosity(verbosity);
                    }
                };

                menu_item_verbosity(Verbosity_Diagnostic, "Verbose");
                menu_item_verbosity(Verbosity_Message, "Message");
                menu_item_verbosity(Verbosity_Warning, "Warning");
                menu_item_verbosity(Verbosity_Error,   "Error");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Experimental"))
            {
                auto checkbox_flag = [&](const char* label, Config_Flag_ flag )
                {
                    bool enabled = cfg->has_flags(flag);
                    if ( ImGui::Checkbox(label, &enabled) )
                    {
                        if ( !enabled )
                            cfg->clear_flags(flag);
                        else
                            cfg->set_flags(flag);
                    }
                };
                checkbox_flag("Hybrid history"       , Config_Flag_EXPERIMENTAL_HYBRID_HISTORY);
                checkbox_flag("Multi-Selection"      , Config_Flag_EXPERIMENTAL_MULTI_SELECTION);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("An issue ?"))
        {
            if (ImGui::MenuItem("Report on Github.com"))
            {
                system_open_url_async("https://github.com/berdal84/nodable/issues");
            }

            if (ImGui::MenuItem("Report by email"))
            {
                system_open_url_async("mail:berenger@42borgata.com");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Show Splash Screen", "F1"))
            {
                view->base.show_splashscreen = true;
            }

            if (ImGui::MenuItem("Browse source code"))
            {
                system_open_url_async("https://www.github.com/berdal84/nodable");
            }

            if (ImGui::MenuItem("Credits"))
            {
                system_open_url_async("https://github.com/berdal84/nodable#credits-");
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    //----------------------------------------------------------------------------------------
    // Draw window content
    //----------------------------------------------------------------------------------------

    // All windows are docked to a dockspace (defined in signal_reset_layout() )

    ImGuiID ds_root = view->base.dockspaces[Dockspace_ROOT];
    if( !view->app()->files.empty() )
    {
        //----------------------------------------------------------------------------------------
        // Draw tool bar
        //----------------------------------------------------------------------------------------

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 5.0f});

        if ( ImGui::Begin( cfg->ui_toolbar_window_label, NULL, flags ) )
        {
            Font_Manager*  font_manager  = get_font_manager();
            Event_Manager* event_manager = event_manager_get();
            const Vec2&   button_size   = cfg->ui_toolButton_size;

            ImGui::PopStyleVar();
            ImGui::PushFont(font_manager->get_font(Font_Slot_ToolBtn));
            ImGui::BeginGroup();

            // reset
            if (ImGui::Button(ICON_FA_UNDO " Reset Graph View", button_size)) {
                event_manager_dispatch( event_manager, Event_Code_RESET_GRAPH_VIEW );
            }
            ImGui::SameLine();

            // enter isolation mode
            if (ImGui::Button(cfg->has_flags(Config_Flag_ISOLATION_ON) ? ICON_FA_CROP " isolation mode: ON " : ICON_FA_CROP " isolation mode: OFF", button_size))
            {
                event_manager_dispatch( event_manager, Event_Code_TOGGLE_ISOLATION_FLAGS );
            }
            ImGui::SameLine();
            ImGui::EndGroup();

            ImGui::PopFont();
        }
        ImGui::End();

        //----------------------------------------------------------------------------------------
        // Draw file views (multiple files may be visible)
        //----------------------------------------------------------------------------------------

        for( File* file : view->app()->files )
        {
            ImGui::SetNextWindowDockID(ds_root, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar
                                        | ImGuiWindowFlags_UnsavedDocument * file->has_flags(File_Flag_NEEDS_TO_BE_SAVED);

            auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
            child_bg.w = 0;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

            bool open        = true;
            bool uncollapsed = ImGui::Begin(file_filename(file).c_str(), &open, window_flags);

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(1);

            if ( uncollapsed )
            {
                // Set current file if window is focused
                if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
                    if ( view->app()->current_file != file )
                        nodable_set_current_file(view->app(), file);

                // Draw content
                fileview_draw( &file->view, view->base.dt_in_s );
            }
            ImGui::End();

            if ( !open )
            {
                nodable_close_file(view->app(), file);
            }
        }

        //----------------------------------------------------------------------------------------
        // Draw file info panel
        //----------------------------------------------------------------------------------------
        
        if ( current_file != nullptr && ImGui::Begin( cfg->ui_file_info_window_label))
        {
            // Basic inFormation
            ImGui::Text("Current file:");
            ImGui::Indent();
            ImGui::TextWrapped("path: %s", current_file->path.string().c_str());
            ImGui::TextWrapped("set_size: %0.3f KiB", float(file_size(current_file)) / 1000.0f );
            ImGui::Unindent();
            ImGui::NewLine();

            // Statistics
            ImGui::Text("Graph statistics:");
            ImGui::Indent();
            ImGui::Text("Node count: %zu", current_file->graph->nodes.size());
            ImGui::Unindent();
            ImGui::NewLine();

            // Hierarchy
            Scope* scope = graph_root_scope(current_file->graph);
            VERIFY(scope, "An Scope root is required to draw the AST as an ImGui tree");
            TreeNode_Scope("Graph's Root Scope", scope);
        }

        ImGui::End();

        //----------------------------------------------------------------------------------------
        // Draw ImGui configuration windows
        //----------------------------------------------------------------------------------------

        if( !tools_cfg->runtime_debug )
        {
            if (ImGui::Begin( cfg->ui_imgui_config_window_label))
            {
                ImGui::ShowStyleEditor();
            }
            ImGui::End();
        }
        
        //----------------------------------------------------------------------------------------
        // Draw configuration window (to edit tools::Config and ndbl::Config)
        //----------------------------------------------------------------------------------------

        if (ImGui::Begin( cfg->ui_config_window_label))
        {
            const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

            ImGui::Text("Nodable Settings");
            if ( ImGui::Button("Reset Settings") )
            {
                cfg->reset();
            }

            if (ImGui::CollapsingHeader("Sizes", flags ))
            {
                ImGui::SliderFloat("set_size factor SM", &cfg->tools_cfg->size_factor[Size_SM], 0.0f, 5.0f);
                ImGui::SliderFloat("set_size factor MD", &cfg->tools_cfg->size_factor[Size_MD], 0.0f, 5.0f);
                ImGui::SliderFloat("set_size factor LR", &cfg->tools_cfg->size_factor[Size_LG], 0.0f, 5.0f);
                ImGui::SliderFloat("set_size factor XL", &cfg->tools_cfg->size_factor[Size_XL], 0.0f, 5.0f);
            }

            if (ImGui::CollapsingHeader("Nodes", flags ))
            {
                ImGui::Indent();
                if ( ImGui::CollapsingHeader("Colors", flags ))
                {
                    ImGui::ColorEdit4("default"     , &cfg->ui_node_fill_color[Node_Type_NULL].x );
                    ImGui::ColorEdit4("entry point" , &cfg->ui_node_fill_color[Node_Type_SCOPE].x );
                    ImGui::ColorEdit4("condition"   , &cfg->ui_node_fill_color[Node_Type_IF_ELSE].x );
                    ImGui::ColorEdit4("for loop"    , &cfg->ui_node_fill_color[Node_Type_FOR_LOOP].x );
                    ImGui::ColorEdit4("while loop"  , &cfg->ui_node_fill_color[Node_Type_WHILE_LOOP].x );
                    ImGui::ColorEdit4("variable"    , &cfg->ui_node_fill_color[Node_Type_VARIABLE].x );
                    ImGui::ColorEdit4("literal"     , &cfg->ui_node_fill_color[Node_Type_LITERAL].x );
                    ImGui::ColorEdit4("function"    , &cfg->ui_node_fill_color[Node_Type_FUNCTION].x );
                    ImGui::ColorEdit4("operator"    , &cfg->ui_node_fill_color[Node_Type_OPERATOR].x );
                    ImGui::Separator();
                    ImGui::ColorEdit4("highlighted"         , &cfg->ui_node_highlightedColor.x);
                    ImGui::ColorEdit4("shadow"              , &cfg->ui_node_shadowColor.x);
                    ImGui::ColorEdit4("border"              , &cfg->ui_slot_border_color.x);
                    ImGui::ColorEdit4("border (highlighted)", &cfg->ui_node_borderHighlightedColor.x);
                    ImGui::ColorEdit4("slot (in)"           , &cfg->ui_slot_color_light.x);
                    ImGui::ColorEdit4("slot (out)"          , &cfg->ui_slot_color_dark.x);
                    ImGui::ColorEdit4("slot (hovered)"      , &cfg->ui_slot_hovered_color.x);
                }

                if ( ImGui::CollapsingHeader("Node_Slots", flags ))
                {
                    ImGui::Text("Property Node_Slots:");
                    ImGui::SliderFloat("slot radius", &cfg->ui_slot_circle_radius_base, 5.0f, 10.0f);

                    ImGui::Separator();

                    ImGui::Text("Code Flow Node_Slots:");
                    ImGui::SliderFloat2("slot set_size##codeflow"   , &cfg->ui_slot_rectangle_size.x, 2.0f, 100.0f);
                    ImGui::SliderFloat("slot padding##codeflow" , &cfg->ui_slot_gap, 0.0f, 100.0f);
                    ImGui::SliderFloat("slot radius##codeflow"  , &cfg->ui_slot_border_radius, 0.0f, 40.0f);
                }

                if ( ImGui::CollapsingHeader("Misc.", flags ))
                {
                    ImGui::SliderFloat2("gap app (x and y-axis)", &cfg->ui_node_gap_base.x, 0.0f, 400.0f);
                    ImGui::SliderFloat("velocity" , &cfg->ui_node_speed, 1.0f, 10.0f);
                    ImGui::SliderFloat4("padding" , &cfg->ui_node_padding.x, 0.0f, 20.0f);
                    ImGui::SliderFloat("border width", &cfg->ui_node_borderWidth, 0.0f, 10.0f);
                    ImGui::SliderFloat("border width ratio (instructions)", &cfg->ui_node_instructionBorderRatio, 0.0f, 10.0f);
                }
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Wires / Code Flow", flags ))
            {
                ImGui::Text("Wires");
                ImGui::SliderFloat("thickness", &cfg->ui_wire_bezier_thickness, 0.5f, 10.0f);
                ImGui::SliderFloat2("roundness (min,max)", &cfg->ui_wire_bezier_roundness.x, 0.0f, 1.0f);
                ImGui::SliderFloat2("fade length (min,max in lensqr)", &cfg->ui_wire_bezier_fade_lensqr_range.x, 0.0f, 100000.0f);
                ImGui::ColorEdit4("color", &cfg->ui_wire_color.x);
                ImGui::ColorEdit4("shadow color", &cfg->ui_wire_shadowColor.x);

                ImGui::Separator();

                ImGui::Text("Code Flow");
                ImGui::ColorEdit4("color##codeflow", &cfg->ui_codeflow_color.x);
                ImGui::SliderFloat("thickness (ratio)##codeflow", &cfg->ui_codeflow_thickness_ratio, 0.1, 1.0);
            }

            if (ImGui::CollapsingHeader("Graph", flags ))
            {
                ImGui::InputFloat("view unfold duration (sec)", &cfg->graph_view_unfold_duration);
                ImGui::ColorEdit4("grid color (major)", &cfg->ui_graph_grid_color_major.x);
                ImGui::ColorEdit4("grid color (minor)", &cfg->ui_graph_grid_color_minor.x);
                ImGui::SliderInt("grid set_size", &cfg->ui_grid_size, 1, 500);
                ImGui::SliderInt("grid subdivisions", &cfg->ui_grid_subdiv_count, 1, 16);
            }

            if (ImGui::CollapsingHeader("Scope", flags ))
            {
                ImGui::SliderFloat4("padding (left, top, right, bottom)", &cfg->ui_scope_padding.left, 2, 25);
                ImGui::SliderFloat("border radius", &cfg->ui_scope_border_radius, 0, 20);
                ImGui::SliderFloat("border thickness", &cfg->ui_scope_border_thickness, 0, 4);
                ImGui::ColorEdit4("fill color (light)", &cfg->ui_scope_fill_col_light.x);
                ImGui::ColorEdit4("fill color (dark)", &cfg->ui_scope_fill_col_dark.x);
                ImGui::ColorEdit4("border color", &cfg->ui_scope_border_col.x);
            }

            if (ImGui::CollapsingHeader("Shortcuts", flags ))
            {
                action_manager_view_draw(action_manager_get());
            }

        #if TOOLS_POOL_ENABLE
            if ( tools_cfg->runtime_debug && ImGui::CollapsingHeader("Pool"))
            {
                ImGui::Text("Pool stats:");
                auto pool = get_pool_manager()->get_pool();
                ImGui::Text(" - Node.................... %8zu", pool->get_all<Node>().size() );
                ImGui::Text(" - Node_View............... %8zu", pool->get_all<Node_View>().size() );
                ImGui::Text(" - Physics................. %8zu", pool->get_all<Physics>().size() );
                ImGui::Text(" - Scope................... %8zu", pool->get_all<Scope>().size() );
            }
        #endif
        }
        ImGui::End();

        //----------------------------------------------------------------------------------------
        // Draw node properties window
        //----------------------------------------------------------------------------------------

        if (ImGui::Begin( cfg->ui_node_properties_window_label))
        {
            if( view->app()->current_file )
            {
                bool node_properties_changed = false;
                const Graph_View* graph_view = componentbag_get<Graph_View>(&view->app()->current_file->graph->component_bag); // Graph can't be null
                switch ( graph_view->selection.count(View_Type_NODE) )
                {
                    case 0:
                        break;
                    case 1:
                    {
                        ImGui::Indent(10.0f);
                        View first = graph_view->selection.first_of(View_Type_NODE);
                        node_properties_changed |= nodeview_draw_as_properties_panel(first.nodeview, &view->show_advanced_node_properties);
                        break;
                    }
                    default:
                        ImGui::Indent(10.0f);
                        ImGui::Text("Multi-Selection");
                }

                if ( node_properties_changed )
                {
                    view->app()->current_file->set_flags(File_Flag_TEXT_IS_DIRTY);
                }
            }
        }
        ImGui::End();
        
        //----------------------------------------------------------------------------------------
        // Draw help window
        //----------------------------------------------------------------------------------------
        {
        if (ImGui::Begin( cfg->ui_help_window_label))
        {
            Font_Manager* font_manager = get_font_manager();
            ImGui::PushFont(font_manager->get_font(Font_Slot_Heading));
            ImGui::Text("Welcome to Nodable!");
            ImGui::PopFont();
            ImGui::NewLine();
            ImGui::TextWrapped(
                    "Nodable is primary_child-able.\n"
                    "\n"
                    "Nodable allows you to edit a program using both text and graph paradigms."
                    "More precisely, it means:"
            );
            ImGuiEx::BulletTextWrapped("any change on the text will affect the graph");
            ImGuiEx::BulletTextWrapped("any change (structure or values) on the graph will affect the text");
            ImGuiEx::BulletTextWrapped(
                    "but keep in mind the app is the text, any change not affecting the text (such as child positions or orphan primary_child) will be lost.");
            ImGui::NewLine();
            ImGui::PushFont(font_manager->get_font(Font_Slot_Heading));
            ImGui::Text("Quick start");
            ImGui::PopFont();
            ImGui::NewLine();
            ImGui::TextWrapped("Nodable UI is designed as following:\n");
            ImGuiEx::BulletTextWrapped("On the left side a (light) text editor allows to edit source code.\n");
            ImGuiEx::BulletTextWrapped(
                    "At the center, there is the graph editor where you can create_new/delete/connect primary_child\n");
            ImGuiEx::BulletTextWrapped(
                    "On the right side (this side) you will find many tabs to manage additional config such as primary_child, interpreter, or app properties\n");
            ImGuiEx::BulletTextWrapped("At the top, between the menu and the editors, there is a tool bar."
                                        " There, few buttons will serve to compile, run and debug your program.");
            ImGuiEx::BulletTextWrapped("And at the bottom, below the editors, there is a status bar."
                                        " This bar will display important messages, warning, and errors. You can expand it to get older messages.");
        }
        ImGui::End();
        }
    }
    else if( !view->base.show_splashscreen ) // splashscreen has to be closed by the user to show the startup window
    {
        //----------------------------------------------------------------------------------------
        // Draw startup window (with file examples to open)
        //----------------------------------------------------------------------------------------

        ImGui::SetNextWindowDockID(ds_root, ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3f, 0.3f, 0.3f, 1.f));

        ImGui::Begin( cfg->ui_startup_window_label);
        {
            Event_Manager* event_manager = event_manager_get();
            Font_Manager*  font_manager  = get_font_manager();

            ImGui::PopStyleColor();

            ImVec2 center_area(500.0f, 250.0f);
            ImVec2 avail = ImGui::GetContentRegionAvail();

            ImGui::SetCursorPosX((avail.x - center_area.x) / 2);
            ImGui::SetCursorPosY((avail.y - center_area.y) / 2);

            ImGui::BeginChild("center_area", center_area);
            {
                ImGui::Indent(center_area.x * 0.05f);

                ImGui::PushFont(font_manager->get_font(Font_Slot_ToolBtn));
                ImGui::NewLine();

                ImVec2 btn_size(center_area.x * 0.44f, 40.0f);
                if (ImGui::Button(ICON_FA_FILE" New File", btn_size))
                    event_manager_dispatch( event_manager, Event_Type_FILE_NEW );
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER_OPEN" Open ...", btn_size))
                    event_manager_dispatch( event_manager, Event_Type_FILE_BROWSE );

                ImGui::NewLine();
                ImGui::Separator();
                ImGui::NewLine();

                ImGui::Text("%s", "Open an example");

                struct Example {
                    const char* label;
                    const char* path;
                };

                const std::array<Example, 4> examples = {
                    Example{ ICON_FA_BOOK" Single expressions    ", "examples/arithmetic.cpp" },
                    Example{ ICON_FA_BOOK" Multi instructions    ", "examples/multi-instructions.cpp" },
                    Example{ ICON_FA_BOOK" Conditional Structures", "examples/if-else.cpp" },
                    Example{ ICON_FA_BOOK" For Loop              ", "examples/for-loop.cpp" }
                };

                const ImVec2 example_btn_size(btn_size.x, btn_size.y * 0.66f);
                const int columns = 2;

                ImGui::NewLine();
                for (size_t i = 0; i < examples.size(); ++i )
                {
                    if (i % columns != 0) ImGui::SameLine();
                    if (ImGui::Button(examples[i].label, example_btn_size))
                    {
                        nodable_open_asset_file(view->app(), examples[i].path);
                    }
                }

                ImGui::PopFont();

                ImGui::NewLine();
                ImGui::Unindent();
            }
            ImGui::EndChild();
        }
        ImGui::End(); // Startup Window
    }
    
    appview_end(&view->base); // end the drawing
}

void ndbl::_nodableview_on_draw_splashscreen_content(App_View_State* view)
{

    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // Image
    ImGui::SameLine((ImGui::GetContentRegionAvail().x - (float)view->logo->width) * 0.5f); // center img
    tools::ImGuiEx::Image(view->logo);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {50.0f, 30.0f});

    // disclaimer
    ImGui::TextWrapped("DISCLAIMER: This software is a prototype, do not expect too much from it. Use at your own risk.");

    ImGui::NewLine();
    ImGui::NewLine();

    // credits
    const char *credit = "by Berdal84";
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(credit).x);
    ImGui::TextWrapped("%s", credit);

    // close on left/rightmouse btn click
    if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1))
    {
        view->base.show_splashscreen = false;
    }
    ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
}

void ndbl::_nodableview_on_reset_layout(App_View_State* view)
{
    using namespace tools;

    Config* cfg = get_config();

    // Dock windows to specific dockspace
    appview_dock_window( &view->base, cfg->ui_help_window_label             , Dockspace_RIGHT );
    appview_dock_window( &view->base, cfg->ui_config_window_label           , Dockspace_RIGHT );
    appview_dock_window( &view->base, cfg->ui_file_info_window_label        , Dockspace_RIGHT );
    appview_dock_window( &view->base, cfg->ui_node_properties_window_label  , Dockspace_RIGHT );
    appview_dock_window( &view->base, cfg->ui_interpreter_window_label      , Dockspace_RIGHT );
    appview_dock_window( &view->base, cfg->ui_imgui_config_window_label     , Dockspace_RIGHT );
    appview_dock_window( &view->base, cfg->ui_toolbar_window_label          , Dockspace_TOP   );
};

void ndbl::nodableview_save_screenshot(const App_View_State* view, const char* relative_path)
{
    TOOLS_LOG(tools::Verbosity_Message, "Test", "Taking screenshot ...\n");
    auto path = tools::Path::get_executable_path().parent_path() / "screenshots" / relative_path;
    if (!tools::Path::exists(path.parent_path()))
    {
        tools::Path::create_directories(path.parent_path());
    }
    appview_save_screenshot(&view->base, path);
}