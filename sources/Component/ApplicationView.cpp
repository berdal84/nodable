#include "ApplicationView.h"

#include "Config.h"
#include "Core/Texture.h"
#include "Core/Settings.h"

#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include "System.h"
#include "Application.h"
#include "NodeView.h"
#include "File.h"
#include "Log.h"
#include "Component/FileView.h"
#include "IconsFontAwesome5.h"

using namespace Nodable;

ApplicationView::ApplicationView(const char* _name, Application* _application):
        application(_application),
        backgroundColor(50, 50, 50),
        isStartupWindowVisible(true),
        isHistoryDragged(false),
        m_glWindowName(_name),
        m_showProperties(false),
        m_showImGuiDemo(false)
{
}

ApplicationView::~ApplicationView()
{
    Texture::ReleaseResources();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext    ();
    SDL_GL_DeleteContext     (glcontext);
    SDL_DestroyWindow        (sdlWindow);
    SDL_Quit                 ();
}

bool ApplicationView::init()
{
    Settings* settings = Settings::GetCurrent();

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        LOG_ERROR( "ApplicationView", "SDL Error: %s\n", SDL_GetError());
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
    sdlWindow = SDL_CreateWindow( m_glWindowName.c_str(),
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                800,
                                600,
                                SDL_WINDOW_OPENGL |
                                SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_MAXIMIZED |
                                SDL_WINDOW_SHOWN
                                );

    this->glcontext = SDL_GL_CreateContext(sdlWindow);
    SDL_GL_SetSwapInterval(1); // Enable vsync


    // Setup Dear ImGui binding
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

	// Setup Dear ImGui style
    settings->setImGuiStyle(ImGui::GetStyle());

    /** Add a paragraph font */
    {
        {
            ImFontConfig config;
            config.OversampleH = 3;
            config.OversampleV = 1;

            //io.Fonts->AddFontDefault();
            auto fontPath = application->getAssetPath(settings->ui.text.p.font).string();
            LOG_MESSAGE("ApplicationView", "Adding font from file: %s\n", fontPath.c_str());
            this->paragraphFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), settings->ui.text.p.size, &config);
        }

        // Add Icons my merging to previous (paragraphFont) font.
        {
            // merge in icons from Font Awesome
            static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
            ImFontConfig config;
            config.OversampleH = 3;
            config.OversampleV = 1;
            config.MergeMode = true;
            config.PixelSnapH = true;
            config.GlyphMinAdvanceX = settings->ui.text.p.size; // monospace to fix text alignment in drop down menus.
            auto fontPath = application->getAssetPath("fa-solid-900.ttf").string();
            LOG_MESSAGE("ApplicationView", "Adding font from file: %s\n", fontPath.c_str());
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), settings->ui.text.p.size, &config, icons_ranges);
        }
    }

    /** Add a heading font */
    {
        ImFontConfig config;
        config.OversampleH    = 3;
        config.OversampleV    = 1;

        //io.Fonts->AddFontDefault();
        auto fontPath = application->getAssetPath(settings->ui.text.h1.font).string();
        LOG_MESSAGE( "ApplicationView", "Adding font from file: %s\n", fontPath.c_str());
        this->headingFont = io.Fonts->AddFontFromFileTTF( fontPath.c_str(), settings->ui.text.h1.size, &config);
    }

    // Configure ImGui Style
    ImGuiStyle& style = ImGui::GetStyle();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    gl3wInit();
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, glcontext);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

	return true;
}

