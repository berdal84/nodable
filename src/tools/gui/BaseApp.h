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
    /*
     * Base Application class
     * See /project/tools/gui-example for usage
     */
	class BaseApp
    {
        using fs_path = std::filesystem::path;
	public:
        BaseApp(AppView*);
        BaseApp(const BaseApp&) = delete;
        ~BaseApp();

        bool               should_stop;           // Set this field true to tell the application to stop its main loop the next frame
        AppView*           view;                 // non-owned ptr

        virtual void       init();
        virtual void       shutdown();
        virtual void       update();
        virtual void       draw(); // Consider overriding AppView::draw instead of App::draw

        static double      elapsed_time() ;  // Get the elapsed time in seconds
        static fs_path     asset_path(const fs_path&); // get asset's absolute path (relative path will be converted)
        static fs_path     asset_path(const char*); // get asset's absolute path (relative path will be converted)
    };
}
