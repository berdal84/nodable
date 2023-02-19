#pragma once

#include <future>
#include <memory>
#include <string>

#include <fw/core/reflection/reflection>
#include <fw/gui/App.h>

#include <ndbl/gui/AppView.h>
#include <ndbl/gui/Config.h>
#include <ndbl/gui/types.h>
#include <ndbl/core/VirtualMachine.h>
#include <ndbl/core/language/Nodlang.h>

namespace ndbl
{
    // forward declarations
    class File;
    class AppView;

    // Nodable application
    // - Only a single instance can exist at the same time
    // - Instantiate it as you want (stack or heap)
    // - The instance will be available statically via: App* App::get_instance()
    // - Is based on fw::App, but extends it using composition instead of inheritance
    class App
	{
	public:
		App();
        App(const App&) = delete;          // Avoid copy (single instance only)
        ~App();

        observe::Event<>  after_init;      // Triggered just after app initialize, for custom code
        fw::App           framework;       // The underlying framework (we use composition instead of inheritance)
        Config            config;          // Nodable configuration (includes framework configuration)
        AppView           view;
        File*             current_file;
        VirtualMachine    virtual_machine; // Virtual Machine to compile/debug/run/pause/... programs

        int             run() { return framework.run(); } // run app main loop
        bool            is_fullscreen() const;
        void            toggle_fullscreen();

        // File related:

        File*           open_file(const ghc::filesystem::path&_path);
        File*           new_file();
        void            save_file(File *pFile) const;
        void            save_file_as(const ghc::filesystem::path &_path) const;
        File*           open_file(File *_file);
        void            close_file(File*);
        bool            is_current(const File* _file ) const { return current_file == _file; }
        const std::vector<File*>&
                        get_files() const { return m_loaded_files; }
        bool            has_files() const { return !m_loaded_files.empty(); }

        // Virtual Machine related:

        void            run_program();
        void            debug_program();
        void            step_over_program();
        void            stop_program();
        void            reset_program();
        bool            compile_and_load_program();

        static App&     get_instance();             // singleton pattern
    private:
        bool            on_init();
        bool            on_shutdown();
        void            on_update();
        bool            pick_file_path(std::string &out, fw::AppView::DialogType type);
        static App*        s_instance;
        std::vector<File*> m_loaded_files;
    };
}
