#include "NodableView.h"

#include <utility>

#include "fw/core/log.h"
#include "fw/core/system.h"
#include "fw/gui/Texture.h"
#include "core/NodeUtils.h"
#include "Config.h"
#include "Event.h"
#include "HybridFile.h"
#include "HybridFileView.h"
#include "History.h"
#include "Nodable.h"
#include "NodeView.h"
#include "build_info.h"
#include "Physics.h"
#include "PropertyView.h"

using namespace ndbl;
using namespace ndbl::assembly;

NodableView::NodableView(Nodable * _app)
    : fw::AppView(_app)
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
    ghc::filesystem::path path = fw::App::asset_path(m_app->config.ui_splashscreen_imagePath);
    m_logo = m_app->texture_manager.load(path.string());

    LOG_VERBOSE("ndbl::NodableView", "on_init " OK "\n");
}

void NodableView::on_draw()
{
    bool redock_all = true;

    HybridFile*       current_file    = m_app->current_file;
    fw::EventManager& event_manager   = m_app->event_manager;
    Config&           config          = m_app->config;
    VirtualMachine&   virtual_machine = m_app->virtual_machine;

    // 1. Draw Menu Bar
    if (ImGui::BeginMenuBar()) {
        History* current_file_history = current_file ? current_file->get_history() : nullptr;

        if (ImGui::BeginMenu("File")) {
            bool has_file = current_file;
            bool changed = current_file != nullptr && current_file->changed;
            fw::ImGuiEx::MenuItem( fw::EventID_REQUEST_FILE_NEW );
            fw::ImGuiEx::MenuItem( fw::EventID_REQUEST_FILE_BROWSE );
            ImGui::Separator();
            fw::ImGuiEx::MenuItem( fw::EventID_REQUEST_FILE_SAVE_AS, false, has_file );
            fw::ImGuiEx::MenuItem( fw::EventID_REQUEST_FILE_SAVE, false, has_file && changed );
            ImGui::Separator();
            fw::ImGuiEx::MenuItem( fw::EventID_REQUEST_FILE_CLOSE, false, has_file );

            auto auto_paste = has_file && current_file->view.experimental_clipboard_auto_paste();

            if (ImGui::MenuItem(ICON_FA_COPY        "  Auto-paste clipboard", "", auto_paste, has_file ) && current_file ) {
                current_file->view.experimental_clipboard_auto_paste(!auto_paste);
            }

            fw::ImGuiEx::MenuItem( fw::EventID_REQUEST_EXIT );

            ImGui::EndMenu();
        }

        bool vm_is_stopped = virtual_machine.is_program_stopped();
        if (ImGui::BeginMenu("Edit")) {
            if (current_file_history) {
                fw::ImGuiEx::MenuItem( fw::EventID_REQUEST_UNDO );
                fw::ImGuiEx::MenuItem( fw::EventID_REQUEST_REDO );
                ImGui::Separator();
            }

            auto has_selection = NodeView::is_any_selected();

            if (ImGui::MenuItem("Delete", "Del.", false, has_selection && vm_is_stopped)) {
                event_manager.dispatch( EventID_REQUEST_DELETE_NODE );
            }

            fw::ImGuiEx::MenuItem( EventID_REQUEST_ARRANGE_HIERARCHY, false, has_selection );
            fw::ImGuiEx::MenuItem( EventID_REQUEST_TOGGLE_FOLDING, false,
                                   has_selection );

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

            fw::ImGuiEx::MenuItem( EventID_REQUEST_TOGGLE_ISOLATE_SELECTION, config.isolate_selection );

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
            if ( ImGui::MenuItem("Show debug info", "", m_app->config.common.debug ) )
            {
                m_app->config.common.debug = !m_app->config.common.debug;
                fw::ImGuiEx::debug = m_app->config.common.debug;
            }
            if ( ImGui::MenuItem("Show FPS", "", m_app->config.common.show_fps ) )
            {
                m_app->config.common.show_fps = !m_app->config.common.show_fps;
            }
            if ( ImGui::MenuItem("Limit FPS", "", m_app->config.common.delta_time_limit ) )
            {
                m_app->config.common.delta_time_limit = !m_app->config.common.delta_time_limit;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Verbosity"))
            {
                auto menu_item_verbosity = [](fw::log::Verbosity _verbosity, const char *_label) {
                    if (ImGui::MenuItem(_label, "", fw::log::get_verbosity() == _verbosity)) {
                        fw::log::set_verbosity(_verbosity);
                    }
                };

#ifndef LOG_DISABLE_VERBOSE
                menu_item_verbosity(fw::log::Verbosity_Verbose, "Verbose");
#endif
                menu_item_verbosity(fw::log::Verbosity_Message, "Message (default)");
                menu_item_verbosity(fw::log::Verbosity_Warning, "Warning");
                menu_item_verbosity(fw::log::Verbosity_Error, "Error");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Experimental")) {
                ImGui::Checkbox("Hybrid history", &config.experimental_hybrid_history);
                ImGui::Checkbox("Graph auto-completion", &config.experimental_graph_autocompletion);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("An issue ?")) {
            if (ImGui::MenuItem("Report on Github.com")) {
                fw::system::open_url_async("https://github.com/berdal84/Nodable/issues");
            }

            if (ImGui::MenuItem("Report by email")) {
                fw::system::open_url_async("mail:berenger@dalle-cort.fr");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Show Splash Screen", "F1")) {
                m_app->config.common.splashscreen = true;
            }

            if (ImGui::MenuItem("Browse source code")) {
                fw::system::open_url_async("https://www.github.com/berdal84/nodable");
            }

            if (ImGui::MenuItem("Credits")) {
                fw::system::open_url_async("https://github.com/berdal84/nodable#credits-");
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // 2. Draw windows
    // All draw_xxx_window() are ImGui windows docked to a dockspace (defined in on_reset_layout() )



    if(!m_app->has_files())
    {
        if( !m_app->config.common.splashscreen )
        {
            draw_startup_window( get_dockspace(fw::AppView::Dockspace_ROOT));
        }
    }
    else
    {
        draw_toolbar_window();

        auto ds_root = get_dockspace(fw::AppView::Dockspace_ROOT);
        for (HybridFile *each_file: m_app->get_files())
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
    if (ImGui::Begin(m_app->config.ui_help_window_label))
    {
        fw::FontManager& font_manager = m_app->font_manager;
        ImGui::PushFont(font_manager.get_font(fw::FontSlot_Heading));
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
        fw::ImGuiEx::BulletTextWrapped(
                "but keep in mind the state is the text, any change not affecting the text (such as node positions or orphan nodes) will be lost.");
        ImGui::NewLine();
        ImGui::PushFont(font_manager.get_font(fw::FontSlot_Heading));
        ImGui::Text("Quick start");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextWrapped("Nodable UI is designed as following:\n");
        fw::ImGuiEx::BulletTextWrapped("On the left side a (light) text editor allows to edit source code.\n");
        fw::ImGuiEx::BulletTextWrapped(
                "At the center, there is the graph editor where you can create/delete/connect nodes\n");
        fw::ImGuiEx::BulletTextWrapped(
                "On the right side (this side) you will find many tabs to manage additional config such as node properties, virtual machine or app properties\n");
        fw::ImGuiEx::BulletTextWrapped("At the top, between the menu and the editors, there is a tool bar."
                                       " There, few buttons will serve to compile, run and debug your program.");
        fw::ImGuiEx::BulletTextWrapped("And at the bottom, below the editors, there is a status bar."
                                       " This bar will display important messages, warning, and errors. You can expand it to get older messages.");
    }
    ImGui::End();
}

void NodableView::draw_imgui_config_window() const
{
    if( !m_app->config.common.debug )
    {
        return;
    }

    if (ImGui::Begin(m_app->config.ui_imgui_config_window_label))
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

    if (ImGui::Begin(m_app->config.ui_file_info_window_label))
    {
        m_app->current_file->view.draw_info_panel();
    }

    ImGui::End();
}

void NodableView::draw_node_properties_window()
{
    if (ImGui::Begin(m_app->config.ui_node_properties_window_label))
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
    if (ImGui::Begin(m_app->config.ui_virtual_machine_window_label))
    {
        auto &vm = m_app->virtual_machine;

        ImGui::Text("Virtual Machine:");
        ImGui::SameLine();
        fw::ImGuiEx::DrawHelper("%s", "The virtual machine - or interpreter - is a sort of implementation of \n"
                                      "an imaginary hardware able to run a set of simple instructions.");
        ImGui::Separator();

        const Code *code = vm.get_program_asm_code();

        // VM state
        {
            ImGui::Indent();
            ImGui::Text("State:         %s", vm.is_program_running() ? "running" : "stopped");
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper("%s", "When virtual machine is running, you cannot edit the code or the graph.");
            ImGui::Text("Debug:         %s", vm.is_debugging() ? "ON" : "OFF");
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper("%s", "When debugging is ON, you can run a program step by step.");
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
        fw::ImGuiEx::DrawHelper("%s", "This is the virtual machine's CPU"
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
            fw::ImGuiEx::DrawHelper("%s", "primary accumulator");
            draw_register_value(Register::rdx);
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper("%s", "base register");
            draw_register_value(Register::eip);
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper("%s", "instruction pointer");

            ImGui::Unindent();
        }
        ImGui::Unindent();

        // Assembly-like code
        ImGui::Separator();
        ImGui::Text("Memory:");
        ImGui::SameLine();
        fw::ImGuiEx::DrawHelper("%s", "Virtual Machine Memory.");
        ImGui::Separator();
        {
            ImGui::Indent();

            ImGui::Text("Bytecode:");
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper("%s", "The bytecode is the result of the Compilation process."
                                          "\nAfter source code has been parsed to a syntax tree, "
                                          "\nthe tree (or graph) is converted by the Compiler to an Assembly-like code.");
            ImGui::Checkbox("Auto-scroll ?", &m_scroll_to_curr_instr);
            ImGui::SameLine();
            fw::ImGuiEx::DrawHelper("%s", "to scroll automatically to the current instruction");
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
                            fw::ImGuiEx::DrawHelper("%s", "This is the next instruction to evaluate");
                        } else {
                            ImGui::Text(" %s", str.c_str());
                        }
                    }
                } else {
                    ImGui::TextWrapped("Nothing loaded, try to compile, run or debug.");
                    ImGui::SameLine();
                    fw::ImGuiEx::DrawHelper("%s", "To see a compiled program here you need first to:"
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

    ImGui::Begin(m_app->config.ui_startup_window_label);
    {
        fw::FontManager&  font_manager  = m_app->font_manager;
        fw::EventManager& event_manager = m_app->event_manager;
        ImGui::PopStyleColor();

        ImVec2 center_area(500.0f, 250.0f);
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImGui::SetCursorPosX((avail.x - center_area.x) / 2);
        ImGui::SetCursorPosY((avail.y - center_area.y) / 2);

        ImGui::BeginChild("center_area", center_area);
        {
            ImGui::Indent(center_area.x * 0.05f);

            ImGui::PushFont(font_manager.get_font(fw::FontSlot_ToolBtn));
            ImGui::NewLine();

            ImVec2 btn_size(center_area.x * 0.44f, 40.0f);
            if (ImGui::Button(ICON_FA_FILE" New File", btn_size))
                event_manager.dispatch( fw::EventID_REQUEST_FILE_NEW );
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER_OPEN" Open ...", btn_size))
                event_manager.dispatch( fw::EventID_REQUEST_FILE_BROWSE );

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

            for (auto [text, path]: examples) {
                std::string label;
                label.append(ICON_FA_BOOK" ");
                label.append(text);
                if (i++ % 2) ImGui::SameLine();
                if (ImGui::Button(label.c_str(), small_btn_size))
                {
                    m_app->open_file( fw::App::asset_path(path.c_str()) );
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

void NodableView::draw_file_window(ImGuiID dockspace_id, bool redock_all, HybridFile *file) {
    auto &vm = m_app->virtual_machine;

    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags =
            (file->changed ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoScrollbar;

    auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
    child_bg.w = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

    bool is_window_open = true;
    bool visible = ImGui::Begin(file->name.c_str(), &is_window_open, window_flags);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(1);

    if(visible)
    {
        const bool is_current_file = m_app->is_current(file);

        if (!is_current_file && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
            m_app->current_file = file;
        }

        // History bar on top
        draw_history_bar(file->get_history());

        // File View in the middle
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.35f));
        ImGui::PushFont(m_app->font_manager.get_font(fw::FontSlot_Code));
        const ImVec2 &size = ImGui::GetContentRegionAvail();

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

    Config& config = m_app->config;
    if (ImGui::Begin(config.ui_config_window_label))
    {
        ImGui::Text("Nodable Settings");
        if ( ImGui::Button("Reset Settings") )
        {
            config.reset_default();
        }

        if (ImGui::CollapsingHeader("Nodes", ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ImGui::Indent();
            if ( ImGui::CollapsingHeader("Colors"))
            {
                ImGui::ColorEdit4("default", &config.ui_node_fillColor.x);
                ImGui::ColorEdit4("highlighted", &config.ui_node_highlightedColor.x);
                ImGui::ColorEdit4("variable", &config.ui_node_variableColor.x);
                ImGui::ColorEdit4("instruction", &config.ui_node_instructionColor.x);
                ImGui::ColorEdit4("literal", &config.ui_node_literalColor.x);
                ImGui::ColorEdit4("function", &config.ui_node_invokableColor.x);
                ImGui::ColorEdit4("shadow", &config.ui_node_shadowColor.x);
                ImGui::ColorEdit4("border", &config.ui_node_slot_border_color.x);
                ImGui::ColorEdit4("border (highlighted)", &config.ui_node_borderHighlightedColor.x);
                ImGui::ColorEdit4("slot", &config.ui_node_slot_color.x);
                ImGui::ColorEdit4("slot (hovered)", &config.ui_node_slot_hovered_color.x);
            }

            if ( ImGui::CollapsingHeader("Slots"))
            {
                ImGui::Text("Property Slots:");
                ImGui::SliderFloat("slot radius", &config.ui_node_propertyslot_radius, 5.0f, 10.0f);

                ImGui::Separator();

                ImGui::Text("Code Flow Slots:");
                ImGui::SliderFloat2("slot size##codeflow", &config.ui_node_slot_size.x, 2.0f, 100.0f);
                ImGui::SliderFloat("slot padding##codeflow", &config.ui_node_slot_gap, 0.0f, 100.0f);
                ImGui::SliderFloat("slot radius##codeflow", &config.ui_node_slot_border_radius, 0.0f, 40.0f);
            }

            if ( ImGui::CollapsingHeader("Misc."))
            {
                ImGui::SliderFloat("spacing", &config.ui_node_spacing, 10.0f, 50.0f);
                ImGui::SliderFloat("velocity", &config.ui_node_speed, 1.0f, 10.0f);
                ImGui::SliderFloat4("padding", &config.ui_node_padding.x, 0.0f, 20.0f);
                ImGui::SliderFloat("border width", &config.ui_node_borderWidth, 0.0f, 10.0f);
                ImGui::SliderFloat("border width ratio (instructions)", &config.ui_node_instructionBorderRatio, 0.0f, 10.0f);
            }
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Wires / Code Flow"))
        {
            ImGui::Text("Wires");
            ImGui::SliderFloat("thickness##wires", &config.ui_wire_bezier_thickness, 0.5f, 10.0f);
            ImGui::SliderFloat2("roundness (min,max)##wires", &config.ui_wire_bezier_roundness.x, 0.0f, 1.0f);
            ImGui::SliderFloat2("fade length (min,max)##wires", &config.ui_wire_bezier_fade_length_minmax.x, 200.0f, 1000.0f);
            ImGui::ColorEdit4("color##wires", &config.ui_wire_color.x);
            ImGui::ColorEdit4("shadow color##wires", &config.ui_wire_shadowColor.x);

            ImGui::Separator();

            ImGui::Text("Code Flow");
            ImGui::ColorEdit4("color##codeflow", &config.ui_codeflow_color.x);
            ImGui::SliderFloat("thickness (ratio)##codeflow", &config.ui_codeflow_thickness_ratio, 0.1, 1.0);
        }

        if (ImGui::CollapsingHeader("Graph"))
        {
            ImGui::InputFloat("unfold delta time", &config.graph_unfold_dt);
            ImGui::InputInt("unfold iterations", &config.graph_unfold_iterations, 1, 1000);
            ImGui::ColorEdit4("grid color (major)", &config.ui_graph_grid_color_major.x);
            ImGui::ColorEdit4("grid color (minor)", &config.ui_graph_grid_color_minor.x);
            ImGui::SliderInt("grid size", &config.ui_graph_grid_size, 1, 500);
            ImGui::SliderInt("grid subdivisions", &config.ui_graph_grid_subdivs, 1, 16);
        }

        if ( m_app->config.common.debug && ImGui::CollapsingHeader("Pool"))
        {
            ImGui::Text("Pool stats:");
            auto pool = fw::Pool::get_pool();
            ImGui::Text(" - Node.................... %8zu", pool->get_all<Node>().size() );
            ImGui::Text(" - NodeView................ %8zu", pool->get_all<NodeView>().size() );
            ImGui::Text(" - Physics................. %8zu", pool->get_all<Physics>().size() );
            ImGui::Text(" - Scope................... %8zu", pool->get_all<Scope>().size() );
        }
    }
    ImGui::End();
}

void NodableView::on_draw_splashscreen()
{
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // Image
    ImGui::SameLine((ImGui::GetContentRegionAvail().x - m_logo->width) * 0.5f); // center img
    fw::ImGuiEx::Image(m_logo);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(50.0f, 30.0f));

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
        m_app->config.common.splashscreen = false;
    }
    ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
}

void NodableView::draw_history_bar(History *currentFileHistory) {
    if (ImGui::IsMouseReleased(0)) {
        m_is_history_dragged = false;
    }
    Config& config = m_app->config;
    float btn_spacing = config.ui_history_btn_spacing;
    float btn_height = config.ui_history_btn_height;
    float btn_width_max = config.ui_history_btn_width_max;

    size_t historySize = currentFileHistory->get_size();
    std::pair<int, int> history_range = currentFileHistory->get_command_id_range();
    float avail_width = ImGui::GetContentRegionAvail().x;
    float btn_width = fmin(btn_width_max, avail_width / float(historySize + 1) - btn_spacing);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(btn_spacing, 0));

    for (int cmd_pos = history_range.first; cmd_pos <= history_range.second; cmd_pos++) {
        ImGui::SameLine();

        std::string label("##" + std::to_string(cmd_pos));

        // Draw an highlighted button for the current history position
        if (cmd_pos == 0) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            ImGui::Button(label.c_str(), ImVec2(btn_width, btn_height));
            ImGui::PopStyleColor();
        } else // or a simple one for other history positions
        {
            ImGui::Button(label.c_str(), ImVec2(btn_width, btn_height));
        }

        // Hovered item
        if (ImGui::IsItemHovered()) {
            if (ImGui::IsMouseDown(0)) // hovered + mouse down
            {
                m_is_history_dragged = true;
            }

            // Draw command description
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, float(0.8));
            if (fw::ImGuiEx::BeginTooltip()) {
                ImGui::Text("%s", currentFileHistory->get_cmd_description_at(cmd_pos).c_str());
                fw::ImGuiEx::EndTooltip();
            }
            ImGui::PopStyleVar();
        }

        // When dragging history
        const auto xMin = ImGui::GetItemRectMin().x;
        const auto xMax = ImGui::GetItemRectMax().x;
        if (m_is_history_dragged &&
            ImGui::GetMousePos().x < xMax && ImGui::GetMousePos().x > xMin) {
            currentFileHistory->move_cursor(cmd_pos); // update history cursor position
        }


    }
    ImGui::PopStyleVar();
}

void NodableView::draw_toolbar_window() {

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
    Config& config = m_app->config;
    if (ImGui::Begin(config.ui_toolbar_window_label, NULL, flags ))
    {
        ImGui::PopStyleVar();
        VirtualMachine& vm   = m_app->virtual_machine;
        bool running         = vm.is_program_running();
        bool debugging       = vm.is_debugging();
        bool stopped         = vm.is_program_stopped();
        ImVec2 button_size   = config.ui_toolButton_size;

        ImGui::PushFont(m_app->font_manager.get_font(fw::FontSlot_ToolBtn));
        ImGui::BeginGroup();

        // compile
        if (ImGui::Button(ICON_FA_DATABASE " compile", button_size) && stopped) {
            m_app->compile_and_load_program();
        }
        ImGui::SameLine();

        // run
        if (running) ImGui::PushStyleColor(ImGuiCol_Button, m_app->config.common.button_activeColor);

        if (ImGui::Button(ICON_FA_PLAY " run", button_size) && stopped) {
            m_app->run_program();
        }
        if (running) ImGui::PopStyleColor();

        ImGui::SameLine();

        // debug
        if (debugging) ImGui::PushStyleColor(ImGuiCol_Button, m_app->config.common.button_activeColor);
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
                config.isolate_selection ? ICON_FA_CROP " isolation mode: ON " : ICON_FA_CROP " isolation mode: OFF",
                button_size)) {
            m_app->event_manager.dispatch( EventID_REQUEST_TOGGLE_ISOLATE_SELECTION );
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
    const Config& config  = m_app->config;

    dock_window(config.ui_help_window_label             , fw::AppView::Dockspace_RIGHT);
    dock_window(config.ui_config_window_label           , fw::AppView::Dockspace_RIGHT);
    dock_window(config.ui_file_info_window_label        , fw::AppView::Dockspace_RIGHT);
    dock_window(config.ui_node_properties_window_label  , fw::AppView::Dockspace_RIGHT);
    dock_window(config.ui_virtual_machine_window_label  , fw::AppView::Dockspace_RIGHT);
    dock_window(config.ui_imgui_config_window_label     , fw::AppView::Dockspace_RIGHT);
    dock_window(config.ui_toolbar_window_label          , fw::AppView::Dockspace_TOP);
}
