#pragma once

// std
#include <future>
#include <memory>
#include <string>

// Nodable
#include <nodable/app/AppView.h>
#include <nodable/app/IAppCtx.h>
#include <nodable/app/Settings.h>
#include <nodable/app/types.h>
#include <nodable/core/ILanguage.h>
#include <nodable/core/Texture.h>
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/reflection/reflection>

namespace ndbl
{
    // forward declarations
    class File;

	class App : public IAppCtx
	{
	public:

		App();
		~App() override = default;

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
        ILanguage&       language() override { return *m_language.get(); }
        const ILanguage& language() const override { return *m_language.get(); }
        TextureManager& texture_manager() override { return m_texture_manager; };

    private:
        const std::chrono::time_point<std::chrono::system_clock>
                        m_start_time = std::chrono::system_clock::now();

        AppView         m_view;
        File*           m_current_file;
        Settings        m_settings;
        TextureManager  m_texture_manager;
        VirtualMachine  m_vm;
        bool            m_should_stop;
        fs_path         m_assets_folder_path;
        size_t          m_current_file_index;
        std::unique_ptr<ILanguage> m_language;
        std::vector<std::unique_ptr<File>> m_loaded_files;

        void            handle_events();

    };
}
