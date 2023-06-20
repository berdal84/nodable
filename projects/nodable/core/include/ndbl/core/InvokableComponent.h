#pragma once
#include <memory>
#include "fw/core/types.h"
#include <fw/core/reflection/invokable.h>
#include <fw/core/reflection/func_type.h>

#include <ndbl/core/Token.h>
#include <ndbl/core/Component.h>

namespace ndbl
{
    // forward declarations
    class Property;

	/**
	  * @brief ComputeFunction extends Compute base to provide a Component that represents a Function.
	  *        This function has some arguments.
	  */
	class InvokableComponent : public Component
    {
	public:
		InvokableComponent(const fw::func_type*, bool _is_operator, const fw::iinvokable*);
		~InvokableComponent() = default;

        Token token;
        bool                               update();
		inline void                        set_arg(size_t _index, Property *_value) { m_args[_index] = _value; }
		inline Property *                     get_arg(size_t _index)const  { return m_args[_index]; }
		inline const std::vector<Property *>& get_args()const { return m_args; }
		inline const fw::func_type*        get_func_type()const { return m_signature; }
		inline const fw::iinvokable*       get_function()const { return m_invokable; }
        inline void                        set_result(Property *_value) { m_result = _value; };
        inline void                        set_l_handed_val(Property *_value) { m_args[0] = _value; }
        inline Property *                  get_l_handed_val() { return m_args[0]; };
        inline void                        set_r_handed_val(Property *_value)  { m_args[1] = _value; }
        inline Property *                  get_r_handed_val() { return m_args[1]; };
        inline bool                        has_function() const { return m_invokable; };
		bool                               is_operator()const { return m_is_operator; };

	protected:
        Property *                 m_result;
        std::vector<Property *>    m_args;
        const fw::func_type*       m_signature;
        const fw::iinvokable*      m_invokable;
        const bool                 m_is_operator;

        REFLECT_DERIVED_CLASS(Component)
	};
}
