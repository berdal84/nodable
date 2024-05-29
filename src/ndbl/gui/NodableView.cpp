#include "NodableView.h"

#include <utility>

#include "tools/core/log.h"
#include "tools/core/system.h"
#include "tools/gui/Texture.h"
#include "tools/gui/ActionManagerView.h"
#include "tools/gui/Config.h"
#include "ndbl/core/NodeUtils.h"
#include "ndbl/core/language/Nodlang.h"

#include "Config.h"
#include "Event.h"
#include "File.h"
#include "FileView.h"
#include "History.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"
#include "build_info.h"
#include "ndbl/core/VirtualMachine.h"
#include "ndbl/core/assembly/Register.h"

using namespace ndbl;
using namespace ndbl::assembly;
using namespace tools;

template<typename T>
static func_type* create_variable_node_signature()
{ return func_type_builder<T(T)>::with_id("variable"); }

template<typename T>
static func_type* create_literal_node_signature()
{ return func_type_builder<T(/*void*/)>::with_id("literal"); }

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
    EXPECT(m_app, "should be defined");
}

NodableView::~NodableView()
{
    LOG_VERBOSE("ndbl::NodableView", "Destructor " OK "\n");
}

void NodableView::init()
{
    LOG_VERBOSE("ndbl::NodableView", "init ...\n");

    // Initialize parent class
    AppView::init();

    // Load splashscreen image
    Config* cfg = get_config();
    std::filesystem::path path = BaseApp::asset_path( cfg->ui_splashscreen_imagePath );
    m_logo = get_texture_manager()->load(path);

    // Add Actions: Bind shortcuts to Events
    action_manager.new_action<Event_DeleteNode>( "Delete", Shortcut{ SDLK_DELETE, KMOD_NONE } );
    action_manager.new_action<Event_ArrangeNode>( "Arrange", Shortcut{ SDLK_a, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager.new_action<Event_ToggleFolding>( "Fold", Shortcut{ SDLK_x, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager.new_action<Event_SelectNext>( "Next", Shortcut{ SDLK_n, KMOD_NONE } );
    action_manager.new_action<Event_FileSave>( ICON_FA_SAVE " Save", Shortcut{ SDLK_s, KMOD_CTRL } );
    action_manager.new_action<Event_FileSaveAs>( ICON_FA_SAVE " Save as", Shortcut{ SDLK_s, KMOD_CTRL } );
    action_manager.new_action<Event_FileClose>( ICON_FA_TIMES "  Close", Shortcut{ SDLK_w, KMOD_CTRL } );
    action_manager.new_action<Event_FileBrowse>( ICON_FA_FOLDER_OPEN " Open", Shortcut{ SDLK_o, KMOD_CTRL } );
    action_manager.new_action<Event_FileNew>( ICON_FA_FILE " New", Shortcut{ SDLK_n, KMOD_CTRL } );
    action_manager.new_action<Event_ShowWindow>( "Splashscreen", Shortcut{ SDLK_F1 }, EventPayload_ShowWindow{ "splashscreen" } );
    action_manager.new_action<Event_Exit>( ICON_FA_SIGN_OUT_ALT " Exit", Shortcut{ SDLK_F4, KMOD_ALT } );
    action_manager.new_action<Event_Undo>( "Undo", Shortcut{ SDLK_z, KMOD_CTRL } );
    action_manager.new_action<Event_Redo>( "Redo", Shortcut{ SDLK_y, KMOD_CTRL } );
    action_manager.new_action<Event_ToggleIsolationFlags>( "Isolate", Shortcut{ SDLK_i, KMOD_CTRL }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_TEXT_EDITOR );
    action_manager.new_action<Event_SelectionChange>( "Deselect", Shortcut{ 0, KMOD_NONE, "Double click on bg" }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager.new_action<Event_MoveSelection>( "Move Graph", Shortcut{ 0, KMOD_NONE, "Drag background" }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager.new_action<Event_FrameSelection>( "Frame Selection", Shortcut{ SDLK_f, KMOD_NONE }, EventPayload_FrameNodeViews{ FRAME_SELECTION_ONLY }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager.new_action<Event_FrameSelection>( "Frame All", Shortcut{ SDLK_f, KMOD_LCTRL }, EventPayload_FrameNodeViews{ FRAME_ALL } );

    // Prepare context menu items
    // 1) Blocks
    action_manager.new_action<Event_CreateNode>( ICON_FA_CODE " Condition", Shortcut{}, EventPayload_CreateNode{ NodeType_BLOCK_CONDITION } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_CODE " For Loop", Shortcut{}, EventPayload_CreateNode{ NodeType_BLOCK_FOR_LOOP } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_CODE " While Loop", Shortcut{}, EventPayload_CreateNode{ NodeType_BLOCK_WHILE_LOOP } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_CODE " Scope", Shortcut{}, EventPayload_CreateNode{ NodeType_BLOCK_SCOPE } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_CODE " Program", Shortcut{}, EventPayload_CreateNode{ NodeType_BLOCK_PROGRAM } );

    // 2) Variables
    action_manager.new_action<Event_CreateNode>( ICON_FA_DATABASE " Boolean Variable", Shortcut{}, EventPayload_CreateNode{ NodeType_VARIABLE_BOOLEAN, create_variable_node_signature<bool>() } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_DATABASE " Double Variable", Shortcut{}, EventPayload_CreateNode{ NodeType_VARIABLE_DOUBLE, create_variable_node_signature<double>() } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_DATABASE " Integer Variable", Shortcut{}, EventPayload_CreateNode{ NodeType_VARIABLE_INTEGER, create_variable_node_signature<int>() } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_DATABASE " String Variable", Shortcut{}, EventPayload_CreateNode{ NodeType_VARIABLE_STRING, create_variable_node_signature<std::string>() } );

    // 3) Literals
    action_manager.new_action<Event_CreateNode>( ICON_FA_FILE " Boolean Literal", Shortcut{}, EventPayload_CreateNode{ NodeType_LITERAL_BOOLEAN, create_variable_node_signature<bool>() } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_FILE " Double Literal", Shortcut{}, EventPayload_CreateNode{ NodeType_LITERAL_DOUBLE, create_variable_node_signature<double>() } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_FILE " Integer Literal", Shortcut{}, EventPayload_CreateNode{ NodeType_LITERAL_INTEGER, create_variable_node_signature<int>() } );
    action_manager.new_action<Event_CreateNode>( ICON_FA_FILE " String Literal", Shortcut{}, EventPayload_CreateNode{ NodeType_LITERAL_STRING, create_variable_node_signature<std::string>() } );

    // 4) Functions/Operators from the API
    const Nodlang* language = get_language();
    EXPECT(language != nullptr, "NodableView: language is null. Did you call init_language() ?")
    for ( auto& each_fct: language->get_api() )
    {
        const func_type* func_type = each_fct->get_type();
        std::string label;
        language->serialize_func_sig( label, func_type );
        action_manager.new_action<Event_CreateNode>( label.c_str(), Shortcut{}, EventPayload_CreateNode{ NodeType_INVOKABLE, func_type } );
    }

    LOG_VERBOSE("ndbl::NodableView", "init " OK "\n");
}

void NodableView::draw()
{
    EXPECT(m_logo != nullptr, "Logo is nullptr, did you call init() ?")

    // 1) Draw parent class
    //---------------------
    AppView::begin_draw();

    // 2) Draw this
    //-------------

    bool            redock_all      = true;
    Config*         cfg             = get_config();
    tools::Config*  tools_cfg       = tools::get_config();
    File*           current_file    = m_app->current_file;
    VirtualMachine* virtual_machine = get_virtual_machine();

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

        bool vm_is_stopped = virtual_machine->is_program_stopped();
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

            if (ImGui::MenuItem("Fullscreen", "", is_fullscreen()))
            {
                set_fullscreen( !is_fullscreen() );
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Reset Layout", "")) {
                set_layout_initialized(false);
            }

            ImGui::Separator();

            ImGuiEx::MenuItem<Event_ToggleIsolationFlags>( cfg->isolation );

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu("Run") )
        {
            bool vm_is_debugging = virtual_machine->is_debugging();

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
            if ( ImGui::MenuItem("Show debug info", "", tools_cfg->runtime_debug ) )
            {
                tools_cfg->runtime_debug = !tools_cfg->runtime_debug;
                ImGuiEx::debug = tools_cfg->runtime_debug;
            }

            if ( ImGui::MenuItem("Limit FPS", "", tools_cfg->delta_time_limit ) )
            {
                tools_cfg->delta_time_limit = !tools_cfg->delta_time_limit;
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
                ImGui::Checkbox("Hybrid history", &cfg->experimental_hybrid_history);
                ImGui::Checkbox("Graph auto-completion", &cfg->experimental_graph_autocompletion);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("An issue ?")) {
            if (ImGui::MenuItem("Report on Github.com")) {
                system::open_url_async("https://github.com/berdal84/nodable/issues");
            }

            if (ImGui::MenuItem("Report by email")) {
                system::open_url_async("mail:berenger@42borgata.com");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Show Splash Screen", "F1")) {
                show_splashscreen = true;
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
        bool show_startup_window = !show_splashscreen;
        if( show_startup_window )
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

    // 3) End parent class
    //---------------------
    AppView::end_draw();
}

void NodableView::draw_help_window() const
{
    Config* cfg = get_config();
    if (ImGui::Begin( cfg->ui_help_window_label))
    {
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
    Config* cfg = get_config();
    tools::Config* tools_cfg = tools::get_config();
    if( !tools_cfg->runtime_debug )
    {
        return;
    }

    if (ImGui::Begin( cfg->ui_imgui_config_window_label))
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

    Config* cfg = get_config();
    if (ImGui::Begin( cfg->ui_file_info_window_label))
    {
        m_app->current_file->view.draw_info_panel();
    }

    ImGui::End();
}

void NodableView::draw_node_properties_window()
{
    Config* cfg = get_config();
    if (ImGui::Begin( cfg->ui_node_properties_window_label))
    {
        if (NodeView* selected_view = NodeView::get_selected().get())
        {
            ImGui::Indent(10.0f);
            NodeView::draw_as_properties_panel(selected_view, &m_show_advanced_node_properties);
        }
    }
    ImGui::End();
}

void NodableView::draw_virtual_machine_window()
{
    Config* cfg = get_config();
    if (ImGui::Begin( cfg->ui_virtual_machine_window_label))
    {
        auto* vm = get_virtual_machine();

        ImGui::Text("Virtual Machine:");
        ImGui::SameLine();
        ImGuiEx::DrawHelper("%s", "The virtual machine - or interpreter - is a sort of implementation of \n"
                                      "an imaginary hardware able to run a set of simple instructions.");
        ImGui::Separator();

        const Code *code = vm->get_program_asm_code();

        // VM state
        {
            ImGui::Indent();
            ImGui::Text("State:         %s", vm->is_program_running() ? "running" : "stopped");
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "When virtual machine is running, you cannot edit the code or the graph.");
            ImGui::Text("Debug:         %s", vm->is_debugging() ? "ON" : "OFF");
            ImGui::SameLine();
            ImGuiEx::DrawHelper("%s", "When debugging is ON, you can run a program step by step.");
            ImGui::Text("Has program:   %s", code ? "YES" : "NO");
            if (code) {
                ImGui::Text("Program over:  %s", !vm->is_there_a_next_instr() ? "YES" : "NO");
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
                            vm->read_cpu_register(_register).to_string().c_str());
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
                    auto current_instr = vm->get_next_instr();
                    for (Instruction *each_instr: code->get_instructions()) {
                        auto str = Instruction::to_string(*each_instr);
                        if (each_instr == current_instr) {
                            if (m_scroll_to_curr_instr && vm->is_program_running()) {
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

void NodableView::draw_startup_window(ImGuiID dockspace_id)
{
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3f, 0.3f, 0.3f, 1.f));
    Config* cfg = get_config();
    ImGui::Begin( cfg->ui_startup_window_label);
    {
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

void NodableView::draw_file_window(ImGuiID dockspace_id, bool redock_all, File*file)
{
    VirtualMachine* vm = get_virtual_machine();

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
        ImGui::PushFont(font_manager.get_font(FontSlot_Code));
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

void NodableView::draw_config_window()
{
    auto* cfg = get_config();
    auto* tools_cfg = tools::get_config();

    if (ImGui::Begin( cfg->ui_config_window_label))
    {
        ImGui::Text("Nodable Settings");
        if ( ImGui::Button("Reset Settings") )
        {
            cfg->reset();
        }

        if (ImGui::CollapsingHeader("Nodes", ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ImGui::Indent();
            if ( ImGui::CollapsingHeader("Colors"))
            {
                ImGui::ColorEdit4("default", &cfg->ui_node_fillColor.x);
                ImGui::ColorEdit4("highlighted", &cfg->ui_node_highlightedColor.x);
                ImGui::ColorEdit4("variable", &cfg->ui_node_variableColor.x);
                ImGui::ColorEdit4("instruction", &cfg->ui_node_instructionColor.x);
                ImGui::ColorEdit4("literal", &cfg->ui_node_literalColor.x);
                ImGui::ColorEdit4("function", &cfg->ui_node_invokableColor.x);
                ImGui::ColorEdit4("shadow", &cfg->ui_node_shadowColor.x);
                ImGui::ColorEdit4("border", &cfg->ui_slot_border_color.x);
                ImGui::ColorEdit4("border (highlighted)", &cfg->ui_node_borderHighlightedColor.x);
                ImGui::ColorEdit4("slot", &cfg->ui_slot_color.x);
                ImGui::ColorEdit4("slot (hovered)", &cfg->ui_slot_hovered_color.x);
            }

            if ( ImGui::CollapsingHeader("Slots"))
            {
                ImGui::Text("Property Slots:");
                ImGui::SliderFloat("slot radius", &cfg->ui_slot_radius, 5.0f, 10.0f);

                ImGui::Separator();

                ImGui::Text("Code Flow Slots:");
                ImGui::SliderFloat2("slot size##codeflow", &cfg->ui_slot_size.x, 2.0f, 100.0f);
                ImGui::SliderFloat("slot padding##codeflow", &cfg->ui_slot_gap, 0.0f, 100.0f);
                ImGui::SliderFloat("slot radius##codeflow", &cfg->ui_slot_border_radius, 0.0f, 40.0f);
            }

            if ( ImGui::CollapsingHeader("Misc."))
            {
                ImGui::SliderFloat("spacing", &cfg->ui_node_spacing, 10.0f, 50.0f);
                ImGui::SliderFloat("velocity", &cfg->ui_node_speed, 1.0f, 10.0f);
                ImGui::SliderFloat4("padding", &cfg->ui_node_padding.x, 0.0f, 20.0f);
                ImGui::SliderFloat("border width", &cfg->ui_node_borderWidth, 0.0f, 10.0f);
                ImGui::SliderFloat("border width ratio (instructions)", &cfg->ui_node_instructionBorderRatio, 0.0f, 10.0f);
            }
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Wires / Code Flow"))
        {
            ImGui::Text("Wires");
            ImGui::SliderFloat("thickness##wires", &cfg->ui_wire_bezier_thickness, 0.5f, 10.0f);
            ImGui::SliderFloat2("roundness (min,max)##wires", &cfg->ui_wire_bezier_roundness.x, 0.0f, 1.0f);
            ImGui::SliderFloat2("fade length (min,max)##wires", &cfg->ui_wire_bezier_fade_length_minmax.x, 200.0f, 1000.0f);
            ImGui::ColorEdit4("color##wires", &cfg->ui_wire_color.x);
            ImGui::ColorEdit4("shadow color##wires", &cfg->ui_wire_shadowColor.x);

            ImGui::Separator();

            ImGui::Text("Code Flow");
            ImGui::ColorEdit4("color##codeflow", &cfg->ui_codeflow_color.x);
            ImGui::SliderFloat("thickness (ratio)##codeflow", &cfg->ui_codeflow_thickness_ratio, 0.1, 1.0);
        }

        if (ImGui::CollapsingHeader("Graph"))
        {
            ImGui::InputFloat("unfold delta time", &cfg->graph_unfold_dt);
            ImGui::InputInt("unfold iterations", &cfg->graph_unfold_iterations, 1, 1000);
            ImGui::ColorEdit4("grid color (major)", &cfg->ui_graph_grid_color_major.x);
            ImGui::ColorEdit4("grid color (minor)", &cfg->ui_graph_grid_color_minor.x);
            ImGui::SliderInt("grid size", &cfg->ui_grid_size, 1, 500);
            ImGui::SliderInt("grid subdivisions", &cfg->ui_grid_subdiv_count, 1, 16);
        }

        if (ImGui::CollapsingHeader("Shortcuts", ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ActionManagerView::draw(&action_manager);
        }

        if ( tools_cfg->runtime_debug && ImGui::CollapsingHeader("Pool"))
        {
            ImGui::Text("Pool stats:");
            auto pool = get_pool_manager()->get_pool();
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
        auto* tools_cfg = tools::get_config();

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
            show_splashscreen = false;
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
    auto* cfg           = get_config();
    float btn_spacing   = cfg->ui_history_btn_spacing;
    float btn_height    = cfg->ui_history_btn_height;
    float btn_width_max = cfg->ui_history_btn_width_max;

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

void NodableView::draw_toolbar_window()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 5.0f});

    auto* cfg = get_config();
    auto* tools_cfg = tools::get_config();

    if (ImGui::Begin( cfg->ui_toolbar_window_label, NULL, flags ))
    {
        ImGui::PopStyleVar();
        VirtualMachine* vm   = get_virtual_machine();
        bool running         = vm->is_program_running();
        bool debugging       = vm->is_debugging();
        bool stopped         = vm->is_program_stopped();
        auto button_size     = cfg->ui_toolButton_size;

        ImGui::PushFont(font_manager.get_font(FontSlot_ToolBtn));
        ImGui::BeginGroup();

        // compile
        if (ImGui::Button(ICON_FA_DATABASE " compile", button_size) && stopped) {
            m_app->compile_and_load_program();
        }
        ImGui::SameLine();

        // run
        if (running) ImGui::PushStyleColor(ImGuiCol_Button, tools_cfg->button_activeColor);

        if (ImGui::Button(ICON_FA_PLAY " run", button_size) && stopped) {
            m_app->run_program();
        }
        if (running) ImGui::PopStyleColor();

        ImGui::SameLine();

        // debug
        if (debugging) ImGui::PushStyleColor(ImGuiCol_Button, tools_cfg->button_activeColor);
        if (ImGui::Button(ICON_FA_BUG " debug", button_size) && stopped) {
            m_app->debug_program();
        }
        if (debugging) ImGui::PopStyleColor();
        ImGui::SameLine();

        // stepOver
        if (ImGui::Button(ICON_FA_ARROW_RIGHT " step over", button_size) && vm->is_debugging()) {
            vm->step_over();
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
        bool isolation_on = cfg->isolation & Isolation_ON;
        if (ImGui::Button(isolation_on ? ICON_FA_CROP " isolation mode: ON " : ICON_FA_CROP " isolation mode: OFF", button_size))
        {
            event_manager.dispatch( EventID_TOGGLE_ISOLATION_FLAGS );
        }
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::PopFont();
    }
    ImGui::End();
}

void NodableView::on_reset_layout()
{
    auto* cfg = get_config();

    // Dock windows to specific dockspace

    dock_window( cfg->ui_help_window_label             , AppView::Dockspace_RIGHT);
    dock_window( cfg->ui_config_window_label           , AppView::Dockspace_RIGHT);
    dock_window( cfg->ui_file_info_window_label        , AppView::Dockspace_RIGHT);
    dock_window( cfg->ui_node_properties_window_label  , AppView::Dockspace_RIGHT);
    dock_window( cfg->ui_virtual_machine_window_label  , AppView::Dockspace_RIGHT);
    dock_window( cfg->ui_imgui_config_window_label     , AppView::Dockspace_RIGHT);
    dock_window( cfg->ui_toolbar_window_label          , AppView::Dockspace_TOP);
}
