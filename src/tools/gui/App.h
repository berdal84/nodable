#pragma once

namespace tools
{
    class PoolManager;
    struct TaskManager;
    struct Config;
    struct AppViewState;

    typedef int AppFlags;
    enum AppFlag_
    {
        AppFlag_NONE               = 0,
        AppFlag_OWNS_CONFIG_MEMORY = 1 << 0, // Since some data (view and config) might be owned or not, those flags are there to keep track of it.
        AppFlag_OWNS_VIEW_MEMORY   = 1 << 1, // ... same ...
        AppFlag_SHOULD_STOP        = 1 << 2  // when set, app will stop next frame.
    };

    struct AppState
    {
        AppFlags        flags           = AppFlag_NONE;
        Config*         config          = nullptr; // owned or not depending on flags
        AppViewState*   view            = nullptr; // owned or not depending on flags
        TaskManager*    task_manager    = nullptr;
    };

    void    app_init(AppState* app);
    void    app_init_ex(AppState* app, AppViewState* , Config*);
    void    app_main_loop(AppState* app);
    void    app_shutdown(AppState* app);
    void    app_update(AppState* app);
    void    app_draw(AppState* app); // Consider overriding AppView::draw instead of App::draw
    static bool app_should_stop(const AppState* app) { return app->flags & AppFlag_SHOULD_STOP; }
    static void app_request_stop(AppState* app) { app->flags |= AppFlag_SHOULD_STOP; }
}
