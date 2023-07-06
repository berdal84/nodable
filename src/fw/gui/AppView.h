#pragma once

#include <imgui/imgui.h>
#include <SDL/include/SDL.h>
#include <array>
#include <map>
#include <string>
#include <observe/event.h>

#include "core/types.h"
#include "EventManager.h"
#include "View.h"
#include "Config.h"

namespace fw
{
    // forward declarations
    class App;
    class File;
    class History;
    struct Texture;
    class VirtualMachine;
    struct Config;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class AppView : public View
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
        AppView(const App &) = delete;
		~AppView() override;

        enum Signal
        {
            Signal_ON_DRAW_MAIN,
            Signal_ON_DRAW_SPLASHSCREEN_CONTENT,
            Signal_ON_RESET_LAYOUT
        };
        std::function<void(Signal)> signal_handler; // override this function to extend the behavior

        ImGuiID            get_dockspace(Dockspace)const;
        bool               pick_file_path(std::string& _out_path, DialogType);   // pick a file and store its path in _out_path
        void               dock_window(const char* window_name, Dockspace)const; // Must be called ON_RESET_LAYOUT
        void               set_layout_initialized(bool b);
    private:
        bool               draw_implem() override;
        void               draw_splashscreen_window();
        void               draw_status_window() const;
        App *               m_app;
        bool                m_is_layout_initialized;
        std::array<ImGuiID, Dockspace_COUNT> m_dockspaces{};

        REFLECT_DERIVED_CLASS(View)
    };
}