#include <nodable/AppView.h>

#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/Config.h>
#include <nodable/Texture.h>
#include <nodable/Settings.h>
#include <nodable/System.h>
#include <nodable/App.h>
#include <nodable/NodeView.h>
#include <nodable/File.h>
#include <nodable/Log.h>
#include <nodable/FileView.h>
#include <nodable/History.h>

using namespace Nodable;

AppView::AppView(const char* _name, App* _application):
        m_app(_application),
        m_bgColor(50, 50, 50),
        m_isStartupWindowVisible(true),
        m_isLayoutInitialized(false),
        m_startupScreenTitle("##STARTUPSCREEN"),
        m_isHistoryDragged(false),
        m_glWindowName(_name),
        m_showProperties(false),
        m_showImGuiDemo(false)
{
}

AppView::~AppView()
{}

bool AppView::init()
{
    Settings* settings = Settings::Get();

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        LOG_ERROR( "AppView", "SDL Error: %s\n", SDL_GetError())
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
    m_sdlWindow = SDL_CreateWindow(m_glWindowName.c_str(),
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   800,
                                   600,
                                SDL_WINDOW_OPENGL |
                                SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_MAXIMIZED |
                                SDL_WINDOW_SHOWN
                                );

    this->m_sdlGLContext = SDL_GL_CreateContext(m_sdlWindow);
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

    // load fonts (TODO: hum, load only if font is used...)
    for ( auto& each_font : settings->ui_text_fonts )
    {
        loadFont(each_font);
    }

    // Assign fonts (user might want to change it later, but we need defaults)
    for( auto each_slot = 0; each_slot < FontSlot_COUNT; ++each_slot )
    {
        const char* font_id = settings->ui_text_defaultFontsId[each_slot];
        m_fonts[each_slot] = getFontById(font_id);
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
    ImGui_ImplSDL2_InitForOpenGL(m_sdlWindow, m_sdlGLContext);
    const char* glsl_version = NULL; // let backend decide wich version to use, usually 130 (pc) or 150 (macos).
    ImGui_ImplOpenGL3_Init(glsl_version);

	return true;
}

ImFont* AppView::getFontById(const char* id ) {
    return m_loadedFonts.at(id );
}

ImFont* AppView::loadFont(const FontConf& fontConf) {

    NODABLE_ASSERT(m_loadedFonts.find(fontConf.id) == m_loadedFonts.end()); // do not allow the use of same key for different fonts

    ImFont*  font     = nullptr;
    auto&    io       = ImGui::GetIO();
    auto     settings = Settings::Get();

    // Create font
    {
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;

        //io.Fonts->AddFontDefault();
        std::string fontPath = m_app->getAssetPath(fontConf.path);
        LOG_VERBOSE("AppView", "Adding font from file ... %s\n", fontPath.c_str())
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontConf.size, &config);
    }

    // Add Icons my merging to previous font.
    if ( fontConf.enableIcons ) {
        // merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphMinAdvanceX = settings->ui_icons.size; // monospace to fix text alignment in drop down menus.
        auto fontPath = m_app->getAssetPath(settings->ui_icons.path);
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), settings->ui_icons.size, &config, icons_ranges);
        LOG_VERBOSE("AppView", "Adding icons to font ...\n")
    }

    m_loadedFonts.insert({fontConf.id, font});
    LOG_MESSAGE("AppView", "Font %s added to register with the id \"%s\"\n", fontConf.path, fontConf.id)
    return font;
}

