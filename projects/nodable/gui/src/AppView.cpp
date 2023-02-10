#include <ndbl/gui/AppView.h>

#include "fw/core/Log.h"
#include "fw/core/System.h"

#include <ndbl/gui/Event.h>
#include <ndbl/gui/App.h>
#include <ndbl/gui/File.h>
#include <ndbl/gui/FileView.h>
#include <ndbl/gui/History.h>
#include <ndbl/gui/NodeView.h>
#include <ndbl/gui/Settings.h>
#include <ndbl/gui/build_info.h>

#include <utility>

using namespace ndbl;
using namespace ndbl::assembly;

AppView::AppView(App *_app, fw::AppView::Conf _conf)
    : fw::AppView(_app, std::move(_conf))
    , m_logo(nullptr)
    , m_is_history_dragged(false)
    , m_show_properties_editor(false)
    , m_show_imgui_demo(false)
    , m_show_advanced_node_properties(false)
    , m_scroll_to_curr_instr(true)
{
}

AppView::~AppView() {}

bool AppView::on_init() {

    auto settings = Settings::get_instance();

    // Load splashscreen image
    App& app = App::get_instance();
    fw::TextureManager& texture_manager = app.texture_manager();
    m_logo = texture_manager.get_or_create_from(app.compute_asset_path(settings.ui_splashscreen_imagePath));

    return true;
}

