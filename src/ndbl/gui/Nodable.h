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

    struct AppViewState : public tools::AppViewState
    {
        tools::Texture*     logo                            = nullptr;
        bool                show_properties_editor          = false;
        bool                show_imgui_demo                 = false;
        bool                show_advanced_node_properties   = false;
        bool                scroll_to_curr_instr            = true;
    };

    struct AppState : public tools::AppState
    {
        AppViewState*       view              = nullptr;
        Config*             config            = nullptr;
        File*               current_file      = nullptr;
        Nodlang*            language          = nullptr;
        u8_t                untitled_file_count = 0;
        std::vector<File*>  loaded_files;
        std::vector<File*>  flagged_to_delete_file;
    };

    // common

    void            nodable_init(AppState*);
    AppState*       nodable_state();
    void            nodable_run(AppState*);
    void            nodable_update(AppState*);
    void            nodable_draw(AppState*);
    void            nodable_shutdown(AppState*);
    bool            nodable_should_stop(const AppState*);
    void            nodable_do_frame(AppState*);

    // file related

    File*           nodable_open_asset_file(AppState*, const tools::Path&);
    File*           nodable_open_file(AppState*,const tools::Path&);
    File*           nodable_new_file(AppState*);
    void            nodable_save_file(const AppState*, File*);
    void            nodable_set_current_file(AppState*, File*);
    void            nodable_save_file_as(const AppState*, File*, const tools::Path&);
    File*           nodable_add_file(AppState*, File*);
    void            nodable_close_file(AppState*);
    void            nodable_close_file(AppState*, File*);
    void            nodable_reset_current_graph(AppState*);

    // secondary draw functions

    void            nodable_draw_file_info_window(AppState*);
    void            nodable_draw_file_window(AppState*, ImGuiID dockspace_id, bool redock_all, File*file);
    void            nodable_draw_help_window(const AppState*);
    void            nodable_draw_imgui_config_window(AppState*);
    bool            nodable_draw_node_properties_window(AppState*);
    void            nodable_draw_config_window(AppState*);
    void            nodable_draw_startup_window(AppState*, ImGuiID dockspace_id);
    void            nodable_draw_toolbar_window(AppState*);

    // 
    void            _nodable_on_draw_splashscreen_content();
    void            _nodable_on_reset_layout();
}
