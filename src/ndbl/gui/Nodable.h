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

    struct NodableState
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

        tools::AppState     base;
        View*               view              = nullptr;
        Config*             config            = nullptr;
        File*               current_file      = nullptr;
        Nodlang*            language          = nullptr;
        u8_t                untitled_file_count = 0;
        std::vector<File*>  loaded_files;
        std::vector<File*>  flagged_to_delete_file;
    };

    // common

    void            nodable_init(NodableState*);
    NodableState*   nodable_state();
    void            nodable_run(NodableState*);
    void            nodable_update(NodableState*);
    void            nodable_draw(NodableState*);
    void            nodable_shutdown(NodableState*);
    bool            nodable_should_stop(const NodableState*);
    void            nodable_do_frame(NodableState*);

    // file related

    File*           nodable_open_asset_file(NodableState*, const tools::Path&);
    File*           nodable_open_file(NodableState*,const tools::Path&);
    File*           nodable_new_file(NodableState*);
    void            nodable_save_file(const NodableState*, File*);
    void            nodable_set_current_file(NodableState*, File*);
    void            nodable_save_file_as(const NodableState*, File*, const tools::Path&);
    File*           nodable_add_file(NodableState*, File*);
    void            nodable_close_file(NodableState*);
    void            nodable_close_file(NodableState*, File*);
    void            nodable_reset_current_graph(NodableState*);

    // secondary draw functions

    void            nodable_draw_file_info_window(NodableState*);
    void            nodable_draw_file_window(NodableState*, ImGuiID dockspace_id, bool redock_all, File*file);
    void            nodable_draw_help_window(const NodableState*);
    void            nodable_draw_imgui_config_window(NodableState*);
    bool            nodable_draw_node_properties_window(NodableState*);
    void            nodable_draw_config_window(NodableState*);
    void            nodable_draw_startup_window(NodableState*, ImGuiID dockspace_id);
    void            nodable_draw_toolbar_window(NodableState*);

    // 
    void            _nodable_on_draw_splashscreen_content();
    void            _nodable_on_reset_layout();
}
