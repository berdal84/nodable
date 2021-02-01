#pragma once

#include <string>
#include <vector>
#include <Language/Common/TokenType.h>
#include <Language/Common/Function.h>
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
    class AbstractCodeBlock;
    class CodeBlock;
    class ScopedCodeBlock;
    struct Instruction;

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
        std::string serialize(const Member*)const;

        /** Serialize a token ( <token-serialized><suffix> ) */
        std::string serialize(const Token*) const;

        std::string serializeUnaryOp(const Operator*, std::vector<Member*>, const Operator*)const;

        /** Serialize a binary operation call using an operator and two operands.
           The last two operators are the source operators that creates the two operands as result.
           Those are used to check precedence and add some brackets if needed.
        */
        std::string serializeBinaryOp(const Operator*, std::vector<Member*>, const Operator*, const Operator*)const;

        /** Serialize a complete scope (a set of instructions) */
        std::string serialize(const CodeBlock*) const;

        /** Serialize a single instruction ( can be a simple expression ) */
        std::string serialize(const Instruction*)const;

        std::string serialize(const ScopedCodeBlock*)const;
    protected:
        const Language* language;
    };
}
