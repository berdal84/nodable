#include "AppExampleView.h"
#include "AppExample.h"

using namespace tools;

// Define window names once here
#define CENTER_WINDOW "Center View"
#define RIGHT_WINDOW  "Right View"
#define TOP_WINDOW    "Top View"

void AppExampleView::init(AppExample *_app)
{
    // Store ptr
    m_app = _app;
    // Initialize our base view
    m_base_view.init(_app->base_app_handle() );
    m_base_view.set_title("AppExample default title - (you can change this title from " __FILE__ ")");
    // Inject some code to draw a custom content for the splashscreen
    m_base_view.on_draw_splashscreen.connect([&](AppView* view) {
        ImGui::TextWrapped( "Welcome to the Tools GUI Example App.\nThis demonstrates how to use the Tools GUI library." );
        ImGui::Separator();
        ImGui::TextWrapped( "\nFor your information, this is the splashscreen window of the app.\n"
                            "You can inject your custom code by editing in " __FILE__ "\n"
                            "You can close it to see the default layout of the application." );
    });
    // Inject some code to reset the layout
    m_base_view.on_layout_reset.connect([&](AppView* view) {
        // Bind each window to a dockspace
        view->dock_window( CENTER_WINDOW, AppView::Dockspace_CENTER );
        view->dock_window( RIGHT_WINDOW,  AppView::Dockspace_RIGHT );
        view->dock_window( TOP_WINDOW,    AppView::Dockspace_TOP );
    });
}

void AppExampleView::shutdown()
{
    // Here we undo what we did in init()
    m_base_view.shutdown(); // base view will release its resources
}

void AppExampleView::draw()
{
    EXPECT(m_app != nullptr, "Did you call init_ex? m_app should not be null.")
    m_base_view.begin_draw();

    // Add a simple menu bar
    if ( ImGui::BeginMainMenuBar() )
    {
        if ( ImGui::BeginMenu( "File" ) )
        {
            if ( ImGui::MenuItem( "Show splashscreen" ) ) m_base_view.show_splashscreen = true;
            if ( ImGui::MenuItem( "Quit" ) ) m_app->request_stop();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // do not draw windows when splashscreen is visible
    if ( m_base_view.show_splashscreen )
    {
        m_base_view.end_draw();
        return;
    };

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

    m_base_view.end_draw();
}

void AppExampleView::update()
{
    m_base_view.update();
}
