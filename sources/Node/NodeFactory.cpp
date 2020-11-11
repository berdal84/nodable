#include "NodeFactory.h"
#include <Component/NodeView.h>
#include <Component/ComputeBinaryOperation.h>
#include <Component/ComputeUnaryOperation.h>

using namespace Nodable;

std::shared_ptr<Variable> NodeFactory::newVariable(std::string _name)
{
    auto node = std::make_shared<Variable>();
    node->setName(_name.c_str());

    if ( mode != Mode::HEADLESS )
    {
        node->newComponent<NodeView>();
    }

    return node;
}

std::shared_ptr<Variable> NodeFactory::newNumber(double _value)
{
    auto node = std::make_shared<Variable>();
    node->set(_value);

    if ( mode != Mode::HEADLESS )
    {
        node->newComponent<NodeView>();
    }

    return node;
}

std::shared_ptr<Variable> NodeFactory::newNumber(const char* _value)
{
    auto node = std::make_shared<Variable>();
    node->set(std::stod(_value));

    if ( mode != Mode::HEADLESS )
    {
        node->newComponent<NodeView>();
    }

    return node;
}

std::shared_ptr<Variable> NodeFactory::newString(const char* _value)
{
    auto node = std::make_shared<Variable>();
    node->newComponent<NodeView>();
    node->set(_value);

    if ( mode != Mode::HEADLESS )
    {
        node->newComponent<NodeView>();
    }

    return node;
}


std::shared_ptr<Node> NodeFactory::newBinOp(std::shared_ptr<const Operator> _operator)
{
    // Create a node with 2 inputs and 1 output
    auto node = std::make_shared<Node>();
    auto signature = _operator->signature;
    node->setLabel(signature->getLabel());
    const auto args = signature->getArgs();
    auto left   = node->add("lvalue", Visibility::Default, language->tokenTypeToType(args[0].type), Way::In);
    auto right  = node->add("rvalue", Visibility::Default, language->tokenTypeToType(args[1].type), Way::In);
    auto result = node->add("result", Visibility::Default, language->tokenTypeToType(signature->getType()), Way::Out);

    // Create ComputeBinaryOperation component and link values.
    auto binOpComponent = node->newComponent<ComputeBinaryOperation>().lock();
    binOpComponent->setLanguage(language);
    binOpComponent->setFunction(_operator);
    binOpComponent->setResult(result);
    binOpComponent->setLValue(left);
    binOpComponent->setRValue(right);

    // Create a NodeView component
    if ( mode != Mode::HEADLESS )
    {
        node->newComponent<NodeView>();
    }

    return node;
}

std::shared_ptr<Node> NodeFactory::newUnaryOp(std::shared_ptr<const Operator> _operator)
{
    // Create a node with 2 inputs and 1 output
    auto node = std::make_shared<Node>();
    auto signature = _operator->signature;
    node->setLabel(signature->getLabel());
    const auto args = signature->getArgs();
    auto left = node->add("lvalue", Visibility::Default, language->tokenTypeToType(args[0].type), Way::In);
    auto result = node->add("result", Visibility::Default, language->tokenTypeToType(signature->getType()), Way::Out);

    // Create ComputeBinaryOperation binOpComponent and link values.
    auto unaryOperationComponent = node->newComponent<ComputeUnaryOperation>().lock();
    unaryOperationComponent->setLanguage(language);
    unaryOperationComponent->setFunction(_operator);
    unaryOperationComponent->setResult(result);
    unaryOperationComponent->setLValue(left);

    // Create a NodeView component
    if ( mode != Mode::HEADLESS )
    {
        node->newComponent<NodeView>();
    }

    return node;
}

std::shared_ptr<Node> NodeFactory::newFunction(std::shared_ptr<const Function> _function)
{
    // Create a node with 2 inputs and 1 output
    auto node = std::make_shared<Node>();
    node->setLabel(_function->signature->getIdentifier());
    node->add("result", Visibility::Default, language->tokenTypeToType(_function->signature->getType()), Way::Out);

    // Create ComputeBase binOpComponent and link values.
    auto computeFunctionComponent = node->newComponent<ComputeFunction>().lock();
    computeFunctionComponent->setLanguage(this->language);
    computeFunctionComponent->setFunction(_function);
    computeFunctionComponent->setResult(node->get("result"));

    auto args = _function->signature->getArgs();
    for (size_t argIndex = 0; argIndex < args.size(); argIndex++) {
        std::string memberName = args[argIndex].name;
        auto member = node->add(memberName.c_str(), Visibility::Default, language->tokenTypeToType(args[argIndex].type), Way::In); // create node input
        computeFunctionComponent->setArg(argIndex, member); // link input to binOpComponent
    }

    // Create a NodeView component
    if ( mode != Mode::HEADLESS )
    {
        node->newComponent<NodeView>();
    }

    return node;
}


std::shared_ptr<Wire> NodeFactory::newWire()
{
    auto wire = std::make_shared<Wire>();
    wire->newView();
    return wire;
}

