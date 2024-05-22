#include "App.h"

#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <lodepng/lodepng.h>
#include <nfd.h>
#include <gl3w.h>

#include "fw/core/system.h"
#include "AppView.h"
#include "ImGuiEx.h"
#include "gui.h"

using namespace fw;

App *App::s_instance = nullptr;

App::App(AppView* _view)
    : should_stop(false)
    , font_manager()
    , event_manager()
    , action_manager()
    , texture_manager()
    , m_sdl_window(nullptr)
    , m_sdl_gl_context()
    , m_view(_view)
{
    LOG_VERBOSE("fw::App", "Constructor ...\n");
    FW_EXPECT( m_view, "View cannot be null");
    FW_EXPECT(s_instance == nullptr, "Only a single fw::App at a time allowed");
    s_instance = this;
    LOG_VERBOSE("fw::App", "Constructor " OK "\n");
}

App::~App()
{
    LOG_VERBOSE("fw::App", "Destructor ...\n");
    s_instance = nullptr;
    LOG_VERBOSE("fw::App", "Destructor " OK "\n");
}

bool App::init()
{
    before_init();
    LOG_VERBOSE("fw::App", "init ...\n");

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        LOG_ERROR( "fw::App", "SDL Error: %s\n", SDL_GetError())
        return false;
    }

    // Setup window
    LOG_VERBOSE("fw::App", "setup SDL ...\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    m_sdl_window = SDL_CreateWindow( g_conf->app_window_label.c_str(),
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
    SDL_GL_SetSwapInterval( g_conf->vsync ? 1 : 0);

    LOG_VERBOSE("fw::App", "gl3w init ...\n");
    gl3wInit();

    // Setup Dear ImGui binding
    LOG_VERBOSE("fw::App", "ImGui init ...\n");
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
    LOG_VERBOSE("fw::App", "patch ImGui's style ...\n");
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
    colors[ImGuiCol_Button]                 = (ImVec4) g_conf->button_color;
    colors[ImGuiCol_ButtonHovered]          = (ImVec4) g_conf->button_hoveredColor;
    colors[ImGuiCol_ButtonActive]           = (ImVec4) g_conf->button_activeColor;
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

    style.WindowBorderSize   = g_conf->border_size;
    style.FrameBorderSize    = g_conf->border_size;
    style.FrameRounding      = g_conf->frame_rounding;
    style.ChildRounding      = g_conf->frame_rounding;
    style.WindowRounding     = g_conf->window_rounding;
    style.AntiAliasedFill    = g_conf->antialiased;
    style.AntiAliasedLines   = g_conf->antialiased;
    style.WindowPadding      = g_conf->padding;

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
    ImGui_ImplSDL2_InitForOpenGL(m_sdl_window, m_sdl_gl_context);
    const char* glsl_version = NULL; // let backend decide wich version to use, usually 130 (pc) or 150 (macos).
    ImGui_ImplOpenGL3_Init(glsl_version);

    if (NFD_Init() != NFD_OKAY)
    {
        LOG_ERROR("fw::App", "Unable to init NFD\n");
    }

    LOG_VERBOSE("fw::App", "state_changes.emit(ON_INIT) ...\n");
    on_init();
    m_view->on_init();
    LOG_VERBOSE("fw::App", "init " OK "\n");
    return true;
}

void App::update()
{
    LOG_VERBOSE("fw::App", "update ...\n");
    handle_events();
    LOG_VERBOSE("fw::App", "state_changes.emit(ON_UPDATE) ...\n");
    on_update();
    LOG_VERBOSE("fw::App", "update " OK "\n");
}

