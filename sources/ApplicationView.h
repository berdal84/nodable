#pragma once

#include "Nodable.h"
#include <imgui.h>
#include <SDL.h>
#include <string>
#include "View.h"

namespace Nodable
{
	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class ApplicationView : public View
	{
	public:
		ApplicationView(const char* _name, Node_Application* _application);
		~ApplicationView();

		/* call this every frame */
		void draw();

		/* call this once just after the instantiation. */
		bool init();

	private:

		Node_Application  *application;
		SDL_Window        *window;
		SDL_GLContext     glcontext;
		ImVec4            clear_color = ImColor(50, 50, 50); // used to fill the framebuffer each frame.
	};
}