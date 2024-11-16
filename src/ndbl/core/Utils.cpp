#include "Utils.h"
#include "FunctionNode.h"
#include "VariableNode.h"

using namespace ndbl;

std::vector<Node*> Utils::get_adjacent_nodes(const Node* _node, SlotFlags _flags)
{
    std::vector<Node*> result;
    for ( Slot* slot : _node->filter_slots( _flags ) )
    {
        for( const Slot* adjacent : slot->adjacent() )
        {
            result.emplace_back(adjacent->node );
        }
    }
    return result;
}

Node* Utils::adjacent_node_at(const Node* _node, SlotFlags _flags, u8_t _pos)
{
    if ( Slot* adjacent_slot = _node->find_adjacent_at( _flags, _pos ) )
    {
        return adjacent_slot->node;
    }
    return {};
}

bool Utils::is_instruction(const Node* node)
{
    if ( is_connected_to_codeflow(node) )
        return true;
    if ( node->type() == NodeType_VARIABLE )
        return true;
    return false;
}

bool Utils::is_connected_to_codeflow(const Node *node)
{
    if (node->flow_inputs().size() )
        return true;
    if (node->flow_outputs().size() )
        return true;
    return false;
}

bool Utils::can_be_instruction(const Node* node)
{
    // TODO: handle case where a variable has inputs/outputs but not connected to the code flow
    return node->slot_count(SlotFlag_TYPE_FLOW) > 0 && node->inputs().empty() && node->outputs().empty();
}

bool Utils::is_unary_operator(const Node* node)
{
    if ( node->type() == NodeType_OPERATOR )
        if (static_cast<const FunctionNode*>(node)->get_func_type()->get_arg_count() == 1 )
            return true;
    return false;
}

bool Utils::is_binary_operator(const Node* node)
{
    if ( node->type() == NodeType_OPERATOR )
        if (static_cast<const FunctionNode*>(node)->get_func_type()->get_arg_count() == 2 )
            return true;
    return false;
}

bool Utils::is_conditional(const Node* node)
{
    switch ( node->type() )
    {
        case NodeType_BLOCK_FOR_LOOP:
        case NodeType_BLOCK_WHILE_LOOP:
        case NodeType_BLOCK_IF:
            return true;
        default:
            return false;
    };
}

bool Utils::is_output_node_in_expression(const Node* input_node, const Node* output_node)
{
    ASSERT(input_node != nullptr);
    ASSERT(output_node != nullptr);

    if ( Utils::is_instruction(input_node ) )
    {
        if (input_node->type() == NodeType_VARIABLE )
        {
            const Slot* declaration_out = static_cast<const VariableNode*>(input_node)->decl_out();
            return declaration_out->first_adjacent_node() == output_node;
        }
        return false;
    }

    return input_node->outputs().front() == output_node;
}