bool App::shutdown()
{
    bool success = true;
    LOG_MESSAGE("fw::App", "Shutting down ...\n");

    success &= texture_manager.release_all();

    LOG_MESSAGE("fw::App", "Shutting down OpenGL3 ...\n");
    ImGui_ImplOpenGL3_Shutdown();
    LOG_MESSAGE("fw::App", "Shutting down SDL2 ...\n");
    ImGui_ImplSDL2_Shutdown();
    LOG_MESSAGE("fw::App", "Destroying ImGui context ...\n");
    ImGui::DestroyContext    ();
    LOG_MESSAGE("fw::App", "Shutdown SDL ...\n");
    SDL_GL_DeleteContext     (m_sdl_gl_context);
    SDL_DestroyWindow        (m_sdl_window);
    SDL_Quit                 ();

    LOG_MESSAGE("fw::App", "Quitting NFD (Native File Dialog) ...\n");
    NFD_Quit();

    LOG_VERBOSE("fw::App", "state_changes.emit(App::ON_SHUTDOWN) ...\n");
    on_shutdown();

    LOG_MESSAGE("fw::App", "Shutdown %s\n", success ? OK : KO)
    return success;
}

void App::draw()
{
    LOG_VERBOSE("fw::App", "_draw_property_view ...\n");
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(m_sdl_window);
    ImGui::NewFrame();
    ImGuiEx::BeginFrame();

    LOG_VERBOSE("fw::App", "state_changes.emit(App::ON_DRAW) ...\n");
    m_view->draw();
    on_draw();

    // 3. End frame and Render
    //------------------------

    ImGuiEx::EndFrame();
    ImGui::Render(); // Finalize draw data

    SDL_GL_MakeCurrent(m_sdl_window, m_sdl_gl_context);
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    Vec4& color = g_conf->background_color.value;
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
    LOG_VERBOSE("fw::App", "_draw_property_view " OK "\n");
}

double App::elapsed_time() const
{
    return ImGui::GetTime();
}

std::filesystem::path App::asset_path(const std::filesystem::path& _path)
{
    FW_EXPECT(!_path.is_absolute(), "_path is not relative, this can't be an asset")
    auto executable_dir = fw::system::get_executable_directory();
    return executable_dir / "assets" / _path;
}

std::filesystem::path App::asset_path(const char* _path)
{
    std::filesystem::path fs_path{_path};
    return  fs_path.is_absolute() ? fs_path
                                  : asset_path(fs_path);
}

void App::handle_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_WINDOWEVENT:
                if( event.window.event == SDL_WINDOWEVENT_CLOSE)
                    should_stop = true;
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


void App::save_screenshot(const char*_path)
{
    LOG_MESSAGE("fw::App", "Taking screenshot ...\n");
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
    auto absolute_path = asset_path(_path);
    lodepng::save_file(out, absolute_path.string());

    LOG_MESSAGE("fw::App", "Taking screenshot OK (%s)\n", _path);
}

bool App::is_fullscreen() const
{
    return SDL_GetWindowFlags(m_sdl_window) & (SDL_WindowFlags::SDL_WINDOW_FULLSCREEN | SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void App::set_fullscreen(bool b)
{
    SDL_SetWindowFullscreen(m_sdl_window, b ? SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

int App::main(int argc, char *argv[])
{
    if (init())
    {
        u32_t frame_start;
        while (!should_stop)
        {
            frame_start = SDL_GetTicks();

            update();
            draw();

            u32_t frame_time = SDL_GetTicks() - frame_start;

            // limit frame rate
            if ( g_conf->delta_time_limit && frame_time < g_conf->delta_time_min )
            {
                SDL_Delay( g_conf->delta_time_min - frame_time );
            }

            if( g_conf->show_fps)
            {
                static u32_t dt = 1000 / 60;
                u32_t all_time = SDL_GetTicks() - frame_start;
                if( all_time <= 0 ) all_time = 1;
                dt = u32_t(0.9f*float(dt) + 0.1f*float(all_time)); // Smooth value
                u32_t fps = 1000 / dt;
                char title[256];
                snprintf( title, 256, "%s | %i fps (dt %d ms, frame %d ms)", g_conf->app_window_label.c_str(), fps, dt, frame_time );
                title[255] = '\0';
                SDL_SetWindowTitle( m_sdl_window, title );
            }
        }
        shutdown();
        LOG_FLUSH()
        return 0;
    }
    else
    {
        LOG_FLUSH()
        return 1;
    }
}

int App::fps()
{
    return (int)ImGui::GetIO().Framerate;
}

void App::show_splashscreen( bool b )
{
    g_conf->splashscreen = b;
}