bool ApplicationView::draw()
{
    // TODO: create an event list (fill, execute, clear)
    auto userWantsToDeleteSelectedNode(false);
    auto userWantsToArrangeSelectedNodeHierarchy(false);
    auto userWantsToSelectedNextNode(false);

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
			else if (key == SDLK_DELETE )
            {
                userWantsToDeleteSelectedNode = true;
            }
			else if (key == SDLK_a)
            {
                userWantsToArrangeSelectedNodeHierarchy = true;
            }
            else if (key == SDLK_n)
            {
                userWantsToSelectedNextNode = true;
            }
			else if (key == SDLK_F1 )
            {
                isStartupWindowVisible = true;
            }

			break;
		}

    }

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(sdlWindow);
	ImGui::NewFrame();
    ImGui::SetCurrentFont(this->paragraphFont);

	// Reset default mouse cursor
	ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

    // Startup Window
    drawStartupWindow();

    // Demo Window
    {
        if (m_showImGuiDemo){
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&m_showImGuiDemo);
        }
    }

    // Fullscreen sdlWindow
    {

		// Get current file's history
		History* currentFileHistory = nullptr;

		if ( auto file = application->getCurrentFile())
			currentFileHistory = file->getHistory();

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


        // Remove padding
        bool isMainWindowOpen = true;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Nodable", &isMainWindowOpen, window_flags);
        {
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(2);

            bool redock_all = false;

            drawMenuBar(currentFileHistory, userWantsToDeleteSelectedNode,
                        userWantsToArrangeSelectedNodeHierarchy, redock_all);
            drawToolBar();


            /*
             * Main Layout
             */

            ImGuiID dockspace_id = ImGui::GetID("dockspace_main");
            ImGuiID dockspace_documents = ImGui::GetID("dockspace_documents");
            ImGuiID dockspace_properties = ImGui::GetID("dockspace_properties");


            if ( !this->isLayoutInitialized )
            {
               ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
               ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace );
               ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
               ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, Settings::GetCurrent()->ui.layout.propertiesRatio, &dockspace_properties, NULL);
               ImGui::DockBuilderDockWindow("Global Props", dockspace_properties);
               ImGui::DockBuilderDockWindow("Properties", dockspace_properties);
               ImGui::DockBuilderDockWindow("File Info", dockspace_properties);
               ImGui::DockBuilderFinish(dockspace_id);
                this->isLayoutInitialized = true;
            }

             /*
             * Fill the layout with content
             */
            ImGui::DockSpace(dockspace_id);

            // Global Props
            if (ImGui::Begin("Global Props"))
            {
                this->drawPropertiesWindow();
                ImGui::NewLine();

                ImGui::ShowStyleEditor();
            }
            ImGui::End();


            // File info
            ImGui::Begin("File Info");
            {
                const File* currFile = application->getCurrentFile();

                if (currFile)
                {
                    FileView* fileView = currFile->getComponent<FileView>();
                    fileView->drawFileInfo();
                }
                else
                {
                    ImGui::Text("No open file");
                }

            }
            ImGui::End();

            // Selected Node Properties
            if ( ImGui::Begin("Properties") )
            {
                NodeView* view = NodeView::GetSelected();
                if ( view )
                {
                    ImGui::Text("Selected Node Properties");
                    ImGui::NewLine();
                    ImGui::Indent(10.0f);
                    ImGui::Text("Type: %s", view->getOwner()->getLabel());
                    ImGui::NewLine();
                    NodeView::DrawNodeViewAsPropertiesPanel(view);
                }
            }
            ImGui::End();


            // Opened documents
            for (size_t fileIndex = 0; fileIndex < application->getFileCount(); fileIndex++)
            {
                this->drawFileEditor(dockspace_id, redock_all, fileIndex);
            }


        }
        ImGui::End(); // Main window


		/*
		   Perform actions on selected node
		*/

		auto selectedNodeView = NodeView::GetSelected();
		if (selectedNodeView)
		{
			if (userWantsToDeleteSelectedNode)
			{
			    auto node = selectedNodeView->getOwner();
                node->flagForDeletion();
            }
			else if (userWantsToArrangeSelectedNodeHierarchy)
            {
				selectedNodeView->arrangeRecursively();
            }
			else if (userWantsToSelectedNextNode)
            {
			    GraphTraversal traversal;
			    Node* next = traversal.getNextInstrToEval(selectedNodeView->getOwner());
			    if ( next )
			        if( auto nextView = next->getComponent<NodeView>())
                        NodeView::SetSelected(nextView);
            }
		}
    }

    drawFileBrowser();

    // Rendering
	ImGui::Render();
	SDL_GL_MakeCurrent(sdlWindow, this->glcontext);
	auto io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(backgroundColor.Value.x, backgroundColor.Value.y, backgroundColor.Value.z, backgroundColor.Value.w);
	glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(sdlWindow);

    // limit frame rate
    constexpr float desiredFrameRate = 1.0f/60.0f;
    if ( ImGui::GetIO().DeltaTime  < desiredFrameRate)
        SDL_Delay((desiredFrameRate - ImGui::GetIO().DeltaTime) * 1000u );

    return false;
}

