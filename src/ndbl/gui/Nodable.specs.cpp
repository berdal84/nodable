
#include "gui/Nodable.h"
#include "fixtures/gui.h"
#include "gui/Nodable_View.h"

typedef ::testing::Gui Gui_App;
using namespace ndbl;

TEST_F(Gui_App, constructor)
{
    ndbl::App_State app;
}

TEST_F(Gui_App, init_deinit)
{
    ndbl::app_init();
    ndbl::app_shutdown();
}

TEST_F(Gui_App, update)
{
    ndbl::app_init();
    ndbl::app_update();
    ndbl::app_shutdown();
}

TEST_F(Gui_App, loop_count_1)
{
    ndbl::app_init();
    loop_count(1);
    ndbl::app_shutdown();
}

TEST_F(Gui_App, loop_duration_5s)
{
    ndbl::app_init();
    loop_for_x_sec(5.0 );
    ndbl::app_shutdown();
}

TEST_F(Gui_App, new_file)
{
    ndbl::app_init();
    ndbl::appview_show_splashscreen(false);
    ndbl::app_new_file();
    loop_for_x_sec(1.0 );
    ndbl::appview_save_screenshot("TEST_Gui_ndbl::app__new_file__0.png");
    ndbl::app_shutdown();
}

TEST_F(Gui_App, open_file)
{
    ndbl::App_State app;
    ndbl::app_init();
    ndbl::appview_show_splashscreen(false);
    loop_for_x_sec(1.0 );
    appview_save_screenshot("TEST_Gui_ndbl::app__open_file__0.png");
    EXPECT_TRUE(ndbl::app_open_asset_file("examples/arithmetic.cpp"));
    loop_for_x_sec(1.0 );
    appview_save_screenshot("TEST_Gui_ndbl::app__open_file__1.png");
    ndbl::app_shutdown();
}

TEST_F(Gui_App, close_file)
{
    ndbl::App_State app;
    ndbl::app_init();
    ndbl::appview_show_splashscreen(false);
    File* file = ndbl::app_open_asset_file("examples/arithmetic.cpp");
    loop_for_x_sec(1.0 );
    appview_save_screenshot("TEST_Gui_ndbl::app__close_file__0.png");
    ndbl::app_close_file(file );
    loop_for_x_sec(1.0 );
    appview_save_screenshot("TEST_Gui_ndbl::app__close_file__1.png");
    ndbl::app_shutdown();
}

TEST_F(Gui_App, open_examples)
{
    ndbl::App_State app;
    ndbl::app_init();
    ndbl::appview_show_splashscreen(false);
    ndbl::app_update();
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__0.png");
    EXPECT_TRUE(ndbl::app_open_asset_file("examples/arithmetic.cpp"));
    loop_for_x_sec(1.0 );
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__1.png");
    EXPECT_TRUE(ndbl::app_open_asset_file("examples/for-loop.cpp"));
    loop_for_x_sec(1.0 );
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__2.png");
    EXPECT_TRUE(ndbl::app_open_asset_file("examples/if-else.cpp"));
    loop_for_x_sec(1.0 );
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__3.png");
    EXPECT_TRUE(ndbl::app_open_asset_file("examples/multi-instructions.cpp"));
    loop_for_x_sec(1.0 );
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__4.png");
    ndbl::app_shutdown();
}



