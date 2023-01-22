#include <nodable/app/AppView.h>

#include <fw/Log.h>
#include <fw/System.h>

#include <nodable/app/Event.h>
#include <nodable/app/App.h>
#include <nodable/app/File.h>
#include <nodable/app/FileView.h>
#include <nodable/app/History.h>
#include <nodable/app/NodeView.h>
#include <nodable/app/Settings.h>
#include <nodable/app/build_info.h>
#include <nodable/app/constants.h>

#include <utility>

using namespace ndbl;
using namespace ndbl::assembly;

AppView::AppView(App* _app, fw::AppView::Conf _conf)
    : fw::AppView(_app, std::move(_conf))
    , m_logo(nullptr)
    , m_show_splashscreen(true)
    , m_splashscreen_title("##STARTUPSCREEN")
    , m_is_history_dragged(false)
    , m_show_properties_editor(false)
    , m_show_imgui_demo(false)
    , m_show_advanced_node_properties(false)
    , m_scroll_to_curr_instr(true)
{
}

AppView::~AppView()
{}

bool AppView::onInit()
{
    for(auto font_config : Settings::get_instance().ui_text_fonts)
    {
        if(!load_font(font_config)) return false;
    }
    return true;
}

bool AppView::onDraw()
{
    bool isMainWindowOpen = true;
    bool redock_all       = false;
    auto& app             = App::get_instance();
    auto& settings        = Settings::get_instance();
    File* current_file    = app.current_file();
    auto& virtual_machine = VirtualMachine::get_instance();

    // Startup Window
    draw_splashcreen();

    // Demo Window
    {
        if (m_show_imgui_demo){
            ImGui::SetNextWindowPos(fw::vec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&m_show_imgui_demo);
        }
    }

    // Fullscreen m_sdlWindow
    {

		// Get current file's history
		History* currentFileHistory = nullptr;
        if ( File* file = current_file )
        {
            currentFileHistory = file->get_history();
        }

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


        // Remove padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, fw::vec2(0.0f, 0.0f));

        ImGui::Begin("Nodable", &isMainWindowOpen, window_flags);
        {
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(2);


            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    bool has_file = current_file;
                    bool changed = current_file != nullptr && current_file->has_changed();
                    fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_new_file_triggered);
                    fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_browse_file_triggered);
                    ImGui::Separator();
                    fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_save_file_as_triggered, false, has_file);
                    fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_save_file_triggered, false, has_file && changed);
                    ImGui::Separator();
                    fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_close_file_triggered, false, has_file);

                    FileView *fileView = nullptr;
                    bool auto_paste;
                    if (has_file)
                    {
                        fileView = current_file->get_view();
                        auto_paste = fileView->experimental_clipboard_auto_paste();
                    }

                    if (ImGui::MenuItem(ICON_FA_COPY        "  Auto-paste clipboard", "", auto_paste, fileView))
                    {
                        fileView->experimental_clipboard_auto_paste(!auto_paste);
                    }

                    fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_exit_triggered);

                    ImGui::EndMenu();
                }

                bool vm_is_stopped = virtual_machine.is_program_stopped();
                if (ImGui::BeginMenu("Edit"))
                {
                    if (currentFileHistory)
                    {
                        fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_undo_triggered);
                        fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_redo_triggered);
                        ImGui::Separator();
                    }

                    auto has_selection = NodeView::get_selected() != nullptr;

                    if ( ImGui::MenuItem("Delete", "Del.", false, has_selection && vm_is_stopped) )
                    {
                        fw::EventManager::push_event(EventType_delete_node_action_triggered);
                    }

                    fw::ImGuiEx::MenuItemBindedToEvent(EventType_arrange_node_action_triggered, false, has_selection);
                    fw::ImGuiEx::MenuItemBindedToEvent(EventType_toggle_folding_selected_node_action_triggered, false, has_selection);

                    if ( ImGui::MenuItem("Expand/Collapse recursive", nullptr, false, has_selection) )
                    {
                        Event event{};
                        event.toggle_folding.type      = EventType_toggle_folding_selected_node_action_triggered;
                        event.toggle_folding.recursive = true;
                        fw::EventManager::push_event((fw::Event&)event);
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View"))
                {
                    //auto frame = ImGui::MenuItem("Frame All", "F");
                    redock_all |= ImGui::MenuItem("Redock documents");

                    ImGui::Separator();

                    auto menu_item_node_view_detail = [](NodeViewDetail _detail, const char* _label)
                    {
                        if (ImGui::MenuItem( _label , "",  NodeView::get_view_detail() == _detail))
                        {
                            NodeView::set_view_detail(_detail);
                        }
                    };

                    menu_item_node_view_detail(NodeViewDetail::Minimalist, "Minimalist View");
                    menu_item_node_view_detail(NodeViewDetail::Essential,  "Essential View");
                    menu_item_node_view_detail(NodeViewDetail::Exhaustive, "Exhaustive View");

                    ImGui::Separator();
                    m_show_properties_editor = ImGui::MenuItem(ICON_FA_COGS " Show Properties", "", m_show_properties_editor);
                    m_show_imgui_demo        = ImGui::MenuItem("Show ImGui Demo", "", m_show_imgui_demo);

                    ImGui::Separator();

                    if ( ImGui::MenuItem("Fullscreen", "", get_fullscreen() ) )
                    {
                        set_fullscreen(!get_fullscreen());
                    }
                    ImGui::Separator();

                    if( ImGui::MenuItem("Reset Layout", "") )
                    {
                        set_layout_initialized(false);
                    }

                    ImGui::Separator();

                    fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_toggle_isolate_selection, settings.isolate_selection);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Run"))
                {
                    bool vm_is_debugging = virtual_machine.is_debugging();

                    if (ImGui::MenuItem(ICON_FA_PLAY" Run", "", false, vm_is_stopped) )
                    {
                        app.run_program();
                    }

                    if (ImGui::MenuItem(ICON_FA_BUG" Debug", "", false, vm_is_stopped) )
                    {
                        app.debug_program();
                    }

                    if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT" Step Over", "", false, vm_is_debugging) )
                    {
                        app.step_over_program();
                    }

                    if (ImGui::MenuItem(ICON_FA_STOP" Stop", "", false, !vm_is_stopped) )
                    {
                        app.stop_program();
                    }

                    if (ImGui::MenuItem(ICON_FA_UNDO " Reset", "", false, vm_is_stopped))
                    {
                        app.reset_program();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Developer"))
                {
                    if (ImGui::BeginMenu("Verbosity"))
                    {
                        auto menu_item_verbosity = [](fw::Log::Verbosity _verbosity, const char* _label)
                        {
                            if (ImGui::MenuItem( _label , "", fw::Log::get_verbosity() == _verbosity))
                            {
                                fw::Log::set_verbosity(_verbosity);
                            }
                        };

                        menu_item_verbosity(fw::Log::Verbosity_Verbose, "Verbose");
                        menu_item_verbosity(fw::Log::Verbosity_Message, "Message (default)");
                        menu_item_verbosity(fw::Log::Verbosity_Warning, "Warning");
                        menu_item_verbosity(fw::Log::Verbosity_Error,   "Error");
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Experimental"))
                    {
                        ImGui::Checkbox( "Hybrid history"       , &settings.experimental_hybrid_history);
                        ImGui::Checkbox( "Graph auto-completion", &settings.experimental_graph_autocompletion);
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("An issue ?"))
                {
                    if (ImGui::MenuItem("Report on Github.com"))
                    {
                        fw::System::open_url_async("https://github.com/berdal84/Nodable/issues");
                    }

                    if (ImGui::MenuItem("Report by email"))
                    {
                        fw::System::open_url_async("mail:berenger@dalle-cort.fr");
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Help"))
                {
                    if (ImGui::MenuItem("Show Splash Screen", "F1"))
                    {
                        m_show_splashscreen = true;
                    }

                    if (ImGui::MenuItem("Browse source code"))
                    {
                        fw::System::open_url_async("https://www.github.com/berdal84/nodable");
                    }

                    if (ImGui::MenuItem("Credits"))
                    {
                        fw::System::open_url_async("https://github.com/berdal84/nodable#credits-");
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            if( current_file )
                draw_tool_bar();


            /*
             * Main Layout
             */

            ImGuiID ds_root   = ImGui::GetID("dockspace_main");
            ImGuiID ds_center = ImGui::GetID("dockspace_center");
            ImGuiID ds_right  = ImGui::GetID("dockspace_right");
            ImGuiID ds_bottom = ImGui::GetID("dockspace_bottom");

            if ( !get_layout_initialized() )
            {
                ImGui::DockBuilderRemoveNode(ds_root); // Clear out existing layout
                ImGui::DockBuilderAddNode(ds_root     , ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(ds_root , ImGui::GetMainViewport()->Size);
                ImGui::DockBuilderSplitNode(ds_root   , ImGuiDir_Down , 0.5f, &ds_bottom, &ds_center);
                ImGui::DockBuilderSetNodeSize(ds_bottom , fw::vec2(ImGui::GetMainViewport()->Size.x, settings.ui_dockspace_down_size));
                ImGui::DockBuilderSplitNode(ds_center , ImGuiDir_Right, settings.ui_dockspace_right_ratio, &ds_right, nullptr );

                ImGui::DockBuilderGetNode(ds_center)->HasCloseButton      = false;
                ImGui::DockBuilderGetNode(ds_right)->HasCloseButton       = false;
                ImGui::DockBuilderGetNode(ds_bottom)->HasCloseButton      = false;
                ImGui::DockBuilderGetNode(ds_bottom)->WantHiddenTabBarToggle = true;


                ImGui::DockBuilderDockWindow(k_status_window_name           , ds_bottom);
                ImGui::DockBuilderDockWindow(k_imgui_settings_window_name   , ds_right);
                ImGui::DockBuilderDockWindow(k_app_settings_window_name     , ds_right);
                ImGui::DockBuilderDockWindow(k_file_info_window_name        , ds_right);
                ImGui::DockBuilderDockWindow(k_vm_window_name               , ds_right);
                ImGui::DockBuilderDockWindow(k_node_props_window_name       , ds_right);
                ImGui::DockBuilderDockWindow(k_help_window_name       , ds_right);
                ImGui::DockBuilderFinish(ds_root);

                set_layout_initialized(true);
                redock_all              = true;
            }

            /*
            * Fill the layout with content
            */
            ImGui::DockSpace(ds_root);

            if( !app.has_files())
            {
                draw_startup_menu(ds_root);
            }
            else
            {
                draw_vm_view();
                draw_properties_editor();
                draw_imgui_style_editor();
                draw_file_info();
                draw_node_properties();
                draw_help_window();

                for (File* each_file : app.get_files() )
                {
                    draw_file_editor(ds_root, redock_all, each_file);
                }
            }
            draw_status_bar();
        }
        ImGui::End(); // Main window
    }
    fw::ImGuiEx::EndFrame();

    return false;
}

void AppView::draw_help_window() const
{
    if (ImGui::Begin(k_help_window_name))
    {
        ImGui::PushFont(get_font(FontSlot_Heading));
        ImGui::Text("Welcome to Nodable!");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextWrapped(
                "Nodable is node-able.\n"
                "\n"
                "Nodable allows you to edit a program using both text and graph paradigms."
                "More precisely, it means:"
                );
        fw::ImGuiEx::BulletTextWrapped("any change on the text will affect the graph");
        fw::ImGuiEx::BulletTextWrapped("any change (structure or values) on the graph will affect the text");
        fw::ImGuiEx::BulletTextWrapped("but keep in mind the state is the text, any change not affecting the text (such as node positions or orphan nodes) will be lost.");
        ImGui::NewLine();
        ImGui::PushFont(get_font(FontSlot_Heading));
        ImGui::Text("Quick start");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextWrapped("Nodable UI is designed as following:\n");
        fw::ImGuiEx::BulletTextWrapped("On the left side a (light) text editor allows to edit source code.\n");
        fw::ImGuiEx::BulletTextWrapped("At the center, there is the graph editor where you can create/delete/connect nodes\n");
        fw::ImGuiEx::BulletTextWrapped("On the right side (this side) you will find many tabs to manage additional settings such as node properties, virtual machine or app properties\n");
        fw::ImGuiEx::BulletTextWrapped("At the top, between the menu and the editors, there is a tool bar."
                           " There, few buttons will serve to compile, run and debug your program.");
        fw::ImGuiEx::BulletTextWrapped("And at the bottom, below the editors, there is a status bar."
                           " This bar will display important messages, warning, and errors. You can expand it to get older messages.");
    }
    ImGui::End();
}

void AppView::draw_imgui_style_editor() const
{
    if (ImGui::Begin(k_imgui_settings_window_name))
    {
        ImGui::ShowStyleEditor();
    }
    ImGui::End();
}

void AppView::draw_file_info() const
{
    if( auto current_file = App::get_instance().current_file() )
    {
        if( ImGui::Begin(k_file_info_window_name) )
        {
            if (current_file) {
                FileView *fileView = current_file->get_view();
                fileView->draw_info();
            } else {
                ImGui::Text("No open file");
            }

        }
        ImGui::End();
    }
}

void AppView::draw_node_properties()
{
    if (ImGui::Begin(k_node_props_window_name))
    {
        NodeView *view = NodeView::get_selected();
        if (view)
        {
            ImGui::Indent(10.0f);
            NodeView::draw_as_properties_panel(view, &m_show_advanced_node_properties);
        }
    }
    ImGui::End();
}

void AppView::draw_vm_view()
{
    if (ImGui::Begin(k_vm_window_name))
    {
        auto& vm = VirtualMachine::get_instance();
        
        ImGui::Text("Virtual Machine:");
        ImGui::SameLine();
        fw::ImGuiEx::DrawHelper( "%s", "The virtual machine - or interpreter - is a sort of implementation of \n"
                                   "an imaginary hardware able to run a set of simple instructions.");
        ImGui::Separator();

        const Code* code = vm.get_program_asm_code();

        // VM state
        {
            ImGui::Indent();
            ImGui::Text("State:         %s", vm.is_program_running() ? "running" : "stopped");
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper( "%s", "When virtual machine is running, you cannot edit the code or the graph.");
            ImGui::Text("Debug:         %s", vm.is_debugging() ? "ON" : "OFF");
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper( "%s", "When debugging is ON, you can run a program step by step.");
            ImGui::Text("Has program:   %s", code ? "YES" : "NO");
            if (code)
            {
            ImGui::Text("Program over:  %s", !vm.is_there_a_next_instr() ? "YES" : "NO");
            }
            ImGui::Unindent();
        }

        // VM Registers
        ImGui::Separator();
        ImGui::Text("CPU:");
        ImGui::SameLine();
        fw::ImGuiEx::DrawHelper( "%s", "This is the virtual machine's CPU"
                                   "\nIt contains few registers to store temporary values "
                                   "\nlike instruction pointer, last node's value or last comparison result");
        ImGui::Indent();
        {
            ImGui::Separator();
            ImGui::Text("registers:");
            ImGui::Separator();

            using assembly::Register;
            ImGui::Indent();

            auto draw_register_value = [&](Register _register)
            {
                ImGui::Text("%4s: %12s", assembly::to_string(_register), vm.read_cpu_register(_register).to_string().c_str() );
            };

            draw_register_value(Register::rax); ImGui::SameLine(); fw::ImGuiEx::DrawHelper( "%s", "primary accumulator");
            draw_register_value(Register::rdx); ImGui::SameLine(); fw::ImGuiEx::DrawHelper( "%s", "base register");
            draw_register_value(Register::eip); ImGui::SameLine(); fw::ImGuiEx::DrawHelper( "%s", "instruction pointer");

            ImGui::Unindent();
        }
        ImGui::Unindent();

        // Assembly-like code
        ImGui::Separator();
        ImGui::Text("Memory:"); ImGui::SameLine(); fw::ImGuiEx::DrawHelper( "%s", "Virtual Machine Memory.");
        ImGui::Separator();
        {
            ImGui::Indent();

            ImGui::Text("Bytecode:");
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper( "%s", "The bytecode is the result of the Compilation process."
                                       "\nAfter source code has been parsed to a syntax tree, "
                                       "\nthe tree (or graph) is converted by the Compiler to an Assembly-like code.");
            ImGui::Checkbox("Auto-scroll ?", &m_scroll_to_curr_instr);
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper( "%s", "to scroll automatically to the current instruction");
            ImGui::Separator();
            {
                ImGui::BeginChild("AssemblyCodeChild", ImGui::GetContentRegionAvail(), true );

                if ( code )
                {
                    auto current_instr = vm.get_next_instr();
                    for( Instruction* each_instr : code->get_instructions() )
                    {
                        auto str = Instruction::to_string(*each_instr );
                        if ( each_instr == current_instr )
                        {
                            if ( m_scroll_to_curr_instr && vm.is_program_running() )
                            {
                                ImGui::SetScrollHereY();
                            }
                            ImGui::TextColored( ImColor(200,0,0), ">%s", str.c_str() );
                            ImGui::SameLine();
                            fw::ImGuiEx::DrawHelper( "%s", "This is the next instruction to evaluate");
                        }
                        else
                        {
                            ImGui::Text(  " %s", str.c_str() );
                        }
                    }
                }
                else
                {
                    ImGui::TextWrapped("Nothing loaded, try to compile, run or debug.");
                    ImGui::SameLine();
                    fw::ImGuiEx::DrawHelper( "%s", "To see a compiled program here you need first to:"
                                               "\n- Select a piece of code in the text editor"
                                               "\n- Click on \"Compile\" button."
                                               "\n- Ensure there is no errors in the status bar (bottom).");
                }
                ImGui::EndChild();
            }
            ImGui::Unindent();
        }
    }
    ImGui::End();
}

void AppView::draw_startup_menu(ImGuiID dockspace_id)
{
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3,0.3,0.3, 1.0));

    ImGui::Begin("Startup");
    {
        auto& app = App::get_instance();

        ImGui::PopStyleColor();

        ImVec2 center_area(500.0f, 250.0f);
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImGui::SetCursorPosX( (avail.x - center_area.x) / 2);
        ImGui::SetCursorPosY( (avail.y - center_area.y) / 2);

        ImGui::BeginChild("center_area", center_area);
        {
            ImGui::Indent(center_area.x * 0.05f);

            ImGui::PushFont(get_font(FontSlot_ToolBtn));
            ImGui::NewLine();

            fw::vec2 btn_size(center_area.x * 0.44f, 40.0f);
            if( ImGui::Button(ICON_FA_FILE" New File", btn_size) ) fw::EventManager::push_event(fw::EventType_new_file_triggered);
            ImGui::SameLine();
            if( ImGui::Button(ICON_FA_FOLDER_OPEN" Open ...", btn_size) ) fw::EventManager::push_event(fw::EventType_browse_file_triggered);

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();

            ImGui::Text("%s", "Open an example");
            std::vector<std::pair<std::string, std::string>> examples;
            examples.emplace_back("Single expressions    "          , "examples/arithmetic.cpp");
            examples.emplace_back("Multi instructions    "          , "examples/multi-instructions.cpp");
            examples.emplace_back("Conditional Structures"          , "examples/if-else.cpp");
            examples.emplace_back("For Loop              "          , "examples/for-loop.cpp");

            int i = 0;
            fw::vec2 small_btn_size(btn_size.x, btn_size.y * 0.66f);

            for( auto [text, path] : examples)
            {
                std::string label;
                label.append(ICON_FA_BOOK" ");
                label.append(text);
                if( i++ % 2) ImGui::SameLine();
                if (ImGui::Button(label.c_str(), small_btn_size))
                {
                    std::string each_path = app.compute_asset_path(path.c_str());
                    app.open_file(each_path);
                }
            }

            ImGui::PopFont();

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::TextColored( fw::vec4(0,0,0, 0.30f), "%s", BuildInfo::version);
            ImGui::Unindent();
        }
        ImGui::EndChild();
    }
    ImGui::End(); // Startup Window
}

void AppView::draw_file_editor(ImGuiID dockspace_id, bool redock_all, File* file)
{
    auto& app = App::get_instance();
    auto& vm  = VirtualMachine::get_instance();

    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags = (file->has_changed() ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, fw::vec2(0, 0));

    auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
    child_bg.w = 0;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

    bool is_window_open = true;
    bool visible = ImGui::Begin(file->get_name().c_str(), &is_window_open, window_flags);
    {
        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar();

        if (visible)
        {
            const bool is_current_file = app.is_current(file);

            if ( ! is_current_file && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                app.current_file(file);
            }

            // History bar on top
            draw_history_bar(file->get_history());

            // File View in the middle
            View* eachFileView = file->get_view();
            ImGui::PushStyleColor(ImGuiCol_ChildBg, fw::vec4(0,0,0,0.35f) );
            ImGui::PushFont(get_font(FontSlot_Code));
            eachFileView->draw_as_child("FileView", ImGui::GetContentRegionAvail(), false);
            ImGui::PopFont();
            ImGui::PopStyleColor();

            if ( is_current_file && file->get_view()->text_has_changed())
            {
                vm.release_program();
            }
        }
    }
    ImGui::End(); // File Window

    if (!is_window_open)
    {
        app.close_file(file);
    }
}

void AppView::draw_properties_editor()
{
    if (ImGui::Begin(k_app_settings_window_name))
    {
        Settings& settings = Settings::get_instance();

        ImGui::Text("Nodable Settings:");
        ImGui::Indent();

            ImGui::Text("Buttons:");
            ImGui::Indent();
                ImGui::SliderFloat2("ui_toolButton_size", &settings.ui_toolButton_size.x, 20.0f, 50.0f);
            ImGui::Unindent();

            ImGui::Text("Wires:");
            ImGui::Indent();
                ImGui::SliderFloat("thickness", &settings.ui_wire_bezier_thickness, 0.5f, 10.0f);
                ImGui::SliderFloat("roundness", &settings.ui_wire_bezier_roundness, 0.0f, 1.0f);
                ImGui::Checkbox   ("arrows"   , &settings.ui_wire_displayArrows);
            ImGui::Unindent();

            ImGui::Text("Nodes:");
            ImGui::Indent();
                ImGui::SliderFloat("property connector radius"    , &settings.ui_node_propertyConnectorRadius, 1.0f, 10.0f);
                ImGui::SliderFloat("padding"                    , &settings.ui_node_padding, 1.0f, 20.0f);
                ImGui::SliderFloat("speed"                      , &settings.ui_node_speed, 0.0f, 100.0f);
                ImGui::SliderFloat("spacing"                    , &settings.ui_node_spacing, 0.0f, 100.0f);
                ImGui::SliderFloat("node connector padding"     , &settings.ui_node_connector_padding, 0.0f, 100.0f);
                ImGui::SliderFloat("node connector height"      , &settings.ui_node_connector_height, 2.0f, 100.0f);
                ImGui::ColorEdit4("variables color"             , &settings.ui_node_variableColor.x);
                ImGui::ColorEdit4("instruction color"           , &settings.ui_node_instructionColor.x);
                ImGui::ColorEdit4("literal color"               , &settings.ui_node_literalColor.x);
                ImGui::ColorEdit4("function color"              , &settings.ui_node_invokableColor.x);
                ImGui::ColorEdit4("shadow color"                , &settings.ui_node_shadowColor.x);
                ImGui::ColorEdit4("border color"                , &settings.ui_node_borderColor.x);
                ImGui::ColorEdit4("high. color"                 , &settings.ui_node_highlightedColor.x);
                ImGui::ColorEdit4("border high. color"          , &settings.ui_node_borderHighlightedColor.x);
                ImGui::ColorEdit4("fill color"                  , &settings.ui_node_fillColor.x);
                ImGui::ColorEdit4("node connector color"        , &settings.ui_node_nodeConnectorColor.x);
                ImGui::ColorEdit4("node connector hovered color", &settings.ui_node_nodeConnectorHoveredColor.x);

            ImGui::Unindent();

            // code flow
            ImGui::Text("Code flow:");
            ImGui::Indent();
                ImGui::SliderFloat("line width min", &settings.ui_node_connector_width, 1.0f, 100.0f);
            ImGui::Unindent();

        ImGui::Unindent();
    }
    ImGui::End();
}

void AppView::draw_splashcreen()
{
    if (m_show_splashscreen && !ImGui::IsPopupOpen(m_splashscreen_title))
    {
        ImGui::OpenPopup(m_splashscreen_title);
    }

    ImGui::SetNextWindowSizeConstraints(fw::vec2(500,200), fw::vec2(500,50000));
    ImGui::SetNextWindowPos( ImGui::GetMainViewport()->GetCenter(), 0, fw::vec2(0.5f,0.5f) );

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if ( ImGui::BeginPopupModal(m_splashscreen_title, nullptr, flags) )
    {
        ImGui::SameLine( (ImGui::GetContentRegionAvail().x - m_logo->width) * 0.5f); // center img
        ImGui::Image((void*)(intptr_t)m_logo->image, fw::vec2((float)m_logo->width, (float)m_logo->height));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, fw::vec2(50.0f, 30.0f) );
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImGui::TextWrapped("DISCLAIMER: This software is a prototype, do not expect too much from it. Use at your own risk." );

        ImGui::NewLine();ImGui::NewLine();

        const char* credit = "by Berdal84";
        ImGui::SameLine( ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(credit).x);
        ImGui::TextWrapped( "%s", credit );
        ImGui::TextWrapped( "%s", BuildInfo::version );
        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
        {
            ImGui::CloseCurrentPopup();
            m_show_splashscreen = false;
        }
        ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
        ImGui::EndPopup();
    }

}

void AppView::draw_status_bar() const
{
    if ( ImGui::Begin(k_status_window_name) )
    {
        if( !fw::Log::get_messages().empty() )
        {
            auto& settings = Settings::get_instance();
            const fw::Log::Messages& messages = fw::Log::get_messages();
            auto it  = messages.rend() - settings.ui_log_tooltip_max_count;
            while( it != messages.rend() )
            {
                auto& each_message = *it;
                ImGui::TextColored(settings.ui_log_color[each_message.verbosity], "%s", each_message.to_full_string().c_str());
                ++it;
            }

            if( !ImGui::IsWindowHovered())
            {
                ImGui::SetScrollHereY();
            }

        }
    }
    ImGui::End();
}

void AppView::draw_history_bar(History *currentFileHistory)
{
    if (currentFileHistory)
    {
        if (ImGui::IsMouseReleased(0))
        {
            m_is_history_dragged = false;
        }
        auto& settings      = Settings::get_instance();
        float btn_spacing   = settings.ui_history_btn_spacing;
        float btn_height    = settings.ui_history_btn_height;
        float btn_width_max = settings.ui_history_btn_width_max;

        size_t              historySize    = currentFileHistory->get_size();
        std::pair<int, int> history_range  = currentFileHistory->get_command_id_range();
        float               avail_width    = ImGui::GetContentRegionAvail().x;
        float               btn_width      = fmin(btn_width_max, avail_width / float(historySize + 1) - btn_spacing);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, fw::vec2(btn_spacing, 0));

        for (int cmd_pos = history_range.first; cmd_pos <= history_range.second; cmd_pos++)
        {
            ImGui::SameLine();

            std::string label("##" + std::to_string(cmd_pos));

            // Draw an highlighted button for the current history position
            if ( cmd_pos == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
                ImGui::Button(label.c_str(), fw::vec2(btn_width, btn_height));
                ImGui::PopStyleColor();
            }
            else // or a simple one for other history positions
            {
                ImGui::Button(label.c_str(), fw::vec2(btn_width, btn_height));
            }

            // Hovered item
            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseDown(0)) // hovered + mouse down
                {
                    m_is_history_dragged = true;
                }

                // Draw command description
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, float(0.8));
                if( fw::ImGuiEx::BeginTooltip() )
                {
                    ImGui::Text("%s", currentFileHistory->get_cmd_description_at(cmd_pos).c_str());
                    fw::ImGuiEx::EndTooltip();
                }
                ImGui::PopStyleVar();
            }

            // When dragging history
            const auto xMin = ImGui::GetItemRectMin().x;
            const auto xMax = ImGui::GetItemRectMax().x;
            if (m_is_history_dragged &&
                ImGui::GetMousePos().x < xMax && ImGui::GetMousePos().x > xMin)
            {
                currentFileHistory->move_cursor(cmd_pos); // update history cursor position
            }


        }
        ImGui::PopStyleVar();
    }
}

