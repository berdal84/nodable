#include <nodable/NodeFactory.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/ProgramNode.h>
#include <nodable/ComputeBinaryOperation.h>
#include <nodable/Language.h>
#include <nodable/NodeView.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

InstructionNode* NodeFactory::newInstruction() const
{
    auto instructionNode = new InstructionNode(ICON_FA_CODE " Instr.");
    instructionNode->addComponent(new NodeView());
    instructionNode->setShortLabel(ICON_FA_CODE);

    return instructionNode;
}

InstructionNode* NodeFactory::newInstruction_UserCreated()const
{
    auto newInstructionNode = newInstruction();

    Token* token = new Token(TokenType_EndOfInstruction);
    m_language->getSerializer()->serialize(token->m_suffix, TokenType_EndOfLine);
    newInstructionNode->setEndOfInstrToken( token );

    return newInstructionNode;
}

VariableNode* NodeFactory::newVariable(Type _type, const std::string& _name, ScopedCodeBlockNode* _scope) const
{
    // create
    auto node = new VariableNode(_type);

    node->addComponent( new NodeView() );
    node->setName(_name.c_str());

    if( _scope)
    {
        _scope->addVariable(node);
    }
    else
    {
        LOG_WARNING("NodeFactory", "You create a variable without defining its scope.")
    }

    return node;
}

Node* NodeFactory::newOperator(const Operator* _operator) const
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

Node* NodeFactory::newBinOp(const Operator* _operator) const
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

    // Create a NodeView component
    node->addComponent(new NodeView());

    return node;
}

Node* NodeFactory::newUnaryOp(const Operator* _operator) const
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

    // Create a NodeView Component
    node->addComponent(new NodeView());

    return node;
}

Node* NodeFactory::newFunction(const Function* _function) const
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
    node->addComponent(new NodeView());

    return node;
}


CodeBlockNode* NodeFactory::newCodeBlock() const
{
    auto codeBlockNode = new CodeBlockNode();
    std::string label = ICON_FA_CODE " Block";
    codeBlockNode->setLabel(label);
    codeBlockNode->setShortLabel(ICON_FA_CODE "Bl");

    codeBlockNode->addComponent(new NodeView() );

    return codeBlockNode;
}


ScopedCodeBlockNode* NodeFactory::newScopedCodeBlock() const
{
    auto scopeNode = new ScopedCodeBlockNode();
    std::string label = ICON_FA_CODE_BRANCH " Scope";
    scopeNode->setLabel(label);
    scopeNode->setShortLabel(ICON_FA_CODE_BRANCH " Scop.");

    scopeNode->addComponent(new NodeView());

    return scopeNode;
}

ConditionalStructNode* NodeFactory::newConditionalStructure() const
{
    auto scopeNode = new ConditionalStructNode();
    std::string label = ICON_FA_QUESTION " Condition";
    scopeNode->setLabel(label);
    scopeNode->setShortLabel(ICON_FA_QUESTION" Cond.");

    scopeNode->addComponent(new NodeView());

    return scopeNode;
}

ProgramNode* NodeFactory::newProgram() const
{
    ProgramNode* programNode = new ProgramNode();
    programNode->setLabel(ICON_FA_FILE_CODE " Program");
    programNode->setShortLabel(ICON_FA_FILE_CODE " Prog.");

    programNode->addComponent( new NodeView() );

    return programNode;
}

Node* NodeFactory::newNode() const
{
    return new Node();
}

LiteralNode* NodeFactory::newLiteral(const Type &type) const
{
    LiteralNode* node = new LiteralNode(type);
    node->setLabel("Literal");
    node->setShortLabel("");

    node->addComponent(new NodeView());

    return node;
}
