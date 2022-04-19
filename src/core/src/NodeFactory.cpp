#include <nodable/core/NodeFactory.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/ILanguage.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/Operator.h>
#include <nodable/core/Scope.h>
#include <nodable/core/ConditionalStructNode.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

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
    node->set_name(_name.c_str());

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

Node* NodeFactory::new_abstract_function(const Signature* _signature) const
{
    auto node = _new_abstract_function(_signature);
    add_invokable_component(node, _signature, nullptr);
    m_post_process(node);
    return node;
}

Node* NodeFactory::_new_abstract_function(const Signature* _signature) const
{
    Node* node = new Node();

    if( _signature->is_operator() )
    {
        node->set_label( _signature->get_operator()->identifier.c_str() );
    }
    else
    {
        std::string id = _signature->get_identifier();
        std::string label       = id + "()";
        std::string short_label = id.substr(0, 2) + "..()"; // ------- improve, not great.
        node->set_label(label.c_str(), short_label.c_str());
    }

    // Create a result/value
    Properties* props = node->props();
    props->add(k_value_member_name, Visibility::Default, _signature->get_return_type(), Way_Out);

    NODABLE_ASSERT(!_signature->is_operator() || _signature->get_arg_count() != 0 )

    // Create arguments
    auto args = _signature->get_args();
    for (auto& arg : args)
    {
        props->add(arg.m_name.c_str(), Visibility::Default, arg.m_type, Way_In); // create node input
    }

    return node;
}

Node* NodeFactory::new_function(const IInvokable* _function) const
{
    // Create an abstract function node
    const Signature* signature = _function->get_signature();
    Node* node = _new_abstract_function(signature);
    add_invokable_component(node, signature, _function);
    m_post_process(node);
    return node;
}

void NodeFactory::add_invokable_component(Node *_node, const Signature* _signature, const IInvokable *_invokable) const
{
    Properties* props = _node->props();

    // Create an InvokableComponent with the function.
    auto component = _invokable ? new InvokableComponent(_invokable) : new InvokableComponent(_signature);
    _node->add_component(component);

    // Link result member
    component->set_result(props->get(k_value_member_name));

    // Link arguments
    auto args = _signature->get_args();
    for (size_t argIndex = 0; argIndex < args.size(); argIndex++)
    {
        Member* member = props->get(args[argIndex].m_name.c_str());
        component->set_arg(argIndex, member);
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
