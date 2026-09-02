#pragma once

namespace tools
{
    struct Task_Manager;
    struct Config;
    struct App_View_State;

    typedef int App_Flags;
    enum App_Flag_ : int
    {
        App_Flag_NONE               = 0,
        App_Flag_OWNS_CONFIG_MEMORY = 1 << 0, // Since some data (view and config) might be owned or not, those flags are there to keep track of it.
        App_Flag_OWNS_VIEW_MEMORY   = 1 << 1, // ... same ...
        App_Flag_SHOULD_STOP        = 1 << 2  // when set, app will stop next frame.
    };

    struct App_State
    {
        App_Flags       flags           = App_Flag_NONE;
        Config*         config          = nullptr; // owned or not depending on flags
        App_View_State* view            = nullptr; // owned or not depending on flags
    };

    void        app_init(App_State* app);
    void        app_init_ex(App_State* app, App_View_State* , Config*);
    void        app_shutdown();
    App_State*  app_state();
    void        app_main_loop();
    void        app_update();
    void        app_draw();
    bool        app_should_stop();
    void        app_request_stop();
}
