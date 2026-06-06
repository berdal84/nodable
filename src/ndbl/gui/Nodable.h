#pragma once

#include "gui/App_View.h"
#include "tools/gui/App.h"
#include "Config.h"

namespace tools
{
    class Path;
    struct Texture;
}

namespace ndbl
{
    // forward declarations
    class Nodlang;
    class File;

    struct App_View_State : public tools::App_View_State
    {
        tools::Texture*     logo                            = nullptr;
        bool                show_properties_editor          = false;
        bool                show_imgui_demo                 = false;
        bool                show_advanced_node_properties   = false;
        bool                scroll_to_curr_instr            = true;
    };

    struct App_State : public tools::App_State
    {
        App_View_State*     view              = nullptr;
        Config*             config            = nullptr;
        File*               current_file      = nullptr;
        Nodlang*            language          = nullptr;
        u8_t                untitled_file_count = 0;
        std::vector<File*>  loaded_files;
        std::vector<File*>  flagged_to_delete_file;
    };

    // common

    void            nodable_init(App_State*);
    App_State*      nodable_state();
    void            nodable_run(App_State*);
    void            nodable_update(App_State*);
    void            nodable_draw(App_State*);
    void            nodable_shutdown(App_State*);
    bool            nodable_should_stop(const App_State*);
    void            nodable_do_frame(App_State*);

    // file related

    File*           nodable_open_asset_file(App_State*, const tools::Path&);
    File*           nodable_open_file(App_State*,const tools::Path&);
    File*           nodable_new_file(App_State*);
    void            nodable_save_file(const App_State*, File*);
    void            nodable_set_current_file(App_State*, File*);
    void            nodable_save_file_as(const App_State*, File*, const tools::Path&);
    File*           nodable_add_file(App_State*, File*);
    void            nodable_close_file(App_State*);
    void            nodable_close_file(App_State*, File*);
    void            nodable_reset_current_graph(App_State*);

    // secondary draw functions

    void            nodable_draw_file_info_window(App_State*);
    void            nodable_draw_file_window(App_State*, ImGuiID dockspace_id, bool redock_all, File*file);
    void            nodable_draw_help_window(const App_State*);
    void            nodable_draw_imgui_config_window(App_State*);
    bool            nodable_draw_node_properties_window(App_State*);
    void            nodable_draw_config_window(App_State*);
    void            nodable_draw_startup_window(App_State*, ImGuiID dockspace_id);
    void            nodable_draw_toolbar_window(App_State*);

    // 
    void            _nodable_on_draw_splashscreen_content();
    void            _nodable_on_reset_layout();
}
