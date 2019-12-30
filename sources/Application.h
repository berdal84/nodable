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

		/* open a file with a file path as parameter, return true if success, false if fail.*/
		bool             openFile(const char*);
		void             saveCurrentFile()const;
		void             closeCurrentFile();
		File*			 getCurrentFile()const;
		unsigned int     getFileCount                          ()const              { return loadedFiles.size(); }
		std::string      getLoadedFileNameAtIndex              (size_t _index)const { return loadedFiles[_index]->getName(); }
		size_t           getCurrentFileIndex                   ()const              { return currentFileIndex; }
		void             setCurrentFileWithIndex (size_t _index);
		

		static void      SaveEntity(Entity* _entity);

	private:
		/** A variable that contains (into the member "value") the current expression string */
		Variable*     currentExpressionStringVariable  = nullptr;	

		/** When set to true, the application will close next frame */
		bool          quit                             = false;

		std::vector<File*> loadedFiles;
		size_t             currentFileIndex; /* index that identify the current file in loadedFiles */
	};
}
