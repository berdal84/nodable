#pragma once

// std
#include <future>
#include <memory>
#include <string>

#include <fw/core/types.h>
#include <fw/gui/AppView.h>
#include <fw/gui/TextureManager.h>
#include <ghc/filesystem.hpp>

namespace fw
{
	class App
	{
	public:
		App(AppView* _view);
        virtual ~App() {};

		bool            init();         // Initialize the application
		bool            shutdown();     // Shutdown the application
		void            update();       // Update the application
		void            draw();         // Draw the application's view

        virtual bool    onInit() = 0;      // For custom code to run during init
        virtual bool    onShutdown() = 0;  // For custom code to run during shutdown
        virtual void    onUpdate() = 0;    // For custom code to run during updates

        bool            should_stop() const { return m_should_stop; }    // Check if application should stop
        void            flag_to_stop();                                  // Flag the application to stop, will stop more likely the next frame.
        u64_t           elapsed_time() const;                            // Get the elapsed time in seconds
        TextureManager& texture_manager() { return m_texture_manager; };
        EventManager&   event_manager() { return m_event_manager; }
        FontManager&    font_manager() { return m_font_manager; }

        static std::string to_absolute_asset_path(const char*);       // convert a relative (to ./assets) path to an absolute path
    protected:
        const std::chrono::time_point<std::chrono::system_clock>
                        m_start_time = std::chrono::system_clock::now();

        AppView*        m_view;
        TextureManager  m_texture_manager;
        EventManager    m_event_manager;
        FontManager     m_font_manager;
        bool            m_should_stop;
        ghc::filesystem::path s_assets_folder_path;
    };
}
