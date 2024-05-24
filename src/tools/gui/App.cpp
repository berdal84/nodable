#include "App.h"

#include "AppView.h"
#include "Config.h"
#include "ImGuiEx.h"
#include "tools/core/async.h"
#include "tools/core/system.h"

using namespace tools;

static App *s_instance = nullptr;

App::App(AppView* _view)
    : should_stop(false)
    , view(_view)
{
    LOG_VERBOSE("tools::App", "Constructor ...\n");
    EXPECT( view, "View cannot be null");
    EXPECT(s_instance == nullptr, "Only a single tools::App at a time allowed");
    s_instance = this;
    LOG_VERBOSE("tools::App", "Constructor " OK "\n");
}

App::~App()
{
    LOG_VERBOSE("tools::App", "Destructor ...\n");
    s_instance = nullptr;
    LOG_VERBOSE("tools::App", "Destructor " OK "\n");
}

void App::init()
{
    LOG_VERBOSE("tools::App", "init ...\n");

    // Initialize a configuration only when necessary
    // TODO: consider having a unique context per app (see ImGui's context management)
    if ( tools::get_config() == nullptr) tools::create_config();

    async::init();
    Pool::init();
    init_texture_manager();
    view->init();
    LOG_VERBOSE("tools::App", "init " OK "\n");
}

void App::shutdown()
{
    LOG_MESSAGE("tools::App", "Shutting down ...\n");
    // n.b: use inverse order of init()
    view->shutdown();
    shutdown_texture_manager();
    Pool::shutdown();
    async::shutdown();
    tools::destroy_config();
    LOG_MESSAGE("tools::App", "Shutdown OK\n");
}

void App::update()
{
    LOG_VERBOSE("tools::App", "update ...\n");
    view->update();
    async::update();
    LOG_VERBOSE("tools::App", "update " OK "\n");
}

void App::draw()
{
    view->draw();
}

double App::elapsed_time() const
{
    return ImGui::GetTime();
}

std::filesystem::path App::asset_path(const std::filesystem::path& _path)
{
    EXPECT(!_path.is_absolute(), "_path is not relative, this can't be an asset")
    auto executable_dir = tools::system::get_executable_directory();
    return executable_dir / "assets" / _path;
}

std::filesystem::path App::asset_path(const char* _path)
{
    std::filesystem::path fs_path{_path};
    return  fs_path.is_absolute() ? fs_path
                                  : asset_path(fs_path);
}
