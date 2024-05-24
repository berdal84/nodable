#pragma once
#include "tools/gui/BaseApp.h"

namespace tools
{
    class AppExampleView : public tools::AppView
    {
    public:
        explicit AppExampleView( BaseApp* _app)
            : AppView(_app)
            , m_app(_app)
        {}

        void draw() override
        {
            AppView::begin_draw();

            // Add a simple menu bar
            if ( ImGui::BeginMainMenuBar() )
            {
                if ( ImGui::BeginMenu( "File" ) )
                {
                    if ( ImGui::MenuItem( "Show splashscreen" ) ) show_splashscreen = true;
                    if ( ImGui::MenuItem( "Quit" ) ) m_app->should_stop = true;
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            // do not draw windows when splashscreen is visible
            if ( show_splashscreen )
            {
                AppView::end_draw();
                return;
            };

            if ( ImGui::Begin( "top" ) )
            {
                ImGui::TextWrapped( "\"top\" window content" );
            }
            ImGui::End();

            if ( ImGui::Begin( "right" ) )
            {
                ImGui::TextWrapped( "\"right\" window content" );
            }
            ImGui::End();

            if ( ImGui::Begin( "center" ) )
            {
                ImGui::TextWrapped( "\"center\" window content" );
            }
            ImGui::End();

            AppView::end_draw();
        }

        void on_reset_layout() override
        {
            // Bind each window to a dockspace
            dock_window( "center", tools::AppView::Dockspace_CENTER );
            dock_window( "right", tools::AppView::Dockspace_RIGHT );
            dock_window( "top", tools::AppView::Dockspace_TOP );
        }

        void draw_splashscreen() override
        {
            if ( AppView::begin_splashscreen() )
            {
                ImGui::TextWrapped( "Welcome to the tool-app-example app.\nThis demonstrates how to use the tool-gui library." );
                ImGui::Separator();
                ImGui::TextWrapped( "\nFor your information, this is the splashscreen window of the app.\n"
                                    "You can inject your custom code by editing AppExampleView::draw_splashscreen()\n" );
                AppView::end_splashscreen();
            }
        }
    private:
        BaseApp* m_app;
    };
}