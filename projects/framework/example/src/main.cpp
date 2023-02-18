#include <fw/gui/App.h>

int main(int argc, char *argv[])
{
    fw::Config conf;
    conf.app_window_label = "framework-example - (based on framework-gui library)";
    fw::App app{conf};

    // Add log message using events
    app.event_after_update.connect([](){
        LOG_MESSAGE("main", "update!\n");
    });

    app.event_after_init.connect([](){
        LOG_MESSAGE("main", "init!\n");
    });

    app.event_after_shutdown.connect([](){
        LOG_MESSAGE("main", "shutdown!\n");
    });

    app.view.event_reset_layout.connect([&](auto view){
        // Bind each window to a dockspace
        view->dock_window("center", fw::AppView::Dockspace_CENTER);
        view->dock_window("right", fw::AppView::Dockspace_RIGHT);
        view->dock_window("top", fw::AppView::Dockspace_TOP);
    });

    app.view.event_draw.connect([&](auto evt){

        // Add a simple menu bar
        if( ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Show splashscreen")) app.config.splashscreen = true;
                if (ImGui::MenuItem("Quit")) app.should_stop = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // do not draw windows when splashscreen is visible
        if( app.config.splashscreen) return;

        if( ImGui::Begin("top"))
        {
            ImGui::TextWrapped("\"top\" window content");
        }
        ImGui::End();

        if( ImGui::Begin("right"))
        {
            ImGui::TextWrapped("\"right\" window content");
        }
        ImGui::End();

        if ( ImGui::Begin("center" )  )
        {
            ImGui::TextWrapped("\"center\" window content");
        }
        ImGui::End();
    });

    app.view.event_draw_splashscreen.connect([](auto view) {
        ImGui::TextWrapped("Welcome to the framework-example app.\nThis demonstrates how to use the framework-gui library.");
        ImGui::Separator();
        ImGui::TextWrapped("\nFor your information, this is the splashscreen window of the app.\n"
                           "You can inject your custom code by doing:\n\n");
        ImGui::Text("app.view.event_draw_splashscreen.connect([](auto view) {\n");
        ImGui::Text("   // your ImGui code here \n");
        ImGui::Text("});\n");
    });
    return app.run();
}
