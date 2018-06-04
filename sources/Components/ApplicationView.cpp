#include "ApplicationView.h"

// Includes for ImGui
#include <GL/gl3w.h>
#include <imgui/imgui.h>
#include <imgui/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h>

#include "Application.h"
#include "Container.h"
#include "NodeView.h"
#include <fstream>
#include <Log.h>

using namespace Nodable;

ApplicationView::ApplicationView(const char* _name, Application* _application):
	application(_application)
{
    addMember("glWindowName");
    setMember("glWindowName", _name);

    // Add a member to know if we should display the properties panel or not
    addMember("showProperties");
    setMember("showProperties", false);

    addMember("showImGuiDemo");
    setMember("showImGuiDemo", false);

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
                                SDL_WINDOW_OPENGL);
    
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
    config.OversampleH    = 2;
    config.OversampleV    = 2;
    io.DeltaTime          = 1.0f/120.0f;
    io.Fonts->AddFontFromFileTTF("data/FreeSerif.ttf", 18.0f, &config);    
    io.FontAllowUserScaling = true;

    // Configure ImGui Style
    ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text]                  = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
	style.Colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
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

    style.WindowBorderSize   = 1.0f;
    style.FrameBorderSize    = 1.0f;
	style.FrameRounding      = 3.0f;
    style.ChildRounding      = 3.0f;
    style.WindowRounding     = 0.0f;
	style.AntiAliasedFill    = true;
	style.AntiAliasedLines   = true;
    style.WindowPadding      = ImVec2(10.0f,10.0f);

    /*
        Testing ImGuiTextColorEdit
    */
    textEditor = new TextEditor;    
    static auto lang = TextEditor::LanguageDefinition::CPlusPlus();   
    textEditor->SetLanguageDefinition(lang);
    textEditor->SetText("// Expression example :\n10 * 50 / 0.1 + 3");
    textEditor->SetPalette(TextEditor::GetLightPalette());


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

            auto selected = NodeView::GetSelected();
            if( selected )
	        {
	            if (hide)
	            	selected->setVisible(false);
				else if (arrange)
					selected->arrangeRecursively();
        	}

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu("View"))
        {
            //auto frame = ImGui::MenuItem("Frame All", "F");
            //ImGui::Separator();
            auto detailSimple   = ImGui::MenuItem("Simple View", "", NodeView::s_drawDetail == DrawDetail_Simple );
            auto detailAdvanced = ImGui::MenuItem("Advanced View", "", NodeView::s_drawDetail == DrawDetail_Advanced );
            auto detailComplex  = ImGui::MenuItem("Complex View", "", NodeView::s_drawDetail == DrawDetail_Complex );
            
            ImGui::Separator();
            auto showProperties = ImGui::MenuItem("Show Properties", "", getMember("showProperties")->getValueAsBoolean());
            auto showImGuiDemo  = ImGui::MenuItem("Show ImGui Demo", "", getMember("showImGuiDemo")->getValueAsBoolean());

            //if( frame)
                // TODO

            if (detailSimple)
                NodeView::s_drawDetail = DrawDetail_Simple;

            if (detailAdvanced)
                NodeView::s_drawDetail = DrawDetail_Advanced;

            if (detailComplex)
                NodeView::s_drawDetail = DrawDetail_Complex;

            if(showProperties)
                 setMember("showProperties", !getMember("showProperties")->getValueAsBoolean());

            if(showImGuiDemo)
                 setMember("showImGuiDemo", !getMember("showImGuiDemo")->getValueAsBoolean());

        	ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
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

    // Demo Window
    {
        bool b = getMember("showImGuiDemo")->getValueAsBoolean();
        if (b){
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&b);
            setMember("showImGuiDemo", b);
        }
    }
    // Fullscreen window
    {
        float menuBarHeight = 20.0f;
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        ImGui::SetNextWindowPos(ImVec2(0.0f, menuBarHeight));
        ImGui::SetNextWindowSize(ImVec2(width, height - menuBarHeight));
        ImGui::Begin("Container", NULL, ImVec2(width,height-menuBarHeight), -1.0f, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
        {
            auto availSize = ImGui::GetContentRegionAvail();

            ImGui::BeginChild("TextEditor", ImVec2(availSize.x * 0.35, availSize.y));
            {
                ImGui::Text("Text Editor");
                auto textEditorSize = ImGui::GetContentRegionAvail();
                textEditorSize.y *= 0.5f;
                textEditor->Render("Text Editor Plugin", textEditorSize);
                
                static bool isExpressionValid = true;
                bool needsToEvaluateString = textEditor->IsTextChanged() || textEditor->IsCursorPositionChanged();

                // Draw the input text field :
                /*
                ImGui::Text("Type an expression, the program will create the graph in realtime :");
                static char inputTextBuffer[1024 * 200];

                auto textColor = isExpressionValid ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.9f, 0.0f, 0.0f,1.0f);
                ImGui::PushStyleColor(ImGuiCol_Text,textColor );
                static bool setKeyboardFocusOnCommandLine = true;
                if ( setKeyboardFocusOnCommandLine){
                   ImGui::SetKeyboardFocusHere();
                   setKeyboardFocusOnCommandLine = false;
                }
                ImVec2 inputTextSize(ImGui::GetContentRegionAvailWidth(), 0);
                bool needsToEvaluateString = ImGui::InputTextMultiline("", inputTextBuffer, NODABLE_ARRAYSIZE(inputTextBuffer), inputTextSize);
                ImGui::PopStyleColor();
                */

                //ImGui::SameLine();
                //needsToEvaluateString |= ImGui::Button("Eval");

                if (!isExpressionValid)
                    ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f,1.0f), "Warning : wrong expression syntax");

                if (needsToEvaluateString)
                {
                    application->clearContext();
                    std::string expr = textEditor->HasSelection() ? textEditor->GetSelectedText() : textEditor->GetCurrentLineText( );
                    isExpressionValid = application->eval(expr);
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::BeginChild("NodeEditor", ImVec2(0.0f,0.0f), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                auto itemRectMin      = ImVec2(0.0f,    0.0f);
                auto itemRectMax      = ImVec2(4096.0f, 4096.0f);
                draw_list->AddRectFilled(itemRectMin, itemRectMax, ImColor(0.2f, 0.2f, 0.2f));  
                
                ImGui::Text("Node Editor");

                application->getContext()->draw();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
    
    // Rendering
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
 	ImGui::Render();
    ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}