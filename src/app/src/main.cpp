
#include <string>

#include <nodable/App.h>
#include <nodable/AppView.h>
#include <nodable/Config.h>
#include <nodable/GraphNodeView.h>
#include <nodable/ComputeBase.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/ComputeUnaryOperation.h>
#include <nodable/ComputeBinaryOperation.h>
#include <nodable/Settings.h>

using namespace Nodable;

int main(int argc, char *argv[]) {
  Log::SetVerbosityLevel("Reflect", Log::Verbosity::Message);
  Log::SetVerbosityLevel("File", Log::Verbosity::Verbose);
  Log::SetVerbosityLevel("GraphTraversal", Log::Verbosity::Verbose);
  Log::SetVerbosityLevel("Parser", Log::Verbosity::Verbose);
  Log::SetVerbosityLevel("GraphNode", Log::Verbosity::Verbose);
  Log::SetVerbosityLevel("Runner", Log::Verbosity::Verbose);

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
