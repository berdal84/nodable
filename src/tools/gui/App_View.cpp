#include "App_View.h"
#include "SDL_timer.h"
#include "core/Asserts.h"
#include "core/Event.h"
#include "gui/Action_Manager.h"
#include "gui/ImGuiEx.h"
#include <lodepng.h> // to save screenshot as PNG
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#ifdef NDBL_DESKTOP
    #include <nfd.h>
#endif

#include "tools/core/Log.h"
#include "tools/core/System.h"
#include "tools/core/Event_Manager.h"
#include "tools/gui/Texture_Manager.h"
#include "tools/gui/Font_Manager.h"
#include "tools/gui/GL_Helpers.h"

#include "tools/core/Math.h"
#include "App.h"
#include "Config.h"

using namespace tools;

constexpr const char* k_status_window_name = "Status Bar";

void tools::appview_init(App_View_State* view, App_State* app)
{
    ASSERT(view!=nullptr);
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "init ...\n");
    ASSERT(app != nullptr);
    
    view->dt_in_ms      = 1000 / 30;
    view->dt_in_s       = 1.f/30.f;
    view->smoothed_fps  = 30.f;
    view->should_reset_layout   = true;
    view->show_splashscreen     = true;

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        TOOLS_LOG(tools::Verbosity_Error,  "tools::AppView", "-- SDL Error: %s\n", SDL_GetError());
        VERIFY(false, "Unable to initialize SDL");
    }

    // Setup window
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Setup SDL ...\n");

    // Decide GL+GLSL versions
#ifdef NDBL_DESKTOP
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif __EMSCRIPTEN__
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    Config* cfg = config();
    view->title = cfg->app_default_title;
    view->sdl_window = SDL_CreateWindow( cfg->app_default_title,
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     800,
                                     600,
                                     SDL_WINDOW_OPENGL |
                                     SDL_WINDOW_RESIZABLE |
                                     SDL_WINDOW_MAXIMIZED |
                                     SDL_WINDOW_SHOWN
    );
    VERIFY(view->sdl_window, "-- SDL_CreateWindow failed" );
    
    view->sdl_gl_context = SDL_GL_CreateContext(view->sdl_window);
    VERIFY(view->sdl_gl_context, "-- SDL_GL_CreateContext failed" );

#ifdef NDBL_DESKTOP
    SDL_GL_SetSwapInterval(1); // https://wiki.libsdl.org/SDL2/SDL_GL_SetSwapInterval
    gl3wInit();
#endif

    // Setup Dear ImGui binding
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Init ImGui ...\n");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.Config_Flags |= ImGuiConfig_Flags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    io.FontAllowUserScaling = true;
    //io.WantCaptureKeyboard  = true;
    //io.WantCaptureMouse     = true;

    // Override ImGui's default Style
    // TODO: consider declaring new members in Config rather than modifying values from there.
    //       see colors[ImGuiCol_Button]
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- patch ImGui's style ...\n");
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

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Init backend for OpenGL ...\n");
    if( !ImGui_ImplSDL2_InitForOpenGL(view->sdl_window, view->sdl_gl_context) )
    {
        TOOLS_LOG(tools::Verbosity_Error, "tools::AppView", "Unable to ImGui_ImplSDL2_InitForOpenGL\n");
    }
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- init OpenGL (glsl_version: %s) ...\n", glsl_version);
    if( !ImGui_ImplOpenGL3_Init(glsl_version) )
    {
        TOOLS_LOG(tools::Verbosity_Error, "tools::AppView", "Unable to ImGui_ImplSDL2_InitForOpenGL\n");
    }
#ifdef NDBL_DESKTOP
    if (NFD_Init() != NFD_OKAY)
    {
        TOOLS_LOG(tools::Verbosity_Error, "tools::AppView", "Unable to NFD_Init\n");
    }
#endif
    view->show_splashscreen = cfg->show_splashscreen_default;

    // init managers
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Init managers ...\n");
    texture_manager_init();
    font_manager_init(&config()->font_manager);
    event_manager_init();
    action_manager_init();

    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Init DONE\n");
}

void tools::appview_deinit(App_View_State* view)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "Shutting down ...\n");
    
    // shutdown managers    
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Shutting down managers ...\n");
    action_manager_shutdown();
    event_manager_shutdown();
    font_manager_shutdown();
    texture_manager_shutdown();

    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Shutting down OpenGL3 ...\n");
    ImGui_ImplOpenGL3_Shutdown();
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Shutting down SDL2 ...\n");
    ImGui_ImplSDL2_Shutdown();
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Destroying ImGui context ...\n");
    ImGui::DestroyContext    ();
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Shutdown SDL ...\n");
    SDL_GL_DeleteContext     (view->sdl_gl_context);
    SDL_DestroyWindow        (view->sdl_window);
    SDL_Quit                 ();
