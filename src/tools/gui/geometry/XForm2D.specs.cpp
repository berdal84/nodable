#include <gtest/gtest.h>
#include "XForm2D.h"

using namespace tools;

TEST(XForm2D, add_child)
{
    // prepare
    XForm2D root, child;

    // act
    root.add_child( &child );

    // verify
    EXPECT_TRUE(child._parent == &root );
    EXPECT_TRUE(root._children[0] == &child );
}

TEST(XForm2D, add_child__with_offset)
{
    // prepare
    XForm2D root, child;

    // act
    child.translate({ 10.f, 15.f});
    root.add_child( &child );

    // verify
    EXPECT_TRUE(child._parent == &root );
    EXPECT_TRUE(root._children[0] == &child );
    EXPECT_FLOAT_EQ(child.get_pos().x, 10.f );
    EXPECT_FLOAT_EQ(child.get_pos().y, 15.f );
}

TEST(XForm2D, set_pos__LOCAL_SPACE)
{
    // !!! This requires add_child to pass !!!

    // prepare
    XForm2D root;
    XForm2D child;
    root.add_child( &child );

    // act
    child.translate({-10.f, -10.f});

    // verify
    EXPECT_FLOAT_EQ(child.get_pos().x, -10.f );
    EXPECT_FLOAT_EQ(child.get_pos().y, -10.f );
}

TEST(XForm2D, set_pos__PARENT_SPACE)
{
    // !!! This requires add_child__with_offset and set_pos__LOCAL_SPACE to pass !!!

    // prepare
    XForm2D root;
    XForm2D child;
    child.translate({10.f, 10.f});
    root.add_child(&child);

    EXPECT_FLOAT_EQ(child.get_pos().x, 10.f );
    EXPECT_FLOAT_EQ(child.get_pos().y, 10.f );

    // act
    child.set_pos({0.f, 0.f}, PARENT_SPACE);

    // verify
    EXPECT_FLOAT_EQ(child.get_pos().x, root.get_pos().x );
    EXPECT_FLOAT_EQ(child.get_pos().y, root.get_pos().y );
}

TEST(XForm2D, get_pos__SCREEN_SPACE)
{
    // !!! This requires add_child and set_pos__LOCAL_SPACE to pass !!!

    // prepare
    XForm2D root;
    XForm2D level0;
    XForm2D level1;

    root.add_child(&level0);
    level0.add_child(&level1);

    root.translate({10.f, 10.f});
    level0.translate({10.f, 10.f});
    level1.translate({10.f, 10.f});

    // pre check
    EXPECT_FLOAT_EQ(level1.get_pos(PARENT_SPACE).x, 10.f );
    EXPECT_FLOAT_EQ(level1.get_pos(PARENT_SPACE).y, 10.f );
    EXPECT_FLOAT_EQ(level1.get_pos(SCREEN_SPACE).x, 30.f );
    EXPECT_FLOAT_EQ(level1.get_pos(SCREEN_SPACE).y, 30.f );
}

TEST(XForm2D, set_pos__SCREEN_SPACE)
{
    // !!! This requires add_child and get_pos__SCREEN_SPACE to pass !!!

    // prepare
    XForm2D root;
    XForm2D child;
    root.add_child(&child);
    root.translate({10.f, 10.f});
    child.translate({10.f, 10.f});

    // pre check
    EXPECT_FLOAT_EQ(child.get_pos(PARENT_SPACE).x, 10.f );
    EXPECT_FLOAT_EQ(child.get_pos(PARENT_SPACE).y, 10.f );

    // act
    child.set_pos({0.f, 0.f}, SCREEN_SPACE);

    // check
    EXPECT_FLOAT_EQ(child.get_pos(PARENT_SPACE).x, -10.f );
    EXPECT_FLOAT_EQ(child.get_pos(PARENT_SPACE).y, -10.f );
    EXPECT_FLOAT_EQ(child.get_pos(SCREEN_SPACE).x, 0.f );
    EXPECT_FLOAT_EQ(child.get_pos(SCREEN_SPACE).y, 0.f );
}