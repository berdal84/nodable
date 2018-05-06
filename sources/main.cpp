// ImGui - standalone example application for SDL2 + OpenGL
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <imgui.h>
#include <examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "Node_Application.h"
#include <string>

int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup Nodable
    Nodable::Node_Application nodable;
    nodable.init();

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
    SDL_Window *window = SDL_CreateWindow("ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_MAXIMIZED|SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplSdlGL3_Init(window);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH = 4;
    config.OversampleV = 4;
    io.DeltaTime = 1.0f/120.0f;
    io.Fonts->AddFontFromFileTTF("data/FreeSerif.ttf", 16.0f, &config);

    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    ImVec4 clear_color = ImColor(50, 50, 50);

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }
        ImGui_ImplSdlGL3_NewFrame(window);

        // 1. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        //ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        //ImGui::ShowTestWindow(&show_test_window);

        if( ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("New", "Ctrl + N");
                ImGui::MenuItem("Save", "Ctrl + N");
                ImGui::MenuItem("Save As.", "Ctrl + N");
                done = ImGui::MenuItem("Quit", "Alt + F4");
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        // 2. Rendering Nodable Application
        bool isCommandLineVisible = true;
        ImGui::Begin("Nodable command line", &isCommandLineVisible, ImGuiWindowFlags_ShowBorders);
        static char buf[1024];

        // Set Keyboard focus here once
        static bool setKeyboardFocusOnCommandLine = true;
        if ( setKeyboardFocusOnCommandLine){
           ImGui::SetKeyboardFocusHere();
           setKeyboardFocusOnCommandLine = false;
        }

        bool needsToEvaluateString = ImGui::InputText("", buf, 1023, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        needsToEvaluateString |= ImGui::Button("Eval");

        if (needsToEvaluateString)
        {
            done = nodable.eval(std::string(buf));
            setKeyboardFocusOnCommandLine = true;
        }


        ImGui::End();

        nodable.draw();

        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
