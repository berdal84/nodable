#pragma once

namespace Nodable
{
    // forward decl
    class FuncSig;
    class Member;

    /**
     * Interface for any invokable class (ex: InvokableFunction/InvokableOperator)
     */
    class IInvokable
    {
    public:
        virtual ~IInvokable() {};
        virtual const FuncSig* get_signature() const = 0;
        virtual void           invoke(Member *_result, const std::vector<Member *> &_args) const = 0;
    };
}