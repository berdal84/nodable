#include <fw/gui/Nodable.h>

int main(int argc, char *argv[])
{
    // Create a framework configuration, and override some fields ...
    fw::Config conf;
    conf.app_window_label = "framework-example - (based on framework-gui library)";

    // Instantiate the application using the predefined configuration
    fw::Nodable app{conf};

    // Plug custom code when app state changes:
    app.changes.connect([](fw::Nodable::StateChange state) {
        switch (state)
        {
            case fw::Nodable::ON_DRAW:
            case fw::Nodable::ON_UPDATE:
                break;
            case fw::Nodable::ON_INIT:
                LOG_MESSAGE("main", "My ON_INIT log!\n");
                break;
            case fw::Nodable::ON_SHUTDOWN:
                LOG_MESSAGE("main", "My ON_SHUTDOWN log!\n");
                break;
        }
    });

    // Plug custom code when app's view state changes:
    app.view.changes.connect([&](fw::NodableView::StateChange state) {
          switch (state)
          {
              case fw::NodableView::ON_DRAW_MAIN:
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
              case fw::NodableView::ON_DRAW_SPLASHSCREEN_CONTENT:
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
              case fw::NodableView::ON_RESET_LAYOUT:
              {
                  // Bind each window to a dockspace
                  app.view.dock_window("center", fw::NodableView::Dockspace_CENTER);
                  app.view.dock_window("right", fw::NodableView::Dockspace_RIGHT);
                  app.view.dock_window("top", fw::NodableView::Dockspace_TOP);
                  break;
              }
          }
      });

    // Run the main loop until user closes the app or a crash happens...
    return app.run();
}
