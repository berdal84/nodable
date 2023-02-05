#include <fw/imgui/AppView.h>

#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <nativefiledialog-extended/src/include/nfd.h>

#include <fw/System.h>
#include <fw/Log.h>
#include <fw/imgui/Texture.h>
#include <fw/imgui/App.h>
#include <fw/imgui/Event.h>

using namespace fw;

constexpr const char* k_status_window_name = "Status";

AppView::AppView(App* _app, Conf _conf )
    : View()
    , m_conf(_conf)
    , m_app(_app)
    , m_is_layout_initialized(false)
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
    m_sdl_window = SDL_CreateWindow(m_conf.title.c_str(),
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

    // Run user code
    if(!on_init()) return false;

	// Setup Dear ImGui style

    // load fonts (TODO: hum, load only if font is used...)
    for ( auto& each_font : m_conf.fonts )
    {
        load_font(each_font);
    }

    // Assign fonts (user might want to change it later, but we need defaults)
    for( int each_slot = 0; each_slot < fw::FontSlot_COUNT; ++each_slot )
    {
        if(auto font = m_conf.fonts_default[each_slot] )
        {
            m_fonts[each_slot] = get_font_by_id( font );
        }
        else
        {
            LOG_WARNING("AppView", "No default font declared for slot #%i, using ImGui's default font as fallback\n", each_slot);
            m_fonts[each_slot] = ImGui::GetDefaultFont();
        }
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
    NDBL_ASSERT(m_loaded_fonts.find(_config.id) == m_loaded_fonts.end()); // do not allow the use of same key for different fonts

    ImFont*   font     = nullptr;
    auto&     io       = ImGui::GetIO();

    // Create font
    {
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;

        //io.Fonts->AddFontDefault();
        std::string fontPath = m_app->compute_asset_path(_config.path);
        LOG_VERBOSE("AppView", "Adding font from file ... %s\n", fontPath.c_str())
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), _config.size, &config);
    }

    // Add Icons my merging to previous font.
    if ( _config.icons_enable )
    {
        if(strlen(m_conf.icon_font.path) == 0)
        {
            LOG_WARNING("AppView", "m_conf.icons is empty, icons will be \"?\"\n");
            return font;
        }

        // merge in icons font
        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;
        config.MergeMode   = true;
        config.PixelSnapH  = true;
        config.GlyphOffset.y = -(_config.icons_size - _config.size)*0.5f;
        config.GlyphMinAdvanceX = _config.icons_size; // monospace to fix text alignment in drop down menus.
        auto fontPath = m_app->compute_asset_path(m_conf.icon_font.path);
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), _config.icons_size, &config, icons_ranges);
        LOG_VERBOSE("AppView", "Adding icons to font ...\n")
    }

    m_loaded_fonts.insert({_config.id, font});
    LOG_MESSAGE("AppView", "Font %s added to register with the id \"%s\"\n", _config.path, _config.id)
    return font;
}

