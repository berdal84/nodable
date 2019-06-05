#pragma once

#include "Nodable.h" /* Forward declarations and defines */
#include "Entity.h"    /* Base class */
#include "History.h"
#include <string>
#include <memory>    /* For unique_ptr */

namespace Nodable
{
	class Application : public Entity
	{
	public:
		Application(const char* /*_applicationName*/);
		~Application();

		/* Clear the application context. */
		void             clearContext();

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
		
		/* Update Current Line Text*/
		void             updateCurrentLineText(std::string _val);

		/* Force application to stops. Always delayed for 1 frame. */
		void             stopExecution();

		/* Get the context (also called a container) of this application.*/
		Container*       getContext()const;

		static void      SaveEntity(Entity* _entity);

	private:
		Variable*     lastString  = nullptr;		
		bool          quit        = false;
	};
}
