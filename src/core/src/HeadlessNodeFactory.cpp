#include <nodable/HeadlessNodeFactory.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/ProgramNode.h>
#include <nodable/ComputeBinaryOperation.h>
#include <nodable/Language.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

InstructionNode* HeadlessNodeFactory::newInstruction() const
{
    InstructionNode* node = new InstructionNode(ICON_FA_CODE " Instr.");
    node->setShortLabel(ICON_FA_CODE " I.");
    return node;
}

InstructionNode* HeadlessNodeFactory::newInstruction_UserCreated()const
{
    InstructionNode* newInstructionNode = newInstruction();

    Token* token = new Token(TokenType_EndOfInstruction);
    m_language->getSerializer()->serialize(token->m_suffix, TokenType_EndOfLine);
    newInstructionNode->setEndOfInstrToken( token );

    return newInstructionNode;
}

VariableNode* HeadlessNodeFactory::newVariable(Type _type, const std::string& _name, ScopedCodeBlockNode* _scope) const
{
    // create
    auto node = new VariableNode(_type);
    node->setName(_name.c_str());

    if( _scope)
    {
        _scope->addVariable(node);
    }
    else
    {
        LOG_WARNING("HeadlessNodeFactory", "Variable %s has been created without defining its scope.\n", _name.c_str())
    }

    return node;
}

Node* HeadlessNodeFactory::newOperator(const Operator* _operator) const
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

Node* HeadlessNodeFactory::newBinOp(const Operator* _operator) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();

    setupNodeLabels(node, _operator);

    const FunctionSignature* signature = _operator->getSignature();
    const auto args = signature->getArgs();
    auto props   = node->getProps();
    Member* left   = props->add("lvalue", Visibility::Default, args[0].type, Way_In);
    Member* right  = props->add("rvalue", Visibility::Default, args[1].type, Way_In);
    Member* result = props->add("result", Visibility::Default, signature->getType(), Way_Out);

    // Create ComputeBinaryOperation component and link values.
    auto binOpComponent = new ComputeBinaryOperation( _operator );
    binOpComponent->setResult(result);
    binOpComponent->setLValue( left );
    binOpComponent->setRValue(right);
    node->addComponent(binOpComponent);

    return node;
}

void HeadlessNodeFactory::setupNodeLabels(Node *_node, const Operator *_operator) {
    _node->setLabel( _operator->getSignature()->getLabel() );
    _node->setShortLabel( _operator->getShortIdentifier().c_str() );
}

Node* HeadlessNodeFactory::newUnaryOp(const Operator* _operator) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();

    setupNodeLabels(node, _operator);

    const FunctionSignature* signature = _operator->getSignature();
    const auto args = signature->getArgs();
    Properties* props = node->getProps();
    Member* left = props->add("lvalue", Visibility::Default, args[0].type, Way_In);
    Member* result = props->add("result", Visibility::Default, signature->getType(), Way_Out);

    // Create ComputeBinaryOperation binOpComponent and link values.
    auto unaryOperationComponent = new ComputeUnaryOperation( _operator );
    unaryOperationComponent->setResult(result);
    unaryOperationComponent->setLValue(left);
    node->addComponent(unaryOperationComponent);

    return node;
}

Node* HeadlessNodeFactory::newFunction(const Invokable* _function) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();
    node->setLabel(_function->getSignature()->getIdentifier() + "()");
    std::string str = _function->getSignature()->getLabel().substr(0, 2) + "..()";
    node->setShortLabel( str.c_str() );
    const Semantic* semantic = m_language->getSemantic();
    auto props = node->getProps();
    Member* result = props->add("result", Visibility::Default, _function->getSignature()->getType(), Way_Out);

    // Create ComputeBase binOpComponent and link values.
    auto functionComponent = new ComputeFunction( _function );
    functionComponent->setResult(result);

    // Arguments
    auto args = _function->getSignature()->getArgs();
    for (size_t argIndex = 0; argIndex < args.size(); argIndex++)
    {
        std::string memberName = args[argIndex].name;
        auto member = props->add(memberName.c_str(), Visibility::Default, args[argIndex].type, Way_In); // create node input
        functionComponent->setArg(argIndex, member); // link input to binOpComponent
    }

    node->addComponent(functionComponent);

    return node;
}


CodeBlockNode* HeadlessNodeFactory::newCodeBlock() const
{
    auto codeBlockNode = new CodeBlockNode();
    std::string label = ICON_FA_CODE " Block";
    codeBlockNode->setLabel(label);
    codeBlockNode->setShortLabel(ICON_FA_CODE " Bl.");

    return codeBlockNode;
}


ScopedCodeBlockNode* HeadlessNodeFactory::newScopedCodeBlock() const
{
    auto scopeNode = new ScopedCodeBlockNode();
    std::string label = ICON_FA_CODE_BRANCH " Scope";
    scopeNode->setLabel(label);
    scopeNode->setShortLabel(ICON_FA_CODE_BRANCH " Sc.");

    return scopeNode;
}

ConditionalStructNode* HeadlessNodeFactory::newConditionalStructure() const
{
    auto scopeNode = new ConditionalStructNode();
    std::string label = ICON_FA_QUESTION " Condition";
    scopeNode->setLabel(label);
    scopeNode->setShortLabel(ICON_FA_QUESTION" Cond.");

    return scopeNode;
}

ProgramNode* HeadlessNodeFactory::newProgram() const
{
    ProgramNode* programNode = new ProgramNode();
    programNode->setLabel(ICON_FA_FILE_CODE " Program");
    programNode->setShortLabel(ICON_FA_FILE_CODE " Prog.");

    return programNode;
}

Node* HeadlessNodeFactory::newNode() const
{
    return new Node();
}

LiteralNode* HeadlessNodeFactory::newLiteral(const Type &type) const
{
    LiteralNode* node = new LiteralNode(type);
    node->setLabel("Literal");
    node->setShortLabel("Lit.");
    return node;
}