bool AppView::draw()
{
    // TODO: create an event list (fill, execute, clear)
    auto delete_node(false);
    auto arrange_node(false);
    auto select_next(false);
    auto expand_node(false);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type)
		{
		case SDL_QUIT:
			m_app->stopExecution();
			break;

		case SDL_KEYUP:
			auto key = event.key.keysym.sym;

			if ((event.key.keysym.mod & KMOD_LCTRL)) {

				// History
				if (auto file = m_app->getCurrentFile()) {
					History* currentFileHistory = file->getHistory();
					     if (key == SDLK_z) currentFileHistory->undo();
					else if (key == SDLK_y) currentFileHistory->redo();
				}

				// File
				     if( key == SDLK_s)  m_app->saveCurrentFile();
				else if( key == SDLK_w)  m_app->closeCurrentFile();
				else if( key == SDLK_o)  this->browseFile();
			}
			else if (key == SDLK_DELETE )
            {
                delete_node = true;
            }
			else if (key == SDLK_a)
            {
                arrange_node = true;
            }
			else if (key == SDLK_x)
            {
                expand_node = true;
            }
            else if (key == SDLK_n)
            {
                select_next = true;
            }
			else if (key == SDLK_F1 )
            {
                m_isStartupWindowVisible = true;
            }

			break;
		}

    }

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_sdlWindow);
	ImGui::NewFrame();
    ImGui::SetCurrentFont( m_fonts[FontSlot_Paragraph] );

    // Startup Window
    drawStartupWindow();

    // Demo Window
    {
        if (m_showImGuiDemo){
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&m_showImGuiDemo);
        }
    }

    // Fullscreen m_sdlWindow
    {

		// Get current file's history
		History* currentFileHistory = nullptr;

		if ( auto file = m_app->getCurrentFile())
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

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    //ImGui::MenuItem(ICON_FA_FILE   "  New", "Ctrl + N");
                    if (ImGui::MenuItem(ICON_FA_FOLDER      "  Open", "Ctrl + O")) browseFile();
                    if (ImGui::MenuItem(ICON_FA_SAVE        "  Save", "Ctrl + S")) m_app->saveCurrentFile();
                    if (ImGui::MenuItem(ICON_FA_TIMES       "  Close", "Ctrl + W")) m_app->closeCurrentFile();

                    FileView* fileView = nullptr;
                    bool auto_paste;
                    if ( auto file = m_app->getCurrentFile())
                    {
                        fileView = file->getView();
                        auto_paste = fileView->experimental_clipboard_auto_paste();
                    }

                    if (ImGui::MenuItem(ICON_FA_COPY        "  Auto-paste clipboard", "", auto_paste, fileView))
                    {
                        fileView->experimental_clipboard_auto_paste(!auto_paste);
                    }

                    if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT"  Quit", "Alt + F4")) m_app->stopExecution();

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit")) {
                    if (currentFileHistory) {
                        if (ImGui::MenuItem("Undo", "Ctrl + Z")) currentFileHistory->undo();
                        if (ImGui::MenuItem("Redo", "Ctrl + Y")) currentFileHistory->redo();
                        ImGui::Separator();
                    }

                    auto has_selection = NodeView::GetSelected() != nullptr;
                    delete_node  |= ImGui::MenuItem("Delete", "Del.", false, has_selection);
                    arrange_node |= ImGui::MenuItem("Arrange nodes", "A", false, has_selection);
                    expand_node  |= ImGui::MenuItem("Expand (toggle)", "X", false, has_selection);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View")) {
                    //auto frame = ImGui::MenuItem("Frame All", "F");
                    redock_all |= ImGui::MenuItem("Redock documents");

                    ImGui::Separator();
                    auto viewDetailMinimalist = ImGui::MenuItem("Minimalist View", "",
                                                                NodeView::s_viewDetail == NodeViewDetail::Minimalist);
                    auto viewDetailEssential = ImGui::MenuItem("Essential View", "",
                                                               NodeView::s_viewDetail == NodeViewDetail::Essential);
                    auto viewDetailExhaustive = ImGui::MenuItem("Exhaustive View", "",
                                                                NodeView::s_viewDetail == NodeViewDetail::Exhaustive);

                    if (viewDetailMinimalist) {
                        NodeView::SetDetail(NodeViewDetail::Minimalist);
                    } else if (viewDetailEssential) {
                        NodeView::SetDetail(NodeViewDetail::Essential);
                    } else if (viewDetailExhaustive) {
                        NodeView::SetDetail(NodeViewDetail::Exhaustive);
                    }

                    ImGui::Separator();
                    m_showProperties = ImGui::MenuItem(ICON_FA_COGS "  Show Properties", "", m_showProperties);
                    m_showImGuiDemo = ImGui::MenuItem("Show ImGui Demo", "", m_showImGuiDemo);

                    ImGui::Separator();

                    if (SDL_GetWindowFlags(m_sdlWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                        auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", true);
                        if (toggleFullscreen)
                            SDL_SetWindowFullscreen(m_sdlWindow, 0);
                    } else {
                        auto toggleFullscreen = ImGui::MenuItem("Fullscreen", "", false);
                        if (toggleFullscreen) {
                            SDL_SetWindowFullscreen(m_sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                    }

                    ImGui::Separator();

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Run")) {
                    auto runner = m_app->getRunner();

                    if (ImGui::MenuItem(ICON_FA_PLAY" Run") && runner.isStopped()) {
                        m_app->runCurrentFileProgram();
                    }

                    if (ImGui::MenuItem(ICON_FA_BUG" Debug") && runner.isStopped()) {
                        m_app->debugCurrentFileProgram();
                    }

                    if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT" Step Over") && runner.isDebugging()) {
                        m_app->stepOverCurrentFileProgram();
                    }

                    if (ImGui::MenuItem(ICON_FA_STOP" Stop") && !runner.isStopped()) {
                        m_app->stopCurrentFileProgram();
                    }

                    if (ImGui::MenuItem(ICON_FA_UNDO " Reset")) {
                        m_app->stopCurrentFileProgram();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("An issue ?")) {
                    if (ImGui::MenuItem("Report on Github.com")) {
                        System::OpenURL("https://github.com/berdal84/Nodable/issues");
                    }

                    if (ImGui::MenuItem("Report by email")) {
                        System::OpenURL("mail:berenger@dalle-cort.fr");
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Help")) {
                    if (ImGui::MenuItem("Show Startup Screen", "F1")) {
                        m_isStartupWindowVisible = true;
                    }

                    if (ImGui::MenuItem("Browse source code")) {
                        System::OpenURL("https://www.github.com/berdal84/nodable");
                    }

                    if (ImGui::MenuItem("Credits")) {
                        System::OpenURL("https://github.com/berdal84/nodable#credits-");
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
            drawToolBar();


            /*
             * Main Layout
             */

            ImGuiID dockspace_id = ImGui::GetID("dockspace_main");
            ImGuiID dockspace_documents = ImGui::GetID("dockspace_documents");
            ImGuiID dockspace_properties = ImGui::GetID("dockspace_properties");


            if ( !m_isLayoutInitialized)
            {
               ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
               ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace );
               ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
               ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, Settings::Get()->ui_layout_propertiesRatio, &dockspace_properties, NULL);
               ImGui::DockBuilderDockWindow("ImGui", dockspace_properties);
               ImGui::DockBuilderDockWindow("Settings", dockspace_properties);
               ImGui::DockBuilderDockWindow("Properties", dockspace_properties);
               ImGui::DockBuilderDockWindow("File Info", dockspace_properties);
               ImGui::DockBuilderFinish(dockspace_id);
                m_isLayoutInitialized = true;
            }

             /*
             * Fill the layout with content
             */
            ImGui::DockSpace(dockspace_id);

            // Global Props
            if (ImGui::Begin("Settings"))
            {
                drawPropertiesWindow();
            }
            ImGui::End();

            if (ImGui::Begin("ImGui"))
            {
                ImGui::ShowStyleEditor();
            }
            ImGui::End();

            // File info
            ImGui::Begin("File Info");
            {
                const File* currFile = m_app->getCurrentFile();

                if (currFile)
                {
                    FileView* fileView = currFile->getView();
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
                    ImGui::Text("Type: %s", view->get_owner()->getLabel());
                    ImGui::NewLine();
                    NodeView::DrawNodeViewAsPropertiesPanel(view);
                }
            }
            ImGui::End();


            // Opened documents
            for (size_t fileIndex = 0; fileIndex < m_app->getFileCount(); fileIndex++)
            {
                drawFileEditor(dockspace_id, redock_all, fileIndex);
            }


        }
        ImGui::End(); // Main window


		/*
		   Perform actions on selected node
		*/

		auto selectedNodeView = NodeView::GetSelected();
		if (selectedNodeView)
		{
			if (delete_node)
			{
			    auto node = selectedNodeView->get_owner();
                node->flagForDeletion();
            }
			else if (arrange_node)
            {
				selectedNodeView->arrangeRecursively();
            }
            else if (expand_node)
            {
                selectedNodeView->toggleExpansion();
            }
			else if (select_next)
            {
			    GraphTraversal traversal;
			    Node* next = traversal.getNextInstrToEval(selectedNodeView->get_owner());
			    if ( next )
			        if( auto nextView = next->getComponent<NodeView>())
                        NodeView::SetSelected(nextView);
            }
		}
    }

    drawFileBrowser();

    // Rendering
	ImGui::Render();
	SDL_GL_MakeCurrent(m_sdlWindow, this->m_sdlGLContext);
	auto io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(m_bgColor.Value.x, m_bgColor.Value.y, m_bgColor.Value.z, m_bgColor.Value.w);
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

    SDL_GL_SwapWindow(m_sdlWindow);

    // limit frame rate
    constexpr float desiredFrameRate = 1.0f/60.0f;
    if ( ImGui::GetIO().DeltaTime  < desiredFrameRate)
        SDL_Delay((desiredFrameRate - ImGui::GetIO().DeltaTime) * 1000u );

    return false;
}

