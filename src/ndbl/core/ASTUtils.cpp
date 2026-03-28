#include "ASTUtils.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "ASTNode.h"
#include "ASTScope.h"
#include "ASTSlotLink.h"
#include "ASTUtils.h"

using namespace ndbl;
using namespace tools;

ASTNode* ASTUtils::create_variable(const TypeDescriptor* _type, const std::string& _name)
{
    // create
    auto node = new ASTNode(ASTNodeType_VARIABLE);
    node->init("Var.");
    node->variable_data().init(_type, _name.c_str());
    return node;
}

ASTNode* ASTUtils::create_variable_ref()
{
    auto node = new ASTNode(ASTNodeType_VARIABLE_REF);
    node->init("Ref.");
    node->variable_ref_data().init();
    return node;
}

ASTNode* ASTUtils::create_function(const FunctionDescriptor& _func_type, ASTNodeType _node_type)
{
    ASSERT(_node_type == ASTNodeType_OPERATOR || _node_type == ASTNodeType_FUNCTION );

    auto* node = new ASTNode(_node_type);

    node->init(_func_type.get_identifier());
    node->invokable_data().init(_func_type);
    
    return node;
}

ASTNode* ASTUtils::create_cond_struct()
{
    auto* node = new ASTNode(ASTNodeType_IF_ELSE);

    node->init("If");
    node->init_internal_scope();
    node->switch_behavior_create_branches(2);
    node->switch_behavior_data().m_branch_prefix = {ASTToken_t::keyword_if};

    return node;
}

ASTNode* ASTUtils::create_for_loop()
{
    auto* node = new ASTNode(ASTNodeType_FOR_LOOP);

    node->init("For");

    node->switch_behavior_data().m_branch_prefix = {ASTToken_t::keyword_for};

    // add initialization property and slot
    auto* init_prop = node->add_prop<any>(INITIALIZATION_PROPERTY);
    node->switch_behavior_data().m_initialization_slot = node->add_slot(init_prop, SlotFlag_INPUT, 1);

    // add conditional-related properties and slots
    node->init_internal_scope();
    node->switch_behavior_create_branches(2);

    // add iteration property and slot
    auto iter_prop = node->add_prop<any>(ITERATION_PROPERTY);
    node->switch_behavior_data().m_iteration_slot = node->add_slot(iter_prop, SlotFlag_INPUT, 1);

    return node;
}

ASTNode* ASTUtils::create_while_loop()
{
    auto* node = new ASTNode(ASTNodeType_WHILE_LOOP);    
    node->init("While");
    node->init_internal_scope();
    node->switch_behavior_create_branches(2);
    node->switch_behavior_data().m_branch_prefix = {ASTToken_t::keyword_while};
    return node;
}

ASTNode* ASTUtils::create_scope()
{
    auto* node = new ASTNode(ASTNodeType_SCOPE);
    node->init("Scope");
    node->add_slot(node->value(), SlotFlag_FLOW_IN);
    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->add_slot(node->value(), SlotFlag_FLOW_OUT | SlotFlag_IS_INTERNAL, 1);
    node->init_internal_scope();
    return node;
}

ASTNode* ASTUtils::create_root_scope()
{
    auto* node = new ASTNode(ASTNodeType_ROOT);
    node->init(ICON_FA_ARROW_ALT_CIRCLE_DOWN " BEGIN");
    // add_slot(node->value(), SlotFlag_FLOW_IN, ASTNodeSlot::MAX_CAPACITY); nothing can be before...
    // add_slot(node->value(), SlotFlag_FLOW_OUT, 1); nothing after either...
    node->add_slot(node->value(), SlotFlag_FLOW_OUT | SlotFlag_IS_INTERNAL, 1); // ...but something inside!
    node->init_internal_scope();
    return node;
}

ASTNode* ASTUtils::create_node()
{
    auto* node = new ASTNode;
    node->init("");
    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->add_slot(node->value(), SlotFlag_FLOW_IN);
    return node;
}

