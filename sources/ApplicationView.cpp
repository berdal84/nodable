#include "ApplicationView.h"

// Includes for ImGui
#include <GL/gl3w.h>
#include <imgui.h>
#include <examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h>

#include "Application.h"
#include "Container.h"
#include "NodeView.h"

using namespace Nodable;

ApplicationView::ApplicationView(const char* _name, Application* _application):
	application(_application)
{
    addMember("glWindowName");
    setMember("glWindowName", _name);

    // Add a member to know if we should display the properties panel or not
    addMember("showProperties");
    setMember("showProperties", false);

    // Add two members for the window size
    addMember("glWindowSizeX");
    setMember("glWindowSizeX", 1280.0f);

    addMember("glWindowSizeY");
    setMember("glWindowSizeY", 720.0f);
}

ApplicationView::~ApplicationView()
{
    ImGui_ImplSdlGL3_Shutdown();
    ImGui::DestroyContext    ();
    SDL_GL_DeleteContext     (glcontext);
    SDL_DestroyWindow        (window);
    SDL_Quit                 ();
}

bool ApplicationView::init()
{
	    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
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
    window = SDL_CreateWindow(  getMember("glWindowName")->getValueAsString().c_str(),
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                getMember("glWindowSizeX")->getValueAsNumber(),
                                getMember("glWindowSizeY")->getValueAsNumber(),
                                SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    
    glcontext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    gl3wInit();

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    ImGui_ImplSdlGL3_Init(window);

    // Setup style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImFontConfig config;
    config.OversampleH    = 4;
    config.OversampleV    = 4;
    io.DeltaTime          = 1.0f/120.0f;
    io.Fonts->AddFontFromFileTTF("data/FreeSerif.ttf", 16.0f, &config);    
    io.FontAllowUserScaling = true;

    // Configure ImGui Style
    ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(1.00f, 1.00f, 1.00f, 0.08f);
	style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
	style.Colors[ImGuiCol_Border]                = ImVec4(1.00f, 1.00f, 1.00f, 0.5f);
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
	style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.55f);

	style.FrameRounding      = 3.0f;
	style.AntiAliasedFill    = true;
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

        if (ImGui::BeginMenu("Edit"))
        {
            auto hide    = ImGui::MenuItem("Hide", "Del.");
            auto arrange = ImGui::MenuItem("Arrange", "A");
            ImGui::Separator();
            auto showProperties = ImGui::MenuItem("Settings", "", getMember("showProperties")->getValueAsBoolean());

            auto selected = NodeView::GetSelected();
            if( selected )
	        {
	            if (hide)
	            	selected->setVisible(false);
				else if (arrange)
					selected->arrangeRecursively();
        	}

            if(showProperties)
                 setMember("showProperties", !getMember("showProperties")->getValueAsBoolean());

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu("View"))
        {
        	//auto frame = ImGui::MenuItem("Frame All", "F");
        	//ImGui::Separator();
        	auto detailSimple   = ImGui::MenuItem("Simple View", "", NodeView::s_drawDetail == DrawDetail_Simple );
        	auto detailAdvanced = ImGui::MenuItem("Advanced View", "", NodeView::s_drawDetail == DrawDetail_Advanced );
        	auto detailComplex  = ImGui::MenuItem("Complex View", "", NodeView::s_drawDetail == DrawDetail_Complex );

        	//if( frame)
        		// TODO

        	if (detailSimple)
        		NodeView::s_drawDetail = DrawDetail_Simple;

        	if (detailAdvanced)
        		NodeView::s_drawDetail = DrawDetail_Advanced;

        	if (detailComplex)
        		NodeView::s_drawDetail = DrawDetail_Complex;

        	ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // 2. Command line window
    {
	    bool isCommandLineVisible = true;

	    if(ImGui::Begin("Nodable command line", &isCommandLineVisible, ImGuiWindowFlags_AlwaysAutoResize))
	    {
		    
		    ImGui::Text("Type an expression, the program will create the graph in realtime :");

		    // Draw the input text field :
		    static char inputTextBuffer[1024];
		    static bool isExpressionValid = true;
		    auto textColor = isExpressionValid ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.9f, 0.0f, 0.0f,1.0f);
		    ImGui::PushStyleColor(ImGuiCol_Text,textColor );
		    static bool setKeyboardFocusOnCommandLine = true;
		    if ( setKeyboardFocusOnCommandLine){
		       ImGui::SetKeyboardFocusHere();
		       setKeyboardFocusOnCommandLine = false;
		    }
		    bool needsToEvaluateString = ImGui::InputText("", inputTextBuffer, 1023 /*, ImGuiInputTextFlags_EnterReturnsTrue*/);
			ImGui::PopStyleColor();

		    //ImGui::SameLine();
		    //needsToEvaluateString |= ImGui::Button("Eval");

		    if (!isExpressionValid)
		    	ImGui::TextColored(textColor, "Warning : wrong expression syntax");

		    if (needsToEvaluateString)
		    {
		    	application->clearContext();
		        isExpressionValid = application->eval(std::string(inputTextBuffer));
		        setKeyboardFocusOnCommandLine = true;
		    }
		}

	    ImGui::End();
	}

    // Properties panel window
    {
        bool b = getMember("showProperties")->getValueAsBoolean();
        if( b ){
    	    if (ImGui::Begin("Properties", &b))
    	    {    	

    		    ImGui::Text("Bezier curves");
    		    ImGui::SliderFloat("thickness", &bezierThickness, 0.5f, 10.0f);
    		    ImGui::SliderFloat("out roundness", &bezierCurveOutRoundness, 0.0f, 1.0f);
    		    ImGui::SliderFloat("in roundness", &bezierCurveInRoundness, 0.0f, 1.0f);
    		    ImGui::Checkbox("arrows", &displayArrows);
    	    
    	    }           
            ImGui::End();
            setMember("showProperties", b);
        }
		
	}

    // Fullscreen window
    {
	    int width, height;
		SDL_GetWindowSize(window, &width, &height);
		ImGui::SetNextWindowPos(ImVec2());
		ImGui::SetNextWindowSize(ImVec2(width, height));
		ImGui::Begin("Container", NULL, ImVec2(width,height), -1.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			application->getContext()->draw();
		}
		ImGui::End();
	}
    
    // Demo Window
    //ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
    //bool show_demo_window = false;
    //ImGui::ShowDemoWindow(&show_demo_window);

    // Rendering
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
 	ImGui::Render();
    ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}