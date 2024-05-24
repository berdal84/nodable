#include "BaseApp.h"

#include "AppView.h"
#include "Config.h"
#include "ImGuiEx.h"
#include "tools/core/async.h"
#include "tools/core/system.h"

using namespace tools;

static BaseApp*s_instance = nullptr;

BaseApp::BaseApp(AppView* _view)
    : should_stop(false)
    , view(_view)
{
    LOG_VERBOSE("tools::App", "Constructor ...\n");
    EXPECT( view, "View cannot be null");
    EXPECT(s_instance == nullptr, "Only a single tools::App at a time allowed");
    s_instance = this;
    LOG_VERBOSE("tools::App", "Constructor " OK "\n");
}

BaseApp::~BaseApp()
{
    LOG_VERBOSE("tools::App", "Destructor ...\n");
    s_instance = nullptr;
    LOG_VERBOSE("tools::App", "Destructor " OK "\n");
}

void BaseApp::init()
{
    LOG_VERBOSE("tools::App", "init ...\n");

    if ( !tools::has_config())
    {
        tools::init_config();
    }

    init_pool_manager();
    init_task_manager();
    init_texture_manager();
    view->init();

    LOG_VERBOSE("tools::App", "init " OK "\n");
}

void BaseApp::shutdown()
{
    LOG_MESSAGE("tools::App", "Shutting down ...\n");
    // n.b: use inverse order of init()
    view->shutdown();
    shutdown_texture_manager();
    shutdown_task_manager();
    shutdown_pool_manager();
    tools::shutdown_config();
    LOG_MESSAGE("tools::App", "Shutdown OK\n");
}

void BaseApp::update()
{
    LOG_VERBOSE("tools::App", "update ...\n");
    view->update();
    update_task_manager();
    LOG_VERBOSE("tools::App", "update " OK "\n");
}

void BaseApp::draw()
{
    view->draw();
}

double BaseApp::elapsed_time()
{
    return ImGui::GetTime();
}

std::filesystem::path BaseApp::asset_path(const std::filesystem::path& _path)
{
    EXPECT(!_path.is_absolute(), "_path is not relative, this can't be an asset")
    auto executable_dir = tools::system::get_executable_directory();
    return executable_dir / "assets" / _path;
}

std::filesystem::path BaseApp::asset_path(const char* _path)
{
    std::filesystem::path fs_path{_path};
    return  fs_path.is_absolute() ? fs_path
                                  : asset_path(fs_path);
}
