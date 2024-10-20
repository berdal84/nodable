#include "ndbl/core/ASTScope.h"
#include "fixtures/core.h"
#include <gtest/gtest.h>

using namespace ndbl;

typedef ::testing::Core ASTNode_;

TEST_F(ASTNode_, find_parent)
{
    auto parent = app.get_graph()->create_scope();
    auto child  = app.get_graph()->create_node();

    app.get_graph()->connect(
        *parent->find_slot( SlotFlag_CHILD ),
        *child->find_slot( SlotFlag_PARENT )
    );

    EXPECT_TRUE( child->find_parent() );
    EXPECT_EQ( child->find_parent(), parent );
}