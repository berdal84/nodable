#pragma once

#include "Nodable.h" /* Forward declarations and defines */
#include "Node.h"
#include <string>
#include <memory>    /* For unique_ptr */
#include <string>
#include <filesystem>
#include <future>

namespace Nodable
{
    class File;

class Application : public Node, public std::enable_shared_from_this<Application>
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
		File*            getFileAtIndex                        (size_t _index)const { return loadedFiles.at(_index).get(); }
		size_t           getCurrentFileIndex                   ()const              { return currentFileIndex; }
		void             setCurrentFileWithIndex               (size_t _index);
		

		static void      SaveNode(std::shared_ptr<Node> _entity);

	private:
		/** When set to true, the application will close next frame */
		bool          quit                             = false;

		std::vector<std::unique_ptr<File>> loadedFiles;
		size_t             currentFileIndex; /* index that identify the current file in loadedFiles */
	
		const std::filesystem::path assetsFolderPath;

	};
}


