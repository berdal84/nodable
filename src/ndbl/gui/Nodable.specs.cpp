
#include "fixtures/gui.h"
#include "tools/gui/Config.h"

typedef ::testing::Gui Gui_App;
using namespace ndbl;

TEST_F(Gui_App, constructor)
{
    NodableState state;
}

TEST_F(Gui_App, init_shutdown)
{
    NodableState state;
    nodable_init(&state);
    nodable_shutdown(&state);
}

TEST_F(Gui_App, update)
{
    NodableState state;
    nodable_init(&state);
    nodable_update(&state);
    nodable_shutdown(&state);
}

TEST_F(Gui_App, loop_count_1)
{
    NodableState state;
    nodable_init(&state);
    loop_count(&state, 1);
    nodable_shutdown(&state);
}

TEST_F(Gui_App, loop_duration_5s)
{
    NodableState state;
    nodable_init(&state);
    loop_for_x_sec(&state, 5.0 );
    nodable_shutdown(&state);
}

TEST_F(Gui_App, new_file)
{
    NodableState state;
    nodable_init(&state);
    state.view->base.show_splashscreen = false;
    nodable_new_file(&state);
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__new_file__0.png");
    nodable_shutdown(&state);
}

TEST_F(Gui_App, open_file)
{
    NodableState state;
    nodable_init(&state);
    state.view->base.show_splashscreen = false;
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__open_file__0.png");
    EXPECT_TRUE(nodable_open_asset_file(&state, "examples/arithmetic.cpp"));
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__open_file__1.png");
    nodable_shutdown(&state);
}

TEST_F(Gui_App, close_file)
{
    NodableState state;
    nodable_init(&state);
    state.view->base.show_splashscreen = false;
    File* file = nodable_open_asset_file(&state, "examples/arithmetic.cpp");
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__close_file__0.png");
    nodable_close_file(&state, file );
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__close_file__1.png");
    nodable_shutdown(&state);
}

TEST_F(Gui_App, open_examples)
{
    NodableState state;
    nodable_init(&state);
    state.view->base.show_splashscreen = false;
    nodable_update(&state);
    save_screenshot(&state, "TEST_Gui_App__open_examples__0.png");
    EXPECT_TRUE(nodable_open_asset_file(&state, "examples/arithmetic.cpp"));
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__open_examples__1.png");
    EXPECT_TRUE(nodable_open_asset_file(&state, "examples/for-loop.cpp"));
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__open_examples__2.png");
    EXPECT_TRUE(nodable_open_asset_file(&state, "examples/if-else.cpp"));
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__open_examples__3.png");
    EXPECT_TRUE(nodable_open_asset_file(&state, "examples/multi-instructions.cpp"));
    loop_for_x_sec(&state, 1.0 );
    save_screenshot(&state, "TEST_Gui_App__open_examples__4.png");
    nodable_shutdown(&state);
}



