#include "App_View_Example.h"
#include "App_Example.h"
#include "gui/App_View.h"

using namespace tools;

// Define window names once here
constexpr const char* CENTER_WINDOW = "Center View";
constexpr const char* RIGHT_WINDOW  = "Right View";
constexpr const char* TOP_WINDOW    = "Top View";

void App_View_Example::init(App_Example *_app)
{
    // Store ptr
    this->m_app = _app;
    // Initialize our base view
    appview_init(this, _app );
    appview_set_title(this, "App_Example default title - (you can change this title from " __FILE__ ")");

    // Change behavior by connecting signals with our custom methods
    this->signal_draw_splashscreen_content.connect<&App_View_Example::_draw_splashscreen_content>(this);
    this->signal_reset_layout.connect<&App_View_Example::_reset_layout>(this);
}

void App_View_Example::_draw_splashscreen_content()
{
    ImGui::TextWrapped( "Welcome to the Tools GUI Example App.\nThis demonstrates how to use the Tools GUI library." );
    ImGui::Separator();
    ImGui::TextWrapped( "\nFor your inFormation, this is the splashscreen window of the app.\n"
    "You can inject your custom code by editing in " __FILE__ "\n"
    "You can close it to see the default layout of the application." );
}

void App_View_Example::_reset_layout()
{
    // Bind each window to a dockspace
    appview_dock_window( this, CENTER_WINDOW, Dockspace_CENTER );
    appview_dock_window( this, RIGHT_WINDOW,  Dockspace_RIGHT );
    appview_dock_window( this, TOP_WINDOW,    Dockspace_TOP );
};

void App_View_Example::shutdown()
{
    this->signal_draw_splashscreen_content.disconnect();
    this->signal_reset_layout.disconnect();

    // Here we undo what we did in init()
    appview_shutdown(this); // base view will release its resources
}

void App_View_Example::draw()
{
    VERIFY(m_app != nullptr, "Did you call init_ex? m_app should not be null.");
    appview_begin(this);

    // Add a simple menu bar
    if ( ImGui::BeginMainMenuBar() )
    {
        if ( ImGui::BeginMenu( "File" ) )
        {
            if ( ImGui::MenuItem( "Show splashscreen" ) ) this->show_splashscreen = true;
            if ( ImGui::MenuItem( "Quit" ) ) m_app->request_stop();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // draw our windows only when splashscreen is not visible
    if ( !this->show_splashscreen )
    {
        if ( ImGui::Begin( TOP_WINDOW ) )
        {
            ImGui::TextWrapped( "This is the TOP_WINDOW content" );
        }
        ImGui::End();

        if ( ImGui::Begin( RIGHT_WINDOW ) )
        {
            ImGui::TextWrapped( "This is the RIGHT_WINDOW content" );
        }
        ImGui::End();

        if ( ImGui::Begin( CENTER_WINDOW ) )
        {
            ImGui::TextWrapped( "This is the CENTER_WINDOW content" );
        }
        ImGui::End();
    }

    appview_end(this);
}

void App_View_Example::update()
{
    appview_update(this);
}
