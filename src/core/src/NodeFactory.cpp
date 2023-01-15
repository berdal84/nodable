#include <nodable/core/NodeFactory.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/Scope.h>
#include <nodable/core/Operator.h>
#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/core/reflection/func_type.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace ndbl;

InstructionNode* NodeFactory::new_instr() const
{
    InstructionNode* instr_node = new InstructionNode();
    instr_node->set_label("Instr.", "");
    m_post_process(instr_node);
    return instr_node;
}

VariableNode* NodeFactory::new_variable(type _type, const std::string& _name, IScope *_scope) const
{
    // create
    auto* node = new VariableNode(_type);
    node->set_identifier(_name.c_str());

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

Node* NodeFactory::new_abstract_function(const func_type* _signature, bool _is_operator) const
{
    auto node = _new_abstract_function(_signature, _is_operator);
    add_invokable_component(node, _signature, nullptr, _is_operator );
    m_post_process(node);
    return node;
}

Node* NodeFactory::_new_abstract_function(const func_type* _func_type, bool _is_operator) const
{
    Node* node = new Node();

    if( _is_operator )
    {
        node->set_label( _func_type->get_identifier().c_str() );
    }
    else
    {
        std::string id = _func_type->get_identifier();
        std::string label       = id + "()";
        std::string short_label = id.substr(0, 2) + "..()"; // ------- improve, not great.
        node->set_label(label.c_str(), short_label.c_str());
    }

    // Create a result/value
    PropertyGrp * props = node->props();
    props->add(k_value_property_name, Visibility::Default, _func_type->get_return_type(), Way_Out);

    // Create arguments
    auto args = _func_type->get_args();

    if( _is_operator ) // rename arguments
    {
        size_t count = _func_type->get_arg_count();

        NDBL_EXPECT( count != 0 , "An operator cannot have zero argument" );
        NDBL_EXPECT( count < 3  , "An operator cannot have more than 2 arguments" );

        switch ( count )
        {
            case 1:
                props->add( k_lh_value_property_name, Visibility::Default, args[0].m_type, Way_In);
                break;
            case 2:
                props->add( k_lh_value_property_name, Visibility::Default, args[0].m_type, Way_In);
                props->add( k_rh_value_property_name, Visibility::Default, args[1].m_type, Way_In);
                break;
            default: /* no warning */ ;
        }
    }
    else
    {
        for (auto& arg : args)
        {
            props->add(arg.m_name.c_str(), Visibility::Default, arg.m_type, Way_In);
        }
    }

    return node;
}

Node* NodeFactory::new_function(const iinvokable* _function, bool _is_operator) const
{
    // Create an abstract function node
    const func_type* type = &_function->get_type();
    Node* node = _new_abstract_function(type, _is_operator);
    add_invokable_component(node, type, _function, _is_operator);
    m_post_process(node);
    return node;
}

void NodeFactory::add_invokable_component(Node *_node, const func_type* _func_type, const iinvokable *_invokable, bool _is_operator) const
{
    PropertyGrp * props = _node->props();

    // Create an InvokableComponent with the function.
    auto component = new InvokableComponent(_func_type, _is_operator, _invokable );
    _node->add_component(component);

    // Link result property
    component->set_result(props->get(k_value_property_name));

    // Link arguments
    auto args = _func_type->get_args();
    for (size_t arg_idx = 0; arg_idx < args.size(); arg_idx++)
    {
        Property * property = props->get_input_at(arg_idx);
        component->set_arg(arg_idx, property);
    }
}

Node* NodeFactory::new_scope() const
{
    auto scope_node = new Node();
    scope_node->set_label("{} Scope", "{}");

    scope_node->predecessors().set_limit(std::numeric_limits<int>::max());
    scope_node->successors().set_limit(1);

    auto* scope = new Scope();
    scope_node->add_component(scope);

    m_post_process(scope_node);

    return scope_node;
}

ConditionalStructNode* NodeFactory::new_cond_struct() const
{
    auto cond_struct_node = new ConditionalStructNode();
    cond_struct_node->set_label("If");

    cond_struct_node->predecessors().set_limit(std::numeric_limits<int>::max());
    cond_struct_node->successors().set_limit(2); // true/false branches

    auto* scope = new Scope();
    cond_struct_node->add_component(scope);

    m_post_process(cond_struct_node);

    return cond_struct_node;
}

ForLoopNode* NodeFactory::new_for_loop_node() const
{
    auto for_loop = new ForLoopNode();
    for_loop->set_label("For loop", "For");

    for_loop->predecessors().set_limit(std::numeric_limits<int>::max());
    for_loop->successors().set_limit(1);

    auto* scope = new Scope();
    for_loop->add_component(scope);

    m_post_process(for_loop);

    return for_loop;
}

Node* NodeFactory::new_program() const
{
    Node* prog = new_scope();
    prog->set_label(ICON_FA_FILE_CODE " Program", ICON_FA_FILE_CODE);

    m_post_process(prog);
    return prog;
}

Node* NodeFactory::new_node() const
{
    auto node = new Node();
    m_post_process(node);
    return node;
}

LiteralNode* NodeFactory::new_literal(type _type) const
{
    LiteralNode* node = new LiteralNode(_type);
    node->set_label("Literal", "");
    m_post_process(node);
    return node;
}
