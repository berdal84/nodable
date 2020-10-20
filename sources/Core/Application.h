#pragma once

#include "Nodable.h" /* Forward declarations and defines */
#include "Node.h"
#include "History.h"
#include "File.h"
#include <string>
#include <memory>    /* For unique_ptr */
#include <mirror.h>
#include <string>
#include <filesystem>
#include <future>

namespace Nodable
{

	class Application : public Node
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
		UpdateResult     update();

		/* Force application to stops. Always delayed for 1 frame. */
		void             stopExecution();

		bool             openFile(std::filesystem::path /* _filePath */);

		/* get the asset path with a fileName (adds assetsFolder)*/
		std::filesystem::path getAssetPath(const char* _fileName)const;

		void             saveCurrentFile()const;
		void             closeCurrentFile();
		File*			 getCurrentFile()const;
		size_t           getFileCount                          ()const              { return loadedFiles.size(); }
		File*            getFileAtIndex                        (size_t _index)const { return loadedFiles[_index]; }
		size_t           getCurrentFileIndex                   ()const              { return currentFileIndex; }
		void             setCurrentFileWithIndex               (size_t _index);
		

		static void      SaveNode(Node* _entity);

	private:
		/** A variable that contains (into the member "value") the current expression string */
		Variable*     currentExpressionStringVariable  = nullptr;	

		/** When set to true, the application will close next frame */
		bool          quit                             = false;

		std::vector<File*> loadedFiles;
		size_t             currentFileIndex; /* index that identify the current file in loadedFiles */
	
		const std::filesystem::path assetsFolderPath;

		/** Reflection using mirror*/
		MIRROR_CLASS(Application)();

	};
}


