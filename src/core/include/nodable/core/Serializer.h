#pragma once

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include <nodable/core/Token_t.h>
#include <nodable/core/InvokableFunction.h>
#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/Token.h>
#include <nodable/core/reflection/R.h>

namespace Nodable
{
    // forward declarations
    class InvokableComponent;
    class Member;
    class InvokableOperator;
    class Language;
    class IScope;
    class InstructionNode;
    class ForLoopNode;
    class Scope;
    class Operator;

    class Serializer
    {
    public:

        explicit Serializer(const Language* _language): language(_language) {};
        ~Serializer() = default;

        std::string& serialize(std::string &_result, const Operator*) const;
        std::string& serialize(std::string &_result, const InvokableComponent*) const;
        std::string& serialize(std::string &_result, const FunctionSignature*, const std::vector<Member*>&)const;
        std::string& serialize(std::string &_result, const FunctionSignature*)const;
        std::string& serialize(std::string &_result, const Token_t&)const;
        std::string& serialize(std::string &_result, std::shared_ptr<const R::MetaType>)const;
        std::string& serialize(std::string &_result, const Member*, bool followConnections = true)const;
        std::string& serialize(std::string &_result, std::shared_ptr<const Token>) const;
        std::string& serialize(std::string &_result, const InstructionNode*)const;
        std::string& serialize(std::string &_result, const Node*)const;
        std::string& serialize(std::string &_result, const Scope*)const;
        std::string& serialize(std::string& _result, const ForLoopNode* _for_loop)const;
        std::string& serialize(std::string &_result, const ConditionalStructNode*) const;
        std::string& serialize(std::string &_result, const Variant* variant) const;
        std::string& serialize(std::string &_result, const VariableNode *_node) const;
    protected:
        const Language* language;
    };
}