ASTNode* ASTUtils::create_literal(const TypeDescriptor *_type)
{
    auto* node = new ASTNode(ASTNodeType_LITERAL);
    node->init("Lit.");
    node->literal_data().init(_type);
    return node;
}

ASTNode* ASTUtils::create_empty_instruction()
{
    auto* node = new ASTNode(ASTNodeType_EMPTY_INSTRUCTION);
    node->init(";");

    // Token will be/or not overriden as a Token_t::end_of_instruction by the parser
    node->value()->set_token({ASTToken_t::ignore});

    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->add_slot(node->value(), SlotFlag_FLOW_IN);
    node->add_slot(node->value(), SlotFlag_OUTPUT  , 1);

    return node;
}

std::vector<ASTNode*> ASTUtils::get_adjacent_nodes(const ASTNode* _node, SlotFlags _flags)
{
    std::vector<ASTNode*> result;
    for ( ASTNodeSlot* slot : _node->filter_slots(_flags ) )
    {
        for( const ASTNodeSlot* adjacent : slot->adjacent() )
        {
            result.emplace_back(adjacent->node );
        }
    }
    return result;
}

ASTNode* ASTUtils::adjacent_node_at(const ASTNode* _node, SlotFlags _flags, u8_t _pos)
{
    if ( ASTNodeSlot* adjacent_slot = _node->find_adjacent_at(_flags, _pos ) )
    {
        return adjacent_slot->node;
    }
    return {};
}

bool ASTUtils::is_instruction(const ASTNode* node)
{
    if ( is_connected_to_codeflow(node) )
        return true;
    if (node->type() == ASTNodeType_VARIABLE )
        return true;
    return false;
}

bool ASTUtils::is_connected_to_codeflow(const ASTNode *node)
{
    if (node->flow_inputs().size() )
        return true;
    if (node->flow_outputs().size() )
        return true;
    return false;
}

bool ASTUtils::can_be_instruction(const ASTNode* node)
{
    // TODO: handle case where a variable has inputs/outputs but not connected to the code flow
    return node->slot_count(SlotFlag_TYPE_FLOW) > 0 && node->inputs().empty() && node->outputs().empty();
}

bool ASTUtils::is_unary_operator(const ASTNode* node)
{
    if (node->type() == ASTNodeType_OPERATOR )
        if (node->invokable_data().get_func_type()->arg_count() == 1 )
            return true;
    return false;
}

bool ASTUtils::is_binary_operator(const ASTNode* node)
{
    if (node->type() == ASTNodeType_OPERATOR )
        if (node->invokable_data().get_func_type()->arg_count() == 2 )
            return true;
    return false;
}

bool ASTUtils::is_conditional(const ASTNode* node)
{
    switch ( node->type() )
    {
        case ASTNodeType_FOR_LOOP:
        case ASTNodeType_WHILE_LOOP:
        case ASTNodeType_IF_ELSE:
            return true;
        default:
            return false;
    };
}

bool ASTUtils::is_output_node_in_expression(const ASTNode* input_node, const ASTNode* output_node)
{
#ifdef NDBL_DEBUG
    ASSERT(input_node);
    ASSERT(output_node);
    const bool is_an_output = std::find(input_node->outputs().begin(), input_node->outputs().end(), output_node) != input_node->outputs().end();
    ASSERT(is_an_output);
#endif

    if ( ASTUtils::is_instruction(input_node ) )
    {
        if (input_node->type() == ASTNodeType_VARIABLE )
        {
            const ASTNodeSlot* declaration_out = input_node->variable_data().decl_out();
            return declaration_out->first_adjacent_node() == output_node;
        }
        return false;
    }
    return input_node->outputs().front() == output_node;
}

bool ASTUtils::is_initialized(const ASTNode* node)
{
    return node != nullptr && node->is_initialized();
}
