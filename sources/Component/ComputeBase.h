#pragma once

#include "Nodable.h"
#include "Component.h"

namespace Nodable
{
    class Language;

	class ComputeBase : public Component {
	public:
		ComputeBase(const Language* _language) :language(_language) {};
		virtual ~ComputeBase() {};
		void         setResult(Member* _value) { result = _value; };

        void setSourceToken(Token *token)
        {
            if ( token )
                this->sourceToken = *token;
            else
                this->sourceToken = TokenType_NULL;
        }

        [[nodiscard]] const Token* getSourceToken()const { return &this->sourceToken; }

	protected:
		const Language* language;
		Member* result = nullptr;
		Token sourceToken = Token::Null;
		MIRROR_CLASS(ComputeBase)(
			MIRROR_PARENT(Component)
			);
	};
}