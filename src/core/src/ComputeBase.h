#pragma once

#include "Nodable.h"
#include "Token.h"
#include "Component.h"

namespace Nodable::core
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

		// reflect class using mirror
		MIRROR_CLASS(ComputeBase)(
			MIRROR_PARENT(Component)
		);
	};
}