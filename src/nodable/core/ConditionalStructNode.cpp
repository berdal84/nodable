#include "ConditionalStructNode.h"
#include "fw/core/reflection/reflection"
#include "Scope.h"
#include "InstructionNode.h"
#include "GraphUtil.h"

using namespace ndbl;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<ConditionalStructNode>("ConditionalStructNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

ConditionalStructNode::ConditionalStructNode()
    : Node()
{
    add_prop<ID<Node>>(CONDITION_PROPERTY, Visibility::Always, Way::In);
}

ID<Scope> ConditionalStructNode::get_condition_true_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, Relation::NEXT_PREVIOUS, Way::Out, 0);
}

ID<Scope> ConditionalStructNode::get_condition_false_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, Relation::NEXT_PREVIOUS, Way::Out, 1);
}

bool ConditionalStructNode::has_elseif() const
{
    if( ID<Node> node = GraphUtil::adjacent_node_at(this, Relation::NEXT_PREVIOUS, Way::Out, 1) )
    {
        return node->get_type()->is_child_of<ConditionalStructNode>();
    }
    return false;
}

const Property* ConditionalStructNode::condition_property() const
{
    return get_prop(CONDITION_PROPERTY);
}