bool AppView::on_draw(bool& redock_all) {
    bool isMainWindowOpen = true;
    auto &app             = App::get_instance();
    auto &settings        = Settings::get_instance();
    File *current_file    = app.current_file();
    auto &virtual_machine = VirtualMachine::get_instance();

    // 1. Draw Menu Bar
    if (ImGui::BeginMenuBar()) {
        History* current_file_history = current_file ? current_file->get_history() : nullptr;

        if (ImGui::BeginMenu("File")) {
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
            if (has_file) {
                fileView = current_file->get_view();
                auto_paste = fileView->experimental_clipboard_auto_paste();
            }

            if (ImGui::MenuItem(ICON_FA_COPY        "  Auto-paste clipboard", "", auto_paste, fileView)) {
                fileView->experimental_clipboard_auto_paste(!auto_paste);
            }

            fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_exit_triggered);

            ImGui::EndMenu();
        }

        bool vm_is_stopped = virtual_machine.is_program_stopped();
        if (ImGui::BeginMenu("Edit")) {
            if (current_file_history) {
                fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_undo_triggered);
                fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_redo_triggered);
                ImGui::Separator();
            }

            auto has_selection = NodeView::get_selected() != nullptr;

            if (ImGui::MenuItem("Delete", "Del.", false, has_selection && vm_is_stopped)) {
                m_app->event_manager().push_event(EventType_delete_node_action_triggered);
            }

            fw::ImGuiEx::MenuItemBindedToEvent(EventType_arrange_node_action_triggered, false, has_selection);
            fw::ImGuiEx::MenuItemBindedToEvent(EventType_toggle_folding_selected_node_action_triggered, false,
                                               has_selection);

            if (ImGui::MenuItem("Expand/Collapse recursive", nullptr, false, has_selection)) {
                Event event{};
                event.toggle_folding.type = EventType_toggle_folding_selected_node_action_triggered;
                event.toggle_folding.recursive = true;
                m_app->event_manager().push_event((fw::Event &) event);
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

            if (ImGui::MenuItem("Fullscreen", "", is_fullscreen())) {
                set_fullscreen(!is_fullscreen());
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Reset Layout", "")) {
                set_layout_initialized(false);
            }

            ImGui::Separator();

            fw::ImGuiEx::MenuItemBindedToEvent(fw::EventType_toggle_isolate_selection, settings.isolate_selection);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Run")) {
            bool vm_is_debugging = virtual_machine.is_debugging();

            if (ImGui::MenuItem(ICON_FA_PLAY" Run", "", false, vm_is_stopped)) {
                app.run_program();
            }

            if (ImGui::MenuItem(ICON_FA_BUG" Debug", "", false, vm_is_stopped)) {
                app.debug_program();
            }

            if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT" Step Over", "", false, vm_is_debugging)) {
                app.step_over_program();
            }

            if (ImGui::MenuItem(ICON_FA_STOP" Stop", "", false, !vm_is_stopped)) {
                app.stop_program();
            }

            if (ImGui::MenuItem(ICON_FA_UNDO " Reset", "", false, vm_is_stopped)) {
                app.reset_program();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Developer")) {
            if (ImGui::BeginMenu("Verbosity")) {
                auto menu_item_verbosity = [](fw::Log::Verbosity _verbosity, const char *_label) {
                    if (ImGui::MenuItem(_label, "", fw::Log::get_verbosity() == _verbosity)) {
                        fw::Log::set_verbosity(_verbosity);
                    }
                };

                menu_item_verbosity(fw::Log::Verbosity_Verbose, "Verbose");
                menu_item_verbosity(fw::Log::Verbosity_Message, "Message (default)");
                menu_item_verbosity(fw::Log::Verbosity_Warning, "Warning");
                menu_item_verbosity(fw::Log::Verbosity_Error, "Error");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Experimental")) {
                ImGui::Checkbox("Hybrid history", &settings.experimental_hybrid_history);
                ImGui::Checkbox("Graph auto-completion", &settings.experimental_graph_autocompletion);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("An issue ?")) {
            if (ImGui::MenuItem("Report on Github.com")) {
                fw::System::open_url_async("https://github.com/berdal84/Nodable/issues");
            }

            if (ImGui::MenuItem("Report by email")) {
                fw::System::open_url_async("mail:berenger@dalle-cort.fr");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Show Splash Screen", "F1")) {
                set_splashscreen_visible(true);
            }

            if (ImGui::MenuItem("Browse source code")) {
                fw::System::open_url_async("https://www.github.com/berdal84/nodable");
            }

            if (ImGui::MenuItem("Credits")) {
                fw::System::open_url_async("https://github.com/berdal84/nodable#credits-");
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // 2. Draw windows
    // All draw_xxx_window() are ImGui windows docked to a dockspace (defined in on_reset_layout() )



    if(!app.has_files())
    {
        if( !is_splashscreen_visible() )
            draw_startup_window(get_dockspace(Dockspace_ROOT));
    }
    else
    {
        draw_toolbar_window();

        auto ds_root = get_dockspace(Dockspace_ROOT);
        for (File *each_file: app.get_files())
        {
            draw_file_window(ds_root, redock_all, each_file);
        }

        draw_virtual_machine_window();
        draw_settings_window();
        draw_imgui_settings_window();
        draw_file_info_window();
        draw_node_properties_window();
        draw_help_window();
    }
    return true;
}

void AppView::draw_help_window() const {
    if (ImGui::Begin(Settings::get_instance().ui_help_window_label)) {
        ImGui::PushFont(get_font(fw::FontSlot_Heading));
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
        ImGui::PushFont(get_font(fw::FontSlot_Heading));
        ImGui::Text("Quick start");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextWrapped("Nodable UI is designed as following:\n");
        fw::ImGuiEx::BulletTextWrapped("On the left side a (light) text editor allows to edit source code.\n");
        fw::ImGuiEx::BulletTextWrapped(
                "At the center, there is the graph editor where you can create/delete/connect nodes\n");
        fw::ImGuiEx::BulletTextWrapped(
                "On the right side (this side) you will find many tabs to manage additional settings such as node properties, virtual machine or app properties\n");
        fw::ImGuiEx::BulletTextWrapped("At the top, between the menu and the editors, there is a tool bar."
                                       " There, few buttons will serve to compile, run and debug your program.");
        fw::ImGuiEx::BulletTextWrapped("And at the bottom, below the editors, there is a status bar."
                                       " This bar will display important messages, warning, and errors. You can expand it to get older messages.");
    }
    ImGui::End();
}

void AppView::draw_imgui_settings_window() const {
    if (ImGui::Begin(Settings::get_instance().ui_imgui_settings_window_label)) {
        ImGui::ShowStyleEditor();
    }
    ImGui::End();
}

void AppView::draw_file_info_window() const {
    if (auto current_file = App::get_instance().current_file()) {
        if (ImGui::Begin(Settings::get_instance().ui_file_info_window_label)) {
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

void AppView::draw_node_properties_window() {
    if (ImGui::Begin(Settings::get_instance().ui_node_properties_window_label)) {
        NodeView *view = NodeView::get_selected();
        if (view) {
            ImGui::Indent(10.0f);
            NodeView::draw_as_properties_panel(view, &m_show_advanced_node_properties);
        }
    }
    ImGui::End();
}

void AppView::draw_virtual_machine_window() {
    if (ImGui::Begin(Settings::get_instance().ui_virtual_machine_window_label)) {
        auto &vm = VirtualMachine::get_instance();

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

void AppView::draw_startup_window(ImGuiID dockspace_id) {
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3f, 0.3f, 0.3f, 1.f));

    ImGui::Begin(Settings::get_instance().ui_startup_window_label);
    {
        auto &app = App::get_instance();

        ImGui::PopStyleColor();

        ImVec2 center_area(500.0f, 250.0f);
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImGui::SetCursorPosX((avail.x - center_area.x) / 2);
        ImGui::SetCursorPosY((avail.y - center_area.y) / 2);

        ImGui::BeginChild("center_area", center_area);
        {
            ImGui::Indent(center_area.x * 0.05f);

            ImGui::PushFont(get_font(fw::FontSlot_ToolBtn));
            ImGui::NewLine();

            fw::vec2 btn_size(center_area.x * 0.44f, 40.0f);
            if (ImGui::Button(ICON_FA_FILE" New File", btn_size))
                m_app->event_manager().push_event(fw::EventType_new_file_triggered);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER_OPEN" Open ...", btn_size))
                m_app->event_manager().push_event(fw::EventType_browse_file_triggered);

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();

            ImGui::Text("%s", "Open an example");
            std::vector<std::pair<std::string, std::string>> examples;
            examples.emplace_back("Single expressions    ", "examples/arithmetic.cpp");
            examples.emplace_back("Multi instructions    ", "examples/multi-instructions.cpp");
            examples.emplace_back("Conditional Structures", "examples/if-else.cpp");
            examples.emplace_back("For Loop              ", "examples/for-loop_count.cpp");

            int i = 0;
            fw::vec2 small_btn_size(btn_size.x, btn_size.y * 0.66f);

            for (auto [text, path]: examples) {
                std::string label;
                label.append(ICON_FA_BOOK" ");
                label.append(text);
                if (i++ % 2) ImGui::SameLine();
                if (ImGui::Button(label.c_str(), small_btn_size)) {
                    std::string each_path = app.compute_asset_path(path.c_str());
                    app.open_file(each_path);
                }
            }

            ImGui::PopFont();

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::TextColored(fw::vec4(0, 0, 0, 0.30f), "%s", BuildInfo::version);
            ImGui::Unindent();
        }
        ImGui::EndChild();
    }
    ImGui::End(); // Startup Window
}

void AppView::draw_file_window(ImGuiID dockspace_id, bool redock_all, File *file) {
    auto &app = App::get_instance();
    auto &vm = VirtualMachine::get_instance();

    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags =
            (file->has_changed() ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, fw::vec2(0, 0));

    auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
    child_bg.w = 0;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

    bool is_window_open = true;
    if( ImGui::Begin(file->get_name().c_str(), &is_window_open, window_flags) )
    {
        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar();
        const bool is_current_file = app.is_current(file);

        if (!is_current_file && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
            app.current_file(file);
        }

        // History bar on top
        draw_history_bar(file->get_history());

        // File View in the middle
        View *eachFileView = file->get_view();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, fw::vec4(0, 0, 0, 0.35f));
        ImGui::PushFont(get_font(fw::FontSlot_Code));
        eachFileView->draw_as_child("FileView", ImGui::GetContentRegionAvail(), false);
        ImGui::PopFont();
        ImGui::PopStyleColor();

        if (is_current_file && file->get_view()->text_has_changed())
        {
            vm.release_program();
        }
    }
    ImGui::End(); // File Window

    if (!is_window_open) {
        app.close_file(file);
    }
}

void AppView::draw_settings_window() {
    if (ImGui::Begin(Settings::get_instance().ui_settings_window_label)) {
        Settings &settings = Settings::get_instance();

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
        ImGui::Checkbox("arrows", &settings.ui_wire_displayArrows);
        ImGui::Unindent();

        ImGui::Text("Nodes:");
        ImGui::Indent();
        ImGui::SliderFloat("property connector radius", &settings.ui_node_propertyConnectorRadius, 1.0f, 10.0f);
        ImGui::SliderFloat("padding", &settings.ui_node_padding, 1.0f, 20.0f);
        ImGui::SliderFloat("speed", &settings.ui_node_speed, 0.0f, 100.0f);
        ImGui::SliderFloat("spacing", &settings.ui_node_spacing, 0.0f, 100.0f);
        ImGui::SliderFloat("node connector padding", &settings.ui_node_connector_padding, 0.0f, 100.0f);
        ImGui::SliderFloat("node connector height", &settings.ui_node_connector_height, 2.0f, 100.0f);
        ImGui::ColorEdit4("variables color", &settings.ui_node_variableColor.x);
        ImGui::ColorEdit4("instruction color", &settings.ui_node_instructionColor.x);
        ImGui::ColorEdit4("literal color", &settings.ui_node_literalColor.x);
        ImGui::ColorEdit4("function color", &settings.ui_node_invokableColor.x);
        ImGui::ColorEdit4("shadow color", &settings.ui_node_shadowColor.x);
        ImGui::ColorEdit4("border color", &settings.ui_node_borderColor.x);
        ImGui::ColorEdit4("high. color", &settings.ui_node_highlightedColor.x);
        ImGui::ColorEdit4("border high. color", &settings.ui_node_borderHighlightedColor.x);
        ImGui::ColorEdit4("fill color", &settings.ui_node_fillColor.x);
        ImGui::ColorEdit4("node connector color", &settings.ui_node_nodeConnectorColor.x);
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

void AppView::on_draw_splashscreen()
{
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // Image
    ImGui::SameLine((ImGui::GetContentRegionAvail().x - m_logo->width) * 0.5f); // center img
    ImGui::Image((void *) (intptr_t) m_logo->image, fw::vec2((float) m_logo->width, (float) m_logo->height));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, fw::vec2(50.0f, 30.0f));

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
        set_splashscreen_visible(false);
    }
    ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
}

void AppView::draw_history_bar(History *currentFileHistory) {
    if (ImGui::IsMouseReleased(0)) {
        m_is_history_dragged = false;
    }
    auto &settings = Settings::get_instance();
    float btn_spacing = settings.ui_history_btn_spacing;
    float btn_height = settings.ui_history_btn_height;
    float btn_width_max = settings.ui_history_btn_width_max;

    size_t historySize = currentFileHistory->get_size();
    std::pair<int, int> history_range = currentFileHistory->get_command_id_range();
    float avail_width = ImGui::GetContentRegionAvail().x;
    float btn_width = fmin(btn_width_max, avail_width / float(historySize + 1) - btn_spacing);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, fw::vec2(btn_spacing, 0));

    for (int cmd_pos = history_range.first; cmd_pos <= history_range.second; cmd_pos++) {
        ImGui::SameLine();

        std::string label("##" + std::to_string(cmd_pos));

        // Draw an highlighted button for the current history position
        if (cmd_pos == 0) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            ImGui::Button(label.c_str(), fw::vec2(btn_width, btn_height));
            ImGui::PopStyleColor();
        } else // or a simple one for other history positions
        {
            ImGui::Button(label.c_str(), fw::vec2(btn_width, btn_height));
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

void AppView::draw_toolbar_window() {

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, fw::vec2(5.0f, 5.0f));
    if (ImGui::Begin(Settings::get_instance().ui_toolbar_window_label, NULL, flags )) {
        ImGui::PopStyleVar();

        auto &app = App::get_instance();
        auto &vm = VirtualMachine::get_instance();
        auto &settings = Settings::get_instance();

        bool running = vm.is_program_running();
        bool debugging = vm.is_debugging();
        bool stopped = vm.is_program_stopped();
        fw::vec2 button_size  = settings.ui_toolButton_size;

        ImGui::PushFont(get_font(fw::FontSlot_ToolBtn));

        ImGui::BeginGroup();

        // compile
        if (ImGui::Button(ICON_FA_DATABASE " compile", button_size) && stopped) {
            app.compile_and_load_program();
        }
        ImGui::SameLine();

        // run
        if (running) ImGui::PushStyleColor(ImGuiCol_Button, m_conf.button_activeColor);

        if (ImGui::Button(ICON_FA_PLAY " run", button_size) && stopped) {
            app.run_program();
        }
        if (running) ImGui::PopStyleColor();

        ImGui::SameLine();

        // debug
        if (debugging) ImGui::PushStyleColor(ImGuiCol_Button, m_conf.button_activeColor);
        if (ImGui::Button(ICON_FA_BUG " debug", button_size) && stopped) {
            app.debug_program();
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
            app.stop_program();
        }
        ImGui::SameLine();

        // reset
        if (ImGui::Button(ICON_FA_UNDO " reset graph", button_size)) {
            app.reset_program();
        }
        ImGui::SameLine();

        // enter isolation mode
        if (ImGui::Button(
                settings.isolate_selection ? ICON_FA_CROP " isolation mode: ON " : ICON_FA_CROP " isolation mode: OFF",
                button_size)) {
            m_app->event_manager().push_event(fw::EventType_toggle_isolate_selection);
        }
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::PopFont();
    }
    ImGui::End();
}

bool AppView::on_reset_layout() {
    // Dock windows to specific dockspace
    auto& settings = Settings::get_instance();
    dock_window(settings.ui_help_window_label             , Dockspace_RIGHT);
    dock_window(settings.ui_settings_window_label         , Dockspace_RIGHT);
    dock_window(settings.ui_file_info_window_label        , Dockspace_RIGHT);
    dock_window(settings.ui_node_properties_window_label  , Dockspace_RIGHT);
    dock_window(settings.ui_virtual_machine_window_label  , Dockspace_RIGHT);
    dock_window(settings.ui_imgui_settings_window_label   , Dockspace_RIGHT);
    dock_window(settings.ui_toolbar_window_label          , Dockspace_TOP);

    return true;
}
