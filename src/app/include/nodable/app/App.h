#pragma once

// std
#include <string>
#include <memory>
#include <future>

// Nodable
#include <nodable/core/VM.h>
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
        bool            is_debugging_program() const override { return m_vm.is_debugging(); }
        bool            is_next_node_in_program(Node* _node) const override { return m_vm.get_next_node() == _node; }
        bool            open_file(const fs_path& _filePath)override;
        File*           new_file()override;
        void            save_file()const override;
        void            save_file_as(const fs_path &_path) override;
        void            close_file(File*) override;
        bool            is_current(const File* _file ) const override { return m_current_file == _file; }
        void            set_curr_file(size_t _index) override;
        void            set_curr_file(File *_file) override;
        const std::vector<File*>& get_files() const override { return m_loaded_files; }
        bool            has_files() const override { return !m_loaded_files.empty(); }
        File*           get_curr_file()const override;
        bool            is_running_program() const override { return m_vm.is_program_running(); }
        void            run_program() override;
        void            debug_program() override;
        void            step_over_program() override;
        void            stop_program() override;
        void            reset_program() override;
        bool            compile_and_load_program() override;
        void            flag_to_stop() override;
        u64_t           get_elapsed_time() const override { return m_start_time.time_since_epoch().count(); };
        std::string     get_absolute_asset_path(const char* _relative_path)const override;
        Settings&       get_settings() override { return m_settings; }
        vm::VM&         get_vm() override { return m_vm; }
        Language&       get_language() override { return *m_language.get(); }
        const Language& get_language() const override { return *m_language.get(); }
        TextureManager& get_texture_manager() override { return m_texture_manager; };

    private:
        const std::chrono::time_point<std::chrono::system_clock>
                        m_start_time = std::chrono::system_clock::now();

        R::Initialiser  m_reflect;
        Settings        m_settings;
        vm::VM          m_vm;
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
