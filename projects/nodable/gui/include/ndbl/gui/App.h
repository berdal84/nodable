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
        App(const App&) = delete;       // Avoid copy (single instance only)
        ~App();

        observe::Event<>  after_init;   // Triggered just after app initialize, for custom code
        fw::App           framework;    // The underlying framework (we use composition instead of inheritance)
        Config            config;       // Nodable configuration (includes framework configuration)

        // wrapped framework application
        int             run() { return framework.run(); }
        bool            is_fullscreen();
        void            toggle_fullscreen();
        bool            open_file(const ghc::filesystem::path& _filePath);
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
        bool            on_init();
        bool            on_shutdown();
        void            on_update();
        bool            pick_file_path(std::string &out, fw::AppView::DialogType type);
        void            set_splashscreen_visible(bool b);
    private:
        static App*     s_instance;
        File*           m_current_file;
        size_t          m_current_file_index;
        AppView*        m_view;
        std::vector<File*> m_loaded_files;

    };
}
