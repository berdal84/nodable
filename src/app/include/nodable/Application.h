#pragma once

// std
#include <string>
#include <memory>
#include <mirror.h>
#include <filesystem>
#include <future>

// Nodable
#include <nodable/Nodable.h>
#include <nodable/Runner.h>

namespace Nodable
{
    // forward declarations
    class ApplicationView;
    class File;

    /**
     * This class is the master class of Nodable.
     * Using it you can launch a Nodable Application with ease.
     *
     * As you can see Application extends Node, because everything (or almost) is a Node in Nodable.
     */
	class Application
	{
	public:

	    /**
	     * Construct a new Application given an application name argument.
	     */
		explicit Application(const char*);

		~Application();

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
		bool openFile(std::filesystem::path _filePath);

		/**
		 * Returns the full path of an asset from a filename.
		 * @param _fileName
		 * @return full path
		 */
		std::filesystem::path getAssetPath(const char* _fileName)const;

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

        inline Runner& getRunner() { return m_runner; }

        ProgramNode *getCurrentFileProgram() const;

        // shortcuts to virtual machine:
        void runCurrentFileProgram();
        void debugCurrentFileProgram();
        void stepOverCurrentFileProgram();
        void stopCurrentFileProgram();
        void resetCurrentFileProgram();
        ApplicationView* getView()const { return m_view; };

		/**
		 * @deprecated
		 *
		 * Save a node to a file (in JSON)
		 * @param _node
		 */
		static void SaveNode(Node* _node);

		static Application* s_instance;
	private:
	    ApplicationView* m_view;
		/** When set to true, the application will close next frame */
		bool quit = false;

		/** The list of loaded files. */
		std::vector<File*> loadedFiles;

        /** An index that identify the current file in loadedFiles */
		size_t currentFileIndex;

		/** The asset base folder path */
		const std::filesystem::path assetsFolderPath;

        /** A minimalist "virtual machine" to run instructions */
        Runner m_runner;

        std::string m_name;
    };
}
