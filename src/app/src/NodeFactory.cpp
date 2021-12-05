#include <nodable/NodeFactory.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/CodeBlockNode.h>
#include <nodable/InvokableComponent.h>
#include <nodable/Language.h>
#include <nodable/NodeView.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

InstructionNode* NodeFactory::newInstruction() const
{
    InstructionNode* node = m_headless_node_factory.newInstruction_UserCreated();
    AddView(node);

    return node;
}

InstructionNode* NodeFactory::newInstruction_UserCreated()const
{
    InstructionNode* node = m_headless_node_factory.newInstruction_UserCreated();
    AddView(node);

    return node;
}

VariableNode* NodeFactory::newVariable(Type _type, const std::string& _name, ScopedCodeBlockNode* _scope) const
{
    VariableNode* node = m_headless_node_factory.newVariable(_type, _name, _scope);
    AddView(node);

    return node;
}

Node* NodeFactory::newOperator(const InvokableOperator* _operator) const
{
    Node* node = m_headless_node_factory.newOperator(_operator);
    AddView(node);

    return node;
}

Node* NodeFactory::newBinOp(const InvokableOperator* _operator) const
{
    Node* node = m_headless_node_factory.newBinOp(_operator);
    AddView(node);

    return node;
}

Node* NodeFactory::newUnaryOp(const InvokableOperator* _operator) const
{
    Node* node = m_headless_node_factory.newUnaryOp(_operator);
    AddView(node);

    return node;
}

Node* NodeFactory::newFunction(const Invokable* _function) const
{
    Node* node = m_headless_node_factory.newFunction(_function);
    AddView(node);

    return node;
}


CodeBlockNode* NodeFactory::newCodeBlock() const
{
    CodeBlockNode* node = m_headless_node_factory.newCodeBlock();
    AddView(node);

    return node;
}


ScopedCodeBlockNode* NodeFactory::newScopedCodeBlock() const
{
    ScopedCodeBlockNode* node = m_headless_node_factory.newScopedCodeBlock();
    AddView(node);

    return node;
}

ConditionalStructNode* NodeFactory::newConditionalStructure() const
{
    ConditionalStructNode* node = m_headless_node_factory.newConditionalStructure();
    AddView(node);

    return node;
}

ScopedCodeBlockNode* NodeFactory::newProgram() const
{
    ScopedCodeBlockNode* node = m_headless_node_factory.newProgram();
    AddView(node);

    return node;
}

Node* NodeFactory::newNode() const
{
    return new Node();
}

LiteralNode* NodeFactory::newLiteral(const Type &type) const
{
    LiteralNode* node = m_headless_node_factory.newLiteral(type);
    AddView(node);

    return node;
}

void NodeFactory::AddView(Node* _node)
{
    _node->addComponent(new NodeView());
}
