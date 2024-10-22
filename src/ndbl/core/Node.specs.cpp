#include "ndbl/core/Scope.h"
#include "fixtures/core.h"
#include <gtest/gtest.h>

using namespace ndbl;

typedef ::testing::Core Node_;

TEST_F(Node_, find_parent)
{
    auto parent = app.get_graph()->create_scope();
    auto child  = app.get_graph()->create_node();

    app.get_graph()->connect(
        *parent->find_slot( SlotFlag_CHILD ),
        *child->find_slot( SlotFlag_PARENT )
    );

    EXPECT_TRUE( child->parent() );
    EXPECT_EQ( child->parent(), parent );
}