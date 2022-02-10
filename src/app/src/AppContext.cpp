#include <nodable/AppContext.h>

#include <nodable/Settings.h>
#include <nodable/App.h>
#include <nodable/VM.h>
#include <nodable/LanguageNodable.h>
#include <nodable/Texture.h>

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
    context->settings = Settings::create_default();
    context->vm       = new Asm::VM();
    context->language = new LanguageNodable();
    context->texture_manager = new TextureManager();
    return context;
}