bool AppView::draw()
{
    bool is_main_window_open = true;
    bool redock_all          = false;

    // 1) Begin a new frame
    //---------------------

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_sdl_window);
	ImGui::NewFrame();
	ImGuiEx::BeginFrame();

    // 2) Draw
    //--------

    ImGui::SetCurrentFont( m_fonts[FontSlot_Paragraph] );

    // Show/Hide ImGui Demo Window
    {
        if (m_conf.show_imgui_demo){
            ImGui::SetNextWindowPos(vec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&m_conf.show_imgui_demo);
        }
    }

    // Splashscreen
    draw_splashcreen_window();

    // Setup main window

    ImGuiWindowFlags window_flags =
          ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoDocking // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        | ImGuiWindowFlags_NoMove    // because it would be confusing to have two docking targets within each others.
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;


    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0.0f, 0.0f)); // Remove padding

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Draw main window

    ImGui::Begin("App", &is_main_window_open, window_flags);
    {
        ImGui::PopStyleVar(3);

        // Build layout
        if (!m_is_layout_initialized)
        {
            // Dockspace IDs
            m_dockspaces[Dockspace_ROOT]   = ImGui::GetID("Dockspace_ROOT");
            m_dockspaces[Dockspace_CENTER] = ImGui::GetID("Dockspace_CENTER");
            m_dockspaces[Dockspace_RIGHT]  = ImGui::GetID("Dockspace_RIGHT");
            m_dockspaces[Dockspace_BOTTOM] = ImGui::GetID("Dockspace_BOTTOM");
            m_dockspaces[Dockspace_TOP]    = ImGui::GetID("Dockspace_TOP");

            // Split root to have N dockspaces
            ImVec2 viewport_size = ImGui::GetMainViewport()->Size;

            ImGui::DockBuilderRemoveNode(m_dockspaces[Dockspace_ROOT]); // Clear out existing layout
            ImGui::DockBuilderAddNode(m_dockspaces[Dockspace_ROOT]     , ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(m_dockspaces[Dockspace_ROOT] , viewport_size);

            ImGui::DockBuilderSplitNode(m_dockspaces[Dockspace_ROOT]   , ImGuiDir_Down , 0.5f, &m_dockspaces[Dockspace_BOTTOM], &m_dockspaces[Dockspace_CENTER]);
            ImGui::DockBuilderSetNodeSize(m_dockspaces[Dockspace_BOTTOM] , vec2(viewport_size.x, m_conf.dockspace_down_size));

            ImGui::DockBuilderSplitNode(m_dockspaces[Dockspace_CENTER]   , ImGuiDir_Up , 0.5f, &m_dockspaces[Dockspace_TOP], &m_dockspaces[Dockspace_CENTER]);
            ImGui::DockBuilderSetNodeSize(m_dockspaces[Dockspace_TOP] , vec2(viewport_size.x, m_conf.dockspace_top_size));

            ImGui::DockBuilderSplitNode(m_dockspaces[Dockspace_CENTER] , ImGuiDir_Right, m_conf.dockspace_right_ratio, &m_dockspaces[Dockspace_RIGHT], nullptr );

            // Configure dockspaces
            ImGui::DockBuilderGetNode(m_dockspaces[Dockspace_CENTER])->HasCloseButton         = false;
            ImGui::DockBuilderGetNode(m_dockspaces[Dockspace_RIGHT])->HasCloseButton          = false;
            ImGuiDockNode *ds_bottom_builder = ImGui::DockBuilderGetNode(m_dockspaces[Dockspace_BOTTOM]);
            ds_bottom_builder->HasCloseButton         = false;
            ds_bottom_builder->WantHiddenTabBarToggle = true;
            ds_bottom_builder->SharedFlags            = ImGuiDockNodeFlags_NoDocking;
            ImGuiDockNode *ds_top_builder = ImGui::DockBuilderGetNode(m_dockspaces[Dockspace_TOP]);
            ds_top_builder->HasCloseButton            = false;
            ds_top_builder->WantHiddenTabBarToggle    = true;

            // Dock windows
            dock_window(k_status_window_name, Dockspace_BOTTOM);

            // Call user defined handler
            on_reset_layout();

            // Finish the build
            ImGui::DockBuilderFinish(m_dockspaces[Dockspace_ROOT]);

            m_is_layout_initialized = true;
            redock_all              = true;
        }

        // Define root as current dockspace
        ImGui::DockSpace(get_dockspace(Dockspace_ROOT));

        // Draw Windows
        draw_status_window();

        // User defined draw
        if (!on_draw(redock_all))
        {
            LOG_ERROR( "AppView", "User defined on_draw() returned false.\n");
        }
    }
    ImGui::End(); // Main window

    // 3. End frame and Render
    //------------------------

    ImGuiEx::EndFrame();
	ImGui::Render(); // Finalize draw data

	SDL_GL_MakeCurrent(m_sdl_window, m_sdl_gl_context);
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(m_conf.background_color.Value.x, m_conf.background_color.Value.y, m_conf.background_color.Value.z, m_conf.background_color.Value.w);
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

bool AppView::pick_file_path(std::string& _out_path, DialogType _dialog_type)
{
    nfdchar_t *out_path;
    nfdresult_t result;

    switch( _dialog_type )
    {
        case DIALOG_SaveAs:
            result = NFD_SaveDialog(&out_path, nullptr, 0, nullptr, nullptr);
            break;
        case DIALOG_Browse:
            result = NFD_OpenDialog(&out_path, nullptr, 0, nullptr);
            break;
    }

    switch (result)
    {
        case NFD_OKAY:
            _out_path = out_path;
            NFD_FreePath(out_path);
            return true;
        case NFD_CANCEL:
            LOG_MESSAGE("AppView", "User pressed cancel.");
            return false;
        default:
            LOG_ERROR("AppView", "%s\n", NFD_GetError());
            return false;
    }
}

void AppView::handle_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_WINDOWEVENT:
                if( event.window.event == SDL_WINDOWEVENT_CLOSE)
                    EventManager::push_event(fw::EventType_exit_triggered);
                break;
            case SDL_KEYDOWN:

                // With mode key only
                if( event.key.keysym.mod & (KMOD_CTRL | KMOD_ALT) )
                {
                    for(auto _binded_event: BindedEventManager::s_binded_events )
                    {
                        // first, priority to shortcuts with mod
                        if ( _binded_event.shortcut.mod != KMOD_NONE
                             && _binded_event.event_t
                             && (_binded_event.shortcut.mod & event.key.keysym.mod)
                             && _binded_event.shortcut.key == event.key.keysym.sym
                                )
                        {
                            EventManager::push_event(_binded_event.event_t);
                            break;
                        }
                    }
                }
                else // without any mod key
                {
                    for(auto _binded_event: BindedEventManager::s_binded_events )
                    {
                        // first, priority to shortcuts with mod
                        if ( _binded_event.shortcut.mod == KMOD_NONE
                             && _binded_event.event_t
                             && _binded_event.shortcut.key == event.key.keysym.sym
                                )
                        {
                            EventManager::push_event(_binded_event.event_t);
                            break;
                        }
                    }
                }
                break;
        }
    }
}

