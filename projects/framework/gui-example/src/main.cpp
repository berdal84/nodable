#include <fw/gui/App.h>

int main(int argc, char *argv[])
{
    // Create a framework configuration, and override some fields ...
    fw::Config conf;
    conf.app_window_label = "framework-example - (based on framework-gui library)";

    // Instantiate the application using the predefined configuration
    fw::App app{conf};

    // Plug custom code when app state changes:
    app.signal_handler = [](fw::App::Signal state) {
        switch (state)
        {
            case fw::App::Signal_ON_DRAW:
            case fw::App::Signal_ON_UPDATE:
                break;
            case fw::App::Signal_ON_INIT:
                LOG_MESSAGE("main", "My ON_INIT log!\n");
                break;
            case fw::App::Signal_ON_SHUTDOWN:
                LOG_MESSAGE("main", "My ON_SHUTDOWN log!\n");
                break;
        }
    };

    // Plug custom code when app's view state changes:
    app.view.signal_handler = [&](fw::AppView::Signal state) {
          switch (state)
          {
              case fw::AppView::Signal_ON_DRAW_MAIN:
              {
                  // Add a simple menu bar
                  if (ImGui::BeginMainMenuBar())
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
                  if (app.config.splashscreen) return;

                  if (ImGui::Begin("top"))
                  {
                      ImGui::TextWrapped("\"top\" window content");
                  }
                  ImGui::End();

                  if (ImGui::Begin("right"))
                  {
                      ImGui::TextWrapped("\"right\" window content");
                  }
                  ImGui::End();

                  if (ImGui::Begin("center"))
                  {
                      ImGui::TextWrapped("\"center\" window content");
                  }
                  ImGui::End();
                  break;
              }
              case fw::AppView::Signal_ON_DRAW_SPLASHSCREEN_CONTENT:
              {
                  ImGui::TextWrapped("Welcome to the framework-gui-example app.\nThis demonstrates how to use the framework-gui library.");
                  ImGui::Separator();
                  ImGui::TextWrapped("\nFor your information, this is the splashscreen window of the app.\n"
                                     "You can inject your custom code by doing:\n\n");
                  ImGui::Text("app.view.event_draw_splashscreen.connect([](auto view) {\n");
                  ImGui::Text("   // your ImGui code here \n");
                  ImGui::Text("});\n");
                  break;
              }
              case fw::AppView::Signal_ON_RESET_LAYOUT:
              {
                  // Bind each window to a dockspace
                  app.view.dock_window("center", fw::AppView::Dockspace_CENTER);
                  app.view.dock_window("right", fw::AppView::Dockspace_RIGHT);
                  app.view.dock_window("top", fw::AppView::Dockspace_TOP);
                  break;
              }
          }
      };

    // Run the main loop until user closes the app or a crash happens...
    return app.run();
}
