#pragma once

namespace Nodable
{
    // forward decl
    class Member;
    class func_type;

    class iinvokable
    {
    public:
        virtual ~iinvokable() {};
        virtual const func_type* get_type() const = 0;
        virtual void invoke(Member *_result, const std::vector<Member *> &_args) const = 0;
    };
}