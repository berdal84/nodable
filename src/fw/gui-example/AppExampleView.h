#pragma once
#include "fw/gui/App.h"

namespace fw
{
    class AppExampleView : public fw::AppView
    {
    public:
        AppExampleView(App* _app)
            : AppView(_app)
        {}

        void on_reset_layout() override
        {
            // Bind each window to a dockspace
            dock_window( "center", fw::AppView::Dockspace_CENTER );
            dock_window( "right", fw::AppView::Dockspace_RIGHT );
            dock_window( "top", fw::AppView::Dockspace_TOP );
        }

        void draw_splashscreen() override
        {
            if ( AppView::begin_splashscreen() )
            {
                ImGui::TextWrapped( "Welcome to the framework-gui-example app.\nThis demonstrates how to use the framework-gui library." );
                ImGui::Separator();
                ImGui::TextWrapped( "\nFor your information, this is the splashscreen window of the app.\n"
                                    "You can inject your custom code by editing Example::draw_splashscreen()\n" );
                AppView::end_splashscreen();
            }
        }
    };
}