void ApplicationView::drawFileBrowser()
{
    fileBrowser.Display();
    if (fileBrowser.HasSelected())
    {
        auto selectedFiles = fileBrowser.GetMultiSelected();
        for (const auto & selectedFile : selectedFiles)
        {
            application->openFile(selectedFile.string().c_str());
        }
        fileBrowser.ClearSelected();
        fileBrowser.Close();
    }
}

void ApplicationView::drawFileEditor(ImGuiID dockspace_id, bool redock_all, size_t fileIndex)
{
    File *file = application->getFileAtIndex(fileIndex);

    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags = (file->isModified() ? ImGuiWindowFlags_UnsavedDocument : 0);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
    child_bg.w = 0;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

    bool open = true;
    bool visible = ImGui::Begin(file->getName().c_str(), &open, window_flags);
    {
        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar();

        if (visible)
        {
            const bool isCurrentFile = fileIndex == application->getCurrentFileIndex();

            if ( !isCurrentFile && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                application->setCurrentFileWithIndex(fileIndex);
            }

            // History bar on top
            drawHistoryBar(file->getHistory());

            // File View in the middle
            View* eachFileView = file->getComponent<View>();
            ImVec2 availSize = ImGui::GetContentRegionAvail();

            if ( isCurrentFile )
            {
                availSize.y -= ImGui::GetTextLineHeightWithSpacing();
            }

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0.35f) );
            eachFileView->drawAsChild("FileView", availSize);
            ImGui::PopStyleColor();

            // Status bar
            if ( isCurrentFile )
            {
                drawStatusBar();
            }

        }
    }
    ImGui::End(); // File Window

    if (!open)
    {
        application->closeFile(fileIndex);
    }
}

void ApplicationView::drawPropertiesWindow()
{
    Settings* config = Settings::GetCurrent();

    ImGui::Text("Nodable Settings:");
    ImGui::Indent();
        ImGui::Text("Wires:");
        ImGui::Indent();
            ImGui::SliderFloat("thickness", &config->ui.wire.bezier.thickness, 0.5f, 10.0f);
            ImGui::SliderFloat("roundness", &config->ui.wire.bezier.roundness, 0.0f, 1.0f);
            ImGui::Checkbox("arrows", &config->ui.wire.displayArrows);
        ImGui::Unindent();

        ImGui::Text("Nodes:");
        ImGui::Indent();
            ImGui::SliderFloat("connector radius", &config->ui.node.connectorRadius, 1.0f, 10.0f);
            ImGui::SliderFloat("padding", &config->ui.node.padding, 1.0f, 20.0f);
            ImGui::SliderFloat("speed", &config->ui.node.speed, 0.0f, 100.0f);
            ImGui::SliderFloat("spacing", &config->ui.node.spacing, 0.0f, 100.0f);

            ImGui::ColorEdit3("variables color", &config->ui.node.variableColor.x);
            ImGui::ColorEdit3("instruction color", &config->ui.node.instructionColor.x);
            ImGui::ColorEdit3("function color", &config->ui.node.functionColor.x);
            ImGui::ColorEdit3("shadow color", &config->ui.node.shadowColor.x);
            ImGui::ColorEdit3("border color", &config->ui.node.borderColor.x);
            ImGui::ColorEdit3("high. color", &config->ui.node.highlightedColor.x);
            ImGui::ColorEdit3("border high. color", &config->ui.node.borderHighlightedColor.x);
            ImGui::ColorEdit3("fill color", &config->ui.node.fillColor.x);

        ImGui::Unindent();

        // code flow
        ImGui::Text("Code flow:");
        ImGui::Indent();
            ImGui::SliderFloat("line width min", &config->ui.codeFlow.lineWidthMax, 1.0f, 100.0f);
        ImGui::Unindent();

    ImGui::Unindent();
}

