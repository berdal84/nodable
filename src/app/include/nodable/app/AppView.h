#pragma once

#include <imgui/imgui.h>

#include <SDL.h>
#include <string>
#include <map>
#include <array>

#include <nodable/core/reflection/reflection>
#include <nodable/app/types.h>
#include <nodable/app/View.h>
#include <nodable/app/FontConf.h>
#include <nodable/app/FontSlot.h>
#include <nodable/app/Shortcut.h>
#include "File.h"

namespace Nodable
{
    // forward declarations
    class IAppCtx;
    class App;
    class History;
    class Language;
    class Texture;
    class Settings;
    class VirtualMachine;
    class Texture;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class AppView : public View
	{
        static constexpr float k_desired_fps        = 60.0f;
        static constexpr float k_desired_delta_time = 1.0f / k_desired_fps;

	public:		
		AppView(IAppCtx&, const char* _name);
		~AppView() override;
        bool init();
        void handle_events();
		bool draw() override;
        void shutdown();
	private:
        void browse_file();
        void close_file();
        void new_file();
        void save_file();
        void save_file_as();
        void draw_file_editor(ImGuiID dockspace_id, bool redock_all, Nodable::File *file);
        void draw_history_bar(History*);
        void draw_properties_editor();
        void draw_startup_menu(ImGuiID dockspace_id);
        void draw_splashcreen();
        void draw_status_bar() const;
        void draw_tool_bar();
        void draw_vm_view();
        void draw_side_panel();
        void draw_node_properties();
        void draw_file_info() const;
        void draw_imgui_style_editor() const;

        ImFont* load_font(const FontConf &_config);
        ImFont* get_font_by_id(const char *id);

        Texture*           m_logo;
        VirtualMachine&    m_vm;
        Settings&          m_settings;
        SDL_Window*        m_sdl_window;
        std::string        m_sdl_window_name;
        SDL_GLContext      m_sdl_gl_context;
        ImColor            m_background_color;
        bool               m_is_history_dragged;
        bool               m_is_layout_initialized;
        const char*        m_splashscreen_title;
        bool               m_show_splashscreen;
        bool               m_show_properties_editor;
        bool               m_show_imgui_demo;
        bool               m_show_advanced_node_properties;
        bool               m_scroll_to_curr_instr;
        std::map<std::string, ImFont*>      m_loaded_fonts; // All fonts loaded in memory
        std::array<ImFont*, FontSlot_COUNT> m_fonts;  // Fonts currently in use
        std::vector<Shortcut>               m_shortcuts;

        REFLECT_ENABLE(AppView, View)
    };
}