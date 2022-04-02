#pragma once

// std
#include <string>
#include <memory>
#include <future>
#include <ghc/filesystem.hpp>

// Nodable
#include <nodable/core/VM.h>
#include <nodable/core/reflection/R.h>
#include <nodable/app/types.h>
#include <nodable/app/Settings.h>

namespace Nodable
{
    // forward declarations
    class AppView;
    class File;
    class AppContext;

	class App
	{
	public:
        using fs_path = ghc::filesystem::path;

		explicit App();
		~App();

		bool            init();
		void            shutdown();
		void            update();
		void            draw();
		void            flag_to_stop();
        bool            should_stop() const { return m_should_stop; }
		bool            open_file(const fs_path& _filePath);
        std::string     get_absolute_asset_path(const char* _relative_path)const;
		File*           new_file();
		void            save_file()const;
		void            close_file();
        void            close_file_at(size_t _fileIndex);
		File*           get_curr_file()const;
		size_t          get_file_count()const;
		File*           get_file_at(size_t _index)const;
        size_t          get_curr_file_index()const;
		void            set_curr_file(size_t _index);
        void            vm_run();
        void            vm_debug();
        void            vm_step_over();
        void            vm_stop();
        void            vm_reset();
        bool            vm_compile_and_load_program();

		void save_file_as(const fs_path &_path);

	private:
	    AppView*        m_view;
        AppContext*     m_context;
		bool            m_should_stop;
		size_t          m_current_file_index;
        std::vector<File*> m_loaded_files;
        fs_path            m_executable_folder_path;
        fs_path            m_assets_folder_path;

        void            handle_events();
	};
}
