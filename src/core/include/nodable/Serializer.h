#pragma once

#include <string>
#include <vector>
#include <nodable/TokenType.h>
#include <nodable/InvokableFunction.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/Token.h>
#include <nodable/R.h>

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

    class Serializer
    {
    public:

        explicit Serializer(const Language* _language): language(_language) {};
        ~Serializer() = default;

        std::string& serialize(std::string &_result, const InvokableComponent*) const;
        std::string& serialize(std::string &_result, const FunctionSignature*, const std::vector<Member*>&)const;
        std::string& serialize(std::string &_result, const FunctionSignature*)const;
        std::string& serialize(std::string &_result, const TokenType&)const;
        std::string& serialize(std::string &_result, const R::Type*)const;
        std::string& serialize(std::string &_result, const Member*, bool followConnections = true)const;
        std::string& serialize(std::string &_result, const Token*) const;
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
