#pragma once

// std
#include <future>
#include <memory>
#include <string>

#include <fw/gui/FontManager.h>
#include <fw/core/types.h>
#include <fw/gui/Conf.h>
#include <fw/gui/AppView.h>
#include <fw/gui/TextureManager.h>
#include <ghc/filesystem.hpp>
#include <observe/event.h>

namespace fw
{
	class App
	{
	public:
		App(Conf&);
        App(const App&) = delete;
        ~App();

        observe::Event<> event_after_init;     // Triggered after init
        observe::Event<> event_on_draw;        // Triggered between ImGui::BeginFrame() and EndFrame()
        observe::Event<> event_after_shutdown; // Triggered after shutdown
        observe::Event<> event_after_update;   // Triggered after update

        int                run();              // Run the main loop
        bool               should_stop() const { return m_should_stop; } // Check if application should stop
        void               flag_to_stop();                               // Flag the application to stop, will stop more likely the next frame.
        u64_t              elapsed_time() const;                         // Get the elapsed time in seconds
        TextureManager*    texture_manager() { return &m_texture_manager; };
        EventManager*      event_manager() { return &m_event_manager; }
        FontManager*       font_manager() { return &m_font_manager; }
        Conf*              conf() { return &m_conf; }
        void               handle_events();                 //              ...
        bool               is_fullscreen() const;
        void               set_fullscreen(bool b);
        void               save_screenshot(const char *relative_file_path);
        static std::string asset_path(const char*);       // convert a relative (to ./assets) path to an absolute path
        fw::AppView*       view();
        bool               init();     // Initialize the application
        bool               shutdown(); // Shutdown the application
        void               update();   // Update the application
        void               draw();     // Draw the application's view
        static int fps();

    private:
        const std::chrono::time_point<std::chrono::system_clock>
                        m_start_time = std::chrono::system_clock::now();

        TextureManager  m_texture_manager;
        EventManager    m_event_manager;
        FontManager     m_font_manager;
        SDL_GLContext   m_sdl_gl_context;
        SDL_Window*     m_sdl_window;
        Conf            m_conf;
        AppView         m_view;
        static App*     s_instance;
        bool            m_should_stop;
    };
}
