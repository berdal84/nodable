#include "AppView.h"

#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <lodepng/lodepng.h>
#include <nfd.h>
#include <gl3w.h>

#include "tools/core/log.h"
#include "tools/core/system.h"

#include "BaseApp.h"
#include "Config.h"
#include "EventManager.h"
#include "tools/core/memory/memory.h"

using namespace tools;

constexpr const char* k_status_window_name = "Status Bar";

AppView::AppView( BaseApp* _app)
    : m_app(_app)
    , m_is_layout_initialized(false)
    , font_manager()
    , m_sdl_window(nullptr)
    , m_sdl_gl_context()
    , action_manager()
    , m_frame_start_time()
    , show_splashscreen(true)
{
    LOG_VERBOSE("tools::AppView", "Constructor " OK "\n");
}

AppView::~AppView()
{
    LOG_VERBOSE("tools::AppView", "Destructor " OK "\n");
}


void AppView::init()
{
    Config* cfg = get_config();

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        LOG_ERROR( "tools::App", "SDL Error: %s\n", SDL_GetError());
        EXPECT(false, "Unable to initialize SDL");
    }

    // Setup window
    LOG_VERBOSE("tools::App", "setup SDL ...\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    m_title = cfg->app_default_title;
    m_sdl_window = SDL_CreateWindow( cfg->app_default_title,
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
    SDL_GL_SetSwapInterval((int)cfg->vsync);

    LOG_VERBOSE("tools::App", "gl3w init ...\n");
    gl3wInit();

    // Setup Dear ImGui binding
    LOG_VERBOSE("tools::App", "ImGui init ...\n");
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

    // Override ImGui's default Style
    // TODO: consider declaring new members in Config rather than modifying values from there.
    //       see colors[ImGuiCol_Button]
    LOG_VERBOSE("tools::App", "patch ImGui's style ...\n");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 * colors = style.Colors;
    colors[ImGuiCol_Text]                   = Vec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = Vec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_WindowBg]               = Vec4(0.76f, 0.76f, 0.76f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg]         = Vec4(0.64f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ChildBg]                = Vec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_PopupBg]                = Vec4(0.66f, 0.66f, 0.66f, 1.00f);
    colors[ImGuiCol_Border]                 = Vec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = Vec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_FrameBg]                = Vec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = Vec4(0.90f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = Vec4(0.90f, 0.65f, 0.65f, 1.00f);
    colors[ImGuiCol_TitleBg]                = Vec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = Vec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = Vec4(0.49f, 0.63f, 0.69f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = Vec4(0.60f, 0.60f, 0.60f, 0.98f);
    colors[ImGuiCol_ScrollbarBg]            = Vec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = Vec4(0.61f, 0.61f, 0.62f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = Vec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = Vec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_CheckMark]              = Vec4(0.31f, 0.23f, 0.14f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = Vec4(0.71f, 0.46f, 0.22f, 0.63f);
    colors[ImGuiCol_SliderGrabActive]       = Vec4(0.71f, 0.46f, 0.22f, 1.00f);
    colors[ImGuiCol_Button]                 = cfg->button_color;
    colors[ImGuiCol_ButtonHovered]          = cfg->button_hoveredColor;
    colors[ImGuiCol_ButtonActive]           = cfg->button_activeColor;
    colors[ImGuiCol_Header]                 = Vec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = Vec4(0.89f, 0.65f, 0.11f, 0.96f);
    colors[ImGuiCol_HeaderActive]           = Vec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Separator]              = Vec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = Vec4(0.71f, 0.71f, 0.71f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = Vec4(1.00f, 0.62f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = Vec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered]      = Vec4(1.00f, 1.00f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = Vec4(1.00f, 1.00f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab]                    = Vec4(0.58f, 0.54f, 0.50f, 0.86f);
    colors[ImGuiCol_TabHovered]             = Vec4(1.00f, 0.79f, 0.45f, 1.00f);
    colors[ImGuiCol_TabActive]              = Vec4(1.00f, 0.73f, 0.25f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = Vec4(0.53f, 0.53f, 0.53f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive]     = Vec4(0.79f, 0.79f, 0.79f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = Vec4(1.00f, 0.70f, 0.09f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]         = Vec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = Vec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = Vec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = Vec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = Vec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = Vec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = Vec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = Vec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = Vec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = Vec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = Vec4(0.20f, 0.20f, 0.20f, 0.55f);
    colors[ImGuiCol_TableBorderLight]       = Vec4(0.20f, 0.20f, 0.20f, 0.80f);
    colors[ImGuiCol_TableBorderStrong]      = Vec4(0.20f, 0.20f, 0.20f, 0.90f);
    colors[ImGuiCol_TableHeaderBg]          = Vec4(0.20f, 0.20f, 0.20f, 0.60f);
    colors[ImGuiCol_TableRowBg]             = Vec4(0.20f, 0.20f, 0.20f, 0.40f);
    colors[ImGuiCol_TableRowBgAlt]          = Vec4(0.20f, 0.20f, 0.20f, 0.20f);

    style.WindowBorderSize   = cfg->border_size;
    style.FrameBorderSize    = cfg->border_size;
    style.FrameRounding      = cfg->frame_rounding;
    style.ChildRounding      = cfg->frame_rounding;
    style.WindowRounding     = cfg->window_rounding;
    style.AntiAliasedFill    = cfg->antialiased;
    style.AntiAliasedLines   = cfg->antialiased;
    style.WindowPadding      = cfg->padding;

    //style.ScaleAllSizes(1.25f);

    // load fonts
    font_manager.init();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    if( !ImGui_ImplSDL2_InitForOpenGL(m_sdl_window, m_sdl_gl_context) )
    {
        LOG_ERROR("tools::App", "Unable to init NFD\n");
        EXPECT(false, "Unable to initialize NFD");
    }
    if( !ImGui_ImplOpenGL3_Init(/* default glsl_version*/) )
    {
        LOG_ERROR("tools::App", "Unable to init NFD\n");
        EXPECT(false, "Unable to initialize NFD");
    }
    if (NFD_Init() != NFD_OKAY)
    {
        LOG_ERROR("tools::App", "Unable to init NFD\n");
        EXPECT(false, "Unable to initialize NFD");
    }

    show_splashscreen = cfg->show_splashscreen_default;
}

void AppView::shutdown()
{
    LOG_MESSAGE("tools::AppView", "Shutting down ...\n");
    LOG_MESSAGE("tools::AppView", "Shutting down OpenGL3 ...\n");
    ImGui_ImplOpenGL3_Shutdown();
    LOG_MESSAGE("tools::AppView", "Shutting down SDL2 ...\n");
    ImGui_ImplSDL2_Shutdown();
    LOG_MESSAGE("tools::AppView", "Destroying ImGui context ...\n");
    ImGui::DestroyContext    ();
    LOG_MESSAGE("tools::AppView", "Shutdown SDL ...\n");
    SDL_GL_DeleteContext     (m_sdl_gl_context);
    SDL_DestroyWindow        (m_sdl_window);
    SDL_Quit                 ();
    LOG_MESSAGE("tools::AppView", "Quitting NFD (Native File Dialog) ...\n");
    NFD_Quit();
    LOG_MESSAGE("tools::AppView", "Shutdown OK\n");
}

void AppView::update()
{
    m_frame_start_time = SDL_GetTicks();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_WINDOWEVENT:
                if( event.window.event == SDL_WINDOWEVENT_CLOSE)
                    m_app->should_stop = true;
                break;
            case SDL_KEYDOWN:

                // With mode key only
                if( event.key.keysym.mod & (KMOD_CTRL | KMOD_ALT) )
                {
                    for(const IAction* _action: action_manager.get_actions() )
                    {
                        // first, priority to shortcuts with mod
                        if ( _action->shortcut.mod != KMOD_NONE
                             && _action->event_id && ( _action->shortcut.mod & event.key.keysym.mod)
                             && _action->shortcut.key == event.key.keysym.sym
                        )
                        {
                            event_manager.dispatch( _action->event_id );
                            break;
                        }
                    }
                }
                else // without any mod key
                {
                    for(const IAction* _action: action_manager.get_actions() )
                    {
                        // first, priority to shortcuts with mod
                        if ( _action->shortcut.mod == KMOD_NONE
                             && _action->event_id && _action->shortcut.key == event.key.keysym.sym
                        )
                        {
                            event_manager.dispatch( _action->event_id );
                            break;
                        }
                    }
                }
                break;
        }
    }
}

void AppView::draw()
{
    begin_draw();
    end_draw();
}

void AppView::begin_draw()
{
    Config* cfg                 = get_config();
    bool    is_main_window_open = true;

    // Begin Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(m_sdl_window);
    ImGuiEx::Tooltip_NewFrame();
    ImGui::NewFrame();

    // Setup main window

    ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            | ImGuiWindowFlags_NoMove                            // because it would be confusing to have two docking targets within each others.
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos( viewport->WorkPos );
    ImGui::SetNextWindowSize( viewport->WorkSize );
    ImGui::SetNextWindowViewport( viewport->ID );

    // Draw main window

    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );// Remove padding
    ImGui::Begin( "App", &is_main_window_open, window_flags ); // End() call is in end_draw()
    {
        ImGui::PopStyleVar( 3 );

        ImGui::SetCurrentFont( font_manager.get_font( FontSlot_Paragraph ) );

        // Show/Hide ImGui Demo Window
        if ( cfg->imgui_demo )
        {
            ImGui::SetNextWindowPos( ImVec2( 650, 20 ), ImGuiCond_FirstUseEver );
            ImGui::ShowDemoWindow( &cfg->imgui_demo );
        }

        // Splashscreen
        draw_splashscreen();

        // Build layout
        if ( !m_is_layout_initialized )
        {
            // Dockspace IDs
            m_dockspaces[Dockspace_ROOT] = ImGui::GetID( "Dockspace_ROOT" );
            m_dockspaces[Dockspace_CENTER] = ImGui::GetID( "Dockspace_CENTER" );
            m_dockspaces[Dockspace_RIGHT] = ImGui::GetID( "Dockspace_RIGHT" );
            m_dockspaces[Dockspace_BOTTOM] = ImGui::GetID( "Dockspace_BOTTOM" );
            m_dockspaces[Dockspace_TOP] = ImGui::GetID( "Dockspace_TOP" );

            // Split root to have N dockspaces
            ImVec2 viewport_size = ImGui::GetMainViewport()->Size;

            ImGui::DockBuilderRemoveNode( m_dockspaces[Dockspace_ROOT] );// Clear out existing layout
            ImGui::DockBuilderAddNode( m_dockspaces[Dockspace_ROOT], ImGuiDockNodeFlags_DockSpace );
            ImGui::DockBuilderSetNodeSize( m_dockspaces[Dockspace_ROOT], viewport_size );

            ImGui::DockBuilderSplitNode( m_dockspaces[Dockspace_ROOT], ImGuiDir_Down, 0.5f, &m_dockspaces[Dockspace_BOTTOM], &m_dockspaces[Dockspace_CENTER] );
            ImGui::DockBuilderSetNodeSize( m_dockspaces[Dockspace_BOTTOM], ImVec2( viewport_size.x, cfg->dockspace_bottom_size ) );

            ImGui::DockBuilderSplitNode( m_dockspaces[Dockspace_CENTER], ImGuiDir_Up, 0.5f, &m_dockspaces[Dockspace_TOP], &m_dockspaces[Dockspace_CENTER] );
            ImGui::DockBuilderSetNodeSize( m_dockspaces[Dockspace_TOP], ImVec2( viewport_size.x, cfg->dockspace_top_size ) );

            ImGui::DockBuilderSplitNode( m_dockspaces[Dockspace_CENTER], ImGuiDir_Right, cfg->dockspace_right_ratio, &m_dockspaces[Dockspace_RIGHT], &m_dockspaces[Dockspace_CENTER] );

            // Configure dockspaces
            ImGui::DockBuilderGetNode( m_dockspaces[Dockspace_CENTER] )->HasCloseButton = false;
            ImGui::DockBuilderGetNode( m_dockspaces[Dockspace_RIGHT] )->HasCloseButton = false;
            ImGuiDockNode* ds_bottom_builder = ImGui::DockBuilderGetNode( m_dockspaces[Dockspace_BOTTOM] );
            ds_bottom_builder->HasCloseButton = false;

            ds_bottom_builder->SharedFlags = ImGuiDockNodeFlags_NoDocking;
            ImGuiDockNode* ds_top_builder = ImGui::DockBuilderGetNode( m_dockspaces[Dockspace_TOP] );
            ds_top_builder->HasCloseButton = false;
            ds_top_builder->WantHiddenTabBarToggle = true;
            ds_top_builder->WantLockSizeOnce = true;

            // Dock windows
            dock_window( k_status_window_name, Dockspace_BOTTOM );

            // Run user defined code
            on_reset_layout();

            // Finish the build
            ImGui::DockBuilderFinish( m_dockspaces[Dockspace_ROOT] );

            m_is_layout_initialized = true;
        }

        // Define root as current dockspace
        ImGui::DockSpace( get_dockspace( Dockspace_ROOT ) );

        // Status Window
        log::MessageDeque& messages = log::get_messages();
        if ( ImGui::Begin( k_status_window_name ) && !messages.empty())
        {
            const float line_height = ImGui::GetTextLineHeightWithSpacing();
            static int verbosity_filter = -1; // all


            if ( ImGui::BeginChild("filters", ImVec2(-1, line_height * 1.2f )) )
            {
                ImGui::BeginGroup();
                ImGui::Text("Filter:"); ImGui::SameLine();
                if ( ImGui::RadioButton("All", verbosity_filter == -1 ) )
                    verbosity_filter = -1; ImGui::SameLine();
                if ( ImGui::RadioButton("Debug", verbosity_filter == log::Verbosity_Verbose ) )
                    verbosity_filter = log::Verbosity_Verbose; ImGui::SameLine();
                if ( ImGui::RadioButton("Messages", verbosity_filter == log::Verbosity_Message ) )
                    verbosity_filter = log::Verbosity_Message; ImGui::SameLine();
                if ( ImGui::RadioButton("Warnings", verbosity_filter == log::Verbosity_Warning ) )
                    verbosity_filter = log::Verbosity_Warning; ImGui::SameLine();
                if ( ImGui::RadioButton("Errors", verbosity_filter == log::Verbosity_Error ) )
                    verbosity_filter = log::Verbosity_Error; ImGui::SameLine();
                ImGui::EndGroup();
            }
            ImGui::EndChild();

            if ( ImGui::BeginChild("messages") )
            {
                u32_t message_to_display_count = std::min( messages.size(), cfg->log_message_display_max_count );
                auto it = messages.rbegin();
                size_t message_processed_count = 0;
                size_t message_displayed_count = 0;

                while ( message_displayed_count < message_to_display_count && it != messages.rend() )
                {
                    if ( it->verbosity <= verbosity_filter || verbosity_filter == -1 )
                    {
                        ImRect line_rect{ ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() };
                        line_rect.Max.y += line_height;
                        line_rect.Max.x += 100.0f;

                        if ( ImGui::IsRectVisible( line_rect.Min, line_rect.Max ) )// draw only when line is visible to optimize rendering
                        {
                            ImGui::TextColored( cfg->log_color[it->verbosity], "%s", it->text.c_str() );
                            ++message_displayed_count;
                        }
                        else
                        {
                            ImGui::NewLine();
                        }
                    }
                    ++message_processed_count;
                    ++it;
                }

                if ( message_displayed_count == 0 )
                {
                    ImGui::Text( "Nothing here..." );
                }

                if ( !ImGui::IsWindowHovered() )
                {
                    ImGui::SetScrollHereY();
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();// Status Window
    }
}

void AppView::end_draw()
{
    Config* cfg = get_config();

    // End Frame
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
    ImGuiEx::Tooltip_EndFrame();

    SDL_GL_MakeCurrent(m_sdl_window, m_sdl_gl_context);
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    Vec4& color = cfg->background_color.value;
    glClearColor( color.x, color.y, color.z, color.w);
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
    u32_t frame_time = SDL_GetTicks() - m_frame_start_time;
    if ( cfg->delta_time_limit && frame_time < cfg->delta_time_min )
    {
        SDL_Delay( cfg->delta_time_min - frame_time );
    }

#ifdef NDBL_DEBUG
    // Update window title with FPS in it (if enabled)
    if( cfg->runtime_debug )
    {
        // compute FPS
        static u32_t dt = 1000 / 60;
        u32_t all_time = SDL_GetTicks() - m_frame_start_time;
        if( all_time <= 0 ) all_time = 1;
        dt = u32_t(0.9f*float(dt) + 0.1f*float(all_time)); // Smooth value
        u32_t fps = 1000 / dt;

        // Memory usage
        const MemoryStats* mem_stats = tools::get_memory_stats();

        // Format nice title
        char title[256];
        snprintf( title, 256, "%s | %i fps (dt %d ms, frame %d ms) | Mem: %zu B (alloc count: %zu)", m_title.c_str(), fps, dt, frame_time, mem_stats->mem_usage(), mem_stats->alloc_count() );
        title[255] = '\0';

        // Update window title
        SDL_SetWindowTitle(m_sdl_window, title);
    }
#endif
}

bool AppView::pick_file_path(std::string& _out_path, DialogType _dialog_type) const
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
            LOG_MESSAGE("tools::AppView", "User pressed cancel.");
            return false;
        default:
            LOG_ERROR("tools::AppView", "%s\n", NFD_GetError());
            return false;
    }
}

void AppView::set_layout_initialized(bool b)
{
    m_is_layout_initialized = b;
}

ImGuiID AppView::get_dockspace(Dockspace dockspace)const
{
    return m_dockspaces[dockspace];
}

void AppView::dock_window(const char* window_name, Dockspace dockspace)const
{
    ImGui::DockBuilderDockWindow(window_name, m_dockspaces[dockspace]);
}

void AppView::draw_splashscreen()
{
    if ( AppView::begin_splashscreen() )
    {
        AppView::end_splashscreen();
    }
}

bool AppView::begin_splashscreen()
{
    Config* cfg = get_config();
    if ( show_splashscreen && !ImGui::IsPopupOpen( cfg->splashscreen_window_label))
    {
        ImGui::OpenPopup( cfg->splashscreen_window_label);
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(550, 300), ImVec2(550, 50000));
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    return ImGui::BeginPopupModal( cfg->splashscreen_window_label, &show_splashscreen, flags);
}

void AppView::end_splashscreen()
{
    ImGui::EndPopup();
}

std::vector<unsigned char> AppView::take_screenshot() const
{
    LOG_MESSAGE("tools::AppView", "Taking screenshot ...\n");
    int width, height;
    SDL_GetWindowSize(m_sdl_window, &width, &height);
    GLsizei stride = 4 * width;
    GLsizei bufferSize = stride * height;
    std::vector<unsigned char> buffer(bufferSize);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer( GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

    // vertical flip
    std::vector<unsigned char> flipped(bufferSize);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < stride; ++x) {
            flipped[y*stride+x] = buffer[(height-y-1)*stride+x];
        }
    }

    std::vector<unsigned char> out;
    lodepng::encode(out, flipped.data(), width, height, LCT_RGBA);
    LOG_MESSAGE("tools::AppView", "Taking screenshot OK\n");
    return out;
}

bool AppView::is_fullscreen() const
{
    return SDL_GetWindowFlags(m_sdl_window) & (SDL_WindowFlags::SDL_WINDOW_FULLSCREEN | SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void AppView::set_fullscreen(bool b)
{
    SDL_SetWindowFullscreen(m_sdl_window, b ? SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

int AppView::fps()
{
    return (int)ImGui::GetIO().Framerate;
}

void AppView::save_screenshot( std::filesystem::path path) const
{
    std::vector<unsigned char> out = take_screenshot();
    LOG_MESSAGE("tools::App", "Save screenshot ...\n");
    lodepng::save_file(out, path.string());
    LOG_MESSAGE("tools::App", "Save screenshot OK (%s)\n", path.c_str());
}

void AppView::set_title( const char* title )
{
    m_title = title;
    SDL_SetWindowTitle( m_sdl_window, title );
}
