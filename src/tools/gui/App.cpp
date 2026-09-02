#include "App.h"

#include "core/Asserts.h"
#include "tools/core/Task_Manager.h"

#include "App_View.h"
#include "Config.h"

#define VERIFY_APPSTATE_IS_INITIALIZED() VERIFY(tools::g_app_state != nullptr, "App_State is not initialized, did you call nodableview_init() ?")

// private
namespace tools
{
    static App_State* g_app_state = {};
}

tools::App_State* tools::app_state()
{
    VERIFY_APPSTATE_IS_INITIALIZED();
    return g_app_state;
}

void tools::app_init(App_State* app)
{
    // Create and initialize a view
    auto* view = bdc::memory_new<App_View_State>();
    appview_init(view, app);
    app->flags |= App_Flag_OWNS_VIEW_MEMORY;

    // Initialize a config
    Config* config = config_init();
    app->flags |= App_Flag_OWNS_CONFIG_MEMORY;

    // Perform additional initialization
    app_init_ex(app, view, config);
}

void tools::app_init_ex(App_State* app, App_View_State* view, Config* config)
{
    // Guards
    VERIFY(app->view == nullptr, "A view already exist. Did you call set_name twice?");
    VERIFY(app->config == nullptr, "A config already exist. Did you call set_name twice?");
    VERIFY(config != nullptr, "You must provide a config");
    VERIFY(view != nullptr, "You must provide a view");

    // Store existing data
    app->view   = view;
    app->config = config;

    // Initialize managers
    task_manager_init();

    g_app_state = app;
}

void tools::app_main_loop()
{   
    App_State* app = app_state();

    while( !app_should_stop() )
    {
        app_update();
        appview_begin(app->view);        
        //
        // Insert any ImGui code here
        //
        appview_end(app->view);
    }
}

void tools::app_shutdown()
{
    App_State* app = app_state();

    TOOLS_LOG(tools::Verbosity_Message, "tools::BaseApp", "Shutting down ...\n");

    // Optionally shutdown view
    if (app->flags & App_Flag_OWNS_VIEW_MEMORY )
    {
        appview_deinit(app->view);
    }

    // Optionally shutdown config
    if (app->flags & App_Flag_OWNS_CONFIG_MEMORY )
    {
        ASSERT(app->config != nullptr);
        config_shutdown();
        app->config = nullptr;
    }

    // managers
    task_manager_shutdown();

    TOOLS_LOG(tools::Verbosity_Message, "tools::BaseApp", "Shutdown OK\n");
}

void tools::app_update()
{
    App_State* app = app_state();
    appview_update(app->view);
    task_manager_update();
} 

bool tools::app_should_stop()
{
    App_State* app = app_state();
    return app->flags & App_Flag_SHOULD_STOP;
}

void tools::app_request_stop()
{
    App_State* app = app_state();
    app->flags |= App_Flag_SHOULD_STOP;
}
