#pragma once

// std
#include <string>
#include <memory>
#include <future>
#include <ghc/filesystem.hpp>

// Nodable
#include <nodable/Nodable.h>
#include <nodable/VM.h>
#include <nodable/Settings.h>
#include <nodable/R.h>

namespace Nodable
{
    // forward declarations
    class AppView;
    class File;
    class AppContext;

	class App
	{
	public:

		explicit App(const char*);
		~App();

		bool            init();
		void            shutdown();
		void            update();
		void            flag_to_stop();
        bool            should_stop() const { return m_should_stop; }
		bool            open_file(const ghc::filesystem::path& _filePath);
        std::string     get_asset_path(const char* _fileName)const;
		void            save_file()const;
		void            close_file();
        void            close_file_at(size_t _fileIndex);
		File*           get_curr_file()const;
		size_t          get_file_count()const;
		File*           get_file_at(size_t _index)const;
        size_t          get_curr_file_index()const;
		void            set_curr_file(size_t _index);
        Node*           get_curr_file_program_root() const;
        void            vm_run();
        void            vm_debug();
        void            vm_step_over();
        void            vm_stop();
        void            vm_reset();
        AppView*        get_view()const { return m_view; };
        AppContext*     get_context()const { return m_context; };

    private:
	    AppView*        m_view;
        AppContext*     m_context;
		bool            m_should_stop;
		size_t          m_currentFileIndex;
        std::string     m_name;
        std::vector<File*>          m_loadedFiles;
        const ghc::filesystem::path m_assetsFolderPath;

        void handle_events();
    };
}
