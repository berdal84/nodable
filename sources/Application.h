#pragma once

#include "Nodable.h" /* Forward declarations and defines */
#include "Entity.h"    /* Base class */
#include "History.h"
#include "File.h"
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

		/* open a file with a file path as parameter, return true if success, false if fail.*/
		bool             openFile(const char*);

		unsigned int     getLoadedFileCount                    ()const              { return loadedFiles.size(); }
		std::string      getLoadedFileContentAtIndex           (size_t _index)const { return loadedFiles[_index]->getContent(); }
		std::string      getLoadedFileNameAtIndex              (size_t _index)const { return loadedFiles[_index]->getName(); }
		size_t           getCurrentlyActiveLoadedFileIndex     ()const              { return currentlyActiveLoadedFileIndex; }
		void             setCurrentlyActiveLoadedFileWithIndex (size_t _index);

		static void      SaveEntity(Entity* _entity);

	private:
		/** A variable that contains (into the member "value") the current expression string */
		Variable*     currentExpressionStringVariable  = nullptr;	

		/** When set to true, the application will close next frame */
		bool          quit                             = false;

		std::vector<File*> loadedFiles;
		size_t             currentlyActiveLoadedFileIndex; /* index that identify the current file in loadedFiles */
	};
}
