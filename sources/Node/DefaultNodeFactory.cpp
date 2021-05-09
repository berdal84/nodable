#include "DefaultNodeFactory.h"
#include "Node/InstructionNode.h"
#include "Node/VariableNode.h"
#include "Node/LiteralNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/ProgramNode.h"
#include "Component/ComputeBinaryOperation.h"
#include "Component/NodeView.h"
#include "Language/Common/Language.h"

#ifndef NODABLE_HEADLESS
#   include "Component/View.h"
#endif

#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

InstructionNode* DefaultNodeFactory::newInstruction() const
{
    auto instructionNode = new InstructionNode(ICON_FA_CODE " Instr.");

#ifndef NODABLE_HEADLESS
    instructionNode->addComponent(new NodeView());
    instructionNode->setShortLabel(ICON_FA_CODE);
#endif

    return instructionNode;
}

InstructionNode* DefaultNodeFactory::newInstruction_UserCreated()const
{
    auto newInstructionNode = newInstruction();

    Token* token = new Token(TokenType_EndOfInstruction);
    m_language->getSerializer()->serialize(token->m_suffix, TokenType_EndOfLine);
    newInstructionNode->setEndOfInstrToken( token );

    return newInstructionNode;
}

VariableNode* DefaultNodeFactory::newVariable(Type _type, const std::string& _name, ScopedCodeBlockNode* _scope) const
{
    // create
    auto node = new VariableNode(_type);

#ifndef NODABLE_HEADLESS
    node->addComponent( new NodeView() );
#endif

    node->setName(_name.c_str());

    if( _scope)
    {
        _scope->addVariable(node);
    }
    else
    {
        LOG_WARNING("NodeFactory", "You create a variable without defining its scope.");
    }

    return node;
}

Node* DefaultNodeFactory::newOperator(const Operator* _operator) const
{
    switch ( _operator->getType() )
    {
        case Operator::Type::Binary:
            return newBinOp(_operator);
        case Operator::Type::Unary:
            return newUnaryOp(_operator);
        default:
            return nullptr;
    }
}

Node* DefaultNodeFactory::newBinOp(const Operator* _operator) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();
    auto signature = _operator->signature;
    node->setLabel(signature.getLabel());
    node->setShortLabel(signature.getLabel().substr(0, 4).c_str());

    const auto args = signature.getArgs();
    const Semantic* semantic = m_language->getSemantic();
    auto props = node->getProps();
    auto left   = props->add("lvalue", Visibility::Default, semantic->tokenTypeToType(args[0].type), Way_In);
    auto right  = props->add("rvalue", Visibility::Default, semantic->tokenTypeToType(args[1].type), Way_In);
    auto result = props->add("result", Visibility::Default, semantic->tokenTypeToType(signature.getType()), Way_Out);

    // Create ComputeBinaryOperation component and link values.
    auto binOpComponent = new ComputeBinaryOperation( _operator );
    binOpComponent->setResult(result);
    binOpComponent->setLValue( left );
    binOpComponent->setRValue(right);
    node->addComponent(binOpComponent);

#ifndef NODABLE_HEADLESS
    // Create a NodeView component
    node->addComponent(new NodeView());
#endif

    return node;
}

Node* DefaultNodeFactory::newUnaryOp(const Operator* _operator) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();
    auto signature = _operator->signature;
    node->setLabel(signature.getLabel());
    node->setShortLabel(signature.getLabel().substr(0, 4).c_str());
    const auto args = signature.getArgs();
    const Semantic* semantic = m_language->getSemantic();
    auto props = node->getProps();
    auto left = props->add("lvalue", Visibility::Default, semantic->tokenTypeToType(args[0].type), Way_In);
    auto result = props->add("result", Visibility::Default, semantic->tokenTypeToType(signature.getType()), Way_Out);

    // Create ComputeBinaryOperation binOpComponent and link values.
    auto unaryOperationComponent = new ComputeUnaryOperation( _operator );
    unaryOperationComponent->setResult(result);
    unaryOperationComponent->setLValue(left);
    node->addComponent(unaryOperationComponent);

#ifndef NODABLE_HEADLESS
    // Create a NodeView Component
    node->addComponent(new NodeView());
#endif

    return node;
}

Node* DefaultNodeFactory::newFunction(const Function* _function) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();
    node->setLabel(_function->signature.getIdentifier() + "()");
    node->setShortLabel("f(x)");
    const Semantic* semantic = m_language->getSemantic();
    auto props = node->getProps();
    props->add("result", Visibility::Default, semantic->tokenTypeToType(_function->signature.getType()), Way_Out);

    // Create ComputeBase binOpComponent and link values.
    auto functionComponent = new ComputeFunction( _function );
    functionComponent->setResult(props->get("result"));

    // Arguments
    auto args = _function->signature.getArgs();
    for (size_t argIndex = 0; argIndex < args.size(); argIndex++) {
        std::string memberName = args[argIndex].name;
        auto member = props->add(memberName.c_str(), Visibility::Default, semantic->tokenTypeToType(args[argIndex].type), Way_In); // create node input
        functionComponent->setArg(argIndex, member); // link input to binOpComponent
    }

    node->addComponent(functionComponent);

#ifndef NODABLE_HEADLESS
    node->addComponent(new NodeView());
#endif

    return node;
}


CodeBlockNode* DefaultNodeFactory::newCodeBlock() const
{
    auto codeBlockNode = new CodeBlockNode();
    std::string label = ICON_FA_CODE " Block";
    codeBlockNode->setLabel(label);
    codeBlockNode->setShortLabel(ICON_FA_CODE "Bl");

#ifndef NODABLE_HEADLESS
    codeBlockNode->addComponent(new NodeView() );
#endif

    return codeBlockNode;
}


ScopedCodeBlockNode* DefaultNodeFactory::newScopedCodeBlock() const
{
    auto scopeNode = new ScopedCodeBlockNode();
    std::string label = ICON_FA_CODE_BRANCH " Scope";
    scopeNode->setLabel(label);
    scopeNode->setShortLabel(ICON_FA_CODE_BRANCH " Scop.");

#ifndef NODABLE_HEADLESS
    scopeNode->addComponent(new NodeView());
#endif

    return scopeNode;
}

ConditionalStructNode* DefaultNodeFactory::newConditionalStructure() const
{
    auto scopeNode = new ConditionalStructNode();
    std::string label = ICON_FA_QUESTION " Condition";
    scopeNode->setLabel(label);
    scopeNode->setShortLabel(ICON_FA_QUESTION" Cond.");

#ifndef NODABLE_HEADLESS
    scopeNode->addComponent(new NodeView());
#endif

    return scopeNode;
}

ProgramNode* DefaultNodeFactory::newProgram() const
{
    ProgramNode* programNode = new ProgramNode();
    programNode->setLabel(ICON_FA_FILE_CODE " Program");
    programNode->setShortLabel(ICON_FA_FILE_CODE " Prog.");

#ifndef NODABLE_HEADLESS
    programNode->addComponent( new NodeView() );
#endif

    return programNode;
}

Node* DefaultNodeFactory::newNode() const
{
    return new Node();
}

LiteralNode* DefaultNodeFactory::newLiteral(const Type &type) const
{
    LiteralNode* node = new LiteralNode(type);
    node->setLabel("Literal");
    node->setShortLabel("");

#ifndef NODABLE_HEADLESS
    node->addComponent(new NodeView());
#endif

    return node;
}
