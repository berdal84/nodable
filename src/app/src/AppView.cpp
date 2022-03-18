#include <nodable/app/AppView.h>

#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/BuildInfo.h>
#include <nodable/core/Texture.h>
#include <nodable/app/Settings.h>
#include <nodable/core/System.h>
#include <nodable/app/App.h>
#include <nodable/app/AppContext.h>
#include <nodable/app/NodeView.h>
#include <nodable/app/File.h>
#include <nodable/core/Log.h>
#include <nodable/app/FileView.h>
#include <nodable/app/History.h>
#include <nodable/app/constants.h>
#include <nodable/app/Event.h>

using namespace Nodable;
using namespace Nodable::Asm;

AppView::AppView(AppContext* _ctx, const char* _name )
    : View(_ctx)
    , m_context(_ctx)
    , m_background_color()
    , m_show_startup_window(true)
    , m_is_layout_initialized(false)
    , m_startup_screen_title("##STARTUPSCREEN")
    , m_is_history_dragged(false)
    , m_sdl_window_name(_name)
    , m_show_properties_editor(false)
    , m_show_imgui_demo(false)
    , m_show_advanced_node_properties(false)
    , m_scroll_to_curr_instr(true)
{
}

AppView::~AppView()
{}

bool AppView::init()
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        LOG_ERROR( "AppView", "SDL Error: %s\n", SDL_GetError())
        return false;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    m_sdl_window = SDL_CreateWindow(m_sdl_window_name.c_str(),
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   800,
                                   600,
                                SDL_WINDOW_OPENGL |
                                SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_MAXIMIZED |
                                SDL_WINDOW_SHOWN
                                );

    this->m_sdl_gl_context = SDL_GL_CreateContext(m_sdl_window);
    SDL_GL_SetSwapInterval(1); // Enable vsync


    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    io.FontAllowUserScaling = true;
	//io.WantCaptureKeyboard  = true;
	//io.WantCaptureMouse     = true;

	// Setup Dear ImGui style
    m_context->settings->patch_imgui_style(ImGui::GetStyle());

    // load fonts (TODO: hum, load only if font is used...)
    for ( auto& each_font : m_context->settings->ui_text_fonts )
    {
        load_font(each_font);
    }

    // Assign fonts (user might want to change it later, but we need defaults)
    for( auto each_slot = 0; each_slot < FontSlot_COUNT; ++each_slot )
    {
        const char* font_id = m_context->settings->ui_text_defaultFontsId[each_slot];
        m_fonts[each_slot] = get_font_by_id(font_id);
    }

    // Configure ImGui Style
    ImGuiStyle& style = ImGui::GetStyle();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    gl3wInit();
    ImGui_ImplSDL2_InitForOpenGL(m_sdl_window, m_sdl_gl_context);
    const char* glsl_version = NULL; // let backend decide wich version to use, usually 130 (pc) or 150 (macos).
    ImGui_ImplOpenGL3_Init(glsl_version);

	return true;
}

ImFont* AppView::get_font_by_id(const char *id) {
    return m_loaded_fonts.at(id );
}

ImFont* AppView::load_font(const FontConf &fontConf) {

    NODABLE_ASSERT(m_loaded_fonts.find(fontConf.id) == m_loaded_fonts.end()); // do not allow the use of same key for different fonts

    ImFont*  font     = nullptr;
    auto&    io       = ImGui::GetIO();
    Settings* settings = m_context->settings;

    // Create font
    {
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;

        //io.Fonts->AddFontDefault();
        std::string fontPath = m_context->app->get_asset_path(fontConf.path);
        LOG_VERBOSE("AppView", "Adding font from file ... %s\n", fontPath.c_str())
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontConf.size, &config);
    }

    // Add Icons my merging to previous font.
    if ( fontConf.enableIcons ) {
        // merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphMinAdvanceX = settings->ui_icons.size; // monospace to fix text alignment in drop down menus.
        auto fontPath = m_context->app->get_asset_path(settings->ui_icons.path);
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), settings->ui_icons.size, &config, icons_ranges);
        LOG_VERBOSE("AppView", "Adding icons to font ...\n")
    }

    m_loaded_fonts.insert({fontConf.id, font});
    LOG_MESSAGE("AppView", "Font %s added to register with the id \"%s\"\n", fontConf.path, fontConf.id)
    return font;
}

