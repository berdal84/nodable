#include "App.h"

#include "core/assertions.h"
#include "tools/core/TaskManager.h"
#include "tools/core/System.h"

#include "AppView.h"
#include "Config.h"
#include "ImGuiEx.h"
#include "TextureManager.h"

using namespace tools;

void tools::app_init(AppState* app)
{
    // Create and initialize a view
    auto* view = new AppViewState();
    appview_init(view, app);
    app->flags |= AppFlag_OWNS_VIEW_MEMORY;

    // Initialize a config
    Config* config = init_config();
    app->flags |= AppFlag_OWNS_CONFIG_MEMORY;

    // Perform additional initialization
    app_init_ex(app, view, config);
}

void tools::app_init_ex(AppState* app, AppViewState* view, Config* config)
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
    app->task_manager = init_task_manager();
}

void tools::app_main_loop(AppState* app)
{    
    while( !app_should_stop(app) )
    {
        app_update(app);

        appview_begin(app->view);
        
        //
        // Insert any ImGui code here
        //

        appview_end(app->view);
    }
}

void tools::app_shutdown(AppState* app)
{
    TOOLS_LOG(tools::Verbosity_Message, "tools::BaseApp", "Shutting down ...\n");

    // Optionally shutdown view
    if (app->flags & AppFlag_OWNS_VIEW_MEMORY )
    {
        appview_shutdown(app->view);
    }

    // Optionally shutdown config
    if (app->flags & AppFlag_OWNS_CONFIG_MEMORY )
    {
        ASSERT(app->config != nullptr);
        shutdown_config(app->config);
        app->config = nullptr;
    }

    // managers
    shutdown_task_manager(app->task_manager);

    TOOLS_LOG(tools::Verbosity_Message, "tools::BaseApp", "Shutdown OK\n");
}

void tools::app_update(AppState* app)
{
    appview_update(app->view);
    app->task_manager->update();
}