#ifdef NDBL_DESKTOP
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Quitting NFD (Native File Dialog) ...\n");
    NFD_Quit();
#endif
    TOOLS_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "-- Shutdown OK\n");
}

void tools::appview_update(App_View_State* view)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_WINDOWEVENT:
                if( event.window.event == SDL_WINDOWEVENT_CLOSE)
                    app_request_stop();
                break;

            case SDL_KEYDOWN:
                if( event.key.keysym.mod & (KMOD_CTRL | KMOD_ALT) )
                {
                    // Test all the shortcuts with Ctrl or Alt modifiers

                    for(const Action& action: action_manager()->actions )
                    {
                        // first, priority to shortcuts with mod
                        if ( action.event.type != Event_Type_NULL )
                            if ( action.shortcut.mod != KMOD_NONE)                                
                                    if ( action.shortcut.mod & event.key.keysym.mod ) // same mod
                                        if ( action.shortcut.key == event.key.keysym.sym) // same key
                                            { event_manager_push_event( action.event ); break; }
                    }
                }
                else
                {
                    // Test all other shortcuts

                    for(const Action& action: action_manager()->actions )
                    {
                        if ( action.event.type != Event_Type_NULL )
                            if ( action.shortcut.mod == KMOD_NONE )                            
                                if ( action.shortcut.key == event.key.keysym.sym)
                                {
                                    event_manager_push_event( action.event );
                                    break;
                                }
                    }
                }
                break;
        }
    }
}

