#pragma once

namespace Nodable
{
    // forward decl
    class FunctionSignature;
    class Member;

    /**
     * Interface for any invokable class (ex: InvokableFunction/InvokableOperator)
     */
    class IInvokable
    {
    public:
        enum class Type {
            Function,
            OperatorFct
        };
        virtual ~IInvokable() {};
        virtual const FunctionSignature* get_signature() const = 0;
        virtual void                     invoke(Member *_result, const std::vector<Member *> &_args) const = 0;
        virtual IInvokable::Type         get_invokable_type() const = 0;
    };
}