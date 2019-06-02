#include "ApplicationView.h"

// Includes for ImGui
#include <GL/gl3w.h>
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_sdl.h>
#include <imgui/examples/imgui_impl_opengl3.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "Application.h"
#include "Container.h"
#include "NodeView.h"
#include <fstream>
#include "Log.h"
#include <algorithm>

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
    ImGui_ImplSDL2_Shutdown();
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
    
    this->glcontext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io             = ImGui::GetIO();
	//io.WantCaptureKeyboard  = true;
	//io.WantCaptureMouse     = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	gl3wInit();
    ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

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
	style.Colors[ImGuiCol_Button]                = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.70f, 0.70f, 0.70f, 0.95f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.98f, 0.73f, 0.29f, 0.95f);
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
	
	// read startup file
	auto filename = "data/startup.txt";
	std::ifstream startupFile(filename);
	std::string expression((std::istreambuf_iterator<char>(startupFile)), std::istreambuf_iterator<char>());

    textEditor->SetText(expression);

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
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            application->stopExecution();
    }

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

    // Properties panel window
    {
        bool b = getMember("showProperties")->getValueAsBoolean();
        if( b ){
    	    if (ImGui::Begin("Properties", &b))
    	    {    	

    		    ImGui::Text("Wires");
    		    ImGui::SliderFloat("thickness", &bezierThickness, 0.5f, 10.0f);
    		    ImGui::SliderFloat("out roundness", &bezierCurveOutRoundness, 0.0f, 1.0f);
    		    ImGui::SliderFloat("in roundness", &bezierCurveInRoundness, 0.0f, 1.0f);
				ImGui::SliderFloat("connector radius", &connectorRadius, 1.0f, 10.0f);
				ImGui::SliderFloat("node padding", &nodePadding, 1.0f, 20.0f);
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

		// Declare variables that can be modified my mouse and keyboard
		auto userWantsToUndo(false);
		auto userWantsToRedo(false);
		auto userWantsToHideSelectedNode(false);
		auto userWantsToArrangeSelectedNodeHierarchy(false);

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
					userWantsToUndo    |= ImGui::MenuItem("Undo", "");
					userWantsToRedo    |= ImGui::MenuItem("Redo", "");
					if (userWantsToUndo)History::global->undo();
					if (userWantsToRedo)History::global->redo();

					ImGui::Separator();

					auto isAtLeastANodeSelected = NodeView::GetSelected() != nullptr;
                    userWantsToHideSelectedNode             |= ImGui::MenuItem("Hide",            "Del.", false, isAtLeastANodeSelected);
                    userWantsToArrangeSelectedNodeHierarchy |= ImGui::MenuItem("ReArrange nodes", "A",    false, isAtLeastANodeSelected);

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
				UNDO HISTORY / TIME SLIDER
			*/
			auto historyButtonSpacing         = float(2);
			auto historyButtonHeight          = float(12);
			auto historyButtonMinWidth        = float(60);

			auto historySize                  = History::global->getSize();
			auto historyCurrentCursorPosition = History::global->getCursorPosition();
			auto availableWidth               = ImGui::GetContentRegionAvailWidth();	
			auto historyButtonWidth           = std::fmin(historyButtonMinWidth, availableWidth / float(historySize) - historyButtonSpacing);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(historyButtonSpacing, 0) );
			
			for (size_t commandId = 1; commandId <= historySize; commandId++)
			{
				// Draw an highlighted button for the current history position
				if (commandId == historyCurrentCursorPosition){
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
					ImGui::Button("", ImVec2(historyButtonWidth, historyButtonHeight));
					ImGui::PopStyleColor();

				// or a simple one for other history positions
				}else
					ImGui::Button("", ImVec2(historyButtonWidth, historyButtonHeight));

				if (ImGui::IsItemHoveredRect())
				{
					if(ImGui::IsMouseDown(0)) // hovered + mouse down
						History::global->setCursorPosition(commandId); // update history cursor position
					
					// Draw command description 
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, float(0.8));
						ImGui::BeginTooltip();					
						ImGui::Text(History::global->getCommandDescriptionAtPosition(commandId - 1));
						ImGui::EndTooltip();
					ImGui::PopStyleVar();
				}

				ImGui::SameLine();
			}
			ImGui::PopStyleVar();
			ImGui::NewLine();

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

				auto textEditorSize         = ImGui::GetContentRegionAvail();

				auto previousCursorPosition = textEditor->GetCursorPosition();
				auto previousSelectedText   = textEditor->GetSelectedText();
				auto previousLineText       = textEditor->GetCurrentLineText();

				auto allowkeyboard          = NodeView::GetSelected() == nullptr; // disable keyboard for text editor when a node is selected.
				auto allowMouse             = true;

				textEditor->ScanUserInputs (allowkeyboard, allowMouse);
				textEditor->Render         ("Text Editor Plugin", availSize);

				auto currentCursorPosition  = textEditor->GetCursorPosition();
				auto currentSelectedText    = textEditor->GetSelectedText();
				auto currentLineText        = textEditor->GetCurrentLineText();

				auto isCurrentLineModified  = currentLineText != previousLineText;
				auto isSelectedTextModified = previousSelectedText != currentSelectedText;

				bool needsToEvaluateString = isCurrentLineModified ||
					                        textEditor->IsTextChanged() ||
					                        isSelectedTextModified;

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

		/*
		   Keyboard shortcuts
		*/

		const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

		auto isDown = [&keyboardState](SDL_Keycode _code) -> bool { return keyboardState[SDL_GetScancodeFromKey(_code)]; };

		// A
		if (isDown(SDLK_a))
			userWantsToArrangeSelectedNodeHierarchy |= true;

		// Del.
		else if (isDown(SDLK_DELETE))
			userWantsToHideSelectedNode |= true;


		/*
		   Perform actions on selected node
		*/

		auto selected = NodeView::GetSelected();
		if (selected)
		{
			if (userWantsToHideSelectedNode)
				selected->setVisible(false);
			else if (userWantsToArrangeSelectedNodeHierarchy)
				selected->arrangeRecursively();
		}
    }
    

	// Rendering
	ImGui::Render();
	SDL_GL_MakeCurrent(window, this->glcontext);
	auto io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);

    return false;
}

void ApplicationView::updateCurrentLineText(std::string _val)
{
    auto coord = textEditor->GetCursorPosition();

    /* If there is no selection, selects current line */
	auto hasSelection = textEditor->HasSelection();

    if ( !hasSelection )
    {
        textEditor->MoveHome(false);
        textEditor->MoveEnd(true);
        textEditor->SetCursorPosition(TextEditor::Coordinates(coord.mLine, 0));
    }

    /* delete selection */
    textEditor->Delete();
	coord = textEditor->GetCursorPosition();

    /* insert text */
    textEditor->InsertText(_val);

	/* Select the new inserted text if needed*/
	if (hasSelection)
	{
		textEditor->SetSelectionStart(coord);
		textEditor->SetSelectionEnd(TextEditor::Coordinates(coord.mLine, coord.mColumn + _val.size()));
	}
}