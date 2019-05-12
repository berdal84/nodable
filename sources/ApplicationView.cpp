#include "ApplicationView.h"

// Includes for ImGui
#include <GL/gl3w.h>
#include <imgui/imgui.h>
#include <imgui/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "Application.h"
#include "Container.h"
#include "NodeView.h"
#include <fstream>
#include "Log.h"

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
    delete textEditor;
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
                                SDL_WINDOW_OPENGL |
                                SDL_WINDOW_RESIZABLE |
                               /* SDL_WINDOW_FULLSCREEN_DESKTOP*/
								SDL_WINDOW_MAXIMIZED
                                );
    
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

    io.DeltaTime          = 1.0f/120.0f;

    // Add a main font
    {
        ImFontConfig config;
        config.OversampleH    = 3;
        config.OversampleV    = 3;
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("data/FreeSerif.ttf", 18.0f, &config);    
        io.Fonts->AddFontFromFileTTF("data/CenturyGothic.ttf", 20.0f, &config);  
        io.FontAllowUserScaling = true;
    }

    // Add Icons   
    {
    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig config;
    config.OversampleH    = 4;
    config.OversampleV    = 4;
    config.MergeMode      = true;
    config.PixelSnapH     = true;
    io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_FAS, 18.0f, &config, icons_ranges );
    }

    // Configure ImGui Style
    ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text]                  = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
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
        Configure ImGuiTextColorEdit
    */

    textEditor = new TextEditor;    
    static auto lang = TextEditor::LanguageDefinition::CPlusPlus();   
    textEditor->SetLanguageDefinition(lang);
    textEditor->SetText("// Expression example :\n10 * 50 / 0.1 + 3");

    TextEditor::Palette palette = {{
        0xffffffff, // None
        0xffd69c56, // Keyword  
        0xff00ff00, // Number
        0xff7070e0, // String
        0xff70a0e0, // Char literal
        0xffffffff, // Punctuation
        0xff409090, // Preprocessor
        0xffaaaaaa, // Identifier
        0xff9bc64d, // Known identifier
        0xffc040a0, // Preproc identifier
        0xff909090, // Comment (single line)
        0xff909090, // Comment (multi line)
        0x30000000, // Background
        0xffe0e0e0, // Cursor
        0x40ffffff, // Selection
        0x800020ff, // ErrorMarker
        0x40f08000, // Breakpoint
        0x88909090, // Line number
        0x40000000, // Current line fill
        0x40808080, // Current line fill (inactive)
        0x40a0a0a0, // Current line edge
        }};

    textEditor->SetPalette(palette);


	return true;
}

bool ApplicationView::draw()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSdlGL3_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            application->stopExecution();
    }
    ImGui_ImplSdlGL3_NewFrame(window);

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
        // Maintain window size to fit with SDL window
        int width, height;
        auto renderer = SDL_GetRenderer(window);
        SDL_GetWindowSize(window, &width, &height);
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(width, height));

        ImGui::Begin("Container", NULL, ImVec2(), -1.0f, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
        {
             if( ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    ImGui::MenuItem(ICON_FA_FILE"  New", "Ctrl + N");
                    ImGui::MenuItem(ICON_FA_SAVE"  Save", "Ctrl + N");
                    ImGui::MenuItem(ICON_FA_SAVE"  Save As.", "Ctrl + N");
                    if ( ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT"  Quit", "Alt + F4"))
                        application->stopExecution();
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit"))
                {
					auto undo    = ImGui::MenuItem("Undo", "");
					auto redo    = ImGui::MenuItem("Redo", "");

					ImGui::Separator();

                    auto hide    = ImGui::MenuItem("Hide", "Del.");
                    auto arrange = ImGui::MenuItem("ReArrange nodes", "A");

                    auto selected = NodeView::GetSelected();
                    if( selected )
                    {
                        if (hide)
                            selected->setVisible(false);
                        else if (arrange)
                            selected->arrangeRecursively();
                    }

					if (undo)History::global->undo();
					if (redo)History::global->redo();

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
                    auto showProperties = ImGui::MenuItem(ICON_FA_COGS "  Show Properties", "", getMember("showProperties")->getValueAsBoolean());
                    auto showImGuiDemo  = ImGui::MenuItem("Show ImGui Demo", "", getMember("showImGuiDemo")->getValueAsBoolean());

                    ImGui::Separator();

                    if ( SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP){
                        auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", true);
                        if (toggleFullscreen)
                            SDL_SetWindowFullscreen(window, 0);
                    }else{
                        auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", false);
                        if (toggleFullscreen)
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }

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
                ImGui::EndMenuBar();
            }
    

            static bool isExpressionValid = true;

			/*
				TIME SLIDER
			*/
			auto size = History::global->getSize();
			auto cursor = History::global->getCursorPosition();

			ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
			ImGui::SetWindowFontScale(0.7f);
			auto historyChanges = ImGui::SliderInt("History", &cursor, 1, size, "Time slider");
			ImGui::PopItemWidth();
			ImGui::SetWindowFontScale(1.0f);

			if (historyChanges)
				History::global->setCursorPosition(cursor);

			/*
				TITLE
			*/

			View::ShadowedText(ImVec2(1.0f, 1.0f), ImColor(1.0f, 1.0f, 1.0f, 0.2f), ICON_FA_FILE_CODE " Hybrid Editor");

			/*
				HYBRID EDITOR			
			*/

            auto availSize = ImGui::GetContentRegionAvail();
            availSize.y -=  ImGui::GetTextLineHeightWithSpacing();;
            ImGui::BeginChild( "TextEditor", ImVec2(availSize.x , availSize.y), false);
			{
				/*
					TEXT EDITOR
				*/


				auto textEditorSize = ImGui::GetContentRegionAvail();
				textEditor->Render("Text Editor Plugin", availSize);


				bool needsToEvaluateString = textEditor->IsTextChanged() || textEditor->IsCursorPositionChanged();

				if (needsToEvaluateString)
				{
					application->clearContext();
					std::string expr = textEditor->HasSelection() ? textEditor->GetSelectedText() : textEditor->GetCurrentLineText();
					isExpressionValid = application->eval(expr);
				}

				/*
					NODE EDITOR
				*/
				ImGui::SetCursorPos(ImVec2(0, 0));
				if (application->getContext()->hasComponent("view"))
					application->getContext()->getComponent("view")->getAs<View*>()->draw();
			}
			ImGui::EndChild();

            /*
                Status bar
            */

            auto statusLineColor = ImVec4(0.0f, 0.0f, 0.0f,0.5f);
            std::string statusLineString = "Status: Everything is OK.";

            if (!isExpressionValid)
            {
                statusLineColor  = ImVec4(0.5f, 0.0f, 0.0f,1.0f);
                statusLineString = "Warning : wrong expression syntax";
            }

            ImGui::TextColored(statusLineColor, "%s", statusLineString.c_str());
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

    return false;
}

void ApplicationView::updateCurrentLineText(std::string _val)
{
    auto coord = textEditor->GetCursorPosition();

    /* If there is no selection, selects current line */
    if ( !textEditor->HasSelection() )
    {
        textEditor->MoveHome(false);
        textEditor->MoveEnd(true);
        textEditor->SetCursorPosition(TextEditor::Coordinates(coord.mLine, 0));
    }

    /* delete selection */
    textEditor->Delete();

    /* insert text */
    textEditor->InsertText(_val);

    /* select line */
    textEditor->MoveHome(true);
}