#include <nodable/HeadlessNodeFactory.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/CodeBlockNode.h>
#include <nodable/Language.h>
#include <nodable/InvokableComponent.h>
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
    newInstructionNode->end_of_instr_token(token);

    return newInstructionNode;
}

VariableNode* HeadlessNodeFactory::newVariable(Type _type, const std::string& _name, ScopedCodeBlockNode* _scope) const
{
    // create
    auto node = new VariableNode(_type);
    node->setName(_name.c_str());

    if( _scope)
    {
        _scope->add_variable(node);
    }
    else
    {
        LOG_WARNING("HeadlessNodeFactory", "Variable %s has been created without defining its scope.\n", _name.c_str())
    }

    return node;
}

Node* HeadlessNodeFactory::newOperator(const InvokableOperator* _operator) const
{
    switch (_operator->get_operator_type() )
    {
        case InvokableOperator::Type::Binary:
            return newBinOp(_operator);
        case InvokableOperator::Type::Unary:
            return newUnaryOp(_operator);
        default:
            return nullptr;
    }
}

Node* HeadlessNodeFactory::newBinOp(const InvokableOperator* _operator) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();

    setupNodeLabels(node, _operator);

    const FunctionSignature* signature = _operator->get_signature();
    const auto args = signature->get_args();
    auto props   = node->getProps();
    Member* left   = props->add("lvalue", Visibility::Default, args[0].m_type, Way_In);
    Member* right  = props->add("rvalue", Visibility::Default, args[1].m_type, Way_In);
    Member* result = props->add("result", Visibility::Default, signature->get_return_type(), Way_Out);

    // Create ComputeBinaryOperation component and link values.
    auto binOpComponent = new InvokableComponent( _operator );
    binOpComponent->set_result(result);
    binOpComponent->set_l_handed_val(left);
    binOpComponent->set_r_handed_val(right);
    node->addComponent(binOpComponent);

    return node;
}

void HeadlessNodeFactory::setupNodeLabels(Node *_node, const InvokableOperator *_operator) {
    _node->setLabel(_operator->get_signature()->get_label() );
    _node->setShortLabel(_operator->get_short_identifier().c_str() );
}

Node* HeadlessNodeFactory::newUnaryOp(const InvokableOperator* _operator) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();

    setupNodeLabels(node, _operator);

    const FunctionSignature* signature = _operator->get_signature();
    const auto args = signature->get_args();
    Properties* props = node->getProps();
    Member* left = props->add("lvalue", Visibility::Default, args[0].m_type, Way_In);
    Member* result = props->add("result", Visibility::Default, signature->get_return_type(), Way_Out);

    // Create ComputeBinaryOperation binOpComponent and link values.
    auto unaryOperationComponent = new InvokableComponent( _operator );
    unaryOperationComponent->set_result(result);
    unaryOperationComponent->set_l_handed_val(left);
    node->addComponent(unaryOperationComponent);

    return node;
}

Node* HeadlessNodeFactory::newFunction(const Invokable* _function) const
{
    // Create a node with 2 inputs and 1 output
    auto node = new Node();
    node->setLabel(_function->get_signature()->get_identifier() + "()");
    std::string str = _function->get_signature()->get_label().substr(0, 2) + "..()";
    node->setShortLabel( str.c_str() );
    const Semantic* semantic = m_language->getSemantic();
    auto props = node->getProps();
    Member* result = props->add("result", Visibility::Default, _function->get_signature()->get_return_type(), Way_Out);

    // Create ComputeBase binOpComponent and link values.
    auto functionComponent = new InvokableComponent( _function );
    functionComponent->set_result(result);

    // Arguments
    auto args = _function->get_signature()->get_args();
    for (size_t argIndex = 0; argIndex < args.size(); argIndex++)
    {
        std::string memberName = args[argIndex].m_name;
        auto member = props->add(memberName.c_str(), Visibility::Default, args[argIndex].m_type, Way_In); // create node input
        functionComponent->set_arg(argIndex, member); // link input to binOpComponent
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

ForLoopNode* HeadlessNodeFactory::new_for_loop_node() const
{
    auto for_loop = new ForLoopNode();
    std::string label = ICON_FA_RECYCLE " For loop";
    for_loop->setLabel(label);
    for_loop->setShortLabel(ICON_FA_RECYCLE" For");

    return for_loop;
}

ScopedCodeBlockNode* HeadlessNodeFactory::newProgram() const
{
    ScopedCodeBlockNode* scope = new ScopedCodeBlockNode();
    scope->setLabel(ICON_FA_FILE_CODE " Program");
    scope->setShortLabel(ICON_FA_FILE_CODE " Prog.");

    return scope;
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
