#pragma once

#include <future>
#include <memory>
#include <string>
#include <filesystem>
#include <observe/event.h>
#include "tools/core/types.h"
#include "AppView.h"

namespace tools
{
    class PoolManager;
    class TextureManager;
    struct TaskManager;

    typedef int BaseAppFlags;
    enum BaseAppFlag_
    {
        BaseAppFlag_DEFAULT     = 0,
        BaseAppFlag_SKIP_VIEW   = 1 << 0, // Skip View init/shutdown() OFF
        BaseAppFlag_SKIP_CONFIG = 1 << 1, // Skip Config init/shutdown() OFF
    };

    /*
     * Base Application class
     * See /project/tools/gui-example for usage
     */
	class BaseApp
    {
        using fs_path = std::filesystem::path;
	public:
        BaseApp() = default;
        virtual ~BaseApp() = default;

    public:
        bool should_stop  = false; // Set this field true to tell the application to stop its main loop the next frame

        virtual void       init(AppView* _view) { init(_view, BaseAppFlag_DEFAULT); }
        virtual void       init(AppView*, BaseAppFlags);
        virtual void       shutdown();
        virtual void       update();
        virtual void       draw(); // Consider overriding AppView::draw instead of App::draw

        static double      elapsed_time() ;  // Get the elapsed time in seconds
        static fs_path     asset_path(const fs_path&); // get asset's absolute path (relative path will be converted)
        static fs_path     asset_path(const char*); // get asset's absolute path (relative path will be converted)

    protected:
        AppView*        m_view = nullptr; // non-owned ptr, user is responsible for it.
        PoolManager*    m_pool_manager = nullptr;
        TaskManager*    m_task_manager = nullptr;
        TextureManager* m_texture_manager = nullptr;
    private:
        BaseAppFlags    m_flags{};
    };
}
