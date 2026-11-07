#pragma once

#include "Try_Catch.h" // for main.cpp's try/catch
#include "File_System.h"
#include "bdc/Array.hpp"

namespace ndbl
{
    // forward declarations
    struct File;

    typedef int App_Flags;
    enum App_Flag_ : int
    {
        App_Flag_NONE        = 0,
        App_Flag_SHOULD_STOP = 1 << 2  // when set, app will stop next frame.
    };

    struct App_State
    {
        App_Flags                   flags               = App_Flag_NONE;
        File*                       current_file        = nullptr;
        u8_t                        untitled_file_count = 0;
        bdc::Resizable_Array<File*> files;
        bdc::Resizable_Array<File*> files_to_delete;
    };

    // common

    App_State*      app_init();
    void            app_shutdown();
    App_State*      app_state();
    App_State*      app_state();
    bool            app_should_stop();
    void            app_do_frame();
    void            app_draw();
    void            app_run();
    void            app_update();

    // file related

    File*           app_open_asset_file(const Path&);
    File*           app_open_file(const Path&);
    File*           app_new_file();
    void            app_save_file(File*);
    void            app_set_current_file(File*);
    void            app_save_file_as(File*, const Path&);
    File*           app_add_file(File*);
    void            app_close_current_file();
    void            app_close_file(File*);
    void            app_reset_current_graph();
}
