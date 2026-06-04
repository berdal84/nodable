#pragma once
#include "tools/gui/AppView.h"

namespace tools
{
    // Forward declarations
    class AppExample;

    // This example is OOP
    class AppExampleView : public AppViewState
    {
    public:
        void            init(AppExample* _app);
        void            shutdown();
        void            update();
        void            draw();
        void            _draw_splashscreen_content();
        void            _reset_layout();
    private:
        AppExample*     m_app = nullptr; // NOT owned
    };
}