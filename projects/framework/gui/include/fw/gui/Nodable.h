#pragma once

// std
#include <future>
#include <memory>
#include <string>

#include <fw/core/types.h>
#include <fw/gui/Config.h>
#include <fw/gui/FontManager.h>
#include <fw/gui/NodableView.h>
#include <fw/gui/TextureManager.h>
#include <ghc/filesystem.hpp>
#include <observe/event.h>

namespace fw
{

    /*
     * Application Framework
     * See /project/framework/example for usage
     */
	class Nodable
    {
	public:
        Nodable(Config&);
        Nodable(const Nodable &) = delete;
        ~Nodable();

        TextureManager   texture_manager;       // Manages Texture resources
        FontManager      font_manager;          // Manages Font resources
        EventManager     event_manager;         // Manages Events and BindedEvents (shortcuts/button triggered)
        bool             should_stop;           // Set this field true to tell the application to stop its main loop the next frame
        Config&          config;                // Application configuration (names, colors, fonts)
        NodableView view;                  // Application View (based on ImGui)

        enum StateChange
        {
            ON_INIT,
            ON_UPDATE,
            ON_DRAW,
            ON_SHUTDOWN
        };
        observe::Event<StateChange> changes; // use changes.connect([](...) { /** you code here */ }); to extend behavior

        int                run();                 // Run the main loop
        u64_t              elapsed_time() const;  // Get the elapsed time in seconds
        void               handle_events();
        bool               is_fullscreen() const;
        void               set_fullscreen(bool b);
        void               save_screenshot(const char*); // Save current view as PNG file to a given path (relative or absolute)
        bool               init();     // Initialize the application
        bool               shutdown(); // Shutdown the application
        void               update();   // Update the application
        void               draw();     // Draw the application's view

        static int         fps();      // get the current frame per second (un-smoothed)
        static ghc::filesystem::path asset_path(const ghc::filesystem::path&); // get asset's absolute path (relative path will be converted)
        static ghc::filesystem::path asset_path(const char*);                  // get asset's absolute path (relative path will be converted)

    private:
        const std::chrono::time_point<std::chrono::system_clock>
                        m_start_time = std::chrono::system_clock::now();
        SDL_GLContext   m_sdl_gl_context;
        SDL_Window*     m_sdl_window;

        static Nodable *     s_instance;

    };
}
