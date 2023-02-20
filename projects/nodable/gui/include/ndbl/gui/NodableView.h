#pragma once

#include <string>
#include <map>
#include <array>

#include <fw/core/reflection/reflection>
#include <fw/gui/AppView.h>

#include <ndbl/gui/types.h>

namespace ndbl
{
    // forward declarations
    class History;
    struct Texture;
    class File;
    class Nodable;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
    class AppView
	{
	public:
		AppView(Nodable*);
        AppView(const Nodable &) = delete;
		~AppView();

    private:
        bool on_init();
        bool on_draw();
        bool on_reset_layout();
        void on_draw_splashscreen();

        void draw_file_info_window() const;
        void draw_file_window(ImGuiID dockspace_id, bool redock_all, File *file);
        void draw_help_window() const;
        void draw_history_bar(History*);
        void draw_imgui_config_window() const;
        void draw_node_properties_window();
        void draw_config_window();
        void draw_startup_window(ImGuiID dockspace_id);
        void draw_toolbar_window();
        void draw_virtual_machine_window();

        fw::Texture*       m_logo;
        bool               m_is_history_dragged;
        bool               m_show_properties_editor;
        bool               m_show_imgui_demo;
        bool               m_show_advanced_node_properties;
        bool               m_scroll_to_curr_instr;
        Nodable *          m_app;
    };
}