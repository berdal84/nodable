#pragma once

#include <future>
#include <memory>
#include <string>

#include <fw/core/reflection/reflection>
#include <fw/gui/App.h>

#include <ndbl/gui/AppView.h>
#include <ndbl/gui/Settings.h>
#include <ndbl/gui/types.h>
#include <ndbl/core/VirtualMachine.h>
#include <ndbl/core/language/Nodlang.h>

namespace ndbl
{
    // forward declarations
    class File;
    class AppView;

    class App : public fw::App
	{
	public:
		App();
        App(const App&) = delete;
        ~App();

        fw::AppView * get_view();

    protected:
		bool            onInit() override;
		bool onShutdown() override;
		void            onUpdate() override;
    public:
        // IAppCtx implementation
        bool            open_file(const ghc::filesystem::path& _filePath, bool relative = false);
        File*           new_file();
        void            save_file()const;
        void            save_file_as(const ghc::filesystem::path &_path);
        void            close_file(File*);
        bool            is_current(const File* _file ) const { return m_current_file == _file; }
        void            current_file(File *_file);
        const std::vector<File*>&
                        get_files() const { return m_loaded_files; }
        bool            has_files() const { return !m_loaded_files.empty(); }
        File*           current_file()const;
        void            run_program();
        void            debug_program();
        void            step_over_program();
        void            stop_program();
        void            reset_program();
        bool            compile_and_load_program();
        static App&     get_instance();             // singleton pattern

    private:
        static App*     s_instance;
        File*           m_current_file;
        size_t          m_current_file_index;
        std::vector<File*> m_loaded_files;
    };
}
