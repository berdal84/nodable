#pragma once

#include <nodable/Nodable.h>
#include <nodable/Token.h>
#include <nodable/InvokableFunction.h>
#include <nodable/InvokableOperator.h>

namespace Nodable
{
    // forward declarations
    class IInvokable;

	/**
	  * @brief ComputeFunction extends Compute base to provide a Component that represents a Function.
	  *        This function has some arguments.
	  */
	class InvokableComponent : public Component
    {
	public:
		InvokableComponent(const IInvokable*);
		~InvokableComponent() = default;

        bool                               update() override;
		inline void                        set_arg(size_t _index, Member *_value) { m_args[_index] = _value; }
		inline Member*                     get_arg(size_t _index)const  { return m_args[_index]; }
		inline const std::vector<Member*>& get_args()const { return m_args; }
		inline const IInvokable*            get_invokable()const { return m_invokable; }
        inline void                        set_result(Member *_value) { m_result = _value; };
        inline void                        set_source_token(Token *token) { m_source_token = token ? *token : TokenType_NULL; }
        inline const Token*                get_source_token()const { return &this->m_source_token; }
        inline void                        set_l_handed_val(Member *_value) { m_args[0] = _value; }
        inline Member*                     get_l_handed_val() { return m_args[0]; };
        inline void                        set_r_handed_val(Member *_value)  { m_args[1] = _value; }
        inline Member*                     get_r_handed_val() { return m_args[1]; };

    protected:
        Member*              m_result;
        Token                m_source_token;
        std::vector<Member*> m_args;
        const IInvokable*     m_invokable;

        R_DERIVED(InvokableComponent)
        R_EXTENDS(Component)
        R_END
	};
}
