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
    struct Texture;
    class File;
    class Nodable;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
    class NodableView : public tools::AppView
	{
	public:
        NodableView(Nodable*);
		~NodableView();

        // Override on_xxx methods from base AppView

        void init() override;
        void draw() override;
        void draw_splashscreen() override;
        void on_reset_layout() override;

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
        void draw_virtual_machine_window();

        tools::Texture*    m_logo;
        bool               m_is_history_dragged;
        bool               m_show_properties_editor;
        bool               m_show_imgui_demo;
        bool               m_show_advanced_node_properties;
        bool               m_scroll_to_curr_instr;
        Nodable *          m_app;
    };
}