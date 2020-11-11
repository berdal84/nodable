#pragma once

#include <imgui/imgui.h>
#include <SDL2/include/SDL.h>
#include <string>
#include "View.h"
#include "mirror.h"

// Override imfilebrowser.h icons
#define IMFILEBROWSER_FILE_ICON ICON_FA_FILE
#define IMFILEBROWSER_FOLDER_ICON ICON_FA_FOLDER
#include "imfilebrowser.h"

namespace Nodable
{
    class Application;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class ApplicationView : public View
	{
		
	public:		
		ApplicationView();
		~ApplicationView();

		/* call this every frame */
		bool draw();
		virtual bool update() { return true; };
		void drawFileTabs();

		/* call this once just after the instantiation. */
		bool init();
        void browseFile();

	private:
        std::shared_ptr<Application> getApplication();

		ImGui::FileBrowser fileBrowser;
		SDL_Window        *sdlWindow{};
		SDL_GLContext     glcontext{};
		ImColor backgroundColor;
        bool isStartupWindowVisible;
        ImFont *paragraphFont{};
        ImFont *headingFont{};
        bool isHistoryDragged;

		MIRROR_CLASS(ApplicationView)(
			MIRROR_PARENT(View)
		);

    };
}