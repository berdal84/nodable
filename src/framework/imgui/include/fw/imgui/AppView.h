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
    class VirtualMachine;

	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class AppView : public View
	{
        static constexpr float k_desired_fps        = 60.0f;
        static constexpr float k_desired_delta_time = 1.0f / k_desired_fps;
	public:

        enum DialogType
        {
            DIALOG_SaveAs,
            DIALOG_Browse
        };

        enum Dockspace
        {
            Dockspace_ROOT,
            Dockspace_CENTER,
            Dockspace_RIGHT,
            Dockspace_BOTTOM,
            Dockspace_TOP,
            Dockspace_COUNT,
        };

        // AppView's configuration
        struct Conf {
            std::string           title                    = "Untitled";
            ImColor               background_color         = ImColor(0.f,0.f,0.f);
            const char*           splashscreen_title       = "##Splashscreen";
            bool                  show_splashscreen        = true;
            bool                  show_imgui_demo          = false;
            FontConf              icon_font                    = {"FA-solid-900", "fonts/fa-solid-900.ttf"};
            float                 dockspace_down_size      = 48.f;
            float                 dockspace_top_size       = 48.f;
            float                 dockspace_right_ratio    = 0.3f;
            size_t                log_tooltip_max_count    = 25;
            std::array<
                fw::vec4,
                fw::Log::Verbosity_COUNT> log_color         {
                                                                vec4(0.5f, 0.0f, 0.0f, 1.0f), // red
                                                                vec4(0.5f, 0.0f, 0.5f, 1.0f), // violet
                                                                vec4(0.5f, 0.5f, 0.5f, 1.0f), // grey
                                                                vec4(0.0f, 0.5f, 0.0f, 1.0f)  // green
                                                            };
            std::vector<FontConf> fonts                    = {{
                                                                "default",                          // id
                                                                "fonts/JetBrainsMono-Medium.ttf",   // path
                                                                18.0f,                              // size in px.
                                                                true,                               // include icons?
                                                                18.0f                               // icons size in px.
                                                            }};
            std::array<
                const char*,
                FontSlot_COUNT> fonts_default               {
                                                                "default", // FontSlot_Paragraph
                                                                "default", // FontSlot_Heading
                                                                "default", // FontSlot_Code
                                                                "default"  // FontSlot_ToolBtn
                                                            };
        };

		AppView(App*, Conf);
		~AppView() override;
    private:
        friend App;
        bool               init();
        void               handle_events();
		bool               draw() override;
        void               shutdown();
    protected:
        virtual bool       on_draw(bool& redock_all) = 0;
        virtual bool       on_init() = 0;
        virtual bool       on_reset_layout() = 0;
        virtual void       on_draw_splashscreen() = 0;
    public:
        ImFont*            get_font(FontSlot slot) const;
        ImFont*            get_font_by_id(const char *id);
        ImFont*            load_font(const FontConf& _config);
        ImGuiID            get_dockspace(Dockspace)const;
        bool               is_fullscreen() const;
        bool               is_splashscreen_visible()const;
        bool               pick_file_path(std::string& _out_path, DialogType);
        void               dock_window(const char* window_name, Dockspace)const;
        void               set_fullscreen(bool b);
        void               set_layout_initialized(bool b);
        void               set_splashscreen_visible(bool b);
    private:
        void               draw_splashcreen_window();
        void               draw_status_window() const;
    protected:
        App*               m_app;
        Conf               m_conf;
    private:
        SDL_GLContext      m_sdl_gl_context;
        SDL_Window*        m_sdl_window;
        bool               m_is_layout_initialized;
        std::array<ImFont*, FontSlot_COUNT>  m_fonts;        // Required fonts
        std::array<ImGuiID, Dockspace_COUNT> m_dockspaces;
        std::map<std::string, ImFont*>       m_loaded_fonts; // Available fonts

        REFLECT_DERIVED_CLASS(View)
    };
}