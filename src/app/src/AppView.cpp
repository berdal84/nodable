#include <nodable/app/AppView.h>

#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/core/Texture.h>
#include <nodable/core/System.h>
#include <nodable/core/Log.h>
#include <nodable/app/build_info.h>
#include <nodable/app/Settings.h>
#include <nodable/app/App.h>
#include <nodable/app/IAppCtx.h>
#include <nodable/app/NodeView.h>
#include <nodable/app/File.h>
#include <nodable/app/FileView.h>
#include <nodable/app/History.h>
#include <nodable/app/constants.h>
#include <nodable/app/Event.h>
#include <nativefiledialog-extended/src/include/nfd.h>

using namespace Nodable;
using namespace Nodable::assembly;

AppView::AppView(IAppCtx& _ctx, const char* _name )
    : View(_ctx)
    , m_logo(nullptr)
    , m_vm(_ctx.virtual_machine())
    , m_settings(_ctx.settings())
    , m_background_color()
    , m_show_splashscreen(true)
    , m_is_layout_initialized(false)
    , m_splashscreen_title("##STARTUPSCREEN")
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
    // Create shortcuts
    m_shortcuts.push_back({ SDLK_DELETE  , KMOD_NONE, EventType::delete_node_action_triggered });
    m_shortcuts.push_back({ SDLK_a       , KMOD_NONE, EventType::arrange_node_action_triggered });
    m_shortcuts.push_back({ SDLK_x       , KMOD_NONE, EventType::toggle_folding_selected_node_action_triggered });
    m_shortcuts.push_back({ SDLK_n       , KMOD_NONE, EventType::select_successor_node_action_triggered });

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

    m_sdl_gl_context = SDL_GL_CreateContext(m_sdl_window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    gl3wInit();

    // preload images
    auto path = m_ctx.compute_asset_path(m_settings.ui_splashscreen_imagePath);
    m_logo = m_ctx.texture_manager().get_or_create_from(path);

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
    m_settings.patch_imgui_style(ImGui::GetStyle());

    // load fonts (TODO: hum, load only if font is used...)
    for ( auto& each_font : m_settings.ui_text_fonts )
    {
        load_font(each_font);
    }

    // Assign fonts (user might want to change it later, but we need defaults)
    for( auto each_slot = 0; each_slot < FontSlot_COUNT; ++each_slot )
    {
        const char* font_id = m_settings.ui_text_defaultFontsId[each_slot];
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
    ImGui_ImplSDL2_InitForOpenGL(m_sdl_window, m_sdl_gl_context);
    const char* glsl_version = NULL; // let backend decide wich version to use, usually 130 (pc) or 150 (macos).
    ImGui_ImplOpenGL3_Init(glsl_version);

    if (NFD_Init() != NFD_OKAY)
    {
        LOG_ERROR("AppView", "Unable to init NFD\n");
    }

	return true;
}

ImFont* AppView::get_font_by_id(const char *id) {
    return m_loaded_fonts.at(id );
}

ImFont* AppView::load_font(const FontConf &_config)
{
    NODABLE_ASSERT(m_loaded_fonts.find(_config.id) == m_loaded_fonts.end()); // do not allow the use of same key for different fonts

    ImFont*   font     = nullptr;
    auto&     io       = ImGui::GetIO();

    // Create font
    {
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;

        //io.Fonts->AddFontDefault();
        std::string fontPath = m_ctx.compute_asset_path(_config.path);
        LOG_VERBOSE("AppView", "Adding font from file ... %s\n", fontPath.c_str())
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), _config.size, &config);
    }

    // Add Icons my merging to previous font.
    if ( _config.icons_enable )
    {
        // merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;
        config.MergeMode   = true;
        config.PixelSnapH  = true;
        config.GlyphOffset.y = -(_config.icons_size - _config.size)*0.5f;
        config.GlyphMinAdvanceX = _config.icons_size; // monospace to fix text alignment in drop down menus.
        auto fontPath = m_ctx.compute_asset_path(m_settings.ui_icons.path);
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), _config.icons_size, &config, icons_ranges);
        LOG_VERBOSE("AppView", "Adding icons to font ...\n")
    }

    m_loaded_fonts.insert({_config.id, font});
    LOG_MESSAGE("AppView", "Font %s added to register with the id \"%s\"\n", _config.path, _config.id)
    return font;
}

