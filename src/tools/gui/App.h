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
    struct TaskManager;
    struct Config;

    /*
     * Application class
     * Written to be wrapped. See /src/tools/gui-example for usage.
     */
	class App
    {
        typedef std::filesystem::path fs_path; // alias
	public:
        void           init(); // default init, an AppView and a Config will be created internally
        void           init_ex(AppView* , Config*); // extended init, allows to provide an existing AppView and/or Config.
        void           shutdown();
        void           update();
        void           draw(); // Consider overriding AppView::draw instead of App::draw
        inline bool    should_stop() const { return m_flags & Flag_SHOULD_STOP; }
        inline void    request_stop() { m_flags |= Flag_SHOULD_STOP; }

        static double  get_time() ;  // Get the elapsed time in seconds
        static fs_path asset_path(const fs_path&); // get asset's absolute path (relative path will be converted)
        static fs_path asset_path(const char*); // get asset's absolute path (relative path will be converted)

    protected:
        typedef int Flags;
        enum Flag_
        {
            Flag_NONE               = 0,
            Flag_OWNS_CONFIG_MEMORY = 1 << 0, // Since some data (view and config) might be owned or not, those flags are there to keep track of it.
            Flag_OWNS_VIEW_MEMORY   = 1 << 1, // ... same ...
            Flag_SHOULD_STOP        = 1 << 2  // when set, app will stop next frame.
        };
        Flags           m_flags           = Flag_NONE;
        Config*         m_config          = nullptr; // owned or not depending on m_flags
        AppView*        m_view            = nullptr; // owned or not depending on m_flags
        TaskManager*    m_task_manager    = nullptr;
    };
}
