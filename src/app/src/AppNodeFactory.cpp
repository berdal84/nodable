#include <nodable/AppNodeFactory.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/InvokableComponent.h>
#include <nodable/Language.h>
#include <nodable/NodeView.h>
#include <nodable/ForLoopNode.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;
using namespace Nodable::R;

InstructionNode* AppNodeFactory::new_instr() const
{
    InstructionNode* node = m_headless_node_factory.new_instr();
    post_instantiation(node);

    return node;
}

VariableNode* AppNodeFactory::newVariable(Type _type, const std::string& _name, IScope *_scope) const
{
    VariableNode* node = m_headless_node_factory.newVariable(_type, _name, _scope);
    post_instantiation(node);

    return node;
}

Node* AppNodeFactory::newOperator(const InvokableOperator* _operator) const
{
    Node* node = m_headless_node_factory.newOperator(_operator);
    post_instantiation(node);

    return node;
}

Node* AppNodeFactory::newBinOp(const InvokableOperator* _operator) const
{
    Node* node = m_headless_node_factory.newBinOp(_operator);
    post_instantiation(node);

    return node;
}

Node* AppNodeFactory::newUnaryOp(const InvokableOperator* _operator) const
{
    Node* node = m_headless_node_factory.newUnaryOp(_operator);
    post_instantiation(node);

    return node;
}

Node* AppNodeFactory::newFunction(const IInvokable* _function) const
{
    Node* node = m_headless_node_factory.newFunction(_function);
    post_instantiation(node);

    return node;
}

Node* AppNodeFactory::newScope() const
{
    Node* node = m_headless_node_factory.newScope();
    post_instantiation(node);

    return node;
}

ConditionalStructNode* AppNodeFactory::newConditionalStructure() const
{
    ConditionalStructNode* node = m_headless_node_factory.newConditionalStructure();
    post_instantiation(node);

    return node;
}

ForLoopNode* AppNodeFactory::new_for_loop_node() const
{
    ForLoopNode* node = m_headless_node_factory.new_for_loop_node();
    post_instantiation(node);

    return node;
}

Node* AppNodeFactory::newProgram() const
{
    Node* node = m_headless_node_factory.newProgram();
    post_instantiation(node);

    return node;
}

Node* AppNodeFactory::newNode() const
{
    return new Node();
}

LiteralNode* AppNodeFactory::newLiteral(const Type &type) const
{
    LiteralNode* node = m_headless_node_factory.newLiteral(type);
    post_instantiation(node);

    return node;
}

void AppNodeFactory::post_instantiation(Node* _node) const
{
    _node->add_component(new NodeView(m_context));
}
