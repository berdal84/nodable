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
     * Application Framework
     * See /project/framework/example for usage
     */
	class App
    {
        using fs_path = std::filesystem::path;
	public:
        App(AppView*);
        App(const App &) = delete;
        ~App();

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
