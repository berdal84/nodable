#include "NodableView.h"

#include <utility>

#include "fw/core/log.h"
#include "fw/core/system.h"
#include "fw/gui/Texture.h"
#include "fw/gui/ActionManagerView.h"
#include "nodable/core/NodeUtils.h"


#include "Config.h"
#include "Event.h"
#include "File.h"
#include "FileView.h"
#include "History.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"
#include "PropertyView.h"
#include "build_info.h"
#include "fw/gui/Config.h"
#include "fw/gui/gui.h"
#include "gui.h"

using namespace ndbl;
using namespace ndbl::assembly;
using namespace fw;

NodableView::NodableView(Nodable * _app)
    : AppView(_app)
    , m_logo(nullptr)
    , m_is_history_dragged(false)
    , m_show_properties_editor(false)
    , m_show_imgui_demo(false)
    , m_show_advanced_node_properties(false)
    , m_scroll_to_curr_instr(true)
    , m_app(_app)
{
    FW_EXPECT(m_app, "should be defined");
}

NodableView::~NodableView()
{
    LOG_VERBOSE("ndbl::NodableView", "Destructor " OK "\n");
}

void NodableView::on_init()
{
    LOG_VERBOSE("ndbl::NodableView", "on_init ...\n");

    // Load splashscreen image
    std::filesystem::path path = App::asset_path( g_conf->ui_splashscreen_imagePath );
    m_logo = m_app->texture_manager.load(path.string());

    LOG_VERBOSE("ndbl::NodableView", "on_init " OK "\n");
}

