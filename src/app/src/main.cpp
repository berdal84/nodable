
#include <string>

#include <nodable/App.h>
#include <nodable/AppView.h>
#include <nodable/Config.h>

using namespace Nodable;

int main(int argc, char *argv[]) {
  Log::SetVerbosityLevel("Reflect", Log::Verbosity::Verbose);
  Log::SetVerbosityLevel("File", Log::Verbosity::Message);
  Log::SetVerbosityLevel("GraphTraversal", Log::Verbosity::Message);
  Log::SetVerbosityLevel("Parser", Log::Verbosity::Message);
  Log::SetVerbosityLevel("GraphNode", Log::Verbosity::Message);
  Log::SetVerbosityLevel("GraphNodeView", Log::Verbosity::Verbose);
  Log::SetVerbosityLevel("VM", Log::Verbosity::Message);

  App app("Nodable " NODABLE_VERSION_EXTENDED);
  app.init();
  auto startupFilePath = app.getAssetPath("txt/startup.txt");
  app.openFile(startupFilePath); // Init and open a startup file

  try {
    while (app.update()) {
      if (AppView *view = app.getView()) {
        view->draw();
      }
    }
  } catch (std::exception &err) {
    LOG_ERROR("main", "Application crashes: %s\n", err.what())
  }
  app.shutdown();
  LOG_FLUSH()
  return 0;
}
