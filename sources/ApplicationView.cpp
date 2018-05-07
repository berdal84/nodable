#include "ApplicationView.h"

// Includes for ImGui
#include <GL/gl3w.h>
#include <imgui.h>
#include <examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h>

#include "Node_Application.h"
#include "Node_Container.h"

using namespace Nodable;

ApplicationView::ApplicationView(const char* _name, Node_Application* _application):
	application(_application),
	name(_name)
{

}

ApplicationView::~ApplicationView()
{
	ImGui_ImplSdlGL3_Shutdown ();
    SDL_GL_DeleteContext      (glcontext);
    SDL_DestroyWindow         (window);
    SDL_Quit                  ();
}

bool ApplicationView::init()
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
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
    window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_MAXIMIZED|SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

    glcontext = SDL_GL_CreateContext(window);
    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplSdlGL3_Init(window);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io           = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH    = 4;
    config.OversampleV    = 4;
    io.DeltaTime          = 1.0f/120.0f;
    io.Fonts->AddFontFromFileTTF("data/FreeSerif.ttf", 16.0f, &config);    

    // Configure ImGui Style
    ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.49f, 0.63f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(1.00f, 1.00f, 1.00f, 0.08f);
	style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
	style.Colors[ImGuiCol_Border]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
	style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.90f, 0.80f, 0.80f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 1.00f);
	style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.49f, 0.63f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.60f, 0.60f, 0.60f, 0.98f);
	style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.61f, 0.61f, 0.62f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.65f, 0.65f, 0.65f, 0.99f);
	style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
	style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.71f, 0.46f, 0.22f, 0.63f);
	style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.71f, 0.46f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_Button]                = ImVec4(0.89f, 0.66f, 0.27f, 0.63f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.98f, 0.73f, 0.29f, 0.95f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_Header]                = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.84f, 0.84f, 0.84f, 0.96f);
	style.Colors[ImGuiCol_HeaderActive]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_Column]                = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.89f, 0.89f, 0.89f, 1.00f);
	style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.55f);

	style.FrameRounding      = 3.0f;
	style.AntiAliasedShapes  = true;
	style.AntiAliasedLines   = true;

	return true;
}

void ApplicationView::draw()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSdlGL3_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            application->stopExecution();
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
            if ( ImGui::MenuItem("Quit", "Alt + F4"))
            	application->stopExecution();
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
        application->eval(std::string(buf));
        setKeyboardFocusOnCommandLine = true;
    }

    ImGui::End();

    // draw the properties panel
    extern float bezierCurveOutRoundness;
    extern float bezierCurveInRoundness;
    extern float bezierThickness ;
    extern bool displayArrows; 

    ImGui::Text("Bezier curves");
    ImGui::SliderFloat("thickness", &bezierThickness, 0.5f, 10.0f);
    ImGui::SliderFloat("out roundness", &bezierCurveOutRoundness, 0.0f, 1.0f);
    ImGui::SliderFloat("in roundness", &bezierCurveInRoundness, 0.0f, 1.0f);
    ImGui::Checkbox("arrows", &displayArrows);

    // draw the main context
	application->getContext()->draw();

    // Rendering
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    SDL_GL_SwapWindow(window);
}