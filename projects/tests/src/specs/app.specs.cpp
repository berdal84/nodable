#include <gtest/gtest.h>

#include <ndbl/gui/App.h>

using namespace ndbl;

TEST(App, constructor)
{
    EXPECT_NO_THROW(App app);
}

TEST(App, get_instance)
{
    App app;
    EXPECT_NO_THROW(App::get_instance());
}

TEST(App, init)
{
    App app;
    EXPECT_NO_THROW(app.init());
    app.shutdown();
}

TEST(App, shutdown)
{
    App app;
    app.init();
    EXPECT_NO_THROW(app.shutdown());
}

TEST(App, update)
{
    App app;
    app.init();
    EXPECT_NO_THROW(app.update());
    app.shutdown();
}

TEST(App, draw)
{
    App app;
    app.init();
    app.update();
    EXPECT_NO_THROW(app.draw());
    app.shutdown();
}

