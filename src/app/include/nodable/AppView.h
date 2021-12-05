#pragma once

#include <imgui/imgui.h>

#define IMFILEBROWSER_FILE_ICON            ICON_FA_FILE    // override icon
#define IMFILEBROWSER_FOLDER_ICON          ICON_FA_FOLDER  // override icon
#define IMGFILEBROWSER_USE_CPP11                           // use ghc::filesystem for c++11 compatibility
#include <imgui-filebrowser/imfilebrowser.h>

#include <SDL.h>
#include <string>
#include <map>
#include <array>

#include <nodable/Reflect.h>
#include <nodable/Nodable.h>
#include <nodable/View.h>
#include <nodable/FontConf.h>
#include <nodable/FontSlot.h>

namespace Nodable
{
    // forward declarations
    class App;
    class History;
    class Language;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class AppView : public View
	{
		
	public:		
		AppView(const char* _name, App* _application);
		~AppView() override;
		bool draw() override;
		bool init();
        void browse_file();
        void shutdown();

	private:
        void draw_history_bar(History*);
        void draw_status_bar() const;
        void draw_startup_window();
        void draw_file_editor(ImGuiID dockspace_id, bool redock_all, size_t fileIndex);
        void draw_file_browser();
        void draw_background();
        void draw_properties_editor();
        void draw_tool_bar();
        ImFont* load_font(const FontConf &fontConf);
        ImFont* get_font_by_id(const char *id);

        App*               m_app;
        ImGui::FileBrowser m_file_browser;
        SDL_Window*        m_sdl_window;
        std::string        m_sdl_window_name;
        SDL_GLContext      m_sdl_gl_context;
        ImColor            m_background_color;
        bool               m_is_history_dragged;
        bool               m_is_layout_initialized;
        const char*        m_startup_screen_title;
        bool               m_show_startup_window;
        bool               m_show_properties_editor;
        bool               m_show_imgui_demo;
        std::map<std::string, ImFont*>      m_loaded_fonts; // All fonts loaded in memory
        std::array<ImFont*, FontSlot_COUNT> m_fonts;  // Fonts currently in use

        REFLECT_DERIVED(AppView)
        REFLECT_EXTENDS(View)
        REFLECT_END
    };
}