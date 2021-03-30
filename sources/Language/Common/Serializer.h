#pragma once

#include <string>
#include <vector>
#include <Language/Common/TokenType.h>
#include <Language/Common/Function.h>
#include <ConditionalStructNode.h>
#include "Token.h"

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

        Serializer(const Language* _language): language(_language) {};
        ~Serializer() = default;

        std::string serialize(const ComputeUnaryOperation * _operation)const;
        std::string serialize(const ComputeBinaryOperation * _operation)const;
        std::string serialize(const ComputeBase * _operation)const;
        std::string serialize(const ComputeFunction* _computeFunction) const;

        /** Serialize a function call with a signature and some values */
        std::string serialize(const FunctionSignature&, std::vector<Member*>)const;

        /** Serialize a function signature */
        std::string serialize(const FunctionSignature&)const;

        /** Serialize a TokenType
           ex:
           TokenType::LBracket => "("
           TokenType::StringType = > "std::string" (for C++) */
        std::string serialize(const TokenType&)const;

        /** Serialize a Member */
        virtual std::string serialize(const Member*)const;

        /** Serialize a token ( <token-serialized><suffix> ) */
        virtual std::string serialize(const Token*) const;

        /** Serialize a complete scope (a set of instructions) */
        virtual std::string serialize(const CodeBlockNode*) const;

        /** Serialize a single instruction ( can be a simple expression ) */
        virtual std::string serialize(const InstructionNode*)const;

        virtual std::string serialize(const ScopedCodeBlockNode*)const;

        virtual std::string serialize(const ConditionalStructNode*) const;

    protected:
        const Language* language;
    };
}
