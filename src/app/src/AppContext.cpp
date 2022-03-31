#include <nodable/app/AppContext.h>

#include <nodable/app/Settings.h>
#include <nodable/app/App.h>
#include <nodable/core/VM.h>
#include <nodable/core/languages/Nodable.h>
#include <nodable/core/Texture.h>

using namespace Nodable;

AppContext::~AppContext()
{
    delete settings;
    delete vm;
    delete language;
    delete texture_manager;
}

AppContext* AppContext::create_default(App* _app)
{
    auto context = new AppContext(_app);
    context->settings        = Settings::create_default();
    context->vm              = new vm::VM();
    context->language        = new LanguageNodable();
    context->texture_manager = new TextureManager();
    return context;
}