void AppView::drawFileBrowser()
{
    m_fileBrowser.Display();
    if (m_fileBrowser.HasSelected())
    {
        auto selectedFiles = m_fileBrowser.GetMultiSelected();
        for (const auto & selectedFile : selectedFiles)
        {
            m_app->openFile(selectedFile.c_str());
        }
        m_fileBrowser.ClearSelected();
        m_fileBrowser.Close();
    }
}

void AppView::drawFileEditor(ImGuiID dockspace_id, bool redock_all, size_t fileIndex)
{
    File *file = m_app->getFileAtIndex(fileIndex);

    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags = (file->isModified() ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoScrollbar;

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
            const bool isCurrentFile = fileIndex == m_app->getCurrentFileIndex();

            if ( !isCurrentFile && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                m_app->setCurrentFileWithIndex(fileIndex);
            }

            // History bar on top
            drawHistoryBar(file->getHistory());

            // File View in the middle
            View* eachFileView = file->getView();
            ImVec2 availSize = ImGui::GetContentRegionAvail();

            if ( isCurrentFile )
            {
                availSize.y -= ImGui::GetTextLineHeightWithSpacing();
            }

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0.35f) );
            ImGui::PushFont(m_fonts[FontSlot_Code] );
            eachFileView->drawAsChild("FileView", availSize, false);
            ImGui::PopFont();
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
        m_app->closeFile(fileIndex);
    }
}

