#pragma once

#include "gui/AppView.h"
#include "tools/gui/App.h"
#include "Config.h"

namespace tools
{
    class Path;
}

namespace ndbl
{
    // forward declarations
    class Nodlang;
    class File;

    class Nodable
    {
        struct View
        {
            tools::AppViewState base{};
            tools::Texture*     logo                            = nullptr;
            bool                show_properties_editor          = false;
            bool                show_imgui_demo                 = false;
            bool                show_advanced_node_properties   = false;
            bool                scroll_to_curr_instr            = true;
        };
        
	public:

        // Common

        void            init();
        void            run();
        void            update();
        void            draw();
        void            shutdown();
        bool            should_stop() const;
        View*           get_view() const;
        void            _do_frame();

        // Files

        File*           open_asset_file(const tools::Path&);
        File*           open_file(const tools::Path&);
        File*           new_file();
        void            save_file(File*) const;
        void            set_current_file(File*);
        void            save_file_as(File*, const tools::Path&) const;
        File*           add_file(File*);
        void            close_file(File*);
        File*           get_current_file() { return m_current_file; };
        bool            is_current(const File* _file) const { return m_current_file == _file; }
        const std::vector<File*>&
                        get_files() const { return m_loaded_files; }
        bool            has_files() const { return !m_loaded_files.empty(); }
        void            reset_current_graph();
        void            show_splashscreen(bool b = true );

        static Nodable* instance() { return s_instance; }

    private:
        void            on_reset_layout();
        void            on_draw_splashscreen_content();
        void            draw_file_info_window(float dt);
        void            draw_file_window(float dt, ImGuiID dockspace_id, bool redock_all, File*file);
        void            draw_help_window(float dt) const;
        void            draw_imgui_config_window(float dt);
        bool            draw_node_properties_window(float dt);
        void            draw_config_window(float dt);
        void            draw_startup_window(float dt, ImGuiID dockspace_id);
        void            draw_toolbar_window(float dt);

        static Nodable*    s_instance;
        tools::AppState    m_base_app;
        View*              m_view              = nullptr;
        Config*            m_config            = nullptr;
        File*              m_current_file      = nullptr;
        Nodlang*           m_language          = nullptr;
        u8_t               m_untitled_file_count = 0;
        std::vector<File*> m_loaded_files;
        std::vector<File*> m_flagged_to_delete_file;
    };
}
