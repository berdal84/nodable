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

using namespace ndbl;

InstructionNode* NodeFactory::new_instr() const
{
    InstructionNode* instr_node = new InstructionNode();
    instr_node->set_name("Instr.");
    m_post_process(instr_node);
    return instr_node;
}

VariableNode* NodeFactory::new_variable(const fw::type *_type, const std::string& _name, IScope *_scope) const
{
    // create
    auto* node = new VariableNode(_type, _name.c_str());

    if( _scope)
    {
        _scope->add_variable(node);
    }
    else
    {
        LOG_WARNING("HeadlessNodeFactory", "Variable %s has been created without defining its scope.\n", _name.c_str())
    }

    m_post_process(node);

    return node;
}

Node* NodeFactory::new_abstract_function(const fw::func_type* _signature, bool _is_operator) const
{
    auto node = _new_abstract_function(_signature, _is_operator);
    add_invokable_component(node, _signature, nullptr, _is_operator );
    m_post_process(node);
    return node;
}

Node* NodeFactory::_new_abstract_function(const fw::func_type* _func_type, bool _is_operator) const
{
    Node* node = new Node();

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
    PropertyGrp * props = node->props();
    props->add(_func_type->get_return_type(), k_value_property_name, Visibility::Default, Way_Out);

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
                props->add(args[0].m_type, k_lh_value_property_name, Visibility::Default, Way_In);
                break;
            case 2:
                props->add( args[0].m_type, k_lh_value_property_name, Visibility::Default, Way_In);
                props->add( args[1].m_type, k_rh_value_property_name, Visibility::Default, Way_In);
                break;
            default: /* no warning */ ;
        }
    }
    else
    {
        for (auto& arg : args)
        {
            props->add(arg.m_type, arg.m_name.c_str(), Visibility::Default, Way_In);
        }
    }

    return node;
}

Node* NodeFactory::new_function(const fw::iinvokable* _function, bool _is_operator) const
{
    // Create an abstract function node
    const fw::func_type* type = _function->get_type();
    Node* node = _new_abstract_function(type, _is_operator);
    add_invokable_component(node, type, _function, _is_operator);
    m_post_process(node);
    return node;
}

void NodeFactory::add_invokable_component(Node *_node, const fw::func_type* _func_type, const fw::iinvokable *_invokable, bool _is_operator) const
{
    PropertyGrp * props = _node->props();

    // Create an InvokableComponent with the function.
    auto component = _node->components().add<InvokableComponent>(_func_type, _is_operator, _invokable );

    // Link result property
    component->set_result(props->get(k_value_property_name));

    // Link arguments
    auto args = _func_type->get_args();
    for (size_t arg_idx = 0; arg_idx < args.size(); arg_idx++)
    {
        Property * property = props->get_input_at((u8_t)arg_idx);
        property->set_reference(args[arg_idx].m_by_reference);  // to handle by reference function args
        component->set_arg(arg_idx, property);                  // link
    }
}

Node* NodeFactory::new_scope() const
{
    auto scope_node = new Node();
    scope_node->set_name("{} Scope");

    scope_node->predecessors().set_limit(std::numeric_limits<int>::max());
    scope_node->successors().set_limit(1);

    scope_node->components().add<Scope>();

    m_post_process(scope_node);

    return scope_node;
}

ConditionalStructNode* NodeFactory::new_cond_struct() const
{
    auto cond_struct_node = new ConditionalStructNode();
    cond_struct_node->set_name("If");

    cond_struct_node->predecessors().set_limit(std::numeric_limits<int>::max());
    cond_struct_node->successors().set_limit(2); // true/false branches

    cond_struct_node->add_component<Scope>();

    m_post_process(cond_struct_node);

    return cond_struct_node;
}

ForLoopNode* NodeFactory::new_for_loop_node() const
{
    auto for_loop = new ForLoopNode();
    for_loop->set_name("For loop_count");

    for_loop->predecessors().set_limit(std::numeric_limits<int>::max());
    for_loop->successors().set_limit(1);

    for_loop->add_component<Scope>();

    m_post_process(for_loop);

    return for_loop;
}

Node* NodeFactory::new_program() const
{
    Node* prog = new_scope();
    prog->set_name(ICON_FA_FILE_CODE " Program");

    m_post_process(prog);
    return prog;
}

Node* NodeFactory::new_node() const
{
    auto node = new Node();
    m_post_process(node);
    return node;
}

LiteralNode* NodeFactory::new_literal(const fw::type *_type) const
{
    LiteralNode* node = new LiteralNode(_type);
    node->set_name("Literal");
    m_post_process(node);
    return node;
}