void AppView::shutdown()
{
    m_app->texture_manager().release_resources();

    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext    ();
    SDL_GL_DeleteContext     (m_sdl_gl_context);
    SDL_DestroyWindow        (m_sdl_window);
    SDL_Quit                 ();

    NFD_Quit();
}

void AppView::set_splashscreen_visible(bool b)
{
    m_conf.show_splashscreen = b;
}

bool AppView::is_fullscreen() const
{
    return SDL_GetWindowFlags(m_sdl_window) & (SDL_WindowFlags::SDL_WINDOW_FULLSCREEN | SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP);
}

ImFont* AppView::get_font(FontSlot slot) const
{
    return m_fonts[slot];
}

void AppView::set_layout_initialized(bool b)
{
    m_is_layout_initialized = b;
}

void AppView::set_fullscreen(bool b)
{
    SDL_SetWindowFullscreen(m_sdl_window, b ? SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

ImGuiID AppView::get_dockspace(Dockspace dockspace)const
{
    return m_dockspaces[dockspace];
}

void AppView::dock_window(const char* window_name, Dockspace dockspace)const
{
    ImGui::DockBuilderDockWindow(window_name, m_dockspaces[dockspace]);
}

void AppView::draw_splashcreen_window()
{
    if (m_conf.show_splashscreen && !ImGui::IsPopupOpen(m_conf.splashscreen_title))
    {
        ImGui::OpenPopup(m_conf.splashscreen_title);
    }

    ImGui::SetNextWindowSizeConstraints(fw::vec2(550, 300), fw::vec2(550, 50000));
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, fw::vec2(0.5f, 0.5f));

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::BeginPopupModal(m_conf.splashscreen_title, &m_conf.show_splashscreen, flags))
    {
        on_draw_splashscreen(); // user defined
        ImGui::EndPopup();
    }
}

bool AppView::is_splashscreen_visible() const
{
    return m_conf.show_splashscreen;
}

void AppView::draw_status_window() const
{
    if (ImGui::Begin(k_status_window_name))
    {
        if (!fw::Log::get_messages().empty())
        {
            const fw::Log::Messages &messages = fw::Log::get_messages();
            auto it = messages.rend() - m_conf.log_tooltip_max_count;
            while (it != messages.rend())
            {
                auto &each_message = *it;
                ImGui::TextColored(m_conf.log_color[each_message.verbosity], "%s",
                                   each_message.to_full_string().c_str());
                ++it;
            }

            if (!ImGui::IsWindowHovered())
            {
                ImGui::SetScrollHereY();
            }

        }
    }
    ImGui::End();
}