#pragma once

#include <string>
#include <vector>
#include <nodable/TokenType.h>
#include <nodable/InvokableFunction.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/Token.h>

namespace Nodable
{
    // forward declarations
    class InvokableComponent;
    class Member;
    class InvokableOperator;
    class Language;
    class AbstractScope;
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
        std::string& serialize(std::string &_result, const Type&)const;
        virtual std::string& serialize(std::string &_result, const Member*, bool followConnections = true)const;
        virtual std::string& serialize(std::string &_result, const Token*) const;
        virtual std::string& serialize(std::string &_result, const InstructionNode*)const;
        virtual std::string& serialize(std::string &_result, const Node*)const;
        virtual std::string& serialize(std::string &_result, const Scope*)const;
        virtual std::string& serialize(std::string& _result, const ForLoopNode* _for_loop)const;
        virtual std::string& serialize(std::string &_result, const ConditionalStructNode*) const;
        virtual std::string& serialize(std::string &_result, const Variant* variant) const;
    protected:
        const Language* language;

        std::string& serialize(std::string &_result, const VariableNode *_node) const;
    };
}
