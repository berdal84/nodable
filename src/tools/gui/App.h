#pragma once

namespace tools
{
    struct Task_Manager;
    struct Config;
    struct App_View_State;

    typedef int AppFlags;
    enum AppFlag_
    {
        AppFlag_NONE               = 0,
        AppFlag_OWNS_CONFIG_MEMORY = 1 << 0, // Since some data (view and config) might be owned or not, those flags are there to keep track of it.
        AppFlag_OWNS_VIEW_MEMORY   = 1 << 1, // ... same ...
        AppFlag_SHOULD_STOP        = 1 << 2  // when set, app will stop next frame.
    };

    struct App_State
    {
        AppFlags        flags           = AppFlag_NONE;
        Config*         config          = nullptr; // owned or not depending on flags
        App_View_State* view            = nullptr; // owned or not depending on flags
        Task_Manager*   task_manager    = nullptr;
    };

    void    app_init(App_State* app);
    void    app_init_ex(App_State* app, App_View_State* , Config*);
    void    app_main_loop(App_State* app);
    void    app_deinit(App_State* app);
    void    app_update(App_State* app);
    void    app_draw(App_State* app); // Consider overriding AppView::draw instead of App::draw
    inline bool app_should_stop(const App_State* app) { return app->flags & AppFlag_SHOULD_STOP; }
    inline void app_request_stop(App_State* app) { app->flags |= AppFlag_SHOULD_STOP; }
}
