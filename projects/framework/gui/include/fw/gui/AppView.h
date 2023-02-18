#pragma once

#include <imgui/imgui.h>

#include <SDL/include/SDL.h>
#include <array>
#include <map>
#include <string>

#include <observe/event.h>
#include <fw/core/types.h>
#include <fw/gui/EventManager.h>
#include <fw/gui/View.h>

namespace fw
{
    // forward declarations
    class App;
    class File;
    class History;
    struct Texture;
    class VirtualMachine;
    struct Conf;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class AppView : public View
	{
	public:

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

		AppView(App*);
        AppView(const App&) = delete;
		~AppView() override;

        struct DrawEvent {
            AppView* view;
            bool redock;
        };
        observe::Event<DrawEvent> event_draw;          // triggered during draw()
        observe::Event<AppView*> event_reset_layout;           // triggered during reset layout
        observe::Event<AppView*> event_draw_splashscreen;      // triggered when drawing splashscreen

        bool               on_draw() override;
        ImGuiID            get_dockspace(Dockspace)const;
        bool               is_splashscreen_visible()const;
        bool               pick_file_path(std::string& _out_path, DialogType);   // pick a file and store its path in _out_path
        void               dock_window(const char* window_name, Dockspace)const; // Call this within on_reset_layout
        void               set_layout_initialized(bool b);
        void               set_splashscreen_visible(bool b);
        Conf*              conf();

    private:
        void               draw_splashcreen_window();
        void               draw_status_window() const;
        App*               m_app;
        Conf*              m_conf;
        bool               m_is_layout_initialized;
        std::array<ImGuiID, Dockspace_COUNT> m_dockspaces{};

        REFLECT_DERIVED_CLASS(View)
    };
}