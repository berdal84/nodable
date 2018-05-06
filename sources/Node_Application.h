#pragma once

#include "Nodable.h" /* Forward declarations and defines */
#include "Node.h"    /* Base class */
#include "Log.h"
#include <string>

namespace Nodable
{
	class Node_Application : public Node
	{
	public:
		Node_Application();
		~Node_Application();

		bool eval(std::string /* literal expression */);
		void init();
		void shutdown();
		void draw()override;
	private:
		Node_Container* ctx;
		Node_String*    exitString;
		Node_String*    lastString;
	};
}