bool AppView::draw()
{
    bool isMainWindowOpen = true;
    bool redock_all       = false;
    File* current_file    = m_ctx.current_file();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_sdl_window);
	ImGui::NewFrame();
	ImGuiEx::BeginFrame();
    ImGui::SetCurrentFont( m_fonts[FontSlot_Paragraph] );

    // Startup Window
    draw_splashcreen();

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
        if ( File* file = current_file )
        {
            currentFileHistory = file->get_history();
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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0.0f, 0.0f));

        ImGui::Begin("Nodable", &isMainWindowOpen, window_flags);
        {
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(2);


            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    bool has_file = current_file;
                    bool changed = current_file ? current_file->has_changed() : false;
                    if (ImGui::MenuItem(ICON_FA_FILE        "  New",     "Ctrl + N"))                  new_file();
                    if (ImGui::MenuItem(ICON_FA_FOLDER      "  Open",    "Ctrl + O"))                  browse_file();
                    if (ImGui::MenuItem(ICON_FA_SAVE        "  Save",    "Ctrl + S", false, changed))  save_file();
                    if (ImGui::MenuItem(ICON_FA_SAVE        "  Save as", "",         false, has_file)) save_file_as();
                    if (ImGui::MenuItem(ICON_FA_TIMES       "  Close",   "Ctrl + W", false, has_file)) close_file();

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

                    if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT"  Quit", "Alt + F4"))
                    {
                        m_ctx.flag_to_stop();
                    }

                    ImGui::EndMenu();
                }

                bool vm_is_stopped = m_vm.is_program_stopped();
                if (ImGui::BeginMenu("Edit"))
                {
                    if (currentFileHistory)
                    {
                        if (ImGui::MenuItem("Undo", "Ctrl + Z")) currentFileHistory->undo();
                        if (ImGui::MenuItem("Redo", "Ctrl + Y")) currentFileHistory->redo();
                        ImGui::Separator();
                    }

                    auto has_selection = NodeView::get_selected() != nullptr;

                    if ( ImGui::MenuItem("Delete", "Del.", false, has_selection && vm_is_stopped) )
                    {
                        EventManager::push_event(EventType::delete_node_action_triggered);
                    }

                    if ( ImGui::MenuItem("Arrange nodes", "A", false, has_selection) )
                    {
                        EventManager::push_event(EventType::arrange_node_action_triggered);
                    }

                    if ( ImGui::MenuItem("Expand/Collapse", "X", false, has_selection) )
                    {
                        Event event;
                        event.toggle_folding.type      = EventType::toggle_folding_selected_node_action_triggered;
                        event.toggle_folding.recursive = false;
                        EventManager::push_event(event);
                    }

                    if ( ImGui::MenuItem("Expand/Collapse recursive", NULL, false, has_selection) )
                    {
                        Event event;
                        event.toggle_folding.type      = EventType::toggle_folding_selected_node_action_triggered;
                        event.toggle_folding.recursive = true;
                        EventManager::push_event(event);
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

                    bool is_fullscreen = SDL_GetWindowFlags(m_sdl_window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
                    if ( ImGui::MenuItem("Fullscreen", "", is_fullscreen) )
                    {
                        if (is_fullscreen)
                        {
                            SDL_SetWindowFullscreen(m_sdl_window, 0);
                        }
                        else
                        {
                            SDL_SetWindowFullscreen(m_sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                    }
                    ImGui::Separator();
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Run"))
                {
                    bool vm_is_debugging = m_vm.is_debugging();

                    if (ImGui::MenuItem(ICON_FA_PLAY" Run", "", false, vm_is_stopped) )
                    {
                        m_ctx.run_program();
                    }

                    if (ImGui::MenuItem(ICON_FA_BUG" Debug", "", false, vm_is_stopped) )
                    {
                        m_ctx.debug_program();
                    }

                    if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT" Step Over", "", false, vm_is_debugging) )
                    {
                        m_ctx.step_over_program();
                    }

                    if (ImGui::MenuItem(ICON_FA_STOP" Stop", "", false, !vm_is_stopped) )
                    {
                        m_ctx.stop_program();
                    }

                    if (ImGui::MenuItem(ICON_FA_UNDO " Reset", "", false, vm_is_stopped))
                    {
                        m_ctx.reset_program();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Developer"))
                {
                    if (ImGui::BeginMenu("Verbosity"))
                    {
                        auto menu_item_verbosity = [](Log::Verbosity _verbosity, const char* _label)
                        {
                            if (ImGui::MenuItem( _label , "", Log::GetVerbosityLevel() == _verbosity))
                            {
                                Log::SetVerbosityLevel(_verbosity);
                            }
                        };

                        menu_item_verbosity(Log::Verbosity::Verbose, "Verbose");
                        menu_item_verbosity(Log::Verbosity::Message, "Message (default)");
                        menu_item_verbosity(Log::Verbosity::Warning, "Warning");
                        menu_item_verbosity(Log::Verbosity::Error,   "Error");
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Experimental"))
                    {
                        ImGui::Checkbox( "Hybrid history"       , &m_settings.experimental_hybrid_history);
                        ImGui::Checkbox( "Graph auto-completion", &m_settings.experimental_graph_autocompletion);
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("An issue ?"))
                {
                    if (ImGui::MenuItem("Report on Github.com"))
                    {
                        System::open_url_async("https://github.com/berdal84/Nodable/issues");
                    }

                    if (ImGui::MenuItem("Report by email"))
                    {
                        System::open_url_async("mail:berenger@dalle-cort.fr");
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
                        System::open_url_async("https://www.github.com/berdal84/nodable");
                    }

                    if (ImGui::MenuItem("Credits"))
                    {
                        System::open_url_async("https://github.com/berdal84/nodable#credits-");
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
            draw_tool_bar();


            /*
             * Main Layout
             */

            ImGuiID dockspace_main       = ImGui::GetID("dockspace_main");
            ImGuiID dockspace_center     = ImGui::GetID("dockspace_center");
            ImGuiID dockspace_side_panel = ImGui::GetID("dockspace_side_panel");


            if (!m_is_layout_initialized) {
                ImGui::DockBuilderRemoveNode(dockspace_main); // Clear out existing layout
                ImGui::DockBuilderAddNode(dockspace_main, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_main, ImGui::GetMainViewport()->Size);
                ImGui::DockBuilderSplitNode(dockspace_main, ImGuiDir_Right, m_settings.ui_layout_propertiesRatio, &dockspace_side_panel, NULL);

                ImGui::DockBuilderDockWindow(k_imgui_settings_window_name, dockspace_side_panel);
                ImGui::DockBuilderDockWindow(k_app_settings_window_name, dockspace_side_panel);
                ImGui::DockBuilderDockWindow(k_file_info_window_name, dockspace_side_panel);
                ImGui::DockBuilderDockWindow(k_vm_window_name, dockspace_side_panel);
                ImGui::DockBuilderDockWindow(k_node_props_window_name, dockspace_side_panel);
                ImGui::DockBuilderFinish(dockspace_main);
                m_is_layout_initialized = true;
            }

            /*
            * Fill the layout with content
            */
            ImGui::DockSpace(dockspace_main);

            draw_side_panel();

            if( !m_ctx.has_files())
            {
                draw_startup_menu(dockspace_main);
            }
            else
            {
                for (File* each_file : m_ctx.get_files() )
                {
                    draw_file_editor(dockspace_main, redock_all, each_file);
                }
            }
        }
        ImGui::End(); // Main window
    }
    ImGuiEx::EndFrame();

    // Rendering
	ImGui::Render();
	SDL_GL_MakeCurrent(m_sdl_window, m_sdl_gl_context);
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
        SDL_Window*   backup_current_window  = SDL_GL_GetCurrentWindow();
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

void AppView::draw_side_panel()
{
    if( !m_ctx.current_file() )
    {
        return;
    }

    draw_vm_view();
    draw_properties_editor();
    draw_imgui_style_editor();
    draw_file_info();
    draw_node_properties();
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
    if( auto current_file = m_ctx.current_file() )
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
            NodeView::draw_as_properties_panel(m_ctx, view, &m_show_advanced_node_properties);
        }
    }
    ImGui::End();
}

void AppView::draw_vm_view()
{
    if (ImGui::Begin(k_vm_window_name))
    {
        ImGui::Text("Virtual Machine:");
        ImGui::SameLine();
        ImGuiEx::DrawHelper( "%s", "The virtual machine - or interpreter - is a sort of implementation of \n"
                                   "an imaginary hardware able to run a set of simple instructions.");
        ImGui::Separator();

        const Code* code = m_vm.get_program_asm_code();

        // VM state
        {
            ImGui::Indent();
            ImGui::Text("State:         %s", m_vm.is_program_running() ? "running" : "stopped");
            ImGui::SameLine();
            ImGuiEx::DrawHelper( "%s", "When virtual machine is running, you cannot edit the code or the graph.");
            ImGui::Text("Debug:         %s", m_vm.is_debugging() ? "ON" : "OFF");
            ImGui::SameLine();
            ImGuiEx::DrawHelper( "%s", "When debugging is ON, you can run a program step by step.");
            ImGui::Text("Has program:   %s", code ? "YES" : "NO");
            if (code)
            {
            ImGui::Text("Program over:  %s", !m_vm.is_there_a_next_instr() ? "YES" : "NO");
            }
            ImGui::Unindent();
        }

        // VM Registers
        ImGui::Separator();
        ImGui::Text("CPU:");
        ImGui::SameLine();
        ImGuiEx::DrawHelper( "%s", "This is the virtual machine's CPU"
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
                ImGui::Text("%4s: %12s", assembly::to_string(_register), m_vm.read_cpu_register(_register).to_string().c_str() );
            };

            draw_register_value(Register::rax); ImGui::SameLine(); ImGuiEx::DrawHelper( "%s", "primary accumulator");
            draw_register_value(Register::rdx); ImGui::SameLine(); ImGuiEx::DrawHelper( "%s", "base register");
            draw_register_value(Register::eip); ImGui::SameLine(); ImGuiEx::DrawHelper( "%s", "instruction pointer");

            ImGui::Unindent();
        }
        ImGui::Unindent();

        // Assembly-like code
        ImGui::Separator();
        ImGui::Text("Memory:"); ImGui::SameLine(); ImGuiEx::DrawHelper( "%s", "Virtual Machine Memory.");
        ImGui::Separator();
        {
            ImGui::Indent();

            ImGui::Text("Bytecode:");
            ImGui::SameLine();
            ImGuiEx::DrawHelper( "%s", "The bytecode is the result of the Compilation process."
                                       "\nAfter source code has been parsed to a syntax tree, "
                                       "\nthe tree (or graph) is converted by the Compiler to an Assembly-like code.");
            ImGui::Checkbox("Auto-scroll ?", &m_scroll_to_curr_instr);
            ImGui::SameLine();
            ImGuiEx::DrawHelper( "%s", "to scroll automatically to the current instruction");
            ImGui::Separator();
            {
                ImGui::BeginChild("AssemblyCodeChild", ImGui::GetContentRegionAvail(), true );

                if ( code )
                {
                    auto current_instr = m_vm.get_next_instr();
                    for( Instruction* each_instr : code->get_instructions() )
                    {
                        auto str = Instruction::to_string(*each_instr );
                        if ( each_instr == current_instr )
                        {
                            if ( m_scroll_to_curr_instr && m_vm.is_program_running() )
                            {
                                ImGui::SetScrollHereY();
                            }
                            ImGui::TextColored( ImColor(200,0,0), ">%s", str.c_str() );
                            ImGui::SameLine();
                            ImGuiEx::DrawHelper( "%s", "This is the next instruction to evaluate");
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
                    ImGuiEx::DrawHelper( "%s", "To see a compiled program here you need first to:"
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
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.3,0.3,0.3, 1.0));

    ImGui::Begin("Startup");
    {
        ImGui::PopStyleColor();

        ImVec2 center_area(500.0f, 250.0f);
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImGui::SetCursorPosX( (avail.x - center_area.x) / 2);
        ImGui::SetCursorPosY( (avail.y - center_area.y) / 2);

        ImGui::BeginChild("center_area", center_area);
        {
            ImGui::Indent(center_area.x * 0.05f);

            ImGui::PushFont(m_fonts.at(FontSlot_ToolBtn));
            ImGui::NewLine();

            vec2 btn_size(center_area.x * 0.44f, 40.0f);
            if( ImGui::Button(ICON_FA_FILE" New File", btn_size) )        new_file();
            ImGui::SameLine();
            if( ImGui::Button(ICON_FA_FOLDER_OPEN" Open ...", btn_size) ) browse_file();

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
            vec2 small_btn_size(btn_size.x, btn_size.y * 0.66f);

            for( auto [text, path] : examples)
            {
                std::string label;
                label.append(ICON_FA_BOOK" ");
                label.append(text);
                if( i++ % 2) ImGui::SameLine();
                if (ImGui::Button(label.c_str(), small_btn_size))
                {
                    std::string each_path = m_ctx.compute_asset_path(path.c_str());
                    m_ctx.open_file(each_path);
                }
            }

            ImGui::PopFont();

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::TextColored( vec4(0,0,0, 0.30f), "Nodable %s", BuildInfo::version);
            ImGui::Unindent();
        }
        ImGui::EndChild();
    }
    ImGui::End(); // Startup Window
}

void AppView::draw_file_editor(ImGuiID dockspace_id, bool redock_all, File* file)
{
    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags = (file->has_changed() ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0, 0));

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
            const bool is_current_file = m_ctx.is_current(file);

            if ( ! is_current_file && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                m_ctx.current_file(file);
            }

            // History bar on top
            draw_history_bar(file->get_history());

            // File View in the middle
            View* eachFileView = file->get_view();
            vec2 availSize = ImGui::GetContentRegionAvail();

            if ( is_current_file )
            {
                availSize.y -= ImGui::GetTextLineHeightWithSpacing();
            }

            ImGui::PushStyleColor(ImGuiCol_ChildBg, vec4(0,0,0,0.35f) );
            ImGui::PushFont(m_fonts[FontSlot_Code] );
            eachFileView->draw_as_child("FileView", availSize, false);
            ImGui::PopFont();
            ImGui::PopStyleColor();

            // Status bar
            if ( is_current_file )
            {
                draw_status_bar();

                if (file->get_view()->text_has_changed())
                {
                    m_vm.release_program();
                }

            }

        }
    }
    ImGui::End(); // File Window

    if (!is_window_open)
    {
        m_ctx.close_file(file);
    }
}

void AppView::draw_properties_editor()
{
    if (ImGui::Begin(k_app_settings_window_name))
    {
        Settings& settings = m_ctx.settings();

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
                ImGui::SliderFloat("member connector radius"    , &settings.ui_node_memberConnectorRadius, 1.0f, 10.0f);
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

    ImGui::SetNextWindowSizeConstraints(vec2(500,200), vec2(500,50000));
    ImGui::SetNextWindowPos( ImGui::GetMainViewport()->GetCenter(), 0, vec2(0.5f,0.5f) );

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if ( ImGui::BeginPopupModal(m_splashscreen_title, nullptr, flags) )
    {
        ImGui::SameLine( (ImGui::GetContentRegionAvailWidth() - m_logo->width) * 0.5f); // center img
        ImGui::Image((void*)(intptr_t)m_logo->image, vec2((float)m_logo->width, (float)m_logo->height));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(50.0f, 30.0f) );
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImGui::TextWrapped("DISCLAIMER: This software is a prototype, do not expect too much from it. Use at your own risk." );

        ImGui::NewLine();ImGui::NewLine();

        const char* credit = "by Berdal84";
        ImGui::SameLine( ImGui::GetContentRegionAvailWidth() - ImGui::CalcTextSize(credit).x);
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
    auto draw_log_line = [](const Log::Message* _log, bool _detailed = false)
    {
        switch ( _log->verbosity )
        {
            case Log::Verbosity::Error:
                ImGui::TextColored(vec4(0.5f, 0.0f, 0.0f, 1.0f), _detailed ? "Error: %s" : "%s", _log->text.c_str());
                break;

            case Log::Verbosity::Warning:
                ImGui::TextColored(vec4(0.5f, 0.0f, 0.0f, 1.0f), _detailed ? "Warning: %s" : "%s", _log->text.c_str());
                break;

            default:
                ImGui::TextColored(vec4(0.5f, 0.5f, 0.5f, 1.0f), _detailed ? "Message: %s" : "%s", _log->text.c_str());

        }
    };

    if( auto last_log = Log::GetLastMessage() )
    {
        draw_log_line(last_log);

        if( ImGui::IsItemHovered())
        {
            ImGuiEx::BeginTooltip();
            draw_log_line(Log::GetLastMessage(), true);
            ImGuiEx::EndTooltip();
        }
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

        float btn_spacing   = m_settings.ui_history_btn_spacing;
        float btn_height    = m_settings.ui_history_btn_height;
        float btn_width_max = m_settings.ui_history_btn_width_max;

        size_t              historySize    = currentFileHistory->get_size();
        std::pair<int, int> history_range  = currentFileHistory->get_command_id_range();
        float               avail_width    = ImGui::GetContentRegionAvailWidth();
        float               btn_width      = fmin(btn_width_max, avail_width / float(historySize + 1) - btn_spacing);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(btn_spacing, 0));

        for (int cmd_pos = history_range.first; cmd_pos <= history_range.second; cmd_pos++)
        {
            ImGui::SameLine();

            std::string label("##" + std::to_string(cmd_pos));

            // Draw an highlighted button for the current history position
            if ( cmd_pos == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
                ImGui::Button(label.c_str(), vec2(btn_width, btn_height));
                ImGui::PopStyleColor();
            }
            else // or a simple one for other history positions
            {
                ImGui::Button(label.c_str(), vec2(btn_width, btn_height));
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
                ImGuiEx::BeginTooltip();
                ImGui::Text("%s", currentFileHistory->get_cmd_description_at(cmd_pos).c_str());
                ImGuiEx::EndTooltip();
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

void AppView::new_file()
{
    m_ctx.new_file();
}

void AppView::save_file()
{
    File *curr_file = m_ctx.current_file();

    if (curr_file->has_path())
    {
        return m_ctx.save_file();
    }
    save_file_as();
}

void AppView::save_file_as()
{
    File *curr_file = m_ctx.current_file();

    nfdchar_t *out_path;
    //nfdfilteritem_t filters[4] = {{"File", NULL }, {"Text", "txt" }, {"Source code", "c,cpp,cc" }, {"Headers", "h,hpp" } };
    nfdresult_t result = NFD_SaveDialog(&out_path, nullptr, 0, nullptr, nullptr);
    if (result == NFD_OKAY)
    {
        LOG_MESSAGE("AppView", "Success!");
        LOG_MESSAGE("AppView", out_path);
        m_ctx.save_file_as(out_path);
        NFD_FreePath(out_path);
    }
    else if (result == NFD_CANCEL)
    {
        puts("User pressed cancel.");
    }
    else
    {
        LOG_ERROR("AppView", "%s\n", NFD_GetError());
    }
}

void AppView::browse_file()
{
    nfdchar_t *out_path;
    //nfdfilteritem_t filters[4] = {{"File", NULL }, {"Text", "txt" }, {"Source code", "c,cpp,cc" }, {"Headers", "h,hpp" } };
    nfdresult_t result = NFD_OpenDialog(&out_path, nullptr, 0, NULL);
    if (result == NFD_OKAY)
    {
        LOG_MESSAGE("AppView", "Success!");
        LOG_MESSAGE("AppView", out_path);
        m_ctx.open_file(out_path);
        NFD_FreePath(out_path);
    }
    else if (result == NFD_CANCEL)
    {
        puts("User pressed cancel.");
    }
    else
    {
        LOG_ERROR("AppView", "%s\n", NFD_GetError());
    }
}

void AppView::draw_tool_bar()
{
    bool running       = m_vm.is_program_running();
    bool debugging     = m_vm.is_debugging();
    bool stopped       = m_vm.is_program_stopped();
    vec2 &button_size  = m_settings.ui_toolButton_size;
    vec4 &active_color = m_settings.ui_button_activeColor;

    ImGui::PushFont(m_fonts[FontSlot_ToolBtn]);

    // small margin
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
    ImGui::BeginGroup();

    // compile
    if (ImGui::Button(ICON_FA_DATABASE " compile", button_size) && stopped)
    {
        m_ctx.compile_and_load_program();
    }
    ImGui::SameLine();

    // run
    if ( running ) ImGui::PushStyleColor(ImGuiCol_Button, active_color);

    if (ImGui::Button(ICON_FA_PLAY " run", button_size) && stopped)
    {
        m_ctx.run_program();
    }
    if ( running ) ImGui::PopStyleColor();

    ImGui::SameLine();

    // debug
    if ( debugging ) ImGui::PushStyleColor(ImGuiCol_Button, active_color);
    if (ImGui::Button(ICON_FA_BUG " debug", button_size) && stopped)
    {
        m_ctx.debug_program();
    }
    if ( debugging ) ImGui::PopStyleColor();
    ImGui::SameLine();

    // stepOver
    if (ImGui::Button(ICON_FA_ARROW_RIGHT " step over", button_size) && m_vm.is_debugging())
    {
        m_vm.step_over();
    }
    ImGui::SameLine();

    // stop
    if (ImGui::Button(ICON_FA_STOP " stop", button_size) && !stopped)
    {
        m_ctx.stop_program();
    }
    ImGui::SameLine();

    // reset
    if ( ImGui::Button(ICON_FA_UNDO " reset graph", button_size))
    {
        m_ctx.reset_program();
    }
    ImGui::SameLine();
    ImGui::EndGroup();

    ImGui::PopFont();
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
                m_ctx.flag_to_stop();
                break;

            case SDL_KEYUP:
                auto key = event.key.keysym.sym;
                auto l_ctrl_pressed = event.key.keysym.mod & KMOD_LCTRL;
                if ( l_ctrl_pressed )
                {

                    if (File* file = m_ctx.current_file())
                    {
                        History* history = file->get_history();
                             if (key == SDLK_z) history->undo();
                        else if (key == SDLK_y) history->redo();
                        else if( key == SDLK_s) save_file();
                        else if( key == SDLK_w) m_ctx.close_file(file);
                    }

                         if( key == SDLK_o) browse_file();
                    else if( key == SDLK_n) new_file();
                }
                else
                {
                    switch( key )
                    {
                        case SDLK_F1:
                            m_show_splashscreen = true;
                            break;
                        default:
                            break;
                    }
                }
                break;
        }

        // Shortcuts (WIP)
        for(auto shortcut : m_shortcuts)
        {
            if ( shortcut.event != EventType::none            // shortcut has an event type.
                 && event.type == SDL_KEYDOWN
                 && shortcut.key == event.key.keysym.sym      // shortcut's key is pressed.
                 && event.key.keysym.mod == shortcut.mod      // shortcut's modifier is pressed.
                 )
            {
                EventManager::push_event(shortcut.event);
                break;
            }
        }

    }
}
void AppView::shutdown()
{
    m_ctx.texture_manager().release_resources();

    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext    ();
    SDL_GL_DeleteContext     (m_sdl_gl_context);
    SDL_DestroyWindow        (m_sdl_window);
    SDL_Quit                 ();

    NFD_Quit();
}

void AppView::close_file()
{
    m_ctx.close_file(m_ctx.current_file() );
}
