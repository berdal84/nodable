#include <fw/gui/App.h>

#include <fw/core/System.h>
#include <algorithm>
#include <fw/gui/AppView.h>
#include <fw/gui/Event.h>

using namespace fw;

App::App(AppView* _view)
    : m_should_stop(false)
    , m_view(_view)
{
}

bool App::init()
{
    LOG_MESSAGE("App", "Initializing App...\n")
    if (!m_view->init())
    {
        LOG_ERROR("App", "Initialization failed!\n");
        return false;
    }

    return onInit();
}

void App::update()
{
    m_view->handle_events();
    return onUpdate();
}

void App::flag_to_stop()
{
    m_should_stop = true;
}

bool App::shutdown()
{
    bool success = true;
    LOG_MESSAGE("App", "Shutting down ...\n");
    success &= m_view->shutdown();
    success &= onShutdown();
    LOG_MESSAGE("App", "Shutdown %s\n", success ? OK : KO)
    return success;
}

std::string App::to_absolute_asset_path(const char* _relative_path)
{
    static ghc::filesystem::path assets_folder_path = fw::System::get_executable_directory();
	return (assets_folder_path / "assets" / _relative_path).string();
}

void App::draw()
{
    m_view->on_draw();
}

u64_t App::elapsed_time() const
{
    return m_start_time.time_since_epoch().count();
}