void AppView::drawPropertiesWindow()
{
    Settings* config = Settings::Get();

    ImGui::Text("Nodable Settings:");
    if ( ImGui::Button("Save to settings/default.cfg") )
    {
        Settings::Save();
    }
    ImGui::Indent();


        ImGui::Text("Buttons:");
        ImGui::Indent();
            ImGui::SliderFloat2("ui_toolButton_size", &config->ui_toolButton_size.x, 20.0f, 50.0f);
        ImGui::Unindent();

        ImGui::Text("Wires:");
        ImGui::Indent();
            ImGui::SliderFloat("thickness", &config->ui_wire_bezier_thickness, 0.5f, 10.0f);
            ImGui::SliderFloat("roundness", &config->ui_wire_bezier_roundness, 0.0f, 1.0f);
            ImGui::Checkbox("arrows", &config->ui_wire_displayArrows);
        ImGui::Unindent();

        ImGui::Text("Nodes:");
        ImGui::Indent();
            ImGui::SliderFloat("member connector radius", &config->ui_node_memberConnectorRadius, 1.0f, 10.0f);
            ImGui::SliderFloat("padding", &config->ui_node_padding, 1.0f, 20.0f);
            ImGui::SliderFloat("speed", &config->ui_node_speed, 0.0f, 100.0f);
            ImGui::SliderFloat("spacing", &config->ui_node_spacing, 0.0f, 100.0f);
            ImGui::SliderFloat("node connector padding", &config->ui_node_nodeConnectorPadding, 0.0f, 100.0f);
            ImGui::SliderFloat("node connector height", &config->ui_node_nodeConnectorHeight, 2.0f, 100.0f);

            ImGui::ColorEdit4("variables color", &config->ui_node_variableColor.x);
            ImGui::ColorEdit4("instruction color", &config->ui_node_instructionColor.x);
            ImGui::ColorEdit4("literal color", &config->ui_node_literalColor.x);
            ImGui::ColorEdit4("function color", &config->ui_node_invokableColor.x);
            ImGui::ColorEdit4("shadow color", &config->ui_node_shadowColor.x);
            ImGui::ColorEdit4("border color", &config->ui_node_borderColor.x);
            ImGui::ColorEdit4("high. color", &config->ui_node_highlightedColor.x);
            ImGui::ColorEdit4("border high. color", &config->ui_node_borderHighlightedColor.x);
            ImGui::ColorEdit4("fill color", &config->ui_node_fillColor.x);
            ImGui::ColorEdit4("node connector color", &config->ui_node_nodeConnectorColor.x);
            ImGui::ColorEdit4("node connector hovered color", &config->ui_node_nodeConnectorHoveredColor.x);

        ImGui::Unindent();

        // code flow
        ImGui::Text("Code flow:");
        ImGui::Indent();
            ImGui::SliderFloat("line width min", &config->ui_codeFlow_lineWidthMax, 1.0f, 100.0f);
        ImGui::Unindent();

    ImGui::Unindent();
}

