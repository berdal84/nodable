#pragma once

#include <imgui/imgui.h>
#include <SDL.h>
#include <string>
#include <map>
#include <array>

#include <nodable/Reflect.h>
#include <nodable/Nodable.h>
#include <nodable/View.h>
#include <nodable/FontConf.h>
#include <nodable/FontSlot.h>
#include <nodable/FileBrowser.h>

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
        void browseFile();
        void shutdown();
	private:
        App*               m_app;
        FileBrowser        m_fileBrowser;
		SDL_Window*        m_sdlWindow;
		SDL_GLContext      m_sdlGLContext;
		ImColor            m_bgColor;
        bool               m_isStartupWindowVisible;
        bool               m_isHistoryDragged;
        const char*        m_startupScreenTitle;
        bool               m_isLayoutInitialized;
        std::string        m_glWindowName;
        bool               m_showProperties;
        bool               m_showImGuiDemo;
        std::map<std::string, ImFont*> m_loadedFonts; // All fonts loaded in memory
        std::array<ImFont*, FontSlot_COUNT> m_fonts;  // Fonts currently in use

        void drawHistoryBar(History *currentFileHistory);
        void drawStatusBar() const;
        void drawStartupWindow();
        void drawFileEditor(ImGuiID dockspace_id, bool redock_all, size_t fileIndex);
        void drawFileBrowser();
        void drawBackground();
        void drawPropertiesWindow();
        void drawToolBar();
        ImFont* loadFont(const FontConf &fontConf);
        ImFont* getFontById(const char *id );

        REFLECT_WITH_INHERITANCE(AppView, View)
    };
}