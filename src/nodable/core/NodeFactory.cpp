#include "NodeFactory.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "fw/core/reflection/reflection"

#include "InstructionNode.h"
#include "VariableNode.h"
#include "LiteralNode.h"
#include "InvokableComponent.h"
#include "Scope.h"
#include "ConditionalStructNode.h"
#include "ForLoopNode.h"
#include "WhileLoopNode.h"
#include "NodeUtils.h"

using namespace ndbl;
using fw::pool::Pool;

ID<InstructionNode> NodeFactory::create_instr() const
{
    auto instr_node = Pool::get_pool()->create<InstructionNode>();
    instr_node->set_name("Instr.");
    m_post_process(instr_node);
    return instr_node;
}

ID<VariableNode> NodeFactory::create_variable(const fw::type *_type, const std::string& _name, ID<Scope> _scope) const
{
    // create
    auto node = Pool::get_pool()->create<VariableNode>(_type, _name.c_str());

    if( _scope )
    {
        _scope->add_variable( node );
    }
    else
    {
        LOG_WARNING("HeadlessNodeFactory", "Variable %s has been created without defining its scope.\n", _name.c_str())
    }

    m_post_process(node);

    return node;
}

ID<Node> NodeFactory::create_abstract_func(const fw::func_type* _signature, bool _is_operator) const
{
    ID<Node> node = _create_abstract_func(_signature, _is_operator);
    add_invokable_component(node, _signature, nullptr, _is_operator );
    m_post_process(node);
    return node;
}

ID<Node> NodeFactory::_create_abstract_func(const fw::func_type* _func_type, bool _is_operator) const
{
    ID<Node> node_id = Pool::get_pool()->create<Node>();
    Node*    node    = node_id.get();

    if( _is_operator )
    {
        node->set_name(_func_type->get_identifier().c_str());
    }
    else
    {
        std::string id = _func_type->get_identifier();
        std::string label       = id + "()";
        std::string short_label = id.substr(0, 2) + "..()"; // ------- improve, not great.
        node->set_name(label.c_str());
    }

    // Create a result/value
    node->props.add(_func_type->get_return_type(), VALUE_PROPERTY, Visibility::Default, Way::Out);

    // Create arguments
    auto args = _func_type->get_args();

    if( _is_operator ) // rename arguments
    {
        size_t count = _func_type->get_arg_count();

        FW_EXPECT( count != 0 , "An operator cannot have zero argument" );
        FW_EXPECT( count < 3  , "An operator cannot have more than 2 arguments" );

        switch ( count )
        {
            case 1:
                node->props.add(args[0].m_type, LEFT_VALUE_PROPERTY, Visibility::Default, Way::In);
                break;
            case 2:
                node->props.add( args[0].m_type, LEFT_VALUE_PROPERTY, Visibility::Default, Way::In);
                node->props.add( args[1].m_type, RIGHT_VALUE_PROPERTY, Visibility::Default, Way::In);
                break;
            default: /* no warning */ ;
        }
    }
    else
    {
        for (auto& arg : args)
        {
            node->props.add(arg.m_type, arg.m_name.c_str(), Visibility::Default, Way::In);
        }
    }

    return node_id;
}

ID<Node> NodeFactory::create_func(const fw::iinvokable* _function, bool _is_operator) const
{
    // Create an abstract function node
    const fw::func_type* type = _function->get_type();
    ID<Node> node = _create_abstract_func(type, _is_operator);
    add_invokable_component(node, type, _function, _is_operator);
    m_post_process(node);
    return node;
}

void NodeFactory::add_invokable_component(ID<Node> _node, const fw::func_type* _func_type, const fw::iinvokable *_invokable, bool _is_operator) const
{
    // Create an InvokableComponent with the function.
    ID<InvokableComponent> component = Pool::get_pool()->create<InvokableComponent>(_func_type, _is_operator, _invokable);
    _node->add_component(component);

    // Bind result property
    size_t property_id = _node->props.get_id(VALUE_PROPERTY);
    component->bind_result_property(property_id);

    // Link arguments
    auto args = _func_type->get_args();
    for (size_t arg_idx = 0; arg_idx < args.size(); arg_idx++)
    {
        Property* property = _node->props.get_input_at(arg_idx);
        if ( args[arg_idx].m_by_reference ) property->set_ref();  // to handle by reference function args
        component->bind_arg(arg_idx, property->id);
    }
}

ID<Node> NodeFactory::create_scope() const
{
    ID<Node> node = Pool::get_pool()->create<Node>();

    node->set_name("{} Scope");
    node->slots.set_limit(NEXT_PREVIOUS, Way::In, EDGE_PER_SLOT_MAX_COUNT);
    node->slots.set_limit(NEXT_PREVIOUS, Way::Out, 1);

    ID<Scope> scope_id = Pool::get_pool()->create<Scope>();
    node->add_component(scope_id);

    m_post_process(node);

    return node;
}

ID<ConditionalStructNode> NodeFactory::create_cond_struct() const
{
    ID<ConditionalStructNode> id = Pool::get_pool()->create<ConditionalStructNode>();
    ConditionalStructNode*    node{ id.get() };

    node->set_name("If");
    node->slots.set_limit(NEXT_PREVIOUS, Way::In, EDGE_PER_SLOT_MAX_COUNT);
    node->slots.set_limit(NEXT_PREVIOUS, Way::Out, 2);
    node->add_component(Pool::get_pool()->create<Scope>());

    m_post_process(id);

    return id;
}

ID<ForLoopNode> NodeFactory::create_for_loop() const
{
    ID<ForLoopNode> node = Pool::get_pool()->create<ForLoopNode>();

    node->set_name("For");
    node->slots.set_limit(NEXT_PREVIOUS, Way::In, EDGE_PER_SLOT_MAX_COUNT);
    node->slots.set_limit(NEXT_PREVIOUS, Way::Out, 1);
    node->add_component(Pool::get_pool()->create<Scope>());

    m_post_process(node);

    return node;
}

ID<WhileLoopNode> NodeFactory::create_while_loop() const
{
    ID<WhileLoopNode> node = Pool::get_pool()->create<WhileLoopNode>();

    node->set_name("While");
    node->slots.set_limit(NEXT_PREVIOUS, Way::In, EDGE_PER_SLOT_MAX_COUNT);
    node->slots.set_limit(NEXT_PREVIOUS, Way::Out, 1);
    node->add_component(Pool::get_pool()->create<Scope>());

    m_post_process(node);

    return node;
}

ID<Node> NodeFactory::create_program() const
{
    ID<Node> node = create_scope(); // A program is a main scope.
    node->set_name(ICON_FA_FILE_CODE " Program");

    m_post_process(node);
    return node;
}

ID<Node> NodeFactory::create_node() const
{
    ID<Node> node = Pool::get_pool()->create<Node>();
    m_post_process(node);
    return node;
}

ID<LiteralNode> NodeFactory::create_literal(const fw::type *_type) const
{
    ID<LiteralNode> node = Pool::get_pool()->create<LiteralNode>(_type);
    node->set_name("Literal");
    m_post_process(node);
    return node;
}

void NodeFactory::destroy_node(ID<Node> node) const
{
    Pool* pool = Pool::get_pool();
    pool->destroy( node->get_components() );
    pool->destroy( node );
}

void NodeFactory::set_post_process_fct(NodeFactory::PostProcessFct f)
{
    FW_EXPECT( m_post_process_set == false, "Cannot set post process function more than once." );
    m_post_process_set = true;
    m_post_process     = f;
}

NodeFactory::NodeFactory()
: m_post_process([](ID<Node> _node){})
{}
