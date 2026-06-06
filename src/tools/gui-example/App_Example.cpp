#include "App_Example.h"
#include "App_View_Example.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

using namespace tools;

void App_Example::init()
{
    // init a config and a view (this class owns them!)
    m_config = init_config();
    m_view.init(this);

    // Init the base application using our data
    app_init_ex(this, &m_view, m_config );

    //
    // Your code here
    //
}

void App_Example::_do_frame()
{
    update();
    draw();
}

#ifdef __EMSCRIPTEN__
namespace tools
{
    App_Example* g_instance = nullptr;
    void emscripten_loop()
    {
        VERIFY(g_instance != nullptr, "Did you forgot to set g_instance prior to set_main_loop?");
        g_instance->_do_frame();
    }
}
#endif

void App_Example::run()
{
  #ifdef __EMSCRIPTEN__
    g_instance = this;
    emscripten_set_main_loop(&tools::emscripten_loop, 0, true);
  #else
    while( !should_stop() )
    {
        _do_frame();
    }    
  #endif
}

void App_Example::update()
{
    app_update(this);
    //
    // Your code here
    //
}

void App_Example::draw()
{
    m_view.draw();
}

void App_Example::shutdown()
{
    // Shutdown our stuff
    app_shutdown(this);
    m_view.shutdown();
    shutdown_config(m_config);

    //
    // Your code here
    //
}

void App_Example::request_stop()
{
    app_request_stop(this);
}