void tools::appview_begin(App_View_State* view)
{
    ASSERT(view != nullptr);

    Config* cfg                 = config();
    bool    is_main_window_open = true;

    // Begin Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(view->sdl_window);
    ImGuiEx::NewFrame();
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

        ImGui::SetCurrentFont(font_manager_get_by_slot(Font_Slot_Paragraph) );

        // Show/Hide ImGui Demo Window
        if ( cfg->imgui_demo )
        {
            ImGui::SetNextWindowPos( ImVec2( 650, 20 ), ImGuiCond_FirstUseEver );
            ImGui::ShowDemoWindow( &cfg->imgui_demo );
        }

        // Splashscreen
        appview_draw_splashscreen(view);

        // Build layout
        if ( view->should_reset_layout )
        {
            // Dockspace IDs
            view->dockspaces[Dockspace_ROOT] = ImGui::GetID( "Dockspace_ROOT" );
            view->dockspaces[Dockspace_CENTER] = ImGui::GetID( "Dockspace_CENTER" );
            view->dockspaces[Dockspace_RIGHT] = ImGui::GetID( "Dockspace_RIGHT" );
            view->dockspaces[Dockspace_BOTTOM] = ImGui::GetID( "Dockspace_BOTTOM" );
            view->dockspaces[Dockspace_TOP] = ImGui::GetID( "Dockspace_TOP" );

            // Split root to have N dockspaces
            ImVec2 viewport_size = ImGui::GetMainViewport()->Size;

            ImGui::DockBuilderRemoveNode( view->dockspaces[Dockspace_ROOT] );// Clear out existing layout
            ImGui::DockBuilderAddNode( view->dockspaces[Dockspace_ROOT], ImGuiDockNodeFlags_DockSpace );
            ImGui::DockBuilderSetNodeSize( view->dockspaces[Dockspace_ROOT], viewport_size );

            ImGui::DockBuilderSplitNode( view->dockspaces[Dockspace_ROOT], ImGuiDir_Down, 0.5f, &view->dockspaces[Dockspace_BOTTOM], &view->dockspaces[Dockspace_CENTER] );
            ImGui::DockBuilderSetNodeSize( view->dockspaces[Dockspace_BOTTOM], ImVec2( viewport_size.x, cfg->dockspace_bottom_size ) );

            ImGui::DockBuilderSplitNode( view->dockspaces[Dockspace_CENTER], ImGuiDir_Up, 0.5f, &view->dockspaces[Dockspace_TOP], &view->dockspaces[Dockspace_CENTER] );
            ImGui::DockBuilderSetNodeSize( view->dockspaces[Dockspace_TOP], ImVec2( viewport_size.x, cfg->dockspace_top_size ) );

            ImGui::DockBuilderSplitNode( view->dockspaces[Dockspace_CENTER], ImGuiDir_Right, cfg->dockspace_right_ratio, &view->dockspaces[Dockspace_RIGHT], &view->dockspaces[Dockspace_CENTER] );

            // Configure dockspaces
            ImGui::DockBuilderGetNode( view->dockspaces[Dockspace_CENTER] )->HasCloseButton = false;
            ImGui::DockBuilderGetNode( view->dockspaces[Dockspace_RIGHT] )->HasCloseButton = false;
            ImGuiDockNode* ds_bottom_builder = ImGui::DockBuilderGetNode( view->dockspaces[Dockspace_BOTTOM] );
            ds_bottom_builder->HasCloseButton = false;

            ds_bottom_builder->SharedFlags = ImGuiDockNodeFlags_NoDocking;
            ImGuiDockNode* ds_top_builder = ImGui::DockBuilderGetNode( view->dockspaces[Dockspace_TOP] );
            ds_top_builder->HasCloseButton = false;
            ds_top_builder->WantHiddenTabBarToggle = true;
            ds_top_builder->WantLockSizeOnce = true;

            // Dock windows
            appview_dock_window(view, k_status_window_name, Dockspace_BOTTOM );

            // Possibly execute some user-defined code
            view->signal_reset_layout.emit();

            // Finish the build
            ImGui::DockBuilderFinish( view->dockspaces[Dockspace_ROOT] );

            view->should_reset_layout = false;
        }

        // Define root as current dockspace
        ImGui::DockSpace( view->dockspaces[Dockspace_ROOT] );

        // Status Window
        if ( ImGui::Begin( k_status_window_name ) && !get_log_state().messages.empty())
        {
            const float line_height = ImGui::GetTextLineHeightWithSpacing();

            if ( ImGui::BeginChild("filters", ImVec2(-1, line_height * 1.2f )) )
            {
                ImGui::BeginGroup();
                ImGui::Text("Filter Messages: "); ImGui::SameLine();

                auto draw_filter = [&](const char* label, tools::Verbosity verbosity)
                {
                    ImGui::Checkbox(label, &get_log_state().verbosity_filter.data[verbosity] );
                };

                auto draw_filter_all = [&](const char* label)
                {
                    bool checked = get_log_state().verbosity_filter.all_checked();
                    if ( ImGui::Checkbox(label, &checked ) )
                    {
                        get_log_state().verbosity_filter.reset_all(checked);
                    }
                };

                draw_filter_all("All" );                            ImGui::SameLine();
                draw_filter("Errors"      , Verbosity_Error );      ImGui::SameLine();
                draw_filter("Warnings"    , Verbosity_Warning );    ImGui::SameLine();
                draw_filter("Messages"    , Verbosity_Message );    ImGui::SameLine();
                draw_filter("Diagnostics" , Verbosity_Diagnostic ); ImGui::SameLine();

                ImGui::EndGroup();
            }
            ImGui::EndChild();

            if ( ImGui::BeginChild("messages") )
            {
                u32_t message_to_display_count = std::min( get_log_state().messages.size(), cfg->log_message_display_max_count );
                size_t message_processed_count = 0;
                size_t message_displayed_count = 0;

                auto it = get_log_state().messages.rbegin();
                while ( message_displayed_count < message_to_display_count && it != get_log_state().messages.rend() )
                {
                    const MessageData& message = *it;
                    if ( show_log_message( message, get_log_state().verbosity_filter ) )
                    {
                        ImRect line_rect{
                            ImGui::GetCursorScreenPos(),
                            ImGui::GetCursorScreenPos()
                        };

                        line_rect.Max.y += line_height;
                        line_rect.Max.x += 100.0f;

                        if ( ImGui::IsRectVisible( line_rect.Min, line_rect.Max ) )// draw only when line is visible to optimize rendering
                        {
                            ImGui::TextColored( cfg->log_color[message.verbosity], "%s", message.text.c_str() );
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

void tools::appview_end(App_View_State* view)
{
    ASSERT(view != nullptr);

    Config* cfg = config();

    // End Frame
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
    ImGuiEx::EndFrame();

    SDL_GL_MakeCurrent(view->sdl_window, view->sdl_gl_context);
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

    SDL_GL_SwapWindow(view->sdl_window);

    static Uint64 before = 0; // to store SDL_GetTicks64();

    // Should we limit the FPS (is it too fast?)
    Uint64 delta = SDL_GetTicks64() - before;
    if ( cfg->fps_limit_on && delta < cfg->dt_cap )
    {
        Uint64 delay = cfg->dt_cap - delta;
        if( delay > 6 ) // Skip 6ms delays, SDL_Delay has no guarantee to be precise
            SDL_Delay( delay );
    }

    
    // update FPS, delta time, etc.

    Uint64 now          = SDL_GetTicks64();
    Uint64 dt           = (now - before);
    float  fps          = 1000.0f / float(dt);
    view->smoothed_fps  = tools::clamped_lerp(view->smoothed_fps, fps, 1.f / 20.f); // smooth the last n frames
    view->dt_in_ms      = float(dt);
    view->dt_in_s       = float(dt) / 1000.f;
    before              = now;

    TOOLS_DEBUG_LOG( TOOLS_DIAG, "tools::App_View", "dt: %f sec, %i msec \n", view->dt_in_s, view->dt_in_ms);

    // Format nice title
    char title[256];
    snprintf(title, 256, "%s | %4.0ffps %s", view->title.c_str(), view->smoothed_fps, cfg->fps_limit_on ? "" : "unlimited!");
    title[255] = '\0';

    // Update window title
    SDL_SetWindowTitle(view->sdl_window, title);
}

#ifdef NDBL_DESKTOP
bool tools::pick_file_path(Path& _out_path, Dialog_Type _dialog_type)
{
    nfdchar_t *picked_path;
    nfdresult_t result;

    switch( _dialog_type )
    {
        case Dialog_Type_SaveAs:
            result = NFD_SaveDialog(&picked_path, nullptr, 0, nullptr, nullptr);
            break;
        case Dialog_Type_Browse:
            result = NFD_OpenDialog(&picked_path, nullptr, 0, nullptr);
            break;
    }

    switch (result)
    {
        case NFD_OKAY:
            _out_path = picked_path;
            NFD_FreePath(picked_path);
            return true;
        case NFD_CANCEL:
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "tools::AppView", "pick_file_path cancelled by user");
            return false;
        default:
            TOOLS_LOG(tools::Verbosity_Error, "tools::AppView", "%s\n", NFD_GetError());
            return false;
    }
}

#elif __EMSCRIPTEN__

EM_JS(void, call_pick_file_path, (bool), {
  alert('pick_file_path_impl not implemented yet');
  throw 'all done';
});
bool tools::pick_file_path(Path& _out_path, Dialog_Type _dialog_type)
{
    bool result;
    call_pick_file_path(result);
    return result;
}
#endif

ImGuiID tools::appview_get_dockspace(App_View_State* view, Dockspace dockspace)
{
    return view->dockspaces[dockspace];
}

void tools::appview_dock_window(App_View_State* view, const bdc::String& window_name, Dockspace dockspace)
{
    ImGui::DockBuilderDockWindow(window_name.c_str(), view->dockspaces[dockspace]);
}

void tools::appview_draw_splashscreen(App_View_State* view)
{
    Config* cfg = config();
    if ( view->show_splashscreen && !ImGui::IsPopupOpen( cfg->splashscreen_window_label))
    {
        ImGui::OpenPopup( cfg->splashscreen_window_label);
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(550, 300), ImVec2(550, 50000));
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    if ( ImGui::BeginPopupModal( cfg->splashscreen_window_label, &view->show_splashscreen, flags) )
    {
        view->signal_draw_splashscreen_content.emit();
        ImGui::EndPopup();
    }
}

std::vector<unsigned char> tools::appview_take_screenshot(const App_View_State* view)
{
#ifdef __EMSCRIPTEN__
    return {};
    // TODO: some glXXX are unavailable, but anyways it's not something we need in WEB, we can use browser for that.
#else
    TOOLS_LOG(tools::Verbosity_Message, "tools::AppView", "Taking screenshot ...\n");
    int width, height;
    SDL_GetWindowSize(view->sdl_window, &width, &height);
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
    TOOLS_LOG(tools::Verbosity_Message, "tools::AppView", "Taking screenshot " TOOLS_OK "\n");
    return out;
#endif
}


bool tools::appview_is_fullscreen(const App_View_State* view)
{
    return SDL_GetWindowFlags(view->sdl_window) & (SDL_WindowFlags::SDL_WINDOW_FULLSCREEN | SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void tools::appview_set_fullscreen(App_View_State* view, bool b)
{
    SDL_SetWindowFullscreen(view->sdl_window, b ? SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void tools::appview_save_screenshot(const App_View_State* view, const tools::Path& path)
{
    std::vector<unsigned char> out = appview_take_screenshot(view);
    TOOLS_LOG(tools::Verbosity_Message, "tools::App", "Save screenshot ...\n");
    lodepng::save_file(out, path.c_str());
    TOOLS_LOG(tools::Verbosity_Message, "tools::App", "Save screenshot " TOOLS_OK " (%s)\n", path.c_str());
}

void tools::appview_set_title(App_View_State* view, const bdc::String title )
{
    view->title = title;
    SDL_SetWindowTitle( view->sdl_window, view->title.c_str() );
}
