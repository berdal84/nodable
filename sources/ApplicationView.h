#pragma once

#include "Nodable.h"
#include <imgui.h>
#include <SDL.h>
#include <string>

namespace Nodable
{
	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class ApplicationView
	{
	public:
		ApplicationView(const char* _name, Node_Application* _application);
		~ApplicationView();

		/* call this every frame */
		void draw();

		/* call this once just after the instantiation.
		This will init the GL context and the window at specified size (in pixel) */
		bool init(ImVec2 _windowSize = ImVec2(1280.0f, 720.0f));

	private:		
		Node_Application  *application;
		SDL_Window        *window;
		SDL_GLContext     glcontext;
		ImVec4            clear_color = ImColor(50, 50, 50); // used to fill the framebuffer each frame.
		std::string       name;
	};
}