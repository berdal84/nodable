#pragma once
#include <memory>

#include "tools/core/types.h"
#include "tools/core/reflection/reflection"

#include "DirectedEdge.h"
#include "NodeComponent.h"
#include "Property.h"
#include "Token.h"

namespace ndbl
{
    typedef int InvokableFlags;
    enum InvokableFlag_
    {
        InvokableFlag_NONE          = 0,
        InvokableFlag_IS_OPERATOR   = 1 << 0,
        InvokableFlag_IS_INVOKABLE  = 1 << 1,
        InvokableFlag_WAS_INVOKED   = 1 << 2,
    };

	/**
	  * @brief ComputeFunction extends Compute base to provide a Component that represents a Function.
	  *        This function has_flags some arguments.
	  */
	class InvokableComponent : public NodeComponent
    {
	public:
        InvokableComponent();
		InvokableComponent(const tools::func_type*, InvokableFlags _flags, const tools::IInvokable*);
		~InvokableComponent() = default;

        Token token;
        void                        invoke();
		void                        bind_arg(size_t _index, Slot*);
        const std::vector<Slot*>&   get_arguments() const;
		const tools::func_type*     get_func_type()const { return m_signature; }
		const tools::IInvokable*    get_function()const { return m_invokable; }
        void                        bind_result(Slot*);
        bool                        has_function() const { return m_invokable; };
		bool                        has_flags(InvokableFlags flags)const { return (m_flags & flags) == flags; };
        void                        set_flags(InvokableFlags flags) { m_flags |= flags; }
        void                        clear_flags(InvokableFlags flags) { m_flags &= ~flags; }
    protected:
        InvokableFlags             m_flags         = InvokableFlag_NONE;
        Slot*                      m_result_slot   = nullptr;
        const tools::func_type*    m_signature     = nullptr;
        const tools::IInvokable*   m_invokable     = nullptr;
        std::vector<Slot*>         m_argument_slot;

        REFLECT_DERIVED_CLASS()
    };
}
