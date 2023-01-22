#pragma once

#include <imgui/imgui.h>

#include <SDL/include/SDL.h>
#include <array>
#include <map>
#include <string>

#include <fw/imgui/FontConf.h>
#include <fw/imgui/FontSlot.h>
#include <fw/imgui/Shortcut.h>
#include <fw/imgui/View.h>
#include <fw/types.h>

namespace fw
{
    // forward declarations
    class App;
    class File;
    class History;
    struct Texture;
    class Settings;
    class VirtualMachine;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class AppView : public View
	{
        static constexpr float k_desired_fps        = 60.0f;
        static constexpr float k_desired_delta_time = 1.0f / k_desired_fps;
	public:

        typedef int DIALOG;
        enum DIALOG_ {
            DIALOG_SaveAs,
            DIALOG_Browse
        };

        struct Conf {
            std::string           title;
            std::vector<FontConf> fonts;
            std::array<const char*, FontSlot_COUNT> fonts_default;
            ImColor               background_color;
            bool                  is_layout_initialized;
            std::string           splashscreen_title;
            bool                  show_splashscreen;
            bool                  show_properties_editor;
            bool                  show_imgui_demo;
            std::string           splashscreen_path;
            std::string           icons_path;
            float                 ui_dockspace_down_size;
            float                 ui_dockspace_right_ratio;
        };

		AppView(App*, Conf);
		~AppView() override;
        virtual bool       init();
        virtual void       handle_events();
		bool               draw() override;
        virtual void       shutdown();
        virtual bool       onInit() = 0;
        virtual bool       onDraw() = 0;
        bool               pick_file_path(std::string& _out_path, DIALOG _dialog_type);
        ImFont*            load_font(const FontConf& _config);
        ImFont*            get_font_by_id(const char *id);
        void               set_splashscreen_visible(bool b);
        ImFont*            get_font(FontSlot_ slot) const;
        bool               get_fullscreen() const;
        void               set_fullscreen(bool b);
        void               set_layout_initialized(bool b);
        bool               get_layout_initialized() const;

    protected:
        App*               m_app;
        Conf               m_conf;

    private:
        fw::Texture*       m_logo;
        fw::Texture*       m_splashscreen_texture;
        SDL_Window*        m_sdl_window;
        SDL_GLContext      m_sdl_gl_context;
        std::map<std::string, ImFont*>      m_loaded_fonts; // All fonts loaded in memory
        std::array<ImFont*, FontSlot_COUNT> m_fonts;        // Fonts currently in use
        bool               m_is_layout_initialized;
        bool               m_is_splashscreen_visible;

        REFLECT_DERIVED_CLASS(View)
    };
}