void NodableView::on_draw()
{
    bool redock_all = true;

    File*           current_file    = m_app->current_file;
    EventManager&   event_manager   = m_app->event_manager;
    VirtualMachine& virtual_machine = m_app->virtual_machine;

    // 1. Draw Menu Bar
    if (ImGui::BeginMenuBar())
    {
        History* current_file_history = current_file ? &current_file->history : nullptr;

        if (ImGui::BeginMenu("File")) {
            bool has_file = current_file != nullptr;
            bool is_current_file_content_dirty = current_file != nullptr && current_file->dirty;
            ImGuiEx::MenuItem<Event_FileNew>();
            ImGuiEx::MenuItem<Event_FileBrowse>();
            ImGui::Separator();
            ImGuiEx::MenuItem<Event_FileSaveAs>(false, has_file);
            ImGuiEx::MenuItem<Event_FileSave>(false, has_file && is_current_file_content_dirty );
            ImGui::Separator();
            ImGuiEx::MenuItem<Event_FileClose>(false, has_file);

            auto auto_paste = has_file && current_file->view.experimental_clipboard_auto_paste();

            if (ImGui::MenuItem(ICON_FA_COPY        "  Auto-paste clipboard", "", auto_paste, has_file ) && has_file ) {
                current_file->view.experimental_clipboard_auto_paste(!auto_paste);
            }

            ImGuiEx::MenuItem<Event_Exit>();

            ImGui::EndMenu();
        }

        bool vm_is_stopped = virtual_machine.is_program_stopped();
        if (ImGui::BeginMenu("Edit"))
        {
            if (current_file_history)
            {
                ImGuiEx::MenuItem<Event_Undo>();
                ImGuiEx::MenuItem<Event_Redo>();
                ImGui::Separator();
            }

            auto has_selection = NodeView::is_any_selected();

            if (ImGui::MenuItem("Delete", "Del.", false, has_selection && vm_is_stopped)) {
                event_manager.dispatch( EventID_DELETE_NODE );
            }

            ImGuiEx::MenuItem<Event_ArrangeNode>( false, has_selection );
            ImGuiEx::MenuItem<Event_ToggleFolding>( false,has_selection );

            if (ImGui::MenuItem("Expand/Collapse recursive", nullptr, false, has_selection))
            {
                event_manager.dispatch<Event_ToggleFolding>( { RECURSIVELY } );
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            //auto frame = ImGui::MenuItem("Frame All", "F");
            redock_all |= ImGui::MenuItem("Redock documents");

            ImGui::Separator();

            auto menu_item_node_view_detail = [](NodeViewDetail _detail, const char *_label) {
                if (ImGui::MenuItem(_label, "", NodeView::get_view_detail() == _detail)) {
                    NodeView::set_view_detail(_detail);
                }
            };

            menu_item_node_view_detail(NodeViewDetail::Minimalist, "Minimalist View");
            menu_item_node_view_detail(NodeViewDetail::Essential, "Essential View");
            menu_item_node_view_detail(NodeViewDetail::Exhaustive, "Exhaustive View");

            ImGui::Separator();
            m_show_properties_editor = ImGui::MenuItem(ICON_FA_COGS " Show Properties", "",
                                                       m_show_properties_editor);
            m_show_imgui_demo = ImGui::MenuItem("Show ImGui Demo", "", m_show_imgui_demo);

            ImGui::Separator();

            if (ImGui::MenuItem("Fullscreen", "", m_app->is_fullscreen()))
            {
                m_app->set_fullscreen( !m_app->is_fullscreen() );
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Reset Layout", "")) {
                set_layout_initialized(false);
            }

            ImGui::Separator();

            ImGuiEx::MenuItem<Event_ToggleIsolationFlags>( g_conf->isolation );

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu("Run") )
        {
            bool vm_is_debugging = virtual_machine.is_debugging();

            if (ImGui::MenuItem(ICON_FA_PLAY" Run", "", false, vm_is_stopped)) {
                m_app->run_program();
            }

            if (ImGui::MenuItem(ICON_FA_BUG" Debug", "", false, vm_is_stopped)) {
                m_app->debug_program();
            }

            if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT" Step Over", "", false, vm_is_debugging)) {
                m_app->step_over_program();
            }

            if (ImGui::MenuItem(ICON_FA_STOP" Stop", "", false, !vm_is_stopped)) {
                m_app->stop_program();
            }

            if (ImGui::MenuItem(ICON_FA_UNDO " Reset", "", false, vm_is_stopped)) {
                m_app->reset_program();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Developer"))
        {
            if ( ImGui::MenuItem("Show debug info", "", fw::g_conf->debug ) )
            {
                fw::g_conf->debug = !fw::g_conf->debug;
                ImGuiEx::debug = fw::g_conf->debug;
            }
            if ( ImGui::MenuItem("Show FPS", "", fw::g_conf->show_fps ) )
            {
                fw::g_conf->show_fps = !fw::g_conf->show_fps;
            }
            if ( ImGui::MenuItem("Limit FPS", "", fw::g_conf->delta_time_limit ) )
            {
                fw::g_conf->delta_time_limit = !fw::g_conf->delta_time_limit;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Verbosity"))
            {
                auto menu_item_verbosity = [](log::Verbosity _verbosity, const char *_label) {
                    if (ImGui::MenuItem(_label, "", log::get_verbosity() == _verbosity)) {
                        log::set_verbosity(_verbosity);
                    }
                };

#ifndef LOG_DISABLE_VERBOSE
                menu_item_verbosity(log::Verbosity_Verbose, "Verbose");
#endif
                menu_item_verbosity(log::Verbosity_Message, "Message (default)");
                menu_item_verbosity(log::Verbosity_Warning, "Warning");
                menu_item_verbosity(log::Verbosity_Error, "Error");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Experimental")) {
                ImGui::Checkbox("Hybrid history", &g_conf->experimental_hybrid_history);
                ImGui::Checkbox("Graph auto-completion", &g_conf->experimental_graph_autocompletion);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("An issue ?")) {
            if (ImGui::MenuItem("Report on Github.com")) {
                system::open_url_async("https://github.com/berdal84/Nodable/issues");
            }

            if (ImGui::MenuItem("Report by email")) {
                system::open_url_async("mail:berenger@dalle-cort.fr");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Show Splash Screen", "F1")) {
                fw::g_conf->splashscreen = true;
            }

            if (ImGui::MenuItem("Browse source code")) {
                system::open_url_async("https://www.github.com/berdal84/nodable");
            }

            if (ImGui::MenuItem("Credits")) {
                system::open_url_async("https://github.com/berdal84/nodable#credits-");
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // 2. Draw windows
    // All draw_xxx_window() are ImGui windows docked to a dockspace (defined in on_reset_layout() )



    if(!m_app->has_files())
    {
        if( !fw::g_conf->splashscreen )
        {
            draw_startup_window( get_dockspace(AppView::Dockspace_ROOT));
        }
    }
    else
    {
        draw_toolbar_window();

        auto ds_root = get_dockspace(AppView::Dockspace_ROOT);
        for ( File*each_file: m_app->get_files())
        {
            draw_file_window(ds_root, redock_all, each_file);
        }

        draw_virtual_machine_window();
        draw_config_window();
        draw_imgui_config_window();
        draw_file_info_window();
        draw_node_properties_window();
        draw_help_window();
    }
}

void NodableView::draw_help_window() const {
    if (ImGui::Begin( g_conf->ui_help_window_label))
    {
        FontManager& font_manager = m_app->font_manager;
        ImGui::PushFont(font_manager.get_font(FontSlot_Heading));
        ImGui::Text("Welcome to Nodable!");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextWrapped(
                "Nodable is node-able.\n"
                "\n"
                "Nodable allows you to edit a program using both text and graph paradigms."
                "More precisely, it means:"
        );
        ImGuiEx::BulletTextWrapped("any change on the text will affect the graph");
        ImGuiEx::BulletTextWrapped("any change (structure or values) on the graph will affect the text");
        ImGuiEx::BulletTextWrapped(
                "but keep in mind the state is the text, any change not affecting the text (such as node positions or orphan nodes) will be lost.");
        ImGui::NewLine();
        ImGui::PushFont(font_manager.get_font(FontSlot_Heading));
        ImGui::Text("Quick start");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextWrapped("Nodable UI is designed as following:\n");
        ImGuiEx::BulletTextWrapped("On the left side a (light) text editor allows to edit source code.\n");
        ImGuiEx::BulletTextWrapped(
                "At the center, there is the graph editor where you can create/delete/connect nodes\n");
        ImGuiEx::BulletTextWrapped(
                "On the right side (this side) you will find many tabs to manage additional config such as node properties, virtual machine or app properties\n");
        ImGuiEx::BulletTextWrapped("At the top, between the menu and the editors, there is a tool bar."
                                       " There, few buttons will serve to compile, run and debug your program.");
        ImGuiEx::BulletTextWrapped("And at the bottom, below the editors, there is a status bar."
                                       " This bar will display important messages, warning, and errors. You can expand it to get older messages.");
    }
    ImGui::End();
}

void NodableView::draw_imgui_config_window() const
{
    if( !fw::g_conf->debug )
    {
        return;
    }

    if (ImGui::Begin( g_conf->ui_imgui_config_window_label))
    {
        ImGui::ShowStyleEditor();
    }
    ImGui::End();
}

void NodableView::draw_file_info_window() const
{
    if ( !m_app->current_file )
    {
        return;
    }

    if (ImGui::Begin( g_conf->ui_file_info_window_label))
    {
        m_app->current_file->view.draw_info_panel();
    }

    ImGui::End();
}

void NodableView::draw_node_properties_window()
{
    if (ImGui::Begin( g_conf->ui_node_properties_window_label))
    {
        if (NodeView* selected_view = NodeView::get_selected().get())
        {
            ImGui::Indent(10.0f);
            NodeView::draw_as_properties_panel(selected_view, &m_show_advanced_node_properties);
        }
    }
    ImGui::End();
}

void NodableView::draw_virtual_machine_window() {
    if (ImGui::Begin( g_conf->ui_virtual_machine_window_label))
    {
        auto &vm = m_app->virtual_machine;

        ImGui::Text("Virtual Machine:");
        ImGui::SameLine();
        ImGuiEx::DrawHelper("%s", "The virtual machine - or interpreter - is a sort of implementation of \n"
                                      "an imaginary hardware able to run a set of simple instructions.");
        ImGui::Separator();

        const Code *code = vm.get_program_asm_code();

        // VM state
        {
            ImGui::Indent();
            ImGui::Text("State:         %s", vm.is_program_running() ? "running" : "stopped");
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "When virtual machine is running, you cannot edit the code or the graph.");
            ImGui::Text("Debug:         %s", vm.is_debugging() ? "ON" : "OFF");
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "When debugging is ON, you can run a program step by step.");
            ImGui::Text("Has program:   %s", code ? "YES" : "NO");
            if (code) {
                ImGui::Text("Program over:  %s", !vm.is_there_a_next_instr() ? "YES" : "NO");
            }
            ImGui::Unindent();
        }

        // VM Registers
        ImGui::Separator();
        ImGui::Text("CPU:");
        ImGui::SameLine();
        ImGuiEx::DrawHelper("%s", "This is the virtual machine's CPU"
                                      "\nIt contains few registers to store temporary values "
                                      "\nlike instruction pointer, last node's value or last comparison result");
        ImGui::Indent();
        {
            ImGui::Separator();
            ImGui::Text("registers:");
            ImGui::Separator();

            using assembly::Register;
            ImGui::Indent();

            auto draw_register_value = [&](Register _register) {
                ImGui::Text("%4s: %12s", assembly::to_string(_register),
                            vm.read_cpu_register(_register).to_string().c_str());
            };

            draw_register_value(Register::rax);
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "primary accumulator");
            draw_register_value(Register::rdx);
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "base register");
            draw_register_value(Register::eip);
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "instruction pointer");

            ImGui::Unindent();
        }
        ImGui::Unindent();

        // Assembly-like code
        ImGui::Separator();
        ImGui::Text("Memory:");
        ImGui::SameLine();
        ImGuiEx::DrawHelper("%s", "Virtual Machine Memory.");
        ImGui::Separator();
        {
            ImGui::Indent();

            ImGui::Text("Bytecode:");
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "The bytecode is the result of the Compilation process."
                                          "\nAfter source code has been parsed to a syntax tree, "
                                          "\nthe tree (or graph) is converted by the Compiler to an Assembly-like code.");
            ImGui::Checkbox("Auto-scroll ?", &m_scroll_to_curr_instr);
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "to scroll automatically to the current instruction");
            ImGui::Separator();
            {
                ImGui::BeginChild("AssemblyCodeChild", ImGui::GetContentRegionAvail(), true);

                if (code) {
                    auto current_instr = vm.get_next_instr();
                    for (Instruction *each_instr: code->get_instructions()) {
                        auto str = Instruction::to_string(*each_instr);
                        if (each_instr == current_instr) {
                            if (m_scroll_to_curr_instr && vm.is_program_running()) {
                                ImGui::SetScrollHereY();
                            }
                            ImGui::TextColored(ImColor(200, 0, 0), ">%s", str.c_str());
                            ImGui::SameLine();
                            ImGuiEx::DrawHelper("%s", "This is the next instruction to evaluate");
                        } else {
                            ImGui::Text(" %s", str.c_str());
                        }
                    }
                } else {
                    ImGui::TextWrapped("Nothing loaded, try to compile, run or debug.");
                    ImGui::SameLine();
                    ImGuiEx::DrawHelper("%s", "To see a compiled program here you need first to:"
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

void NodableView::draw_startup_window(ImGuiID dockspace_id) {
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3f, 0.3f, 0.3f, 1.f));

    ImGui::Begin( g_conf->ui_startup_window_label);
    {
        FontManager&  font_manager  = m_app->font_manager;
        EventManager& event_manager = m_app->event_manager;
        ImGui::PopStyleColor();

        ImVec2 center_area(500.0f, 250.0f);
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImGui::SetCursorPosX((avail.x - center_area.x) / 2);
        ImGui::SetCursorPosY((avail.y - center_area.y) / 2);

        ImGui::BeginChild("center_area", center_area);
        {
            ImGui::Indent(center_area.x * 0.05f);

            ImGui::PushFont(font_manager.get_font(FontSlot_ToolBtn));
            ImGui::NewLine();

            ImVec2 btn_size(center_area.x * 0.44f, 40.0f);
            if (ImGui::Button(ICON_FA_FILE" New File", btn_size))
                event_manager.dispatch( EventID_FILE_NEW );
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER_OPEN" Open ...", btn_size))
                event_manager.dispatch( EventID_FILE_BROWSE );

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();

            ImGui::Text("%s", "Open an example");
            std::vector<std::pair<std::string, std::string>> examples;
            examples.emplace_back("Single expressions    ", "examples/arithmetic.cpp");
            examples.emplace_back("Multi instructions    ", "examples/multi-instructions.cpp");
            examples.emplace_back("Conditional Structures", "examples/if-else.cpp");
            examples.emplace_back("For Loop              ", "examples/for-loop.cpp");

            int i = 0;
            ImVec2 small_btn_size(btn_size.x, btn_size.y * 0.66f);

            for (const auto& [text, path]: examples) {
                std::string label;
                label.append(ICON_FA_BOOK" ");
                label.append(text);
                if (i++ % 2) ImGui::SameLine();
                if (ImGui::Button(label.c_str(), small_btn_size))
                {
                    m_app->open_asset_file(path.c_str());
                }
            }

            ImGui::PopFont();

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0, 0, 0, 0.30f), "%s", BuildInfo::version);
            ImGui::Unindent();
        }
        ImGui::EndChild();
    }
    ImGui::End(); // Startup Window
}

