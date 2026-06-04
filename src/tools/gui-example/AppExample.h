#pragma once
#include "tools/gui/App.h"
#include "tools/gui/Config.h"
#include "AppExampleView.h"

namespace tools
{
    // forward declaration
    class AppExampleView;
    
    class AppExample : public AppState
    {
        friend AppExampleView;
    public:
        void        init();
        void        run();
        void        shutdown();
        void        update();
        void        draw();
        bool        should_stop() const { return app_should_stop(this); };
        void        _do_frame();
    private:
        void        request_stop();

        AppExampleView  m_view;
        Config*         m_config = nullptr;
    };
}