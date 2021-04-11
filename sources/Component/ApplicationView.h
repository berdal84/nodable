#pragma once

#include "Nodable.h"

#include <imgui/imgui.h>
#include <SDL2/include/SDL.h>
#include <string>
#include "View.h"
#include <mirror.h>

// Override imfilebrowser.h icons
#define IMFILEBROWSER_FILE_ICON ICON_FA_FILE
#define IMFILEBROWSER_FOLDER_ICON ICON_FA_FOLDER
#include "imfilebrowser.h"

namespace Nodable
{
    // forward declarations
    class Language;
    class Application;
    class History;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class ApplicationView : public View
	{
		
	public:		
		ApplicationView(const char* _name, Application* _application);
		~ApplicationView() override;
		bool draw() override;
		bool init();
        void browseFile();

	private:
		ImGui::FileBrowser fileBrowser;
		Application*       application;
		SDL_Window*        sdlWindow;
		SDL_GLContext      glcontext;
		ImColor            backgroundColor;
        bool               isStartupWindowVisible;
        ImFont*            paragraphFont;
        ImFont*            headingFont;
        bool               isHistoryDragged;
        const char*        startupScreenTitle = "##STARTUPSCREEN";
        bool               isLayoutInitialized = false;
        std::string        m_glWindowName;
        bool               m_showProperties;
        bool               m_showImGuiDemo;

        void drawHistoryBar(History *currentFileHistory);
        void drawStatusBar() const;
        void drawMenuBar(History*, bool &userWantsToDeleteSelectedNode,
                         bool &userWantsToArrangeSelectedNodeHierarchy, bool &redock_all);
        void drawStartupWindow();
        void drawFileEditor(ImGuiID dockspace_id, bool redock_all, size_t fileIndex);
        void drawFileBrowser();
        void drawBackground();
        void drawPropertiesWindow();

        /* reflect class using mirror */
        MIRROR_CLASS(ApplicationView)
        (
            MIRROR_PARENT(View) // we only need to know parent
        );

        void drawToolBar();
    };
}