void NodableView::draw_file_window(ImGuiID dockspace_id, bool redock_all, File*file) {
    auto &vm = m_app->virtual_machine;

    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags =
            (file->dirty ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoScrollbar;

    auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
    child_bg.w = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

    bool is_window_open = true;
    bool visible = ImGui::Begin(file->filename().c_str(), &is_window_open, window_flags);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(1);

    if(visible)
    {
        const bool is_current_file = m_app->is_current(file);

        if (!is_current_file && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
            m_app->current_file = file;
        }

        // History bar on top
        draw_history_bar(file->history);

        // File View in the middle
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.35f));
        ImGui::PushFont(m_app->font_manager.get_font(FontSlot_Code));
        const ImVec2 size = ImGui::GetContentRegionAvail();

        ImGui::BeginChild("FileView", size, false, 0);
        {
            file->view.draw();
        }
        ImGui::EndChild();

        ImGui::PopFont();
        ImGui::PopStyleColor();
    }
    ImGui::End(); // File Window

    if (!is_window_open) m_app->close_file(file);
}

void NodableView::draw_config_window() {

    if (ImGui::Begin( g_conf->ui_config_window_label))
    {
        ImGui::Text("Nodable Settings");
        if ( ImGui::Button("Reset Settings") )
        {
            g_conf->reset();
        }

        if (ImGui::CollapsingHeader("Nodes", ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ImGui::Indent();
            if ( ImGui::CollapsingHeader("Colors"))
            {
                ImGui::ColorEdit4("default", &g_conf->ui_node_fillColor.x);
                ImGui::ColorEdit4("highlighted", &g_conf->ui_node_highlightedColor.x);
                ImGui::ColorEdit4("variable", &g_conf->ui_node_variableColor.x);
                ImGui::ColorEdit4("instruction", &g_conf->ui_node_instructionColor.x);
                ImGui::ColorEdit4("literal", &g_conf->ui_node_literalColor.x);
                ImGui::ColorEdit4("function", &g_conf->ui_node_invokableColor.x);
                ImGui::ColorEdit4("shadow", &g_conf->ui_node_shadowColor.x);
                ImGui::ColorEdit4("border", &g_conf->ui_slot_border_color.x);
                ImGui::ColorEdit4("border (highlighted)", &g_conf->ui_node_borderHighlightedColor.x);
                ImGui::ColorEdit4("slot", &g_conf->ui_slot_color.x);
                ImGui::ColorEdit4("slot (hovered)", &g_conf->ui_slot_hovered_color.x);
            }

            if ( ImGui::CollapsingHeader("Slots"))
            {
                ImGui::Text("Property Slots:");
                ImGui::SliderFloat("slot radius", &g_conf->ui_slot_radius, 5.0f, 10.0f);

                ImGui::Separator();

                ImGui::Text("Code Flow Slots:");
                ImGui::SliderFloat2("slot size##codeflow", &g_conf->ui_slot_size.x, 2.0f, 100.0f);
                ImGui::SliderFloat("slot padding##codeflow", &g_conf->ui_slot_gap, 0.0f, 100.0f);
                ImGui::SliderFloat("slot radius##codeflow", &g_conf->ui_slot_border_radius, 0.0f, 40.0f);
            }

            if ( ImGui::CollapsingHeader("Misc."))
            {
                ImGui::SliderFloat("spacing", &g_conf->ui_node_spacing, 10.0f, 50.0f);
                ImGui::SliderFloat("velocity", &g_conf->ui_node_speed, 1.0f, 10.0f);
                ImGui::SliderFloat4("padding", &g_conf->ui_node_padding.x, 0.0f, 20.0f);
                ImGui::SliderFloat("border width", &g_conf->ui_node_borderWidth, 0.0f, 10.0f);
                ImGui::SliderFloat("border width ratio (instructions)", &g_conf->ui_node_instructionBorderRatio, 0.0f, 10.0f);
            }
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Wires / Code Flow"))
        {
            ImGui::Text("Wires");
            ImGui::SliderFloat("thickness##wires", &g_conf->ui_wire_bezier_thickness, 0.5f, 10.0f);
            ImGui::SliderFloat2("roundness (min,max)##wires", &g_conf->ui_wire_bezier_roundness.x, 0.0f, 1.0f);
            ImGui::SliderFloat2("fade length (min,max)##wires", &g_conf->ui_wire_bezier_fade_length_minmax.x, 200.0f, 1000.0f);
            ImGui::ColorEdit4("color##wires", &g_conf->ui_wire_color.x);
            ImGui::ColorEdit4("shadow color##wires", &g_conf->ui_wire_shadowColor.x);

            ImGui::Separator();

            ImGui::Text("Code Flow");
            ImGui::ColorEdit4("color##codeflow", &g_conf->ui_codeflow_color.x);
            ImGui::SliderFloat("thickness (ratio)##codeflow", &g_conf->ui_codeflow_thickness_ratio, 0.1, 1.0);
        }

        if (ImGui::CollapsingHeader("Graph"))
        {
            ImGui::InputFloat("unfold delta time", &g_conf->graph_unfold_dt);
            ImGui::InputInt("unfold iterations", &g_conf->graph_unfold_iterations, 1, 1000);
            ImGui::ColorEdit4("grid color (major)", &g_conf->ui_graph_grid_color_major.x);
            ImGui::ColorEdit4("grid color (minor)", &g_conf->ui_graph_grid_color_minor.x);
            ImGui::SliderInt("grid size", &g_conf->ui_grid_size, 1, 500);
            ImGui::SliderInt("grid subdivisions", &g_conf->ui_grid_subdiv_count, 1, 16);
        }

        if (ImGui::CollapsingHeader("Shortcuts", ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ActionManagerView::draw(&m_app->action_manager);
        }

        if ( fw::g_conf->debug && ImGui::CollapsingHeader("Pool"))
        {
            ImGui::Text("Pool stats:");
            auto pool = Pool::get_pool();
            ImGui::Text(" - Node.................... %8zu", pool->get_all<Node>().size() );
            ImGui::Text(" - NodeView................ %8zu", pool->get_all<NodeView>().size() );
            ImGui::Text(" - Physics................. %8zu", pool->get_all<Physics>().size() );
            ImGui::Text(" - Scope................... %8zu", pool->get_all<Scope>().size() );
        }
    }
    ImGui::End();
}

void NodableView::draw_splashscreen()
{
    if ( AppView::begin_splashscreen() )
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        // Image
        ImGui::SameLine((ImGui::GetContentRegionAvail().x - m_logo->width) * 0.5f); // center img
        ImGuiEx::Image(m_logo);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {50.0f, 30.0f});

        // disclaimer
        ImGui::TextWrapped(
                "DISCLAIMER: This software is a prototype, do not expect too much from it. Use at your own risk.");

        ImGui::NewLine();
        ImGui::NewLine();

        // credits
        const char *credit = "by Berdal84";
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(credit).x);
        ImGui::TextWrapped("%s", credit);

        // build version
        ImGui::TextWrapped("%s", BuildInfo::version);

        // close on left/rightmouse btn click
        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1))
        {
            fw::g_conf->splashscreen = false;
        }
        ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
        AppView::end_splashscreen();
    }
}

