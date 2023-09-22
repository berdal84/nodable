#include "App.h"

#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <nativefiledialog-extended/src/include/nfd.h>
#include "gl3w/GL/gl3w.h"
#include "lodepng/lodepng.h"

#include "core/system.h"
#include "AppView.h"

using namespace fw;

App *App::s_instance = nullptr;

App::App(Config& _config)
    : config(_config)
    , should_stop(false)
    , font_manager(_config.font_manager)
    , event_manager()
    , texture_manager()
    , m_sdl_window(nullptr)
    , m_sdl_gl_context()
    , view(this)
{
    LOG_VERBOSE("fw::App", "Constructor ...\n");
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
    LOG_VERBOSE("fw::App", "init ...\n");
    LOG_VERBOSE("fw::NodableView", "init ...\n");

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        LOG_ERROR( "fw::NodableView", "SDL Error: %s\n", SDL_GetError())
        return false;
    }

    // Setup window
    LOG_VERBOSE("fw::NodableView", "setup SDL ...\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    m_sdl_window = SDL_CreateWindow(config.app_window_label.c_str(),
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

    LOG_VERBOSE("fw::NodableView", "gl3w init ...\n");
    gl3wInit();

    // Setup Dear ImGui binding
    LOG_VERBOSE("fw::NodableView", "ImGui init ...\n");
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
    LOG_VERBOSE("fw::NodableView", "patch ImGui's style ...\n");
    ImGuiStyle& style = ImGui::GetStyle();
    config.patch_imgui_style(style);
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
        LOG_ERROR("fw::NodableView", "Unable to init NFD\n");
    }

    LOG_VERBOSE("fw::App", "state_changes.emit(ON_INIT) ...\n");
    signal_handler(Signal_ON_INIT);
    LOG_VERBOSE("fw::App", "init " OK "\n");
    return true;
}

void App::update()
{
    LOG_VERBOSE("fw::App", "update ...\n");
    handle_events();
    LOG_VERBOSE("fw::App", "state_changes.emit(ON_UPDATE) ...\n");
    signal_handler(Signal_ON_UPDATE);
    LOG_VERBOSE("fw::App", "update " OK "\n");
}

bool App::shutdown()
{
    bool success = true;
    LOG_MESSAGE("fw::App", "Shutting down ...\n");

    success &= texture_manager.release_all();

    LOG_MESSAGE("fw::App", "Shutting down ImGui_ImplSDL2 ...\n");
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
    signal_handler(App::Signal_ON_SHUTDOWN);
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
    view.draw();
    signal_handler(App::Signal_ON_DRAW);

    // 3. End frame and Render
    //------------------------

    ImGuiEx::EndFrame();
    ImGui::Render(); // Finalize draw data

    SDL_GL_MakeCurrent(m_sdl_window, m_sdl_gl_context);
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(config.background_color.Value.x, config.background_color.Value.y, config.background_color.Value.z, config.background_color.Value.w);
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
    if (ImGui::GetIO().DeltaTime < config.min_frame_time)
    {
        SDL_Delay((unsigned int)((config.min_frame_time - ImGui::GetIO().DeltaTime) * 1000.f) );
    }
    LOG_VERBOSE("fw::App", "_draw_property_view " OK "\n");
}

double App::elapsed_time() const
{
    return ImGui::GetTime();
}

ghc::filesystem::path App::asset_path(const ghc::filesystem::path& _path)
{
    // If the path is absolute, we use it as-is,
    // Else we append assets path.
    if( _path.is_absolute() ) return _path;

    return fw::system::get_executable_directory() / "assets" / _path;
}

ghc::filesystem::path App::asset_path(const char* _path)
{
    const ghc::filesystem::path fs_path{_path};
    return asset_path(fs_path);
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
                    for(const auto& _binded_event: event_manager.get_binded_events() )
                    {
                        // first, priority to shortcuts with mod
                        if ( _binded_event.shortcut.mod != KMOD_NONE
                            && _binded_event.event_t
                            && (_binded_event.shortcut.mod & event.key.keysym.mod)
                            && _binded_event.shortcut.key == event.key.keysym.sym
                        )
                        {
                            event_manager.push(_binded_event.event_t);
                            break;
                        }
                    }
                }
                else // without any mod key
                {
                    for(const auto& _binded_event: event_manager.get_binded_events() )
                    {
                        // first, priority to shortcuts with mod
                        if ( _binded_event.shortcut.mod == KMOD_NONE
                            && _binded_event.event_t
                            && _binded_event.shortcut.key == event.key.keysym.sym
                        )
                        {
                            event_manager.push(_binded_event.event_t);
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
    LOG_MESSAGE("fw::NodableView", "Taking screenshot ...\n");
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

    LOG_MESSAGE("fw::NodableView", "Taking screenshot OK (%s)\n", _path);
}

bool App::is_fullscreen() const
{
    return SDL_GetWindowFlags(m_sdl_window) & (SDL_WindowFlags::SDL_WINDOW_FULLSCREEN | SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void App::set_fullscreen(bool b)
{
    SDL_SetWindowFullscreen(m_sdl_window, b ? SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

int App::main()
{
    if (init())
    {
        while (!should_stop)
        {
            update();
            draw();
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
