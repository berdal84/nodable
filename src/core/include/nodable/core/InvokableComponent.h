#pragma once
#include <nodable/core/memory.h>
#include <nodable/core/types.h>
#include <nodable/core/Token.h>
#include <nodable/core/Component.h>
#include <nodable/core/reflection/invokable.h>

namespace ndbl
{
    // forward declarations
    class iinvokable;
    class func_type;

	/**
	  * @brief ComputeFunction extends Compute base to provide a Component that represents a Function.
	  *        This function has some arguments.
	  */
	class InvokableComponent : public Component
    {
	public:
		InvokableComponent(const func_type*, bool _is_operator, const iinvokable*);
		~InvokableComponent() = default;

        bool                               update() override;
		inline void                        set_arg(size_t _index, Member *_value) { m_args[_index] = _value; }
		inline Member*                     get_arg(size_t _index)const  { return m_args[_index]; }
		inline const std::vector<Member*>& get_args()const { return m_args; }
		inline const func_type*              get_func_type()const { return m_signature; }
		inline const iinvokable*           get_function()const { return m_invokable; }
        inline void                        set_result(Member *_value) { m_result = _value; };
        inline void                        set_source_token(s_ptr<Token> token) { m_source_token = token ? token : Token::s_null; }
        inline s_ptr<Token>      get_source_token()const { return this->m_source_token; }
        inline void                        set_l_handed_val(Member *_value) { m_args[0] = _value; }
        inline Member*                     get_l_handed_val() { return m_args[0]; };
        inline void                        set_r_handed_val(Member *_value)  { m_args[1] = _value; }
        inline Member*                     get_r_handed_val() { return m_args[1]; };
        inline bool                        has_function() const { return m_invokable; };
		bool                               is_operator()const { return m_is_operator; };

	protected:
        Member*                 m_result;
		s_ptr<Token>  m_source_token;
        std::vector<Member*>    m_args;
        const func_type*        m_signature;
        const iinvokable*       m_invokable;
        const bool              m_is_operator;

        REFLECT_DERIVED_CLASS(Component)
	};
}