void NodableView::draw_history_bar(History& currentFileHistory)
{
    if (ImGui::IsMouseReleased(0))
    {
        m_is_history_dragged = false;
    }
    float btn_spacing   = g_conf->ui_history_btn_spacing;
    float btn_height    = g_conf->ui_history_btn_height;
    float btn_width_max = g_conf->ui_history_btn_width_max;

    size_t historySize = currentFileHistory.get_size();
    std::pair<int, int> history_range = currentFileHistory.get_command_id_range();
    float avail_width = ImGui::GetContentRegionAvail().x;
    float btn_width = fmin(btn_width_max, avail_width / float(historySize + 1) - btn_spacing);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { btn_spacing, 0});

    for (int cmd_pos = history_range.first; cmd_pos <= history_range.second; cmd_pos++) {
        ImGui::SameLine();

        std::string label("##" + std::to_string(cmd_pos));

        // Draw an highlighted button for the current history position
        if (cmd_pos == 0) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            ImGui::Button(label.c_str(), {btn_width, btn_height});
            ImGui::PopStyleColor();
        } else // or a simple one for other history positions
        {
            ImGui::Button(label.c_str(), {btn_width, btn_height});
        }

        // Hovered item
        if (ImGui::IsItemHovered()) {
            if (ImGui::IsMouseDown(0)) // hovered + mouse down
            {
                m_is_history_dragged = true;
            }

            // Draw command description
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, float(0.8));
            if (ImGuiEx::BeginTooltip()) {
                ImGui::Text("%s", currentFileHistory.get_cmd_description_at(cmd_pos).c_str());
                ImGuiEx::EndTooltip();
            }
            ImGui::PopStyleVar();
        }

        // When dragging history
        if (m_is_history_dragged &&
            ImGui::GetMousePos().x > ImGui::GetItemRectMin().x &&
            ImGui::GetMousePos().x < ImGui::GetItemRectMax().x)
        {
            currentFileHistory.move_cursor(cmd_pos); // update history cursor position
        }


    }
    ImGui::PopStyleVar();
}

