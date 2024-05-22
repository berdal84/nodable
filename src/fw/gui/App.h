#pragma once

#include <future>
#include <memory>
#include <string>
#include <filesystem>

#include <observe/event.h>

#include "fw/core/types.h"

#include "FontManager.h"
#include "AppView.h"
#include "TextureManager.h"
#include "EventManager.h"
#include "ActionManager.h"

namespace fw
{
    /*
     * Application Framework
     * See /project/framework/example for usage
     */
	class App
    {
	public:
        App(AppView*);
        App(const App &) = delete;
        ~App();

        TextureManager   texture_manager;       // Manages Texture resources
        FontManager      font_manager;          // Manages Font resources
        EventManager     event_manager;         // Manages Events and BindedEvents (shortcuts/button triggered)
        ActionManager    action_manager;        // Manages Events and BindedEvents (shortcuts/button triggered)
        bool             should_stop;           // Set this field true to tell the application to stop its main loop the next frame

    protected:
        AppView*         m_view;                 // non-owned ptr

        // virtual methods user can override

        virtual bool on_init() { return true; };
        virtual bool on_shutdown() { return true; };
        virtual void on_update() {};
        virtual void on_draw() {};

    public:
        int                main(int argc = 0, char *argv[] = nullptr); // Run the main loop
        bool               init();     // Initialize the application
        bool               shutdown(); // Shutdown the application
        void               update();   // Update the application
        void               draw();     // Draw the application's view
        double             elapsed_time() const;  // Get the elapsed time in seconds
        void               handle_events();
        bool               is_fullscreen() const;
        void               set_fullscreen(bool b);
        void               save_screenshot(const char*); // Save current view as PNG file to a given path (relative or absolute)

        static int         fps();      // get the current frame per second (un-smoothed)
        static std::filesystem::path asset_path(const std::filesystem::path&); // get asset's absolute path (relative path will be converted)
        static std::filesystem::path asset_path(const char*);            // get asset's absolute path (relative path will be converted)
    private:
        const std::chrono::time_point<std::chrono::system_clock>
                        m_start_time = std::chrono::system_clock::now();
        SDL_GLContext   m_sdl_gl_context;
        SDL_Window*     m_sdl_window;

        static App *     s_instance;

    };
}