void ApplicationView::drawStartupWindow() {
    if (isStartupWindowVisible && !ImGui::IsPopupOpen(startupScreenTitle))
    {
        ImGui::OpenPopup(startupScreenTitle);
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(500,200), ImVec2(500,50000));
    ImGui::SetNextWindowPos( ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f,0.5f) );

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if ( ImGui::BeginPopupModal(startupScreenTitle, nullptr, flags) )
    {

        std::filesystem::path path(NODABLE_ASSETS_DIR"/nodable-logo-xs.png");
        auto logo = Texture::GetWithPath(path);
        ImGui::SameLine( (ImGui::GetContentRegionAvailWidth() - logo->width) * 0.5f); // center img
        ImGui::Image((void*)(intptr_t)logo->image, ImVec2((float)logo->width, (float)logo->height));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(25.0f, 20.0f) );
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        {
            ImGui::PushFont(headingFont);
            ImGui::NewLine();
            ImGui::Text("Nodable is node-able");
            ImGui::PopFont();
            ImGui::NewLine();
        }


        ImGui::TextWrapped("The goal of Nodable is to allow you to edit a computer program in a textual and nodal way at the same time." );

        {
            ImGui::PushFont(headingFont);
            ImGui::NewLine();
            ImGui::Text("Manifest");
            ImGui::PopFont();
            ImGui::NewLine();
        }

        ImGui::TextWrapped( "The nodal and textual points of view each have pros and cons. The user should not be forced to choose one of the two." );

        {
            ImGui::PushFont(headingFont);
            ImGui::NewLine();
            ImGui::Text("Disclaimer");
            ImGui::PopFont();
            ImGui::NewLine();
        }

        ImGui::TextWrapped( "This software is a prototype, use at your own risk." );

        ImGui::NewLine();ImGui::NewLine();

        const char* credit = "berenger@dalle-cort.fr";
        ImGui::SameLine( ImGui::GetContentRegionAvailWidth() - ImGui::CalcTextSize(credit).x);
        ImGui::TextWrapped( credit );

        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
        {
            ImGui::CloseCurrentPopup();
            isStartupWindowVisible = false;
        }
        ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
        ImGui::EndPopup();
    }

}