void NodableView::draw_toolbar_window() {

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 5.0f});
    if (ImGui::Begin( g_conf->ui_toolbar_window_label, NULL, flags ))
    {
        ImGui::PopStyleVar();
        VirtualMachine& vm   = m_app->virtual_machine;
        bool running         = vm.is_program_running();
        bool debugging       = vm.is_debugging();
        bool stopped         = vm.is_program_stopped();
        auto button_size   = g_conf->ui_toolButton_size;

        ImGui::PushFont(m_app->font_manager.get_font(FontSlot_ToolBtn));
        ImGui::BeginGroup();

        // compile
        if (ImGui::Button(ICON_FA_DATABASE " compile", button_size) && stopped) {
            m_app->compile_and_load_program();
        }
        ImGui::SameLine();

        // run
        if (running) ImGui::PushStyleColor(ImGuiCol_Button, fw::g_conf->button_activeColor);

        if (ImGui::Button(ICON_FA_PLAY " run", button_size) && stopped) {
            m_app->run_program();
        }
        if (running) ImGui::PopStyleColor();

        ImGui::SameLine();

        // debug
        if (debugging) ImGui::PushStyleColor(ImGuiCol_Button, fw::g_conf->button_activeColor);
        if (ImGui::Button(ICON_FA_BUG " debug", button_size) && stopped) {
            m_app->debug_program();
        }
        if (debugging) ImGui::PopStyleColor();
        ImGui::SameLine();

        // stepOver
        if (ImGui::Button(ICON_FA_ARROW_RIGHT " step over", button_size) && vm.is_debugging()) {
            vm.step_over();
        }
        ImGui::SameLine();

        // stop
        if (ImGui::Button(ICON_FA_STOP " stop", button_size) && !stopped) {
            m_app->stop_program();
        }
        ImGui::SameLine();

        // reset
        if (ImGui::Button(ICON_FA_UNDO " reset graph", button_size)) {
            m_app->reset_program();
        }
        ImGui::SameLine();

        // enter isolation mode
        if (ImGui::Button(
                     g_conf->isolation & Isolation_ON ? ICON_FA_CROP " isolation mode: ON " : ICON_FA_CROP " isolation mode: OFF",
                button_size)) {
            m_app->event_manager.dispatch( EventID_TOGGLE_ISOLATION_FLAGS );
        }
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::PopFont();
    }
    ImGui::End();
}

void NodableView::on_reset_layout()
{
    // Dock windows to specific dockspace

    dock_window( g_conf->ui_help_window_label             , AppView::Dockspace_RIGHT);
    dock_window( g_conf->ui_config_window_label           , AppView::Dockspace_RIGHT);
    dock_window( g_conf->ui_file_info_window_label        , AppView::Dockspace_RIGHT);
    dock_window( g_conf->ui_node_properties_window_label  , AppView::Dockspace_RIGHT);
    dock_window( g_conf->ui_virtual_machine_window_label  , AppView::Dockspace_RIGHT);
    dock_window( g_conf->ui_imgui_config_window_label     , AppView::Dockspace_RIGHT);
    dock_window( g_conf->ui_toolbar_window_label          , AppView::Dockspace_TOP);
}