void AppView::draw_tool_bar()
{
    auto& app      = App::get_instance();
    auto& vm       = VirtualMachine::get_instance();
    auto& settings = Settings::get_instance();

    bool running       = vm.is_program_running();
    bool debugging     = vm.is_debugging();
    bool stopped       = vm.is_program_stopped();
    fw::vec2 &button_size  = settings.ui_toolButton_size;
    fw::vec4 &active_color = settings.ui_button_activeColor;

    ImGui::PushFont(get_font(FontSlot_ToolBtn));

    // small margin
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
    ImGui::BeginGroup();

    // compile
    if (ImGui::Button(ICON_FA_DATABASE " compile", button_size) && stopped)
    {
        app.compile_and_load_program();
    }
    ImGui::SameLine();

    // run
    if ( running ) ImGui::PushStyleColor(ImGuiCol_Button, active_color);

    if (ImGui::Button(ICON_FA_PLAY " run", button_size) && stopped)
    {
        app.run_program();
    }
    if ( running ) ImGui::PopStyleColor();

    ImGui::SameLine();

    // debug
    if ( debugging ) ImGui::PushStyleColor(ImGuiCol_Button, active_color);
    if (ImGui::Button(ICON_FA_BUG " debug", button_size) && stopped)
    {
        app.debug_program();
    }
    if ( debugging ) ImGui::PopStyleColor();
    ImGui::SameLine();

    // stepOver
    if (ImGui::Button(ICON_FA_ARROW_RIGHT " step over", button_size) && vm.is_debugging())
    {
        vm.step_over();
    }
    ImGui::SameLine();

    // stop
    if (ImGui::Button(ICON_FA_STOP " stop", button_size) && !stopped)
    {
        app.stop_program();
    }
    ImGui::SameLine();

    // reset
    if ( ImGui::Button(ICON_FA_UNDO " reset graph", button_size))
    {
        app.reset_program();
    }
    ImGui::SameLine();

    // enter isolation mode
    if ( ImGui::Button( settings.isolate_selection ? ICON_FA_CROP " isolation mode: ON " :  ICON_FA_CROP " isolation mode: OFF", button_size))
    {
        fw::EventManager::push_event(fw::EventType_toggle_isolate_selection);
    }
    ImGui::SameLine();
    ImGui::EndGroup();

    ImGui::PopFont();
}
