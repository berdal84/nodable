#pragma once

#include <string>
#include <map>
#include <array>

#include "tools/core/reflection/reflection"
#include "tools/core/types.h"
#include "tools/gui/AppView.h"

namespace ndbl
{
    // forward declarations
    class History;
    class File;
    class Nodable;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
    class NodableView
	{
	public:

        typedef tools::AppView::DialogType DialogType;

        // Common

        void init(Nodable*);
        void shutdown();
        void draw();
        void show_splashscreen(bool b);
        bool is_fullscreen() const;
        void toggle_fullscreen();
        bool pick_file_path(std::string& _out_path, DialogType) const;
        void save_screenshot(std::filesystem::path&) const;
        inline tools::AppView* get_base_view_handle() { return &m_base_view; }

    protected:

        // draw_xxx_window

        void draw_file_info_window() const;
        void draw_file_window(ImGuiID dockspace_id, bool redock_all, File*file);
        void draw_help_window() const;
        void draw_history_bar(History&);
        void draw_imgui_config_window() const;
        void draw_node_properties_window();
        void draw_config_window();
        void draw_startup_window(ImGuiID dockspace_id);
        void draw_toolbar_window();
        void draw_interpreter_window();

        tools::Texture*    m_logo                    = nullptr;
        bool               m_is_history_dragged      = false;
        bool               m_show_properties_editor  = false;
        bool               m_show_imgui_demo         = false;
        bool               m_show_advanced_node_properties = false;
        bool               m_scroll_to_curr_instr = true;
        Nodable*           m_app                  = nullptr;
        tools::AppView     m_base_view; // wrapped
    };
}