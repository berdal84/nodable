#pragma once

namespace Nodable
{
    // forward decl
    class FunctionSignature;
    class Member;

    /**
     * Interface for any invokable class (ex: InvokableFunction/InvokableOperator)
     */
    class Invokable
    {
    public:
        enum Type {
            Function,
            Operator
        };
        virtual const FunctionSignature* get_signature() const = 0;
        virtual void                     invoke(Member *_result, const std::vector<Member *> &_args) const = 0;
        virtual Invokable::Type          get_invokable_type() const = 0;
    };
}