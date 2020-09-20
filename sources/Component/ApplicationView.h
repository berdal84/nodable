#pragma once

#include "Nodable.h"
#include <imgui/imgui.h>
#include <SDL2/include/SDL.h>
#include <string>
#include "View.h"
#include <mirror.h>

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
		Application       *application;
		SDL_Window        *sdlWindow;
		SDL_GLContext     glcontext;
		ImVec4            clear_color = ImColor(50, 50, 50); // used to fill the framebuffer each frame.
	public:
		void browseFile();

		MIRROR_CLASS(ApplicationView)(
			MIRROR_PARENT(View)
		);
	};
}