void ApplicationView::drawMenuBar(
        History *currentFileHistory,
        bool &userWantsToDeleteSelectedNode,
        bool &userWantsToArrangeSelectedNodeHierarchy,
        bool &redock_all)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            //ImGui::MenuItem(ICON_FA_FILE   "  New", "Ctrl + N");
            if (ImGui::MenuItem(ICON_FA_FOLDER      "  Open", "Ctrl + O")) browseFile();
            if (ImGui::MenuItem(ICON_FA_SAVE        "  Save", "Ctrl + S")) application->saveCurrentFile();
            if (ImGui::MenuItem(ICON_FA_TIMES       "  Close", "Ctrl + W")) application->closeCurrentFile();
            if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT"  Quit", "Alt + F4")) application->stopExecution();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (currentFileHistory)
            {
                if (ImGui::MenuItem("Undo", "Ctrl + Z")) currentFileHistory->undo();
                if (ImGui::MenuItem("Redo", "Ctrl + Y")) currentFileHistory->redo();
                ImGui::Separator();
            }

            auto isAtLeastANodeSelected = NodeView::GetSelected() != nullptr;
            userWantsToDeleteSelectedNode |= ImGui::MenuItem("Delete", "Del.", false, isAtLeastANodeSelected);
            userWantsToArrangeSelectedNodeHierarchy |= ImGui::MenuItem("ReArrange nodes", "A", false, isAtLeastANodeSelected);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            //auto frame = ImGui::MenuItem("Frame All", "F");
            redock_all |= ImGui::MenuItem("Redock documents");

            ImGui::Separator();
            auto viewDetail_Minimalist  = ImGui::MenuItem("Minimalist View", "", NodeView::s_viewDetail == NodeViewDetail::Minimalist);
            auto viewDetail_Essential   = ImGui::MenuItem("Essential View",  "", NodeView::s_viewDetail == NodeViewDetail::Essential);
            auto viewDetail_Exhaustive  = ImGui::MenuItem("Exhaustive View", "", NodeView::s_viewDetail == NodeViewDetail::Exhaustive);

            if (viewDetail_Minimalist)
            {
                NodeView::SetDetail(NodeViewDetail::Minimalist);
            }
            else if (viewDetail_Essential)
            {
                NodeView::SetDetail(NodeViewDetail::Essential);
            }
            else if (viewDetail_Exhaustive)
            {
                NodeView::SetDetail(NodeViewDetail::Exhaustive);
            }

            ImGui::Separator();
            m_showProperties = ImGui::MenuItem(ICON_FA_COGS "  Show Properties", "", m_showProperties);
            m_showImGuiDemo = ImGui::MenuItem("Show ImGui Demo", "", m_showImGuiDemo);

            ImGui::Separator();

            if (SDL_GetWindowFlags(sdlWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP)
            {
                auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", true);
                if (toggleFullscreen)
                    SDL_SetWindowFullscreen(sdlWindow, 0);
            }
            else
            {
                auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", false);
                if (toggleFullscreen)
                {
                    SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
            }

            ImGui::Separator();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Run"))
        {
            auto vm = application->getVirtualMachine();

            if (ImGui::MenuItem(ICON_FA_PLAY" Run") && vm.isStopped())
            {
               application->runCurrentFileProgram();
            }

            if (ImGui::MenuItem(ICON_FA_BUG" Debug") && vm.isStopped())
            {
               application->debugCurrentFileProgram();
            }

            if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT" Step Over") && vm.isDebugging())
            {
                application->stepOverCurrentFileProgram();
            }

            if (ImGui::MenuItem(ICON_FA_STOP" Stop") && !vm.isStopped())
            {
                application->stopCurrentFileProgram();
            }

            if (ImGui::MenuItem(ICON_FA_UNDO " Reset"))
            {
                application->stopCurrentFileProgram();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("An issue ?"))
        {
            if (ImGui::MenuItem("Report on Github.com"))
            {
                System::OpenURL("https://github.com/berdal84/Nodable/issues");
            }

            if (ImGui::MenuItem("Report by email"))
            {
                System::OpenURL("mail:berenger@dalle-cort.fr");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Show Startup Screen", "F1"))
            {
                isStartupWindowVisible = true;
            }

            if (ImGui::MenuItem("Browse source code"))
            {
                System::OpenURL("https://www.github.com/berdal84/nodable");
            }

            if (ImGui::MenuItem("Credits"))
            {
                System::OpenURL("https://github.com/berdal84/nodable#credits-");
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void ApplicationView::drawStatusBar() const {/*
  Status bar
*/

    auto lastLog = Log::GetLastMessage();

    if( lastLog != nullptr )
    {
        ImVec4 statusLineColor;

        switch ( lastLog->verbosity )
        {
            case Log::Verbosity::Error:
                statusLineColor  = ImVec4(0.5f, 0.0f, 0.0f,1.0f);
                break;

            case Log::Verbosity::Warning:
                statusLineColor  = ImVec4(0.5f, 0.0f, 0.0f,1.0f);
                break;

            default:
                statusLineColor  = ImVec4(0.0f, 0.0f, 0.0f,0.5f);
        }

        ImGui::TextColored(statusLineColor, "%s", lastLog->text.c_str());
    }
}

void ApplicationView::drawHistoryBar(History *currentFileHistory) {
    if (currentFileHistory) {

        if (ImGui::IsMouseReleased(0)) {
            isHistoryDragged = false;
        }

//				ImGui::Text(ICON_FA_CLOCK " History: ");

        auto historyButtonSpacing = float(1);
        auto historyButtonHeight = float(10);
        auto historyButtonMaxWidth = float(40);

        auto historySize = currentFileHistory->getSize();
        auto historyCurrentCursorPosition = currentFileHistory->getCursorPosition();
        auto availableWidth = ImGui::GetContentRegionAvailWidth();
        auto historyButtonWidth = fmin(historyButtonMaxWidth,
                                       availableWidth / float(historySize + 1) - historyButtonSpacing);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(historyButtonSpacing, 0));

        for (size_t commandId = 0; commandId <= historySize; commandId++) {
            ImGui::SameLine();

            std::string label("##" + std::to_string(commandId));

            // Draw an highlighted button for the current history position
            if (commandId == historyCurrentCursorPosition) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
                ImGui::Button(label.c_str(), ImVec2(historyButtonWidth, historyButtonHeight));
                ImGui::PopStyleColor();

                // or a simple one for other history positions
            } else
                ImGui::Button(label.c_str(), ImVec2(historyButtonWidth, historyButtonHeight));

            // Hovered item
            if (ImGui::IsItemHovered()) {
                if (ImGui::IsMouseDown(0)) // hovered + mouse down
                {
                    isHistoryDragged = true;
                }

                // Draw command description
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, float(0.8));
                ImGui::BeginTooltip();
                ImGui::Text("%s", currentFileHistory->getCommandDescriptionAtPosition(commandId).c_str());
                ImGui::EndTooltip();
                ImGui::PopStyleVar();
            }

            // When dragging history
            const auto xMin = ImGui::GetItemRectMin().x;
            const auto xMax = ImGui::GetItemRectMax().x;
            if (isHistoryDragged &&
                ImGui::GetMousePos().x < xMax &&
                ImGui::GetMousePos().x > xMin) {
                currentFileHistory->setCursorPosition(commandId); // update history cursor position
            }


        }
        ImGui::PopStyleVar();
    }
}

void Nodable::ApplicationView::browseFile()
{
	fileBrowser.Open();
}

void ApplicationView::drawBackground()
{
    ImGui::BeginChild("background");
    std::filesystem::path path(NODABLE_ASSETS_DIR"/nodable-logo-xs.png");
    auto logo = Texture::GetWithPath(path);

    for( int x = 0; x < 5; x++ )
    {
        for( int y = 0; y < 5; y++ )
        {
            ImGui::SameLine( (ImGui::GetContentRegionAvailWidth() - logo->width) * 0.5f); // center img
            ImGui::Image((void*)(intptr_t)logo->image, ImVec2((float)logo->width, (float)logo->height));
        }
        ImGui::NewLine();
    }
    ImGui::EndChild();

}

void ApplicationView::drawToolBar()
{
    auto settings = Settings::GetCurrent();

    // small margin
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);

    auto vm = application->getVirtualMachine();

    ImGui::BeginGroup();

    // run
    bool isRunning = vm.isRunning();
    if ( isRunning )
        ImGui::PushStyleColor(ImGuiCol_Button, settings->ui.button.activeColor);

    if ( ImGui::Button(ICON_FA_PLAY) && vm.isStopped())
    {
        application->runCurrentFileProgram();
    }
    if ( isRunning )
        ImGui::PopStyleColor();
    ImGui::SameLine();

    // debug
    bool isDebugging = vm.isDebugging();
    if ( isDebugging )
        ImGui::PushStyleColor(ImGuiCol_Button, settings->ui.button.activeColor);
    if ( ImGui::Button(ICON_FA_BUG) && vm.isStopped())
    {
        application->debugCurrentFileProgram();
    }
    if ( isDebugging )
        ImGui::PopStyleColor();
    ImGui::SameLine();

    // stepOver
    if ( ImGui::Button(ICON_FA_ARROW_RIGHT) && vm.isDebugging())
    {
        application->stepOverCurrentFileProgram();
    }
    ImGui::SameLine();

    // stop
    if ( ImGui::Button(ICON_FA_STOP) && !vm.isStopped())
    {
        application->stopCurrentFileProgram();
    }
    ImGui::SameLine();

    // reset
    if ( ImGui::Button(ICON_FA_UNDO))
    {
        application->resetCurrentFileProgram();
    }
    ImGui::SameLine();
    ImGui::EndGroup();

}
