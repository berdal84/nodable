#pragma once
#include "AppExampleView.h"
#include "tools/gui/App.h"
#include "tools/gui/Config.h"
#include "tools/gui/gui.h"

namespace tools
{
    class AppExample : public tools::App
    {
    public:
        AppExample()
        : App(new AppExampleView(this))
        {}

        ~AppExample()
        {
            delete m_view;
        }

        void on_draw() override
        {
            // Add a simple menu bar
            if ( ImGui::BeginMainMenuBar() )
            {
                if ( ImGui::BeginMenu( "File" ) )
                {
                    if ( ImGui::MenuItem( "Show splashscreen" ) ) g_conf->splashscreen = true;
                    if ( ImGui::MenuItem( "Quit" ) ) should_stop = true;
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            // do not draw windows when splashscreen is visible
            if ( g_conf->splashscreen )
            {
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
        }
        bool on_init() override
        {
            LOG_MESSAGE( "Example", "My ON_INIT log!\n" );
            return true;
        }
        bool on_shutdown() override
        {
            LOG_MESSAGE( "Example", "My ON_SHUTDOWN log!\n" );
            return true;
        }
    };
}