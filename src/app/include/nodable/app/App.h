#pragma once

// std
#include <string>
#include <memory>
#include <future>

// Nodable
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/reflection/R.h>
#include <nodable/app/types.h>
#include <nodable/app/Settings.h>
#include <nodable/core/Language.h>
#include <nodable/core/Texture.h>
#include "IAppCtx.h"

namespace Nodable
{
    // forward declarations
    class AppView;
    class File;

	class App : public IAppCtx
	{
	public:

		App();
		~App() override;

		bool            init();
		void            shutdown();
		void            update();
		void            draw();
        bool            should_stop() const { return m_should_stop; }

        // IAppCtx implementation
        bool            open_file(const fs_path& _filePath)override;
        File*           new_file()override;
        void            save_file()const override;
        void            save_file_as(const fs_path &_path) override;
        void            close_file(File*) override;
        bool            is_current(const File* _file ) const override { return m_current_file == _file; }
        void            current_file(File *_file) override;
        const Files&    get_files() const override { return m_loaded_files; }
        bool            has_files() const override { return !m_loaded_files.empty(); }
        File*           current_file()const override;
        void            run_program() override;
        void            debug_program() override;
        void            step_over_program() override;
        void            stop_program() override;
        void            reset_program() override;
        bool            compile_and_load_program() override;
        void            flag_to_stop() override;
        u64_t           elapsed_time() const override { return m_start_time.time_since_epoch().count(); };
        std::string     compute_asset_path(const char *_relative_path) const override;
        Settings&       settings() override { return m_settings; }
        VirtualMachine& virtual_machine() override { return m_vm; }
        Language&       language() override { return *m_language.get(); }
        const Language& language() const override { return *m_language.get(); }
        TextureManager& texture_manager() override { return m_texture_manager; };

    private:
        const std::chrono::time_point<std::chrono::system_clock>
                        m_start_time = std::chrono::system_clock::now();

        R::Initialiser  m_reflect;
        Settings        m_settings;
        VirtualMachine  m_vm;
        TextureManager  m_texture_manager;
        bool            m_should_stop;
        size_t          m_current_file_index;
        File*           m_current_file;
        fs_path         m_executable_folder_path;
        fs_path         m_assets_folder_path;
        std::vector<File*>        m_loaded_files;
        std::unique_ptr<Language> m_language;
        std::unique_ptr<AppView>  m_view;

        void            handle_events();

    };
}
