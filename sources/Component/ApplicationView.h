#pragma once

#include "Nodable.h"

#include <imgui/imgui.h>
#include <SDL2/include/SDL.h>
#include <string>
#include "View.h"
#include <mirror.h>
#include "IconsFontAwesome5.h"

// Override imfilebrowser.h icons
#define IMFILEBROWSER_FILE_ICON ICON_FA_FILE
#define IMFILEBROWSER_FOLDER_ICON ICON_FA_FOLDER
#include "imfilebrowser.h"

namespace Nodable
{
	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class ApplicationView : public View
	{
		
	public:		
		ApplicationView(const char* _name, Application* _application);
		~ApplicationView();

		/* call this every frame */
		bool draw();
		virtual bool update() { return true; };
		void drawFileTabs();

		/* call this once just after the instantiation. */
		bool init();

		

	private:
		ImGui::FileBrowser fileBrowser;
		Application       *application;
		SDL_Window        *sdlWindow;
		SDL_GLContext     glcontext;
		ImColor backgroundColor;
        bool isStartupWindowVisible;
        ImFont *paragraphFont;
        ImFont *headingFont;
        bool isHistoryDragged;
        const char* startupScreenTitle = "##STARTUPSCREEN";
	public:
		void browseFile();

		MIRROR_CLASS(ApplicationView)(
			MIRROR_PARENT(View)
		);

        void drawHistoryBar(History *currentFileHistory);

        void drawStatusBar() const;

        void drawMenuBar(History *currentFileHistory, bool &userWantsToDeleteSelectedNode,
                         bool &userWantsToArrangeSelectedNodeHierarchy, bool &redock_all);

        void drawStartupWindow();

        void drawPropertiesWindow();
    };
}