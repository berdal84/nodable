#pragma once

#include <string>
#include <vector>
#include <Language/Common/TokenType.h>
#include <Language/Common/Function.h>

namespace Nodable
{
    class ComputeBase;
    class ComputeFunction;
    class ComputeUnaryOperation;
    class ComputeBinaryOperation;
    class Member;
    class Operator;
    class Language;

    class Serializer
    {
    public:

        Serializer(const Language* _language): language(_language) {};
        ~Serializer() = default;

        std::string serialize(const ComputeUnaryOperation * _operation)const;
        std::string serialize(const ComputeBinaryOperation * _operation)const;
        std::string serialize(const ComputeBase * _operation)const;
        std::string serialize(const ComputeFunction* _computeFunction) const;

        /* Serialize a function call with a signature and some values */
        std::string                   serialize(const FunctionSignature&, std::vector<Member*>)const;

        /* Serialize a function signature */
        std::string                   serialize(const FunctionSignature&)const;

        /* Serialize a TokenType
           ex:
           TokenType::LBracket => "("
           TokenType::StringType = > "std::string" (for C++) */
        std::string                   serialize(const TokenType&)const;

        /* Serialize a Member */
        std::string                   serialize(const Member*)const;

        /* Serialize a binary operation call using an operator and two operands.
           The last two operators are the source operators that creates the two operands as result.
           Those are used to check precedence and add some brackets if needed.
        */
        std::string                   serializeUnaryOp(const Operator*, std::vector<Member*>, const Operator*)const;
        std::string                   serializeBinaryOp(const Operator*, std::vector<Member*>, const Operator*, const Operator*)const;

    protected:
        const Language* language;
    };
}
