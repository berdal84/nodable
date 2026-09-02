#include <gtest/gtest.h>
#include "Spatial_Node.h"

using namespace tools;

TEST(Spatial_Node, add_child)
{
    // prepare
    Spatial_Node child, root;

    // act
    spatialnode_add_child(&root, &child );

    // verify
    EXPECT_TRUE(child.parent == &root );
    EXPECT_TRUE(*root.children.begin() == &child );
}

TEST(Spatial_Node, remove_child)
{
    // requires add_child to succeed

    // prepare
    Spatial_Node child, root;
    spatialnode_add_child( &root, &child );

    // act
    spatialnode_remove_child( &root, &child );

    // verify
    EXPECT_TRUE(child.parent == nullptr );
    EXPECT_TRUE(root.children.size() == 0 );
}


TEST(Spatial_Node_2D, add_child__with_offset)
{
    // prepare
    Spatial_Node child, root;

    // act
    spatialnode_translate(&child, { 10.f, 15.f});
    spatialnode_add_child(&root, &child);

    // verify
    EXPECT_TRUE(child.parent == &root );
    EXPECT_TRUE(*root.children.begin() == &child );
    EXPECT_FLOAT_EQ( spatialnode_position(&child).x, 10.f );
    EXPECT_FLOAT_EQ( spatialnode_position(&child).y, 15.f );
}

TEST(Spatial_Node_2D, set_pos__LOCAL_SPACE)
{
    // !!! This requires add_child to pass !!!

    // prepare
    Spatial_Node child, root;
    spatialnode_add_child( &root, &child );

    // act
    spatialnode_translate( &child, {-10.f, -10.f});

    // verify
    EXPECT_FLOAT_EQ(spatialnode_position(&child).x, -10.f );
    EXPECT_FLOAT_EQ(spatialnode_position(&child).y, -10.f );
}

TEST(Spatial_Node_2D, set_pos__PARENT_SPACE)
{
    // !!! This requires add_child__with_offset and set_pos__LOCAL_SPACE to pass !!!

    // prepare
    Spatial_Node child, root;
    spatialnode_translate( &child, {10.f, 10.f});
    spatialnode_add_child( &root, &child );

    EXPECT_FLOAT_EQ(spatialnode_position(&child).x, 10.f );
    EXPECT_FLOAT_EQ(spatialnode_position(&child).y, 10.f );

    // act
    spatialnode_set_position( &child, {0.f, 0.f}, PARENT_SPACE);

    // verify
    EXPECT_FLOAT_EQ(spatialnode_position(&child).x, spatialnode_position(&root).x );
    EXPECT_FLOAT_EQ(spatialnode_position(&child).y, spatialnode_position(&root).y );
}

TEST(Spatial_Node_2D, get_pos__GLOBAL_SPACE)
{
    // !!! This requires add_child and set_pos__LOCAL_SPACE to pass !!!

    // prepare
    Spatial_Node level1, level0, root;

    spatialnode_add_child(&root, &level0);
    spatialnode_add_child(&level0, &level1);

    spatialnode_translate(&root, {10.f, 10.f});
    spatialnode_translate(&level0, {10.f, 10.f});
    spatialnode_translate(&level1, {10.f, 10.f});

    // pre check
    EXPECT_FLOAT_EQ(spatialnode_position( &level1 ).x, 10.f );
    EXPECT_FLOAT_EQ(spatialnode_position( &level1 ).y, 10.f );
    EXPECT_FLOAT_EQ(spatialnode_position( &level1, WORLD_SPACE).x, 30.f );
    EXPECT_FLOAT_EQ(spatialnode_position( &level1, WORLD_SPACE).y, 30.f );
}

TEST(Spatial_Node_2D, set_pos__GLOBAL_SPACE)
{
    // !!! This requires add_child and get_pos__SCREEN_SPACE to pass !!!

    // prepare
    Spatial_Node child, root;
    spatialnode_add_child(&root, &child);
    spatialnode_translate(&root, {10.f, 10.f});
    spatialnode_translate(&child, {10.f, 10.f});

    // pre check
    EXPECT_FLOAT_EQ(spatialnode_position( &child, PARENT_SPACE).x, 10.f );
    EXPECT_FLOAT_EQ(spatialnode_position( &child, PARENT_SPACE).y, 10.f );

    // act
    spatialnode_set_position(&child, {0.f, 0.f}, WORLD_SPACE);

    // check
    EXPECT_FLOAT_EQ(spatialnode_position( &child, PARENT_SPACE).x, -10.f );
    EXPECT_FLOAT_EQ(spatialnode_position( &child, PARENT_SPACE).y, -10.f );
    EXPECT_FLOAT_EQ(spatialnode_position( &child, WORLD_SPACE).x, 0.f );
    EXPECT_FLOAT_EQ(spatialnode_position( &child, WORLD_SPACE).y, 0.f );
}