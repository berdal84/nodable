#include <fw/gui/App.h>

#include <algorithm>
#include <fw/gui/AppView.h>
#include <fw/gui/Event.h>

using namespace fw;

App::App(fs_path _assets_folder_path, AppView* _view)
    : m_should_stop(false)
    , m_assets_folder_path( _assets_folder_path )
    , m_view(_view)
{

    LOG_MESSAGE("App", "Asset folder path:      %s\n", m_assets_folder_path.c_str() )
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
    LOG_MESSAGE("App", "Shutting down ...\n")
    success &= m_view->shutdown();
    success &= onShutdown();
    LOG_MESSAGE("App", "Shutdown %s\n", success ? OK : KO)
    return success;
}

std::string App::compute_asset_path(const char* _relative_path) const
{
    fs_path result = m_assets_folder_path / _relative_path;
	return result.string();
}

void App::draw()
{
    m_view->draw();
}
