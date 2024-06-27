#include "App.h"

#include "tools/core/TaskManager.h"
#include "tools/core/memory/memory.h"
#include "tools/core/system.h"

#include "AppView.h"
#include "Config.h"
#include "ImGuiEx.h"
#include "TextureManager.h"

using namespace tools;

void App::init()
{
    // Create and initialize a view
    auto* view = new AppView();
    view->init(this);
    m_flags |= Flag_OWNS_VIEW_MEMORY;

    // Initialize a config
    Config* config = init_config();
    m_flags |= Flag_OWNS_CONFIG_MEMORY;

    // Perform additional initialization
    init_ex(m_view, config);
}

void App::init_ex(AppView* _view, Config* _config)
{
    // Guards
    EXPECT(m_view == nullptr, "A view already exist. Did you call init twice?")
    EXPECT(m_config == nullptr, "A config already exist. Did you call init twice?")
    EXPECT(_config != nullptr, "You must provide a config")
    EXPECT(_view != nullptr, "You must provide a view")

    // Store existing data
    m_view   = _view;
    m_config = _config;

    // Initialize managers
    m_task_manager    = init_task_manager();
}

void App::shutdown()
{
    LOG_MESSAGE("tools::BaseApp", "Shutting down ...\n");

    // Optionally shutdown view
    if (m_flags & Flag_OWNS_VIEW_MEMORY )
    {
        m_view->shutdown();
    }

    // Optionally shutdown config
    if (m_flags & Flag_OWNS_CONFIG_MEMORY )
    {
        ASSERT(m_config != nullptr)
        shutdown_config(m_config);
    }

    // managers
    shutdown_task_manager(m_task_manager);

    LOG_MESSAGE("tools::BaseApp", "Shutdown OK\n");
}

void App::update()
{
    m_view->update();
    m_task_manager->update();
}

void App::draw()
{
    m_view->begin_draw();
    //
    // You can add some ImGui stuff here to debug
    //
    m_view->end_draw();
}

double App::get_time()
{
    return ImGui::GetTime();
}

std::filesystem::path App::asset_path(const fs_path& _path)
{
    EXPECT(!_path.is_absolute(), "_path is not relative, this can't be an asset")
    fs_path executable_dir = system::get_executable_directory();
    return executable_dir / "assets" / _path;
}

std::filesystem::path App::asset_path(const char* _path)
{
    fs_path fs_path{_path};
    return  fs_path.is_absolute() ? fs_path
                                  : asset_path(fs_path);
}
