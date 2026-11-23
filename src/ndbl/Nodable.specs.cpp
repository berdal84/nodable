
#include "test/fixtures/nodable_gui_test.h"

typedef ::testing::Nodable_Gui_Test Gui_App;
using namespace ndbl;

const float wait_time_in_secs = 0.200;

TEST_F(Gui_App, init_deinit)
{
}

TEST_F(Gui_App, update)
{
    ndbl::app_update();
}

TEST_F(Gui_App, loop_for_n_frame)
{
    loop_for_n_frame(1);
}

TEST_F(Gui_App, loop_for_n_sec)
{
    loop_for_n_sec(wait_time_in_secs);
}

TEST_F(Gui_App, new_file)
{
    ndbl::appview_show_splashscreen(false);
    ndbl::app_new_file();
    loop_for_n_sec(wait_time_in_secs);
    ndbl::appview_save_screenshot("TEST_Gui_ndbl::app__new_file__0.png");
}

TEST_F(Gui_App, open_file)
{
    ndbl::appview_show_splashscreen(false);
    loop_for_n_sec(wait_time_in_secs);
    appview_save_screenshot("TEST_Gui_ndbl::app__open_file__0.png");

    EXPECT_TRUE(ndbl::app_open_asset_file("examples/arithmetic.cpp"));
    loop_for_n_sec(wait_time_in_secs);
    appview_save_screenshot("TEST_Gui_ndbl::app__open_file__1.png");
}

TEST_F(Gui_App, close_file)
{
    ndbl::appview_show_splashscreen(false);

    File* file = ndbl::app_open_asset_file("examples/arithmetic.cpp");

    loop_for_n_sec(wait_time_in_secs);
    appview_save_screenshot("TEST_Gui_ndbl::app__close_file__0.png");

    ndbl::app_close_file(file );
    
    loop_for_n_sec(wait_time_in_secs);
    appview_save_screenshot("TEST_Gui_ndbl::app__close_file__1.png");
}

TEST_F(Gui_App, open_examples)
{
    ndbl::appview_show_splashscreen(false);
    loop_for_n_sec(wait_time_in_secs);
    
    EXPECT_TRUE(ndbl::app_open_asset_file("examples/arithmetic.cpp"));
    loop_for_n_sec(wait_time_in_secs);
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__arithmetic.cpp.png");

    EXPECT_TRUE(ndbl::app_open_asset_file("examples/for-loop.cpp"));
    loop_for_n_sec(wait_time_in_secs);
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__for-loop.cpp.png");
    
    EXPECT_TRUE(ndbl::app_open_asset_file("examples/if-else.cpp"));
    loop_for_n_sec(wait_time_in_secs);
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__if-else.cpp.png");
    
    EXPECT_TRUE(ndbl::app_open_asset_file("examples/multi-instructions.cpp"));
    loop_for_n_sec(wait_time_in_secs);
    appview_save_screenshot("TEST_Gui_ndbl::app__open_examples__multi-instructions.cpp.png");
}