void AppView::drawStartupWindow() {
    if (m_isStartupWindowVisible && !ImGui::IsPopupOpen(m_startupScreenTitle))
    {
        ImGui::OpenPopup(m_startupScreenTitle);
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(500,200), ImVec2(500,50000));
    ImGui::SetNextWindowPos( ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f,0.5f) );

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if ( ImGui::BeginPopupModal(m_startupScreenTitle, nullptr, flags) )
    {

        auto path = m_app->getAssetPath(Settings::Get()->ui_splashscreen_imagePath);
        auto logo = Texture::GetWithPath(path);
        ImGui::SameLine( (ImGui::GetContentRegionAvailWidth() - logo->width) * 0.5f); // center img
        ImGui::Image((void*)(intptr_t)logo->image, ImVec2((float)logo->width, (float)logo->height));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(25.0f, 20.0f) );
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        {
            ImGui::PushFont(m_fonts[FontSlot_Heading] );
            ImGui::NewLine();
            ImGui::Text("Nodable is node-able");
            ImGui::PopFont();
            ImGui::NewLine();
        }


        ImGui::TextWrapped("The goal of Nodable is to allow you to edit a computer program in a textual and nodal way at the same time." );

        {
            ImGui::PushFont(m_fonts[FontSlot_Heading] );
            ImGui::NewLine();
            ImGui::Text("Manifest");
            ImGui::PopFont();
            ImGui::NewLine();
        }

        ImGui::TextWrapped( "The nodal and textual points of view each have pros and cons. The user should not be forced to choose one of the two." );

        {
            ImGui::PushFont(m_fonts[FontSlot_Heading] );
            ImGui::NewLine();
            ImGui::Text("Disclaimer");
            ImGui::PopFont();
            ImGui::NewLine();
        }

        ImGui::TextWrapped( "Nodable is a prototype, do not expect too much from it. Use at your own risk." );

        ImGui::NewLine();ImGui::NewLine();

        const char* credit = "berenger@dalle-cort.fr";
        ImGui::SameLine( ImGui::GetContentRegionAvailWidth() - ImGui::CalcTextSize(credit).x);
        ImGui::TextWrapped( "%s", credit );
        ImGui::TextWrapped( NODABLE_VERSION );
        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
        {
            ImGui::CloseCurrentPopup();
            m_isStartupWindowVisible = false;
        }
        ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
        ImGui::EndPopup();
    }

}

