#pragma once

#include <string>
#include <vector>
#include <nodable/TokenType.h>
#include <nodable/Function.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/Token.h>

namespace Nodable
{
    // forward declarations
    class ComputeBase;
    class ComputeFunction;
    class ComputeUnaryOperation;
    class ComputeBinaryOperation;
    class Member;
    class Operator;
    class Language;
    class AbstractCodeBlockNode;
    class CodeBlockNode;
    class ScopedCodeBlockNode;
    class InstructionNode;

    class Serializer
    {
    public:

        explicit Serializer(const Language* _language): language(_language) {};
        ~Serializer() = default;

        std::string& serialize(std::string &_result, const ComputeUnaryOperation * _operation)const;
        std::string& serialize(std::string &_result, const ComputeBinaryOperation * _operation)const;
        std::string& serialize(std::string &_result, const ComputeBase * _operation)const;
        std::string& serialize(std::string &_result, const ComputeFunction* _computeFunction) const;
        std::string& serialize(std::string &_result, const FunctionSignature*, const std::vector<Member*>&)const;
        std::string& serialize(std::string &_result, const FunctionSignature*)const;
        std::string& serialize(std::string &_result, const TokenType&)const;
        std::string& serialize(std::string &_result, const Type&)const;
        virtual std::string& serialize(std::string &_result, const Member*, bool followConnections = true)const;
        virtual std::string& serialize(std::string &_result, const Token*) const;
        virtual std::string& serialize(std::string &_result, const CodeBlockNode*) const;
        virtual std::string& serialize(std::string &_result, const InstructionNode*)const;
        virtual std::string& serialize(std::string &_result, const ScopedCodeBlockNode*)const;
        virtual std::string& serialize(std::string &_result, const ConditionalStructNode*) const;
        virtual std::string& serialize(std::string &_result, const Variant* variant) const;
    protected:
        const Language* language;

        std::string& serialize(std::string &_result, const VariableNode *_node) const;
    };
}
