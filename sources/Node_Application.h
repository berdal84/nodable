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
		Node_Application(const char* /*_applicationName*/);
		~Node_Application();

		/* Clear the application context. */
		void             clearContext();

		/* Draw the application's view*/
		void             draw();

		/* Converts an expression to a graph. 
		The graph will be created within the application context (this->ctx)*/
		bool             eval(std::string /* literal expression */);

		/* Initialize the application.
		Should be called once before the first update() and draw()*/
		bool             init();

		/* Release resources */
		void             shutdown();

		/* Update the state of the application.
		Call this once per frame */
		bool             update();

		/* Force application to stops. Always delayed for 1 frame. */
		void             stopExecution();

		/* Get the context (also called a container) of this application.*/
		Node_Container*  getContext()const;

	private:
		std::unique_ptr<Node_Container>  ctx;
		std::unique_ptr<ApplicationView> view;
		Node_Variable*                   lastString;		
		bool                             quit        = false;
	};
}
