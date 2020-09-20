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
#include "File.h"
#include "Log.h"
#include "FileView.h"

using namespace Nodable;

ApplicationView::ApplicationView(const char* _name, Application* _application):
	application(_application)
{
    add("glWindowName");
    set("glWindowName", _name);

    // Add a member to know if we should display the properties panel or not
    add("showProperties");
    set("showProperties", false);

    add("showImGuiDemo");
    set("showImGuiDemo", false);

    // Add two members for the window size
    add("glWindowSizeX");
    set("glWindowSizeX", 1280.0f);

    add("glWindowSizeY");
    set("glWindowSizeY", 720.0f);
}

ApplicationView::~ApplicationView()
{
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext    ();
    SDL_GL_DeleteContext     (glcontext);
    SDL_DestroyWindow        (sdlWindow);
    SDL_Quit                 ();
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
    sdlWindow = SDL_CreateWindow( ((std::string)*get("glWindowName")).c_str(),
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                (int)*get("glWindowSizeX"),
                                (int)*get("glWindowSizeY"),
                                SDL_WINDOW_OPENGL |
                                SDL_WINDOW_RESIZABLE |
                               /* SDL_WINDOW_FULLSCREEN_DESKTOP*/
								SDL_WINDOW_MAXIMIZED
                                );
    
    this->glcontext = SDL_GL_CreateContext(sdlWindow);
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
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, glcontext);
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
		config.OversampleH      = 4;
		config.OversampleV      = 4;
		config.MergeMode        = true;
		config.PixelSnapH       = true;
		config.GlyphMinAdvanceX = 18.0f; // monospace to fix text alignment in drop down menus.
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
	style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
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
	style.Colors[ImGuiCol_Tab]                   = ImVec4(0.44f, 0.44f, 0.44f, 0.86f);
	style.Colors[ImGuiCol_TabHovered]            = ImVec4(1.00f, 1.00f, 1.00f, 0.80f);
	style.Colors[ImGuiCol_TabActive]             = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.15f, 0.15f, 0.15f, 0.97f);
	style.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
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

	return true;
}

