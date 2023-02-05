#pragma once

#include <string>
#include <map>
#include <array>

#include <fw/imgui/AppView.h>
#include <fw/reflection/reflection>

#include <nodable/app/types.h>

namespace ndbl
{
    // forward declarations
    class History;
    struct Texture;
    class File;
    class App;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
    class AppView : public fw::AppView
	{
	public:
		AppView(App* _app, fw::AppView::Conf _conf);
		~AppView() override;
        bool on_init() override;
		bool on_draw(bool& redock_all) override;
    private:
        bool on_reset_layout() override;
        void draw_file_info_window() const;
        void draw_file_window(ImGuiID dockspace_id, bool redock_all, File *file);
        void draw_help_window() const;
        void draw_history_bar(History*);
        void draw_imgui_settings_window() const;
        void draw_node_properties_window();
        void draw_settings_window();
        void draw_startup_window(ImGuiID dockspace_id);
        void draw_toolbar_window();
        void draw_virtual_machine_window();
        void on_draw_splashscreen() override;

        fw::Texture*       m_logo;
        bool               m_is_history_dragged;
        bool               m_show_properties_editor;
        bool               m_show_imgui_demo;
        bool               m_show_advanced_node_properties;
        bool               m_scroll_to_curr_instr;

        REFLECT_DERIVED_CLASS(fw::View)
    };
}