void AppView::drawStatusBar() const {/*
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

void AppView::drawHistoryBar(History *currentFileHistory) {
    if (currentFileHistory) {

        if (ImGui::IsMouseReleased(0)) {
            m_isHistoryDragged = false;
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
                    m_isHistoryDragged = true;
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
            if (m_isHistoryDragged &&
                ImGui::GetMousePos().x < xMax &&
                ImGui::GetMousePos().x > xMin) {
                currentFileHistory->setCursorPosition(commandId); // update history cursor position
            }


        }
        ImGui::PopStyleVar();
    }
}

void AppView::browseFile()
{
	m_fileBrowser.Open();
}

void AppView::drawBackground()
{
    ImGui::BeginChild("background");
    std::string path(NODABLE_ASSETS_DIR"/nodable-logo-xs.png");
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

void AppView::drawToolBar()
{
    Settings* settings = Settings::Get();
    Runner& runner = m_app->getRunner();

    // small margin
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
    ImGui::BeginGroup();

    // run
    bool isRunning = runner.isRunning();
    if ( isRunning )
        ImGui::PushStyleColor(ImGuiCol_Button, settings->ui_button_activeColor);

    if (ImGui::Button(ICON_FA_PLAY, settings->ui_toolButton_size) && runner.isStopped())
    {
        m_app->runCurrentFileProgram();
    }
    if ( isRunning )
        ImGui::PopStyleColor();
    ImGui::SameLine();

    // debug
    bool isDebugging = runner.isDebugging();
    if ( isDebugging )
        ImGui::PushStyleColor(ImGuiCol_Button, settings->ui_button_activeColor);
    if (ImGui::Button(ICON_FA_BUG, settings->ui_toolButton_size) && runner.isStopped())
    {
        m_app->debugCurrentFileProgram();
    }
    if ( isDebugging )
        ImGui::PopStyleColor();
    ImGui::SameLine();

    // stepOver
    if (ImGui::Button(ICON_FA_ARROW_RIGHT, settings->ui_toolButton_size) && runner.isDebugging())
    {
        m_app->stepOverCurrentFileProgram();
    }
    ImGui::SameLine();

    // stop
    if ( ImGui::Button(ICON_FA_STOP, settings->ui_toolButton_size) && !runner.isStopped())
    {
        m_app->stopCurrentFileProgram();
    }
    ImGui::SameLine();

    // reset
    if ( ImGui::Button(ICON_FA_UNDO, settings->ui_toolButton_size))
    {
        m_app->resetCurrentFileProgram();
    }
    ImGui::SameLine();
    ImGui::EndGroup();

}

void AppView::shutdown()
{
    Texture::ReleaseResources();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext    ();
    SDL_GL_DeleteContext     (m_sdlGLContext);
    SDL_DestroyWindow        (m_sdlWindow);
    SDL_Quit                 ();
}
