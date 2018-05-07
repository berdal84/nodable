#pragma once

#include "Nodable.h" /* Forward declarations and defines */
#include "Node.h"    /* Base class */
#include <string>

namespace Nodable
{
	class Node_Application : public Node
	{
	public:
		Node_Application(const char*);
		~Node_Application();

		void             eval(std::string /* literal expression */);
		bool             init();
		void             shutdown();
		bool             update();
		void             draw();
		void             stopExecution();
		Node_Container*  getContext()const;

	private:
		Node_Container*  ctx;
		Node_String*     exitString;
		Node_String*     lastString;
		ApplicationView* view;
		bool             quit = false;
	};
}
