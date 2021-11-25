#pragma once

#include <nodable/Nodable.h>
#include <nodable/Token.h>
#include <nodable/Component.h>

namespace Nodable
{
    // forward declarations
    class Language;

    /**
     * @brief Base for Compute Components.
     */
	class ComputeBase : public Component
    {
    protected:
		ComputeBase()
		    : m_result( nullptr )
		    , m_sourceToken( Token::s_null )
		    {};
		~ComputeBase() = default;

    public:
		void         setResult(Member* _value) { m_result = _value; };
        void         setSourceToken(Token *token) { m_sourceToken = token ? *token : TokenType_NULL; }
        const Token* getSourceToken()const { return &this->m_sourceToken; }

	protected:
		Member*         m_result;
		Token           m_sourceToken;

		REFLECT_DERIVED(ComputeBase)
    REFLECT_EXTENDS(Component)
    REFLECT_END
	};
}