bool AppView::draw()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_sdl_window);
	ImGui::NewFrame();
    ImGui::SetCurrentFont( m_fonts[FontSlot_Paragraph] );

    // Startup Window
    draw_startup_window();

    // Demo Window
    {
        if (m_show_imgui_demo){
            ImGui::SetNextWindowPos(vec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&m_show_imgui_demo);
        }
    }

    // Fullscreen m_sdlWindow
    {

		// Get current file's history
		History* currentFileHistory = nullptr;

		if ( File* file = m_context->app->get_curr_file())
        {
            currentFileHistory = file->getHistory();
        }

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


        // Remove padding
        bool isMainWindowOpen = true;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0.0f, 0.0f));
        ImGui::Begin("Nodable", &isMainWindowOpen, window_flags);
        {
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(2);

            bool redock_all = false;

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    //ImGui::MenuItem(ICON_FA_FILE   "  New", "Ctrl + N");
                    if (ImGui::MenuItem(ICON_FA_FOLDER      "  Open", "Ctrl + O")) browse_file();
                    if (ImGui::MenuItem(ICON_FA_SAVE        "  Save", "Ctrl + S")) m_context->app->save_file();
                    if (ImGui::MenuItem(ICON_FA_TIMES       "  Close", "Ctrl + W")) m_context->app->close_file();

                    FileView *fileView = nullptr;
                    bool auto_paste;
                    if (auto file = m_context->app->get_curr_file()) {
                        fileView = file->getView();
                        auto_paste = fileView->experimental_clipboard_auto_paste();
                    }

                    if (ImGui::MenuItem(ICON_FA_COPY        "  Auto-paste clipboard", "", auto_paste, fileView)) {
                        fileView->experimental_clipboard_auto_paste(!auto_paste);
                    }

                    if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT"  Quit", "Alt + F4")) m_context->app->flag_to_stop();

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit")) {
                    if (currentFileHistory) {
                        if (ImGui::MenuItem("Undo", "Ctrl + Z")) currentFileHistory->undo();
                        if (ImGui::MenuItem("Redo", "Ctrl + Y")) currentFileHistory->redo();
                        ImGui::Separator();
                    }

                    auto has_selection = NodeView::GetSelected() != nullptr;

                    if ( ImGui::MenuItem("Delete", "Del.", false, has_selection && m_context->vm->is_program_stopped() ) )
                    {
                        EventManager::push_event(EventType::delete_node_action_triggered);
                    }

                    if ( ImGui::MenuItem("Arrange nodes", "A", false, has_selection) )
                    {
                        EventManager::push_event(EventType::arrange_node_action_triggered);
                    }

                    if ( ImGui::MenuItem("Expand (toggle)", "X", false, has_selection) )
                    {
                        EventManager::push_event(EventType::expand_selected_node_action_triggered);
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View")) {
                    //auto frame = ImGui::MenuItem("Frame All", "F");
                    redock_all |= ImGui::MenuItem("Redock documents");

                    ImGui::Separator();
                    auto viewDetailMinimalist = ImGui::MenuItem("Minimalist View", "",
                                                                NodeView::s_viewDetail == NodeViewDetail::Minimalist);
                    auto viewDetailEssential = ImGui::MenuItem("Essential View", "",
                                                               NodeView::s_viewDetail == NodeViewDetail::Essential);
                    auto viewDetailExhaustive = ImGui::MenuItem("Exhaustive View", "",
                                                                NodeView::s_viewDetail == NodeViewDetail::Exhaustive);

                    if (viewDetailMinimalist) {
                        NodeView::SetDetail(NodeViewDetail::Minimalist);
                    } else if (viewDetailEssential) {
                        NodeView::SetDetail(NodeViewDetail::Essential);
                    } else if (viewDetailExhaustive) {
                        NodeView::SetDetail(NodeViewDetail::Exhaustive);
                    }

                    ImGui::Separator();
                    m_show_properties_editor = ImGui::MenuItem(ICON_FA_COGS "  Show Properties", "",
                                                               m_show_properties_editor);
                    m_show_imgui_demo = ImGui::MenuItem("Show ImGui Demo", "", m_show_imgui_demo);

                    ImGui::Separator();

                    if (SDL_GetWindowFlags(m_sdl_window) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                        auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", true);
                        if (toggleFullscreen)
                            SDL_SetWindowFullscreen(m_sdl_window, 0);
                    } else {
                        auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", false);
                        if (toggleFullscreen) {
                            SDL_SetWindowFullscreen(m_sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                    }

                    ImGui::Separator();

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Run")) {
                    if (ImGui::MenuItem(ICON_FA_PLAY" Run") && m_context->vm->is_program_stopped()) {
                        m_context->app->vm_run();
                    }

                    if (ImGui::MenuItem(ICON_FA_BUG" Debug") && m_context->vm->is_program_stopped()) {
                        m_context->app->vm_debug();
                    }

                    if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT" Step Over") && m_context->vm->is_debugging()) {
                        m_context->app->vm_step_over();
                    }

                    if (ImGui::MenuItem(ICON_FA_STOP" Stop") && !m_context->vm->is_program_stopped()) {
                        m_context->app->vm_stop();
                    }

                    if (ImGui::MenuItem(ICON_FA_UNDO " Reset")) {
                        m_context->app->vm_stop();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Experimental"))
                {
                    bool& hybrid = m_context->settings->experimental_hybrid_history;
                    if (ImGui::MenuItem(ICON_FA_EXCLAMATION " Enable hybrid history", "", hybrid))
                    {
                        hybrid = !hybrid;
                    }

                    bool& autocompletion = m_context->settings->experimental_graph_autocompletion;
                    if (ImGui::MenuItem(ICON_FA_EXCLAMATION " Enable graph autocompletion", "", autocompletion))
                    {
                        autocompletion = !autocompletion;
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("An issue ?")) {
                    if (ImGui::MenuItem("Report on Github.com")) {
                        System::OpenURL("https://github.com/berdal84/Nodable/issues");
                    }

                    if (ImGui::MenuItem("Report by email")) {
                        System::OpenURL("mail:berenger@dalle-cort.fr");
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Help")) {
                    if (ImGui::MenuItem("Show Startup Screen", "F1")) {
                        m_show_startup_window = true;
                    }

                    if (ImGui::MenuItem("Browse source code")) {
                        System::OpenURL("https://www.github.com/berdal84/nodable");
                    }

                    if (ImGui::MenuItem("Credits")) {
                        System::OpenURL("https://github.com/berdal84/nodable#credits-");
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
            draw_tool_bar();


            /*
             * Main Layout
             */

            ImGuiID dockspace_main = ImGui::GetID("dockspace_main");
            ImGuiID dockspace_center = ImGui::GetID("dockspace_center");
            ImGuiID dockspace_side_panel = ImGui::GetID("dockspace_side_panel");


            if (!m_is_layout_initialized) {
                ImGui::DockBuilderRemoveNode(dockspace_main); // Clear out existing layout
                ImGui::DockBuilderAddNode(dockspace_main, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_main, ImGui::GetMainViewport()->Size);
                ImGui::DockBuilderSplitNode(dockspace_main, ImGuiDir_Right,
                                            m_context->settings->ui_layout_propertiesRatio, &dockspace_side_panel,
                                            NULL);

                ImGui::DockBuilderDockWindow(k_node_props_window_name, dockspace_side_panel);
                ImGui::DockBuilderDockWindow(k_assembly_window_name, dockspace_side_panel);
                ImGui::DockBuilderDockWindow(k_app_settings_window_name, dockspace_side_panel);
                ImGui::DockBuilderDockWindow(k_file_info_window_name, dockspace_side_panel);
                ImGui::DockBuilderDockWindow(k_imgui_settings_window_name, dockspace_side_panel);
                ImGui::DockBuilderFinish(dockspace_main);
                m_is_layout_initialized = true;
            }

            /*
            * Fill the layout with content
            */
            ImGui::DockSpace(dockspace_main);

            if (ImGui::Begin(k_node_props_window_name)) {
                NodeView *view = NodeView::GetSelected();
                if (view) {
                    ImGui::Indent(10.0f);
                    NodeView::DrawNodeViewAsPropertiesPanel(view, &m_show_advanced_node_properties);
                }
            }
            ImGui::End();

            if (ImGui::Begin(k_assembly_window_name))
            {
                draw_vm_view();
            }
            ImGui::End();

            if (ImGui::Begin(k_app_settings_window_name))
            {
                draw_properties_editor();
            }
            ImGui::End();

            if (ImGui::Begin(k_imgui_settings_window_name))
            {
                ImGui::ShowStyleEditor();
            }
            ImGui::End();

            // File info
            ImGui::Begin(k_file_info_window_name);
            {
                const File* currFile = m_context->app->get_curr_file();

                if (currFile)
                {
                    FileView* fileView = currFile->getView();
                    fileView->drawFileInfo();
                }
                else
                {
                    ImGui::Text("No open file");
                }

            }
            ImGui::End();

            // Opened documents
            for (size_t fileIndex = 0; fileIndex < m_context->app->get_file_count(); fileIndex++)
            {
                draw_file_editor(dockspace_main, redock_all, fileIndex);
            }


        }
        ImGui::End(); // Main window
    }

    draw_file_browser();

    // Rendering
	ImGui::Render();
	SDL_GL_MakeCurrent(m_sdl_window, this->m_sdl_gl_context);
	auto io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(m_background_color.Value.x, m_background_color.Value.y, m_background_color.Value.z, m_background_color.Value.w);
	glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(m_sdl_window);

    // limit frame rate
    if (ImGui::GetIO().DeltaTime < k_desired_delta_time)
    {
        SDL_Delay((unsigned int)((k_desired_delta_time - ImGui::GetIO().DeltaTime) * 1000.f) );
    }

    return false;
}

void AppView::draw_vm_view()
{
    ImGui::Text("Virtual Machine State");
    ImGui::Separator();

    Asm::VM* vm = m_context->vm;
    if ( !vm )
    {
        ImGui::Text("Sorry... Apparently the Virtual Machine can't be found.");
    }
    else
    {
        // VM state
        {
            ImGui::Indent();
            ImGui::Text("VM is %s", vm->is_program_running() ? "running" : "stopped");
            ImGui::Text("Debug: %s", vm->is_debugging() ? "ON" : "OFF");
            auto *code = vm->get_program_asm_code();
            ImGui::Text("Has program: %s", code ? "YES" : "NO");
            if (code)
            {
                ImGui::Text("Program over: %s", vm->is_program_over() ? "YES" : "NO");

            }
            ImGui::Unindent();
        }

        // VM Registers
        ImGui::Separator();
        ImGui::Text("Registers:");
        ImGui::Separator();
        {
            ImGui::Indent();
            ImGui::Text("%s: %#16llx (primary accumulator)",
                        Asm::to_string(Asm::Register::rax),
                        vm->get_register_val(Asm::Register::rax));
            ImGui::Text("%s: %#16llx (base register)",
                        Asm::to_string(Asm::Register::rdx),
                        vm->get_register_val(Asm::Register::rdx));
            ImGui::Text("%s: %#16llx (instruction pointer)",
                        Asm::to_string(Asm::Register::eip),
                        vm->get_register_val(Asm::Register::eip));
            //ImGui::Text("epb: %#16llx", vm->get_register_val(Asm::Register::ebp));
            ImGui::Unindent();
        }

        // Assembly-like code
        ImGui::Separator();
        ImGui::Text("Loaded Program:");
        ImGui::Separator();
        {
            ImGui::Indent();
            ImGui::Checkbox("Auto-scroll to current line", &m_scroll_to_curr_instr);
            ImGui::Separator();

            ImGui::Text("Assembly-like source:");
            ImGui::Separator();
            {
                ImGui::BeginChild("AssemblyCodeChild", ImGui::GetContentRegionAvail(), true );
                const Code* code = vm->get_program_asm_code();
                if ( code  )
                {
                    auto current_instr = vm->get_next_instr();
                    for( Instr* each_instr : code->get_instructions() )
                    {
                        auto str = Instr::to_string( *each_instr );
                        if ( each_instr == current_instr )
                        {
                            if ( m_scroll_to_curr_instr && vm->is_program_running() )
                            {
                                ImGui::SetScrollHereY();
                            }
                            ImGui::TextColored( ImColor(200,0,0), ">%s", str.c_str() );
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
                }
                ImGui::EndChild();
            }
            ImGui::Unindent();
        }
    }
}

void AppView::draw_file_browser()
{
    m_file_browser.Display();
    if (m_file_browser.HasSelected())
    {
        auto selectedFiles = m_file_browser.GetMultiSelected();
        for (const auto & selectedFile : selectedFiles)
        {
            m_context->app->open_file(selectedFile.c_str());
        }
        m_file_browser.ClearSelected();
        m_file_browser.Close();
    }
}

void AppView::draw_file_editor(ImGuiID dockspace_id, bool redock_all, size_t fileIndex)
{
    File *file = m_context->app->get_file_at(fileIndex);

    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags = (file->isModified() ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0, 0));

    auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
    child_bg.w = 0;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

    bool open = true;
    bool visible = ImGui::Begin(file->getName().c_str(), &open, window_flags);
    {
        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar();

        if (visible)
        {
            const bool isCurrentFile = fileIndex == m_context->app->get_curr_file_index();

            if ( !isCurrentFile && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                m_context->app->set_curr_file(fileIndex);
            }

            // History bar on top
            draw_history_bar(file->getHistory());

            // File View in the middle
            View* eachFileView = file->getView();
            vec2 availSize = ImGui::GetContentRegionAvail();

            if ( isCurrentFile )
            {
                availSize.y -= ImGui::GetTextLineHeightWithSpacing();
            }

            ImGui::PushStyleColor(ImGuiCol_ChildBg, vec4(0,0,0,0.35f) );
            ImGui::PushFont(m_fonts[FontSlot_Code] );
            eachFileView->drawAsChild("FileView", availSize, false);
            ImGui::PopFont();
            ImGui::PopStyleColor();

            // Status bar
            if ( isCurrentFile )
            {
                draw_status_bar();
            }

        }
    }
    ImGui::End(); // File Window

    if (!open)
    {
        m_context->app->close_file_at(fileIndex);
    }
}

void AppView::draw_properties_editor()
{
    Settings* settings = m_context->settings;

    ImGui::Text("Nodable Settings:");
    ImGui::Indent();

        ImGui::Text("Buttons:");
        ImGui::Indent();
            ImGui::SliderFloat2("ui_toolButton_size", &settings->ui_toolButton_size.x, 20.0f, 50.0f);
        ImGui::Unindent();

        ImGui::Text("Wires:");
        ImGui::Indent();
            ImGui::SliderFloat("thickness", &settings->ui_wire_bezier_thickness, 0.5f, 10.0f);
            ImGui::SliderFloat("roundness", &settings->ui_wire_bezier_roundness, 0.0f, 1.0f);
            ImGui::Checkbox("arrows", &settings->ui_wire_displayArrows);
        ImGui::Unindent();

        ImGui::Text("Nodes:");
        ImGui::Indent();
            ImGui::SliderFloat("member connector radius", &settings->ui_node_memberConnectorRadius, 1.0f, 10.0f);
            ImGui::SliderFloat("padding", &settings->ui_node_padding, 1.0f, 20.0f);
            ImGui::SliderFloat("speed", &settings->ui_node_speed, 0.0f, 100.0f);
            ImGui::SliderFloat("spacing", &settings->ui_node_spacing, 0.0f, 100.0f);
            ImGui::SliderFloat("node connector padding", &settings->ui_node_connector_padding, 0.0f, 100.0f);
            ImGui::SliderFloat("node connector height", &settings->ui_node_connector_height, 2.0f, 100.0f);

            ImGui::ColorEdit4("variables color", &settings->ui_node_variableColor.x);
            ImGui::ColorEdit4("instruction color", &settings->ui_node_instructionColor.x);
            ImGui::ColorEdit4("literal color", &settings->ui_node_literalColor.x);
            ImGui::ColorEdit4("function color", &settings->ui_node_invokableColor.x);
            ImGui::ColorEdit4("shadow color", &settings->ui_node_shadowColor.x);
            ImGui::ColorEdit4("border color", &settings->ui_node_borderColor.x);
            ImGui::ColorEdit4("high. color", &settings->ui_node_highlightedColor.x);
            ImGui::ColorEdit4("border high. color", &settings->ui_node_borderHighlightedColor.x);
            ImGui::ColorEdit4("fill color", &settings->ui_node_fillColor.x);
            ImGui::ColorEdit4("node connector color", &settings->ui_node_nodeConnectorColor.x);
            ImGui::ColorEdit4("node connector hovered color", &settings->ui_node_nodeConnectorHoveredColor.x);

        ImGui::Unindent();

        // code flow
        ImGui::Text("Code flow:");
        ImGui::Indent();
            ImGui::SliderFloat("line width min", &settings->ui_node_connector_width, 1.0f, 100.0f);
        ImGui::Unindent();

    ImGui::Unindent();
}

void AppView::draw_startup_window() {
    if (m_show_startup_window && !ImGui::IsPopupOpen(m_startup_screen_title))
    {
        ImGui::OpenPopup(m_startup_screen_title);
    }

    ImGui::SetNextWindowSizeConstraints(vec2(500,200), vec2(500,50000));
    ImGui::SetNextWindowPos( ImGui::GetMainViewport()->GetCenter(), 0, vec2(0.5f,0.5f) );

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if ( ImGui::BeginPopupModal(m_startup_screen_title, nullptr, flags) )
    {

        auto path = m_context->app->get_asset_path(m_context->settings->ui_splashscreen_imagePath);
        Texture* logo = m_context->texture_manager->get_or_create(path);
        ImGui::SameLine( (ImGui::GetContentRegionAvailWidth() - logo->width) * 0.5f); // center img
        ImGui::Image((void*)(intptr_t)logo->image, vec2((float)logo->width, (float)logo->height));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(25.0f, 20.0f) );
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        {
            ImGui::PushFont(m_fonts[FontSlot_Heading] );
            ImGui::NewLine();
            ImGui::Text("Nodable is node-able");
            ImGui::PopFont();
            ImGui::NewLine();
        }


        ImGui::TextWrapped("The goal of Nodable is to allow you to edit a computer program in a textual and nodal way at the same time." );

        {
            ImGui::PushFont(m_fonts[FontSlot_Heading] );
            ImGui::NewLine();
            ImGui::Text("Manifest");
            ImGui::PopFont();
            ImGui::NewLine();
        }

        ImGui::TextWrapped( "The nodal and textual points of view each have pros and cons. The user should not be forced to choose one of the two." );

        {
            ImGui::PushFont(m_fonts[FontSlot_Heading] );
            ImGui::NewLine();
            ImGui::Text("Disclaimer");
            ImGui::PopFont();
            ImGui::NewLine();
        }

        ImGui::TextWrapped( "Nodable is a prototype, do not expect too much from it. Use at your own risk." );

        ImGui::NewLine();ImGui::NewLine();

        const char* credit = "berenger@dalle-cort.fr";
        ImGui::SameLine( ImGui::GetContentRegionAvailWidth() - ImGui::CalcTextSize(credit).x);
        ImGui::TextWrapped( "%s", credit );
        ImGui::TextWrapped( "%s", BuildInfo::version.c_str() );
        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
        {
            ImGui::CloseCurrentPopup();
            m_show_startup_window = false;
        }
        ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
        ImGui::EndPopup();
    }

}

void AppView::draw_status_bar() const {/*
  Status bar
*/

    auto lastLog = Log::GetLastMessage();

    if( lastLog != nullptr )
    {
        vec4 statusLineColor;

        switch ( lastLog->verbosity )
        {
            case Log::Verbosity::Error:
                statusLineColor  = vec4(0.5f, 0.0f, 0.0f,1.0f);
                break;

            case Log::Verbosity::Warning:
                statusLineColor  = vec4(0.5f, 0.0f, 0.0f,1.0f);
                break;

            default:
                statusLineColor  = vec4(0.0f, 0.0f, 0.0f,0.5f);
        }

        ImGui::TextColored(statusLineColor, "%s", lastLog->text.c_str());
    }
}

void AppView::draw_history_bar(History *currentFileHistory)
{
    if (currentFileHistory)
    {
        if (ImGui::IsMouseReleased(0))
        {
            m_is_history_dragged = false;
        }

        auto historyButtonSpacing = float(1);
        auto historyButtonHeight = float(10);
        auto historyButtonMaxWidth = float(20);

        auto historySize = currentFileHistory->get_size();
        auto history_range = currentFileHistory->get_command_id_range();
        auto availableWidth = ImGui::GetContentRegionAvailWidth();
        auto historyButtonWidth = fmin(historyButtonMaxWidth,
                                       availableWidth / float(historySize + 1) - historyButtonSpacing);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(historyButtonSpacing, 0));

        for (int cmd_pos = history_range.first; cmd_pos <= history_range.second; cmd_pos++)
        {
            ImGui::SameLine();

            std::string label("##" + std::to_string(cmd_pos));

            // Draw an highlighted button for the current history position
            if ( cmd_pos == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
                ImGui::Button(label.c_str(), vec2(historyButtonWidth, historyButtonHeight));
                ImGui::PopStyleColor();
            }
            else // or a simple one for other history positions
            {
                ImGui::Button(label.c_str(), vec2(historyButtonWidth, historyButtonHeight));
            }

            // Hovered item
            if (ImGui::IsItemHovered()) {
                if (ImGui::IsMouseDown(0)) // hovered + mouse down
                {
                    m_is_history_dragged = true;
                }

                // Draw command description
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, float(0.8));
                ImGui::BeginTooltip();
                ImGui::Text("%s", currentFileHistory->get_cmd_description_at(cmd_pos).c_str());
                ImGui::EndTooltip();
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

void AppView::browse_file()
{
	m_file_browser.Open();
}

void AppView::draw_background()
{
    ImGui::BeginChild("background");
    auto logo = m_context->texture_manager->get_or_create(BuildInfo::assets_dir + "/nodable-logo-xs.png");

    for( int x = 0; x < 5; x++ )
    {
        for( int y = 0; y < 5; y++ )
        {
            ImGui::SameLine( (ImGui::GetContentRegionAvailWidth() - logo->width) * 0.5f); // center img
            ImGui::Image((void*)(intptr_t)logo->image, vec2((float)logo->width, (float)logo->height));
        }
        ImGui::NewLine();
    }
    ImGui::EndChild();

}

void AppView::draw_tool_bar()
{
    // small margin
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
    ImGui::BeginGroup();

    // run
    bool isRunning = m_context->vm->is_program_running();
    if ( isRunning )
        ImGui::PushStyleColor(ImGuiCol_Button, m_context->settings->ui_button_activeColor);

    if (ImGui::Button(ICON_FA_PLAY, m_context->settings->ui_toolButton_size) && m_context->vm->is_program_stopped())
    {
        m_context->app->vm_run();
    }
    if ( isRunning )
        ImGui::PopStyleColor();
    ImGui::SameLine();

    // debug
    bool isDebugging = m_context->vm->is_debugging();
    if ( isDebugging )
        ImGui::PushStyleColor(ImGuiCol_Button, m_context->settings->ui_button_activeColor);
    if (ImGui::Button(ICON_FA_BUG, m_context->settings->ui_toolButton_size) && m_context->vm->is_program_stopped())
    {
        m_context->app->vm_debug();
    }
    if ( isDebugging )
        ImGui::PopStyleColor();
    ImGui::SameLine();

    // stepOver
    if (ImGui::Button(ICON_FA_ARROW_RIGHT, m_context->settings->ui_toolButton_size) && m_context->vm->is_debugging())
    {
        m_context->vm->step_over();
    }
    ImGui::SameLine();

    // stop
    if (ImGui::Button(ICON_FA_STOP, m_context->settings->ui_toolButton_size) && !m_context->vm->is_program_stopped())
    {
        m_context->app->vm_stop();
    }
    ImGui::SameLine();

    // reset
    if ( ImGui::Button(ICON_FA_UNDO, m_context->settings->ui_toolButton_size))
    {
        m_context->app->vm_reset();
    }
    ImGui::SameLine();
    ImGui::EndGroup();

    m_context->elapsed_time += ImGui::GetIO().DeltaTime;
}

void AppView::handle_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                m_context->app->flag_to_stop();
                break;

            case SDL_KEYUP:
                auto key = event.key.keysym.sym;
                auto l_ctrl_pressed = event.key.keysym.mod & KMOD_LCTRL;
                if ( l_ctrl_pressed )
                {
                    // History
                    if (File* file = m_context->app->get_curr_file())
                    {
                        History* currentFileHistory = file->getHistory();
                        if (key == SDLK_z) currentFileHistory->undo();
                        else if (key == SDLK_y) currentFileHistory->redo();
                    }

                    // File
                    if( key == SDLK_s) m_context->app->save_file();
                    else if( key == SDLK_w) m_context->app->close_file();
                    else if( key == SDLK_o) browse_file();
                }
                else
                {
                    switch( key )
                    {
                        case SDLK_DELETE:
                            EventManager::push_event(EventType::delete_node_action_triggered);
                            break;
                        case SDLK_a:
                            EventManager::push_event(EventType::arrange_node_action_triggered);
                            break;
                        case SDLK_x:
                            EventManager::push_event(EventType::expand_selected_node_action_triggered);
                            break;
                        case SDLK_n:
                            EventManager::push_event(EventType::select_successor_node_action_triggered);
                            break;
                        case SDLK_F1:
                            m_show_startup_window = true;
                            break;
                    }
                }
                break;
        }

    }
}
void AppView::shutdown()
{
    m_context->texture_manager->release_resources();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext    ();
    SDL_GL_DeleteContext     (m_sdl_gl_context);
    SDL_DestroyWindow        (m_sdl_window);
    SDL_Quit                 ();
}
