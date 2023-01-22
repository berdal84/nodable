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
        static constexpr float k_desired_fps        = 60.0f;
        static constexpr float k_desired_delta_time = 1.0f / k_desired_fps;

	public:		
		AppView(App* _app, fw::AppView::Conf _conf);
		~AppView() override;
        bool onInit() override;
		bool onDraw() override;
    private:
        void draw_file_editor(ImGuiID dockspace_id, bool redock_all, File *file);
        void draw_history_bar(History*);
        void draw_properties_editor();
        void draw_startup_menu(ImGuiID dockspace_id);
        void draw_splashcreen();
        void draw_status_bar() const;
        void draw_tool_bar();
        void draw_vm_view();
        void draw_node_properties();
        void draw_file_info() const;
        void draw_imgui_style_editor() const;
        void draw_help_window() const;

        fw::Texture*       m_logo;
        bool               m_is_history_dragged;
        const char*        m_splashscreen_title;
        bool               m_show_splashscreen;
        bool               m_show_properties_editor;
        bool               m_show_imgui_demo;
        bool               m_show_advanced_node_properties;
        bool               m_scroll_to_curr_instr;

        REFLECT_DERIVED_CLASS(fw::View)
    };
}