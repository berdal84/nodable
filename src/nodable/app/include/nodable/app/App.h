#pragma once

#include <future>
#include <memory>
#include <string>

#include <fw/reflection/reflection>
#include <fw/imgui/App.h>

#include <nodable/app/AppView.h>
#include <nodable/app/Settings.h>
#include <nodable/app/types.h>
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/language/Nodlang.h>

namespace ndbl
{
    // forward declarations
    class File;
    class AppView;

    class App : public fw::App
	{
	public:
		App();

    protected:
		bool            onInit() override;
		void            onShutdown() override;
		void            onUpdate() override;
    public:
        // IAppCtx implementation
        bool            open_file(const fs_path& _filePath);
        File*           new_file();
        void            save_file()const;
        void            save_file_as(const fs_path &_path);
        void            close_file(File*);
        bool            is_current(const File* _file ) const { return m_current_file == _file; }
        void            current_file(File *_file);
        const std::vector<File*>& get_files() const { return m_loaded_files; }
        bool            has_files() const { return !m_loaded_files.empty(); }
        File*           current_file()const;
        void            run_program();
        void            debug_program();
        void            step_over_program();
        void            stop_program();
        void            reset_program();
        bool            compile_and_load_program();
        static App&     get_instance();

    private:
        File*           m_current_file;
        VirtualMachine  m_vm;
        size_t          m_current_file_index;
        std::vector<File*> m_loaded_files;
    };
}
