#pragma once

#include <imgui/imgui.h>

#include <SDL/include/SDL.h>
#include <array>
#include <map>
#include <string>

#include <fw/gui/FontConf.h>
#include <fw/gui/FontSlot.h>
#include <fw/gui/Shortcut.h>
#include <fw/gui/View.h>
#include <fw/core/types.h>

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
	public:

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

        // AppView's configuration
        struct Conf {
            std::string           app_window_label         = "Framework AppView";
            float                 min_frame_time           = 1.0f / 60.0f;            // limit to 60fps
            ImColor               background_color         = ImColor(0.f,0.f,0.f);
            fw::vec4              button_activeColor       = vec4(0.98f, 0.73f, 0.29f, 0.95f); // orange
            fw::vec4              button_hoveredColor      = vec4(0.70f, 0.70f, 0.70f, 0.95f); // light grey
            fw::vec4              button_color             = vec4(0.50f, 0.50f, 0.50f, 0.63f); // grey
            const char*           splashscreen_window_label= "##Splashscreen";
            bool                  show_splashscreen        = true;
            bool                  show_imgui_demo          = false;
            FontConf              icon_font                = {"FA-solid-900", "fonts/fa-solid-900.ttf"};
            float                 dockspace_bottom_size      = 48.f;
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

            void patch_imgui_style(ImGuiStyle& _style) // Apply the configuration to an existing ImGuiStyle
            {
                vec4* colors = _style.Colors;
                colors[ImGuiCol_Text]                   = vec4(0.20f, 0.20f, 0.20f, 1.00f);
                colors[ImGuiCol_TextDisabled]           = vec4(0.21f, 0.21f, 0.21f, 1.00f);
                colors[ImGuiCol_WindowBg]               = vec4(0.76f, 0.76f, 0.76f, 1.00f);
                colors[ImGuiCol_DockingEmptyBg]         = vec4(0.64f, 0.24f, 0.24f, 1.00f);
                colors[ImGuiCol_ChildBg]                = vec4(0.69f, 0.69f, 0.69f, 1.00f);
                colors[ImGuiCol_PopupBg]                = vec4(0.66f, 0.66f, 0.66f, 1.00f);
                colors[ImGuiCol_Border]                 = vec4(0.70f, 0.70f, 0.70f, 1.00f);
                colors[ImGuiCol_BorderShadow]           = vec4(0.30f, 0.30f, 0.30f, 0.50f);
                colors[ImGuiCol_FrameBg]                = vec4(1.00f, 1.00f, 1.00f, 1.00f);
                colors[ImGuiCol_FrameBgHovered]         = vec4(0.90f, 0.80f, 0.80f, 1.00f);
                colors[ImGuiCol_FrameBgActive]          = vec4(0.90f, 0.65f, 0.65f, 1.00f);
                colors[ImGuiCol_TitleBg]                = vec4(0.60f, 0.60f, 0.60f, 1.00f);
                colors[ImGuiCol_TitleBgActive]          = vec4(0.60f, 0.60f, 0.60f, 1.00f);
                colors[ImGuiCol_TitleBgCollapsed]       = vec4(0.49f, 0.63f, 0.69f, 1.00f);
                colors[ImGuiCol_MenuBarBg]              = vec4(0.60f, 0.60f, 0.60f, 0.98f);
                colors[ImGuiCol_ScrollbarBg]            = vec4(0.40f, 0.40f, 0.40f, 1.00f);
                colors[ImGuiCol_ScrollbarGrab]          = vec4(0.61f, 0.61f, 0.62f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabHovered]   = vec4(0.70f, 0.70f, 0.70f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabActive]    = vec4(0.80f, 0.80f, 0.80f, 1.00f);
                colors[ImGuiCol_CheckMark]              = vec4(0.31f, 0.23f, 0.14f, 1.00f);
                colors[ImGuiCol_SliderGrab]             = vec4(0.71f, 0.46f, 0.22f, 0.63f);
                colors[ImGuiCol_SliderGrabActive]       = vec4(0.71f, 0.46f, 0.22f, 1.00f);
                colors[ImGuiCol_Button]                 = button_color;
                colors[ImGuiCol_ButtonHovered]          = button_hoveredColor;
                colors[ImGuiCol_ButtonActive]           = button_activeColor;
                colors[ImGuiCol_Header]                 = vec4(0.70f, 0.70f, 0.70f, 1.00f);
                colors[ImGuiCol_HeaderHovered]          = vec4(0.89f, 0.65f, 0.11f, 0.96f);
                colors[ImGuiCol_HeaderActive]           = vec4(1.00f, 1.00f, 1.00f, 1.00f);
                colors[ImGuiCol_Separator]              = vec4(0.43f, 0.43f, 0.50f, 0.50f);
                colors[ImGuiCol_SeparatorHovered]       = vec4(0.71f, 0.71f, 0.71f, 0.78f);
                colors[ImGuiCol_SeparatorActive]        = vec4(1.00f, 0.62f, 0.00f, 1.00f);
                colors[ImGuiCol_ResizeGrip]             = vec4(1.00f, 1.00f, 1.00f, 0.30f);
                colors[ImGuiCol_ResizeGripHovered]      = vec4(1.00f, 1.00f, 1.00f, 0.60f);
                colors[ImGuiCol_ResizeGripActive]       = vec4(1.00f, 1.00f, 1.00f, 0.90f);
                colors[ImGuiCol_Tab]                    = vec4(0.58f, 0.54f, 0.50f, 0.86f);
                colors[ImGuiCol_TabHovered]             = vec4(1.00f, 0.79f, 0.45f, 1.00f);
                colors[ImGuiCol_TabActive]              = vec4(1.00f, 0.73f, 0.25f, 1.00f);
                colors[ImGuiCol_TabUnfocused]           = vec4(0.53f, 0.53f, 0.53f, 0.97f);
                colors[ImGuiCol_TabUnfocusedActive]     = vec4(0.79f, 0.79f, 0.79f, 1.00f);
                colors[ImGuiCol_DockingPreview]         = vec4(1.00f, 0.70f, 0.09f, 0.70f);
                colors[ImGuiCol_DockingEmptyBg]         = vec4(0.20f, 0.20f, 0.20f, 1.00f);
                colors[ImGuiCol_PlotLines]              = vec4(1.00f, 1.00f, 1.00f, 1.00f);
                colors[ImGuiCol_PlotLinesHovered]       = vec4(0.90f, 0.70f, 0.00f, 1.00f);
                colors[ImGuiCol_PlotHistogram]          = vec4(0.90f, 0.70f, 0.00f, 1.00f);
                colors[ImGuiCol_PlotHistogramHovered]   = vec4(1.00f, 0.60f, 0.00f, 1.00f);
                colors[ImGuiCol_TextSelectedBg]         = vec4(0.00f, 0.00f, 1.00f, 0.35f);
                colors[ImGuiCol_DragDropTarget]         = vec4(1.00f, 1.00f, 0.00f, 0.90f);
                colors[ImGuiCol_NavHighlight]           = vec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_NavWindowingHighlight]  = vec4(1.00f, 1.00f, 1.00f, 0.70f);
                colors[ImGuiCol_NavWindowingDimBg]      = vec4(0.80f, 0.80f, 0.80f, 0.20f);
                colors[ImGuiCol_ModalWindowDimBg]       = vec4(0.20f, 0.20f, 0.20f, 0.55f);

                _style.WindowBorderSize   = 1.0f;
                _style.FrameBorderSize    = 1.0f;
                _style.FrameRounding      = 3.0f;
                _style.ChildRounding      = 3.0f;
                _style.WindowRounding     = 0.0f;
                _style.AntiAliasedFill    = true;
                _style.AntiAliasedLines   = true;
                _style.WindowPadding      = vec2(10.0f,10.0f);
            }
        };

		AppView(App*, Conf);
		~AppView() override;
    private:
        friend App;
        bool               init();                          // called by fw::App automatically
        void               handle_events();                 //              ...
		bool               draw() override;                 //              ...
        bool shutdown();                      //              ...
    protected:
        virtual bool       on_draw(bool& redock_all) = 0;   // implement here your app ui using ImGui
        virtual bool       on_init() = 0;                   // initialize your view here (SDL and ImGui are ready)
        virtual bool       on_reset_layout() = 0;           // implement behavior when layout is reset
        virtual void       on_draw_splashscreen() = 0;      // implement here the splashscreen windows content
    public:
        ImFont*            get_font(FontSlot) const;
        ImFont*            get_font_by_id(const char*);
        ImFont*            load_font(const FontConf&);
        ImGuiID            get_dockspace(Dockspace)const;
        bool               is_fullscreen() const;
        bool               is_splashscreen_visible()const;
        bool               pick_file_path(std::string& _out_path, DialogType);   // pick a file and store its path in _out_path
        void               dock_window(const char* window_name, Dockspace)const; // Call this within on_reset_layout
        void               set_fullscreen(bool b);
        void               set_layout_initialized(bool b);
        void               set_splashscreen_visible(bool b);
        void               save_screenshot(const char *relative_file_path);
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