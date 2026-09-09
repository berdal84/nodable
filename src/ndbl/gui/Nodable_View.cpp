#include "Nodable_View.h"
#include <cstddef>
#include <lodepng.h> // to save screenshot as PNG
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <SDL_timer.h>

#ifdef NDBL_DESKTOP
    #include <nfd.h>
#endif

#include "ndbl/core/Asserts.h"
#include "ndbl/core/Event_Manager.h"
#include "ndbl/core/Event.h"
#include "ndbl/core/Flags.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/Log.h"
#include "ndbl/core/Math.h"
#include "ndbl/core/System.h"
#include "Action_Manager_View.h"
#include "Action_Manager.h"
#include "Config.h"
#include "Event.h"
#include "File.h"
#include "Font_Manager.h"
#include "GL_Helpers.h"
#include "Graph_View.h"
#include "ImGuiEx.h"
#include "Nodable.h"
#include "Scope_View.h"
#include "Texture_Manager.h"
#include "View.h"

#define VERIFY_NODABLEVIEW_IS_INITIALIZED() VERIFY(g_app_view != nullptr, "Nodable_View is not initialized, did you call nodableview_init() ?")

namespace ndbl
{

constexpr const char*   k_status_window_name = "Status Bar";
static App_View_State*  g_app_view = {};
ImGuiID                 _nodableview_get_dockspace(Dockspace);
void                    _nodableview_dock_window(const bdc::String& window_name, Dockspace); // Must be called within signal_reset_layout

App_View_State* appview()
{
    VERIFY_NODABLEVIEW_IS_INITIALIZED();
    return g_app_view;
}

App_View_State* appview_init()
{
    VERIFY(g_app_view == nullptr, "Nodable_View is already initialized, did you forgot to call nodableview_shutdown() or called init twice?");
    VERIFY(app_state() != nullptr, "Nodable is not initialized, did you call nodable_init() ?");

    auto* view = bdc::memory_new<App_View_State>();
    g_app_view = view;

    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "init ...\n");
    ASSERT(app_state() != nullptr);
    
    view->dt_in_ms      = 1000 / 30;
    view->dt_in_s       = 1.f/30.f;
    view->smoothed_fps  = 30.f;
    view->should_reset_layout   = true;
    view->show_splashscreen     = true;

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        NDBL_LOG(Verbosity_Error,  __FILE__, "-- SDL Error: %s\n", SDL_GetError());
        VERIFY(false, "Unable to initialize SDL");
    }

    // Setup window
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Setup SDL ...\n");

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
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Init ImGui ...\n");
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
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- patch ImGui's style ...\n");
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
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Init backend for OpenGL ...\n");
    if( !ImGui_ImplSDL2_InitForOpenGL(view->sdl_window, view->sdl_gl_context) )
    {
        NDBL_LOG(Verbosity_Error, __FILE__, "Unable to ImGui_ImplSDL2_InitForOpenGL\n");
    }
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- init OpenGL (glsl_version: %s) ...\n", glsl_version);
    if( !ImGui_ImplOpenGL3_Init(glsl_version) )
    {
        NDBL_LOG(Verbosity_Error, __FILE__, "Unable to ImGui_ImplSDL2_InitForOpenGL\n");
    }
#ifdef NDBL_DESKTOP
    if (NFD_Init() != NFD_OKAY)
    {
        NDBL_LOG(Verbosity_Error, __FILE__, "Unable to NFD_Init\n");
    }
#endif
    view->show_splashscreen = cfg->show_splashscreen_default;

    // init managers
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Init managers ...\n");
    texture_manager_init();
    font_manager_init(&config()->font_manager);
    event_manager_init();
    action_manager_init();

    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Init DONE\n");

    // Load splashscreen image
    Path path        = Path::get_asset_path(config()->ui_splashscreen_imagePath );
    g_app_view->logo = texture_manager_load(path);

    return g_app_view;
}

void appview_shutdown()
{
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "Shutting down ...\n");

    App_View_State* view = appview();
    
    // shutdown managers    
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Shutting down managers ...\n");
    action_manager_shutdown();
    event_manager_shutdown();
    font_manager_shutdown();
    texture_manager_shutdown();

    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Shutting down OpenGL3 ...\n");
    ImGui_ImplOpenGL3_Shutdown();
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Shutting down SDL2 ...\n");
    ImGui_ImplSDL2_Shutdown();
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Destroying ImGui context ...\n");
    ImGui::DestroyContext    ();
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Shutdown SDL ...\n");
    SDL_GL_DeleteContext     (view->sdl_gl_context);
    SDL_DestroyWindow        (view->sdl_window);
    SDL_Quit                 ();
