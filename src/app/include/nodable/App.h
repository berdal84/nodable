#pragma once

// std
#include <string>
#include <memory>
#include <nodable/R.h>
#include <future>

#include <ghc/filesystem.hpp>

// Nodable
#include <nodable/Nodable.h>
#include <nodable/VM.h>
#include <nodable/Settings.h>

namespace Nodable
{
    // forward declarations
    class AppView;
    class File;
    class AppContext;

    /**
     * This class is the master class of Nodable.
     * Using it you can launch a Nodable Application with ease.
     *
     * As you can see Application extends Node, because everything (or almost) is a Node in Nodable.
     */
	class App
	{
	public:

	    /**
	     * Construct a new Application given an application name argument.
	     */
		explicit App(const char*);

		~App();

		/**
		 * Initialize the application.
		 * Should be called once before the first update() and draw()
         * @return true if succeed
         */
		bool init();

		/**
		 * Shutdown the application
		 */
		void shutdown();

		/**
		 * Update the state of the application.
		 * this must be called once per frame
		 * @return
		 */
		bool update();

		/**
		 * Force application to stops.
		 * The application will effectively stops after 1 frame.
		 */
		void stopExecution();

		/**
		 * Open a new file with a path
		 * @param _filePath
		 * @return true if succeed
		 */
		bool openFile(const ghc::filesystem::path& _filePath);

		/**
		 * Returns the full path of an asset from a filename.
		 * @param _fileName
		 * @return full path
		 */
        std::string getAssetPath(const char* _fileName)const;

		/**
		 * Save the current file.
		 * Does nothing if no file is open.
		 */
		void saveCurrentFile()const;

		/**
		 * Close current file.
		 * Does nothing if not file is open.
		 */
		void closeCurrentFile();
        void closeFile(size_t _fileIndex);

		/**
		 * Get the current file.
		 * @return a pointer to the File, can be nullptr is no file is open.
		 */
		[[nodiscard]] File* getCurrentFile()const;

		/**
		 * Get the opened file count.
		 * @return
		 */
		[[nodiscard]] size_t getFileCount()const;

        /**
         * Get the opened file by index.
         * @param _index must be lower to the getFileCount().
         * @return a pointer to the file, can't be nullptr.
         */
		[[nodiscard]] File*  getFileAtIndex(size_t _index)const;

		/**
		 * Get the index of the current file.
		 * @return
		 */
		[[nodiscard]] size_t getCurrentFileIndex()const;

		/**
		 * Set the current file with an index.
		 * @param _index must be lower that getFileCount().
		 */
		void setCurrentFileWithIndex(size_t _index);

        Node* getCurrentFileProgram() const;

        // shortcuts to virtual machine:
        void runCurrentFileProgram();
        void debugCurrentFileProgram();
        void stepOverCurrentFileProgram();
        void stopCurrentFileProgram();
        void resetCurrentFileProgram();
        inline AppView* getView()const { return m_view; };

        AppContext* get_context()const { return m_context; };
    private:
	    AppView*    m_view;
        AppContext* m_context;
		bool        m_quit = false;
		std::vector<File*> m_loadedFiles;
		size_t      m_currentFileIndex;
        std::string m_name;
        const ghc::filesystem::path m_assetsFolderPath;
    };
}