bool ApplicationView::draw()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type)
		{
		case SDL_QUIT:
			application->stopExecution();
			break;

		case SDL_KEYUP:
			auto key = event.key.keysym.sym;

			if ((event.key.keysym.mod & KMOD_LCTRL)) {

				// History
				if (auto file = application->getCurrentFile()) {
					History* currentFileHistory = file->getHistory();
					     if (key == SDLK_z) currentFileHistory->undo();
					else if (key == SDLK_y) currentFileHistory->redo();
				}

				// File
				     if( key == SDLK_s)  application->saveCurrentFile();
				else if( key == SDLK_w)  application->closeCurrentFile();
				else if( key == SDLK_o)  this->browseFile();
			}
			break;
		}

    }

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(sdlWindow);
	ImGui::NewFrame();

	// Reset default mouse cursor
	ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

    // Properties panel sdlWindow
    {
        bool b = *get("showProperties");
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
            set("showProperties", b);
        }
    }

    // Demo Window
    {
        bool b = *get("showImGuiDemo");
        if (b){
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&b);
            set("showImGuiDemo", b);
        }
    }
    // Fullscreen sdlWindow
    {
        // Maintain sdlWindow size to fit with SDL sdlWindow
        int width, height;
        auto renderer = SDL_GetRenderer(sdlWindow);
        SDL_GetWindowSize(sdlWindow, &width, &height);
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2( float(width), float(height) ));

		// Declare variables that can be modified my mouse and keyboard
		auto userWantsToUndo(false);
		auto userWantsToRedo(false);
		auto userWantsToHideSelectedNode(false);
		auto userWantsToArrangeSelectedNodeHierarchy(false);

		
		// Get current file's history
		History* currentFileHistory = nullptr;
		
		if ( auto file = application->getCurrentFile())
			currentFileHistory = file->getHistory();
		

        ImGui::Begin("Container", NULL, ImVec2(), -1.0f, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			if (ImGui::BeginMenuBar()) {

				if (ImGui::BeginMenu("File")) {

					//ImGui::MenuItem(ICON_FA_FILE   "  New", "Ctrl + N");
					if (ImGui::MenuItem(ICON_FA_FOLDER      "  Open", "Ctrl + O")) this->browseFile();
					if (ImGui::MenuItem(ICON_FA_SAVE        "  Save", "Ctrl + S")) application->saveCurrentFile();
					if (ImGui::MenuItem(ICON_FA_TIMES       "  Close","Ctrl + W")) application->closeCurrentFile();
					if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT"  Quit", "Alt + F4")) application->stopExecution();

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Edit")) {

					if (currentFileHistory) {
						if (ImGui::MenuItem("Undo", "Ctrl + Z")) currentFileHistory->undo();
						if (ImGui::MenuItem("Redo", "Ctrl + Y")) currentFileHistory->redo();
						ImGui::Separator();
					}

					auto isAtLeastANodeSelected = NodeView::GetSelected() != nullptr;
					userWantsToHideSelectedNode |= ImGui::MenuItem("Hide", "Del.", false, isAtLeastANodeSelected);
					userWantsToArrangeSelectedNodeHierarchy |= ImGui::MenuItem("ReArrange nodes", "A", false, isAtLeastANodeSelected);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					//auto frame = ImGui::MenuItem("Frame All", "F");
					//ImGui::Separator();
					auto detailSimple = ImGui::MenuItem("Simple View", "", NodeView::s_drawDetail == DrawDetail_Simple);
					auto detailAdvanced = ImGui::MenuItem("Advanced View", "", NodeView::s_drawDetail == DrawDetail_Advanced);
					auto detailComplex = ImGui::MenuItem("Complex View", "", NodeView::s_drawDetail == DrawDetail_Complex);

					ImGui::Separator();
					auto showProperties = ImGui::MenuItem(ICON_FA_COGS "  Show Properties", "", (bool)*get("showProperties"));
					auto showImGuiDemo = ImGui::MenuItem("Show ImGui Demo", "", (bool)*get("showImGuiDemo"));

					ImGui::Separator();

					if (SDL_GetWindowFlags(sdlWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
						auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", true);
						if (toggleFullscreen)
							SDL_SetWindowFullscreen(sdlWindow, 0);
					}
					else {
						auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", false);
						if (toggleFullscreen)
							SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
					}

					//if( frame)
						// TODO

					if (detailSimple)
						NodeView::s_drawDetail = DrawDetail_Simple;

					if (detailAdvanced)
						NodeView::s_drawDetail = DrawDetail_Advanced;

					if (detailComplex)
						NodeView::s_drawDetail = DrawDetail_Complex;

					if (showProperties)
						set("showProperties", !(bool)*get("showProperties"));

					if (showImGuiDemo)
						set("showImGuiDemo", !(bool)*get("showImGuiDemo"));



					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			static bool isExpressionValid = true;

			/*
				UNDO HISTORY / TIME SLIDER
			*/

			if (currentFileHistory) {

				ImGui::Text(ICON_FA_CLOCK " History: ");

				auto historyButtonSpacing = float(1);
				auto historyButtonHeight = float(20);
				auto historyButtonMaxWidth = float(10);

				auto historySize = currentFileHistory->getSize();
				auto historyCurrentCursorPosition = currentFileHistory->getCursorPosition();
				auto availableWidth = ImGui::GetContentRegionAvailWidth();
				auto historyButtonWidth = std::fmin(historyButtonMaxWidth, availableWidth / float(historySize) - historyButtonSpacing);
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(historyButtonSpacing, 0));
			
				

				for (size_t commandId = 0; commandId < historySize; commandId++)
				{
					ImGui::SameLine();

					// Draw an highlighted button for the current history position
					if (commandId == historyCurrentCursorPosition) {
						ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
						ImGui::Button("", ImVec2(historyButtonWidth, historyButtonHeight));
						ImGui::PopStyleColor();

						// or a simple one for other history positions
					}
					else
						ImGui::Button("", ImVec2(historyButtonWidth, historyButtonHeight));

					if (ImGui::IsItemHovered())
					{
						if (ImGui::IsMouseDown(0)) // hovered + mouse down
							currentFileHistory->setCursorPosition(commandId); // update history cursor position
							
						// Draw command description 
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, float(0.8));
						ImGui::BeginTooltip();
						ImGui::Text(currentFileHistory->getCommandDescriptionAtPosition(commandId));
						ImGui::EndTooltip();
						ImGui::PopStyleVar();
					}

					
				}
				ImGui::PopStyleVar();
			}

			/*
				HYBRID EDITOR
			*/
			if (application->getFileCount() > 0) {

				drawFileTabs();

				auto availSize = ImGui::GetContentRegionAvail();
				availSize.y -= ImGui::GetTextLineHeightWithSpacing();;

				auto currentFile     = application->getCurrentFile();
				auto currentFileView = currentFile->getComponent<View>();
				currentFileView->drawAsChild("FileView", availSize);
			}

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
    
	// Modals
	fileBrowser.Display();
	if (fileBrowser.HasSelected())
	{
		auto selectedFiles = fileBrowser.GetMultiSelected();
		for (auto it = selectedFiles.cbegin(); it != selectedFiles.cend(); it++ )
		{
			application->openFile(it->string().c_str());
		}
		fileBrowser.ClearSelected();
		fileBrowser.Close();
	}

	// Rendering
	ImGui::Render();
	SDL_GL_MakeCurrent(sdlWindow, this->glcontext);
	auto io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(sdlWindow);

    return false;
}

void Nodable::ApplicationView::drawFileTabs()
{
	bool userSwitchesFile = false;
	{

		float tabsVerticalOffset = ImGui::GetStyle().FramePadding.y;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + tabsVerticalOffset);

		ImGui::BeginTabBar("FileTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs);

		for (size_t i = 0; i < application->getFileCount(); i++)
		{
			auto file = application->getFileAtIndex(i);
			std::string tabLabel = file->getName();
			if (file->isModified())
				tabLabel.append("*");
			tabLabel.append("##");
			tabLabel.append(std::to_string(i));

			if (ImGui::BeginTabItem(tabLabel.c_str()))
				ImGui::EndTabItem();

			if (ImGui::IsItemClicked(0))
			{
				application->setCurrentFileWithIndex(i);
				userSwitchesFile = true;
			}
		}

		ImGui::EndTabBar();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - tabsVerticalOffset);

	}
}

void Nodable::ApplicationView::browseFile()
{
	fileBrowser.Open();
	//application->openFile(fileAbsolutePath.c_str());
}