#ifdef NDBL_DESKTOP
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Quitting NFD (Native File Dialog) ...\n");
    NFD_Quit();
#endif
    NDBL_LOG(Verbosity_Diagnostic, __FILE__, "-- Shutdown OK\n");

    g_app_view = nullptr;
}

void appview_draw()
{
    App_View_State* view = appview();

    VERIFY(view->logo != nullptr, "Logo is nullptr, did you call init_ex() ?");

    const float dt = view->dt_in_s;

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

        if ( view->show_splashscreen && !ImGui::IsPopupOpen( cfg->splashscreen_window_label))
        {
            ImGui::OpenPopup( cfg->splashscreen_window_label);
        }
        ImGui::SetNextWindowSizeConstraints(ImVec2(550, 300), ImVec2(550, 50000));
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));

        auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        if ( ImGui::BeginPopupModal( cfg->splashscreen_window_label, &view->show_splashscreen, flags) )
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            // Image
            ImGui::SameLine((ImGui::GetContentRegionAvail().x - (float)view->logo->width) * 0.5f); // center img
            ImGuiEx::Image(view->logo);

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {50.0f, 30.0f});

            // disclaimer
            ImGui::TextWrapped("DISCLAIMER: This software is a prototype, do not expect too much from it. Use at your own risk.");

            ImGui::NewLine();
            ImGui::NewLine();

            // credits
            const char *credit = "by Berdal84";
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(credit).x);
            ImGui::TextWrapped("%s", credit);

            // close on left/rightmouse btn click
            if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1))
            {
                view->show_splashscreen = false;
            }
            ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
            ImGui::EndPopup();
        }

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
            _nodableview_dock_window( k_status_window_name, Dockspace_BOTTOM );

            // Redock windows
            _nodableview_dock_window( cfg->ui_help_window_label             , Dockspace_RIGHT );
            _nodableview_dock_window( cfg->ui_config_window_label           , Dockspace_RIGHT );
            _nodableview_dock_window( cfg->ui_file_info_window_label        , Dockspace_RIGHT );
            _nodableview_dock_window( cfg->ui_node_properties_window_label  , Dockspace_RIGHT );
            _nodableview_dock_window( cfg->ui_interpreter_window_label      , Dockspace_RIGHT );
            _nodableview_dock_window( cfg->ui_imgui_config_window_label     , Dockspace_RIGHT );
            _nodableview_dock_window( cfg->ui_toolbar_window_label          , Dockspace_TOP   );

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

                auto draw_filter = [&](const char* label, Verbosity verbosity)
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

    bool  redock_all      = true;
    File* current_file    = app_state()->current_file;

    //----------------------------------------------------------------------------------------
    // Draw menu bar
    //----------------------------------------------------------------------------------------

    if (ImGui::BeginMenuBar())
    {
        View_Selection selection;
        
        if ( current_file != nullptr )
        {
            selection = current_file->graph->view->selection;
        }

        if (ImGui::BeginMenu("File"))
        {
            bool has_file = current_file != nullptr;
            bool is_current_file_content_dirty = current_file != nullptr && current_file->has_flags(File_Flag_NEEDS_TO_BE_SAVED);

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_NEW))
                event_manager_push_event(action->event);

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_BROWSE))
                event_manager_push_event(action->event);

            ImGui::Separator();

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_SAVE_AS, false, has_file))
                event_manager_push_event(action->event);

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_SAVE, false, has_file && is_current_file_content_dirty))
                event_manager_push_event(action->event);
            ImGui::Separator();

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_FILE_CLOSE, false, has_file))
                event_manager_push_event(action->event);

            auto auto_paste = has_file && current_file->view.experimental_clipboard_auto_paste;

            if (ImGui::MenuItem(ICON_FA_COPY "  Auto-paste clipboard", "", auto_paste, has_file ) && has_file )
            {
                fileview_set_experimental_clipboard_auto_paste(&current_file->view, !auto_paste);
            }
            
            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_REQUEST_EXIT))
                event_manager_push_event(action->event);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_UNDO))
                event_manager_push_event(action->event);

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_REDO))
                event_manager_push_event(action->event);

            ImGui::Separator();
            
            if (ImGui::MenuItem("Delete", "Del.", false, !selection.empty() ))
            {
                event_manager_push_event({ Event_Type_DELETE });
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            //auto frame = ImGui::MenuItem("Frame All", "F");
            redock_all |= ImGui::MenuItem("Redock documents");

            ImGui::Separator();

            auto menu_item_node_view_detail = [current_file](View_Detail _detail, const char *_label) {
                if (ImGui::MenuItem(_label, "", config()->ui_node_detail == _detail))
                {
                    config()->ui_node_detail = _detail;
                    if (current_file != nullptr)
                    {
                        graphview_reset_all_properties( current_file->graph->view );
                    }
                }
            };

            ImGui::Text("View Detail:");
            ImGui::Indent();
            menu_item_node_view_detail(View_Detail_COMPACT  , "Compact");
            menu_item_node_view_detail(View_Detail_NORMAL   , "Normal");
            ImGui::Unindent();

            ImGui::Separator();
            view->show_properties_editor = ImGui::MenuItem(ICON_FA_COGS " Show Properties", "", view->show_properties_editor);
            view->show_imgui_demo = ImGui::MenuItem("Show ImGui Demo", "", view->show_imgui_demo);

            ImGui::Separator();

            const bool is_fullscreen = appview_is_fullscreen();
            if (ImGui::MenuItem("Fullscreen", "", is_fullscreen ))
            {
                appview_set_fullscreen(!is_fullscreen);
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Reset Layout", ""))
            {
                view->should_reset_layout = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Code"))
        {
            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_TOGGLE_ISOLATION_FLAGS, HAS_FLAGS(config()->flags, Config_Flag_ISOLATION_ON)))
            {
                event_manager_push_event(action->event);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Graph"))
        {

            if(const Action* action = ImGuiEx::MenuItem_for_event_type( Event_Type_RESET_GRAPH_VIEW) )
            {
                event_manager_push_event( action->event);
            }

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_RESET_LAYOUT,false, !selection.empty() ) )
            {
                event_manager_push_event(action->event);
            }

            if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_TOGGLE_FOLDING, false, !selection.empty() ) )
            {
                event_manager_push_event(action->event);
            }

            if (ImGui::MenuItem("Expand/Collapse recursive", nullptr, false, !selection.empty() ))
            {
                if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_TOGGLE_FOLDING,false, !selection.empty() ))
                {
                    event_manager_push_event(action->event);
                }
            }

            ImGui::Separator();
            {
                if(const Action* action = ImGuiEx::MenuItem_for_event_type(Event_Type_TOGGLE_ISOLATION_FLAGS, HAS_FLAGS(config()->flags, Config_Flag_ISOLATION_ON)))
                {
                    event_manager_push_event(action->event);
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Developer"))
        {
            Debug_Flags& debug_flags = config()->debug_flags;

            if ( ImGui::MenuItem("Debug Mode", "", debug_flags ) )
            {
                SET_FLAGS(debug_flags, Debug_Flags_ALL);
            }

            if ( debug_flags )
            {
                CHECKBOX_FLAG("Draw ImGuiEx Debug Lines", debug_flags, Debug_Flags_DRAW_IMGUIEX_DEBUG_LINES)
                CHECKBOX_FLAG("Draw Layout Debug Lines",  debug_flags, Debug_Flags_DRAW_LAYOUT_DEBUG_LINES)
                CHECKBOX_FLAG("Show ImGui Config Window", debug_flags, Debug_Flags_SHOW_IMGUI_CONFIG_WINDOW)
            }

            ImGuiEx::set_debug( (bool)HAS_FLAGS(debug_flags, Debug_Flags_DRAW_IMGUIEX_DEBUG_LINES) );

            ImGui::Separator();

            if ( ImGui::MenuItem("Limit FPS", "", config()->fps_limit_on ) )
            {
                config()->fps_limit_on ^= true;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Verbosity"))
            {
                auto menu_item_verbosity = [](Verbosity verbosity, const char* label)
                {
                    if (ImGui::MenuItem(label, "", get_log_verbosity() == verbosity))
                    {
                        set_log_verbosity(verbosity);
                    }
                };

                menu_item_verbosity(Verbosity_Diagnostic, "Verbose" );
                menu_item_verbosity(Verbosity_Message   , "Message" );
                menu_item_verbosity(Verbosity_Warning   , "Warning" );
                menu_item_verbosity(Verbosity_Error     , "Error"   );

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Experimental"))
            {
                CHECKBOX_FLAG("Hybrid history" , config()->flags, Config_Flag_EXPERIMENTAL_HYBRID_COMMAND_MANAGER);
                CHECKBOX_FLAG("Multi-Selection", config()->flags, Config_Flag_EXPERIMENTAL_MULTI_SELECTION);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("An issue ?"))
        {
            if (ImGui::MenuItem("Report on Github.com"))
            {
                system_open_url_async( "https://github.com/berdal84/nodable/issues" );
            }

            if (ImGui::MenuItem("Report by email"))
            {
                system_open_url_async( "mail:berenger@42borgata.com" );
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Show Splash Screen", "F1"))
            {
                view->show_splashscreen = true;
            }

            if (ImGui::MenuItem("Browse source code"))
            {
                system_open_url_async("https://www.github.com/berdal84/nodable" );
            }

            if (ImGui::MenuItem("Credits"))
            {
                system_open_url_async("https://github.com/berdal84/nodable#credits-" );
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    //----------------------------------------------------------------------------------------
    // Draw window content
    //----------------------------------------------------------------------------------------

    // All windows are docked to a dockspace (defined in signal_reset_layout() )

    ImGuiID ds_root = view->dockspaces[Dockspace_ROOT];
    if( !app_state()->files.empty() )
    {
        //----------------------------------------------------------------------------------------
        // Draw tool bar
        //----------------------------------------------------------------------------------------

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 5.0f});

        if ( ImGui::Begin( config()->ui_toolbar_window_label.data, NULL, flags ) )
        {
            const Vec2&   button_size   = config()->ui_toolButton_size;

            ImGui::PopStyleVar();
            ImGui::PushFont(font_manager_get_by_slot(Font_Slot_ToolBtn));
            ImGui::BeginGroup();

            // reset
            if (ImGui::Button(ICON_FA_UNDO " Reset Graph View", button_size)) {
                event_manager_push_event({ Event_Type_RESET_GRAPH_VIEW });
            }
            ImGui::SameLine();

            // enter isolation mode
            if (ImGui::Button(HAS_FLAGS(flags, Config_Flag_ISOLATION_ON)
                ? ICON_FA_CROP " isolation mode: ON "
                : ICON_FA_CROP " isolation mode: OFF",
                button_size))
            {
                event_manager_push_event({ Event_Type_TOGGLE_ISOLATION_FLAGS });
            }
            ImGui::SameLine();
            ImGui::EndGroup();

            ImGui::PopFont();
        }
        ImGui::End();

        //----------------------------------------------------------------------------------------
        // Draw file views (multiple files may be visible)
        //----------------------------------------------------------------------------------------

        for( File* file : app_state()->files )
        {
            ImGui::SetNextWindowDockID(ds_root, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar
                                        | ImGuiWindowFlags_UnsavedDocument * file->has_flags(File_Flag_NEEDS_TO_BE_SAVED);

            auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
            child_bg.w = 0;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

            bool open        = true;
            bool uncollapsed = ImGui::Begin( file->name.data, &open, window_flags);

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(1);

            if ( uncollapsed )
            {
                // Set current file if window is focused
                if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
                    if ( app_state()->current_file != file )
                        app_set_current_file(file);

                // Draw content
                fileview_draw( &file->view, view->dt_in_s );
            }
            ImGui::End();

            if ( !open )
            {
                app_close_file(file);
            }
        }

        //----------------------------------------------------------------------------------------
        // Draw file info panel
        //----------------------------------------------------------------------------------------
        
        if ( current_file != nullptr && ImGui::Begin( config()->ui_file_info_window_label.data ))
        {
            // Basic inFormation
            ImGui::Text("Current file:");
            ImGui::Indent();
            ImGui::TextWrapped("path: %s", current_file->path.c_str());
            ImGui::TextWrapped("set_size: %0.3f KiB", float(file_size(current_file)) / 1000.0f );
            ImGui::Unindent();
            ImGui::NewLine();

            // Statistics
            ImGui::Text("Graph statistics:");
            ImGui::Indent();
            ImGui::Text("Node count: %u", current_file->graph->nodes.size);
            ImGui::Unindent();
            ImGui::NewLine();

            // Hierarchy
            Scope* scope = graph_root_scope(current_file->graph);
            VERIFY(scope, "An Scope root is required to draw the AST as an ImGui tree");
            TreeNode_Scope("Graph's Root Scope", scope);
        }

        ImGui::End();

        //----------------------------------------------------------------------------------------
        // Draw ImGui configuration windows
        //----------------------------------------------------------------------------------------

        if( HAS_FLAGS( config()->debug_flags, Debug_Flags_SHOW_IMGUI_CONFIG_WINDOW) )
        {
            if (ImGui::Begin( config()->ui_imgui_config_window_label.data ))
            {
                ImGui::ShowStyleEditor();
            }
            ImGui::End();
        }
        
        //----------------------------------------------------------------------------------------
        // Draw configuration window (to edit Config and Config)
        //----------------------------------------------------------------------------------------

        if (ImGui::Begin( config()->ui_config_window_label.data ))
        {
            const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

            ImGui::Text("Nodable Settings");
            if ( ImGui::Button("Reset Settings") )
            {
                config_reset();
            }

            if (ImGui::CollapsingHeader("Sizes", flags ))
            {
                ImGui::SliderFloat("set_size factor SM", &config()->size_factor[Size_SM], 0.0f, 5.0f);
                ImGui::SliderFloat("set_size factor MD", &config()->size_factor[Size_MD], 0.0f, 5.0f);
                ImGui::SliderFloat("set_size factor LR", &config()->size_factor[Size_LG], 0.0f, 5.0f);
                ImGui::SliderFloat("set_size factor XL", &config()->size_factor[Size_XL], 0.0f, 5.0f);
            }

            if (ImGui::CollapsingHeader("Nodes", flags ))
            {
                ImGui::Indent();
                if ( ImGui::CollapsingHeader("Colors", flags ))
                {
                    ImGui::ColorEdit4("default"     , &config()->ui_node_fill_color[Node_Type_NULL].x );
                    ImGui::ColorEdit4("entry point" , &config()->ui_node_fill_color[Node_Type_SCOPE].x );
                    ImGui::ColorEdit4("condition"   , &config()->ui_node_fill_color[Node_Type_IF_ELSE].x );
                    ImGui::ColorEdit4("for loop"    , &config()->ui_node_fill_color[Node_Type_FOR_LOOP].x );
                    ImGui::ColorEdit4("while loop"  , &config()->ui_node_fill_color[Node_Type_WHILE_LOOP].x );
                    ImGui::ColorEdit4("variable"    , &config()->ui_node_fill_color[Node_Type_VARIABLE].x );
                    ImGui::ColorEdit4("literal"     , &config()->ui_node_fill_color[Node_Type_LITERAL].x );
                    ImGui::ColorEdit4("function"    , &config()->ui_node_fill_color[Node_Type_FUNCTION].x );
                    ImGui::ColorEdit4("operator"    , &config()->ui_node_fill_color[Node_Type_OPERATOR].x );
                    ImGui::Separator();
                    ImGui::ColorEdit4("highlighted"         , &config()->ui_node_highlightedColor.x);
                    ImGui::ColorEdit4("shadow"              , &config()->ui_node_shadowColor.x);
                    ImGui::ColorEdit4("border"              , &config()->ui_slot_border_color.x);
                    ImGui::ColorEdit4("border (highlighted)", &config()->ui_node_borderHighlightedColor.x);
                    ImGui::ColorEdit4("slot (in)"           , &config()->ui_slot_color_light.x);
                    ImGui::ColorEdit4("slot (out)"          , &config()->ui_slot_color_dark.x);
                    ImGui::ColorEdit4("slot (hovered)"      , &config()->ui_slot_hovered_color.x);
                }

                if ( ImGui::CollapsingHeader("Node_Slots", flags ))
                {
                    ImGui::Text("Property Node_Slots:");
                    ImGui::SliderFloat("slot radius", &config()->ui_slot_circle_radius_base, 5.0f, 10.0f);

                    ImGui::Separator();

                    ImGui::Text("Code Flow Node_Slots:");
                    ImGui::SliderFloat2("slot set_size##codeflow"   , &config()->ui_slot_rectangle_size.x, 2.0f, 100.0f);
                    ImGui::SliderFloat("slot padding##codeflow" , &config()->ui_slot_gap, 0.0f, 100.0f);
                    ImGui::SliderFloat("slot radius##codeflow"  , &config()->ui_slot_border_radius, 0.0f, 40.0f);
                }

                if ( ImGui::CollapsingHeader("Misc.", flags ))
                {
                    ImGui::SliderFloat2("gap app (x and y-axis)", &config()->ui_node_gap_base.x, 0.0f, 400.0f);
                    ImGui::SliderFloat("velocity" , &config()->ui_node_speed, 1.0f, 10.0f);
                    ImGui::SliderFloat4("padding" , &config()->ui_node_padding.x, 0.0f, 20.0f);
                    ImGui::SliderFloat("border width", &config()->ui_node_borderWidth, 0.0f, 10.0f);
                    ImGui::SliderFloat("border width ratio (instructions)", &config()->ui_node_instructionBorderRatio, 0.0f, 10.0f);
                }
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Wires / Code Flow", flags ))
            {
                ImGui::Text("Wires");
                ImGui::SliderFloat("thickness", &config()->ui_wire_bezier_thickness, 0.5f, 10.0f);
                ImGui::SliderFloat2("roundness (min,max)", &config()->ui_wire_bezier_roundness.x, 0.0f, 1.0f);
                ImGui::SliderFloat2("fade length (min,max in lensqr)", &config()->ui_wire_bezier_fade_lensqr_range.x, 0.0f, 100000.0f);
                ImGui::ColorEdit4("color", &config()->ui_wire_color.x);
                ImGui::ColorEdit4("shadow color", &config()->ui_wire_shadowColor.x);

                ImGui::Separator();

                ImGui::Text("Code Flow");
                ImGui::ColorEdit4("color##codeflow", &config()->ui_codeflow_color.x);
                ImGui::SliderFloat("thickness (ratio)##codeflow", &config()->ui_codeflow_thickness_ratio, 0.1, 1.0);
            }

            if (ImGui::CollapsingHeader("Graph", flags ))
            {
                ImGui::InputFloat("view unfold duration (sec)", &config()->graph_view_unfold_duration);
                ImGui::ColorEdit4("grid color (major)", &config()->ui_graph_grid_color_major.x);
                ImGui::ColorEdit4("grid color (minor)", &config()->ui_graph_grid_color_minor.x);
                ImGui::SliderInt("grid set_size", &config()->ui_grid_size, 1, 500);
                ImGui::SliderInt("grid subdivisions", &config()->ui_grid_subdiv_count, 1, 16);
            }

            if (ImGui::CollapsingHeader("Scope", flags ))
            {
                ImGui::SliderFloat4("padding (left, top, right, bottom)", &config()->ui_scope_padding.left, 2, 25);
                ImGui::SliderFloat("border radius", &config()->ui_scope_border_radius, 0, 20);
                ImGui::SliderFloat("border thickness", &config()->ui_scope_border_thickness, 0, 4);
                ImGui::ColorEdit4("fill color (light)", &config()->ui_scope_fill_col_light.x);
                ImGui::ColorEdit4("fill color (dark)", &config()->ui_scope_fill_col_dark.x);
                ImGui::ColorEdit4("border color", &config()->ui_scope_border_col.x);
            }

            if (ImGui::CollapsingHeader("Shortcuts", flags ))
            {
                action_manager_view_draw(action_manager());
            }

        #if NDBL_POOL_ENABLE
            if ( tools_config()->runtime_debug && ImGui::CollapsingHeader("Pool"))
            {
                ImGui::Text("Pool stats:");
                auto pool = get_pool_manager()->get_pool();
                ImGui::Text(" - Node.................... %8zu", pool->get_all<Node>().size() );
                ImGui::Text(" - Node_View............... %8zu", pool->get_all<Node_View>().size() );
                ImGui::Text(" - Physics................. %8zu", pool->get_all<Physics>().size() );
                ImGui::Text(" - Scope................... %8zu", pool->get_all<Scope>().size() );
            }
        #endif
        }
        ImGui::End();

        //----------------------------------------------------------------------------------------
        // Draw node properties window
        //----------------------------------------------------------------------------------------

        if (ImGui::Begin( config()->ui_node_properties_window_label.data ))
        {
            if( app_state()->current_file )
            {
                bool node_properties_changed = false;
                const Graph_View* graph_view = app_state()->current_file->graph->view; // Graph can't be null
                switch ( view_selection_count(&graph_view->selection, View_Type_NODE) )
                {
                    case 0:
                        break;
                    case 1:
                    {
                        ImGui::Indent(10.0f);
                        View first = view_selection_first_of(&graph_view->selection, View_Type_NODE);
                        node_properties_changed |= nodeview_draw_as_properties_panel(first.nodeview, &view->show_advanced_node_properties);
                        break;
                    }
                    default:
                        ImGui::Indent(10.0f);
                        ImGui::Text("Multi-Selection");
                }

                if ( node_properties_changed )
                {
                    app_state()->current_file->set_flags(File_Flag_TEXT_IS_DIRTY);
                }
            }
        }
        ImGui::End();
        
        //----------------------------------------------------------------------------------------
        // Draw help window
        //----------------------------------------------------------------------------------------
        {
        if (ImGui::Begin( config()->ui_help_window_label.data ))
        {
            ImGui::PushFont(font_manager_get_by_slot(Font_Slot_Heading));
            ImGui::Text("Welcome to Nodable!");
            ImGui::PopFont();
            ImGui::NewLine();
            ImGui::TextWrapped(
                    "Nodable is primary_child-able.\n"
                    "\n"
                    "Nodable allows you to edit a program using both text and graph paradigms."
                    "More precisely, it means:"
            );
            ImGuiEx::BulletTextWrapped("any change on the text will affect the graph");
            ImGuiEx::BulletTextWrapped("any change (structure or values) on the graph will affect the text");
            ImGuiEx::BulletTextWrapped(
                    "but keep in mind the app is the text, any change not affecting the text (such as child positions or orphan primary_child) will be lost.");
            ImGui::NewLine();
            ImGui::PushFont(font_manager_get_by_slot(Font_Slot_Heading));
            ImGui::Text("Quick start");
            ImGui::PopFont();
            ImGui::NewLine();
            ImGui::TextWrapped("Nodable UI is designed as following:\n");
            ImGuiEx::BulletTextWrapped("On the left side a (light) text editor allows to edit source code.\n");
            ImGuiEx::BulletTextWrapped(
                    "At the center, there is the graph editor where you can create_new/delete/connect primary_child\n");
            ImGuiEx::BulletTextWrapped(
                    "On the right side (this side) you will find many tabs to manage additional config such as primary_child, interpreter, or app properties\n");
            ImGuiEx::BulletTextWrapped("At the top, between the menu and the editors, there is a tool bar."
                                        " There, few buttons will serve to compile, run and debug your program.");
            ImGuiEx::BulletTextWrapped("And at the bottom, below the editors, there is a status bar."
                                        " This bar will display important messages, warning, and errors. You can expand it to get older messages.");
        }
        ImGui::End();
        }
    }
    else if( !view->show_splashscreen ) // splashscreen has to be closed by the user to show the startup window
    {
        //----------------------------------------------------------------------------------------
        // Draw startup window (with file examples to open)
        //----------------------------------------------------------------------------------------

        ImGui::SetNextWindowDockID(ds_root, ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3f, 0.3f, 0.3f, 1.f));

        ImGui::Begin( config()->ui_startup_window_label.data );
        {
            ImGui::PopStyleColor();

            ImVec2 center_area(500.0f, 300.0f);
            ImVec2 avail = ImGui::GetContentRegionAvail();

            ImGui::SetCursorPosX((avail.x - center_area.x) / 2);
            ImGui::SetCursorPosY((avail.y - center_area.y) / 2);

            ImGui::BeginChild("center_area", center_area);
            {
                ImGui::Indent(center_area.x * 0.05f);

                ImGui::PushFont(font_manager_get_by_slot(Font_Slot_ToolBtn));
                ImGui::NewLine();

                ImVec2 btn_size(center_area.x * 0.44f, 40.0f);
                if (ImGui::Button(ICON_FA_FILE" New File", btn_size))
                    event_manager_push_event({ Event_Type_FILE_NEW });
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER_OPEN" Open ...", btn_size))
                    event_manager_push_event({ Event_Type_FILE_BROWSE });

                ImGui::NewLine();
                ImGui::Separator();
                ImGui::NewLine();

                ImGui::Text("%s", "Open an example");

                struct Example {
                    const bdc::String label;
                    const bdc::String path;
                };

                const std::array<Example, 4> examples = {
                    Example{ ICON_FA_BOOK" Single expressions    ", "examples/arithmetic.cpp" },
                    Example{ ICON_FA_BOOK" Multi instructions    ", "examples/multi-instructions.cpp" },
                    Example{ ICON_FA_BOOK" Conditional Structures", "examples/if-else.cpp" },
                    Example{ ICON_FA_BOOK" For Loop              ", "examples/for-loop.cpp" }
                };

                const ImVec2 example_btn_size(btn_size.x, btn_size.y * 0.66f);
                const int columns = 2;

                ImGui::NewLine();
                size_t i = 0;
                for (const Example& example : examples)
                {
                    if (i % columns != 0) ImGui::SameLine();
                    if (ImGui::Button(example.label.c_str(), example_btn_size))
                    {
                        app_open_asset_file( example.path );
                    }
                    i++;
                }

                ImGui::NewLine();

                if ( ImGui::Button(ICON_FA_BOOK" Open All", example_btn_size) )
                {
                    for (const Example& example : examples)
                        app_open_asset_file( example.path );
                }   
                ImGui::NewLine();
                
                ImGui::PopFont();

                ImGui::NewLine();
                ImGui::Unindent();
            }
            ImGui::EndChild();
        }
        ImGui::End(); // Startup Window
    }
    
    // END FRAME

    ASSERT(view != nullptr);

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
    Uint64 new_dt       = (now - before);
    float  fps          = 1000.0f / float(new_dt);
    view->smoothed_fps  = clamped_lerp(view->smoothed_fps, fps, 1.f / 20.f); // smooth the last n frames
    view->dt_in_ms      = float(new_dt);
    view->dt_in_s       = float(new_dt) / 1000.f;
    before              = now;

    NDBL_DEBUG_LOG( NDBL_DIAG, "App_View", "dt: %f sec, %i msec \n", view->dt_in_s, view->dt_in_ms);

    // Format nice title
    bdc::String title = string_tprintf("%s | %4.0ffps %s", view->title.data, view->smoothed_fps, cfg->fps_limit_on ? "" : "unlimited!");

    // Update window title
    SDL_SetWindowTitle(view->sdl_window, title.data);
}

void appview_save_screenshot(const bdc::String relative_path)
{
    App_View_State* view = appview();
    
    NDBL_LOG(Verbosity_Message, __FILE__, "Taking screenshot ...\n");
    auto path = Path::get_executable_path().parent_path() / "screenshots" / relative_path;
    if (!Path::exists(path.parent_path()))
    {
        Path::create_directories(path.parent_path());
    }
    
    std::vector<unsigned char> out = appview_take_screenshot();
    NDBL_LOG(Verbosity_Message, __FILE__, "Save screenshot ...\n");
    lodepng::save_file(out, path.c_str());
    NDBL_LOG(Verbosity_Message, __FILE__, "Save screenshot " NDBL_OK " (%s)\n", path.c_str());
}

void appview_update()
{
    App_View_State* view = appview();
    File* current_file = app_state()->current_file;

    if( current_file != nullptr )
    {
        fileview_update(&current_file->view, view->dt_in_s);
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_WINDOWEVENT:
                if( event.window.event == SDL_WINDOWEVENT_CLOSE)
                    SET_FLAGS(app_state()->flags, App_Flag_SHOULD_STOP);
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

#ifdef NDBL_DESKTOP
bool appview_pick_file_path(Path& _out_path, Dialog_Type _dialog_type)
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
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, __FILE__, "pick_file_path cancelled by user");
            return false;
        default:
            NDBL_LOG(Verbosity_Error, __FILE__, "%s\n", NFD_GetError());
            return false;
    }
}

#elif __EMSCRIPTEN__

EM_JS(void, call_appview_pick_file_path, (bool), {
  alert('appview_pick_file_path not implemented yet');
  throw 'all done';
});
bool appview_pick_file_path(Path& _out_path, Dialog_Type _dialog_type)
{
    bool result;
    call_pick_file_path(result);
    return result;
}
#endif

ImGuiID _nodableview_get_dockspace(Dockspace dockspace)
{
    return appview()->dockspaces[dockspace];
}

void _nodableview_dock_window(const bdc::String& window_name, Dockspace dockspace)
{
    ImGui::DockBuilderDockWindow(window_name.data, appview()->dockspaces[dockspace]);
}

std::vector<unsigned char> appview_take_screenshot()
{
#ifdef __EMSCRIPTEN__
    return {};
    // TODO: some glXXX are unavailable, but anyways it's not something we need in WEB, we can use browser for that.
#else
    NDBL_LOG(Verbosity_Message, __FILE__, "Taking screenshot ...\n");
    int width, height;
    SDL_GetWindowSize(appview()->sdl_window, &width, &height);
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
    NDBL_LOG(Verbosity_Message, __FILE__, "Taking screenshot " NDBL_OK "\n");
    return out;
#endif
}


bool appview_is_fullscreen()
{
    return SDL_GetWindowFlags(appview()->sdl_window) & (SDL_WindowFlags::SDL_WINDOW_FULLSCREEN | SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void appview_set_fullscreen(bool b)
{
    SDL_SetWindowFullscreen(appview()->sdl_window, b ? SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void appview_set_title(const bdc::String& title )
{
    appview()->title = title;
    SDL_SetWindowTitle( appview()->sdl_window, appview()->title.data );
}

} // namespace ndbl