#pragma once

#include "tools/core/reflection/reflection"
#include "tools/core/memory/memory.h"

namespace ndbl
{
    ///
    /// Generic template to declare a component for a given OwnerT
    /// \tparam T owner type
    template<class T>
	class TComponent
	{
    public:
        using OwnerT = T;

        static_assert(std::is_default_constructible_v<OwnerT>);
        static_assert(std::is_copy_assignable_v<OwnerT>);
        static_assert(std::is_copy_constructible_v<OwnerT>);

        REFLECT_BASE_CLASS()

    public:
        OwnerT get_owner()const
        { return m_owner; }

        virtual void set_owner(OwnerT owner)
        { m_owner = owner; }
	protected:
        OwnerT m_owner{};
    };
}