#pragma once

#include "Nodable.h" /* Forward declarations and defines */
#include "Node.h"    /* Base class */
#include <string>
#include <memory>    /* For unique_ptr */

namespace Nodable
{
	class Node_Application : public Node
	{
	public:
		Node_Application(const char*);
		~Node_Application();

		bool             eval(){return true;}
		bool             eval(std::string /* literal expression */);
		bool             init();
		void             clear();
		void             shutdown();
		bool             update();
		void             draw();
		void             stopExecution();
		Node_Container*  getContext()const;

	private:
		std::unique_ptr<Node_Container>  ctx;
		Node_Variable*   lastString;
		std::unique_ptr<ApplicationView> view;
		bool             quit = false;
	};
}
