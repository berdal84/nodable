#pragma once

#include <SDL.h>
#include <array>
#include <filesystem>
#include <map>
#include <observe/event.h>
#include <string>

#include "EventManager.h"
#include "FontManager.h"
#include "ImGuiEx.h"
#include "TextureManager.h"
#include "View.h"
#include "tools/core/types.h"

namespace tools
{
    // forward declarations
    class App;
    class History;
    struct Texture;
    class VirtualMachine;
    struct FontManagerConfig;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class AppView
	{
	public:
        friend App;

        enum DialogType // Helps to configure the file browse dialog
        {
            DIALOG_SaveAs,   // Allows to set a new file or select an existing file
            DIALOG_Browse    // Only allows to pick a file
        };

        /*
         * Dockspace: enum to identify dockspaces
         * ----------------------------------------
         *                 TOP                    |
         * ----------------------------------------
         *                           |            |
         *                           |            |
         *            CENTER         |    RIGHT   |
         *                           |            |
         *                           |            |
         *                           |            |
         * ---------------------------------------|
         *                   BOTTOM               |
         * ----------------------------------------
         */
        enum Dockspace
        {
            Dockspace_ROOT,
            Dockspace_CENTER,
            Dockspace_RIGHT,
            Dockspace_BOTTOM,
            Dockspace_TOP,
            Dockspace_COUNT,
        };

        AppView(App *);
        AppView(const AppView &) = delete;
		~AppView();

        bool               show_splashscreen{};

        //
        // note: those managers could perhaps be stored in a Context struct (e.g. ImGui's context)
        //
        // ex:
        //        // in Context.h
        //
        //        struct Context {
        //            TextureManager     texture_manager;
        //            FontManager        font_manager;
        //            EventManager       event_manager;
        //            ActionManager      action_manager;
        //        };
        //
        //        Context* create_context();
        //        void     set_context(Context*);
        //        Context* get_context();
        //        void     destroy_context(Context*);
        //
        //        // in a Context.cpp
        //        static Context* g_context{nullptr}
        //        void     set_context(Context*) { ... }
        //        Context* get_context() { return g_context; }
        //        void     destroy_context() { ... }
        //

        FontManager        font_manager;
        EventManager       event_manager;
        ActionManager      action_manager;

        virtual void       init();
        virtual void       update();
        virtual void       draw();
        virtual void       on_reset_layout() {};
        virtual void       shutdown();
        virtual void       draw_splashscreen(); // If needed, use begin/end_splashscreen static methods to override this. Ex: if ( AppView::begin_splashscreen(m_app->config) ) { /* your code here */; AppView::end_splashscreen(); }
        ImGuiID            get_dockspace(Dockspace)const;
        bool               pick_file_path(std::string& _out_path, DialogType) const;   // pick a file and store its path in _out_path
        void               set_layout_initialized(bool b);
        static int         fps();      // get the current frame per second (un-smoothed)
        void               save_screenshot(std::filesystem::path) const; // Save an LCT_RGBA PNG image to path
        bool               is_fullscreen() const;
        void               set_fullscreen( bool b );
        void               set_title( const char* string );

    protected:
        void               begin_draw();
        void               end_draw();
        bool               begin_splashscreen();
        void               end_splashscreen();
        void               dock_window(const char* window_name, Dockspace)const; // Must be called within on_reset_layout
        std::vector<unsigned char> take_screenshot() const;

    private:

        SDL_GLContext       m_sdl_gl_context;
        SDL_Window*         m_sdl_window;
        u32_t               m_frame_start_time{};
        App *               m_app;
        std::string         m_title;
        bool                m_is_layout_initialized;
        std::array<ImGuiID, Dockspace_COUNT>
                            m_dockspaces{};
        const std::chrono::time_point<std::chrono::system_clock>
                            m_start_time = std::chrono::system_clock::now();
    };
}