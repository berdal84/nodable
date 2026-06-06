
#include "fixtures/gui.h"
#include "tools/gui/Config.h"

typedef ::testing::Gui Gui_App;
using namespace ndbl;

TEST_F(Gui_App, constructor)
{
    ndbl::App_State app;
}

TEST_F(Gui_App, init_shutdown)
{
    ndbl::App_State app;
    nodable_init(&app);
    nodable_shutdown(&app);
}

TEST_F(Gui_App, update)
{
    ndbl::App_State app;
    nodable_init(&app);
    nodable_update(&app);
    nodable_shutdown(&app);
}

TEST_F(Gui_App, loop_count_1)
{
    ndbl::App_State app;
    nodable_init(&app);
    loop_count(&app, 1);
    nodable_shutdown(&app);
}

TEST_F(Gui_App, loop_duration_5s)
{
    ndbl::App_State app;
    nodable_init(&app);
    loop_for_x_sec(&app, 5.0 );
    nodable_shutdown(&app);
}

TEST_F(Gui_App, new_file)
{
    ndbl::App_State app;
    nodable_init(&app);
    app.view->show_splashscreen = false;
    nodable_new_file(&app);
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__new_file__0.png");
    nodable_shutdown(&app);
}

TEST_F(Gui_App, open_file)
{
    ndbl::App_State app;
    nodable_init(&app);
    app.view->show_splashscreen = false;
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__open_file__0.png");
    EXPECT_TRUE(nodable_open_asset_file(&app, "examples/arithmetic.cpp"));
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__open_file__1.png");
    nodable_shutdown(&app);
}

TEST_F(Gui_App, close_file)
{
    ndbl::App_State app;
    nodable_init(&app);
    app.view->show_splashscreen = false;
    File* file = nodable_open_asset_file(&app, "examples/arithmetic.cpp");
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__close_file__0.png");
    nodable_close_file(&app, file );
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__close_file__1.png");
    nodable_shutdown(&app);
}

TEST_F(Gui_App, open_examples)
{
    ndbl::App_State app;
    nodable_init(&app);
    app.view->show_splashscreen = false;
    nodable_update(&app);
    save_screenshot(&app, "TEST_Gui_App__open_examples__0.png");
    EXPECT_TRUE(nodable_open_asset_file(&app, "examples/arithmetic.cpp"));
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__open_examples__1.png");
    EXPECT_TRUE(nodable_open_asset_file(&app, "examples/for-loop.cpp"));
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__open_examples__2.png");
    EXPECT_TRUE(nodable_open_asset_file(&app, "examples/if-else.cpp"));
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__open_examples__3.png");
    EXPECT_TRUE(nodable_open_asset_file(&app, "examples/multi-instructions.cpp"));
    loop_for_x_sec(&app, 1.0 );
    save_screenshot(&app, "TEST_Gui_App__open_examples__4.png");
    nodable_shutdown(&app);
}



