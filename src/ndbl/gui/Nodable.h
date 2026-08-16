#pragma once

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
    struct File;

    struct App_State
    {
        tools::App_State    base;
        Config*             config              = nullptr;
        File*               current_file        = nullptr;
        Nodlang*            language            = nullptr;
        u8_t                untitled_file_count = 0;
        std::vector<File*>  files;
        std::vector<File*>  files_to_delete;
    };

    // common

    App_State*      app_init();
    void            app_shutdown();
    App_State*      app_state();
    void            app_run();
    void            app_update();
    void            app_draw();
    bool            app_should_stop();
    void            app_do_frame();

    // file related

    File*           app_open_asset_file(const tools::Path&);
    File*           app_open_file(const tools::Path&);
    File*           app_new_file();
    void            app_save_file(File*);
    void            app_set_current_file(File*);
    void            app_save_file_as(File*, const tools::Path&);
    File*           app_add_file(File*);
    void            app_close_file();
    void            app_close_file(File*);
    void            app_reset_current_graph();
}
