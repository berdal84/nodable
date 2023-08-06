#include "nodable/gui/Nodable.h"
#include "nodable/core/ComponentManager.h"
#include "nodable/core/InvokableComponent.h"
#include "nodable/core/Scope.h"
#include "nodable/gui/NodeView.h"
#include "nodable/gui/Physics.h"

int main(int argc, char *argv[])
{
    using namespace ndbl;
    using namespace fw;

    type_register::log_statistics();  // log statistics relative to the reflection system
    ComponentManager::init<NodeView, Physics, Scope, InvokableComponent>();
    Nodable app; // Instantiate app
    return app.run();  // Run main loop
}
