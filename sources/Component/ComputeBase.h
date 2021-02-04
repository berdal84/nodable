#pragma once

#include "Nodable.h"
#include "Component.h"

namespace Nodable
{
    class Language;

	class ComputeBase : public Component {
	public:
		ComputeBase(const Language* _language) :language(_language) {};
		virtual ~ComputeBase() { delete this->sourceToken; };
		void         setResult(Member* _value) { result = _value; };

        void setSourceToken(Token *token)
        {
            this->sourceToken = token;
        }

        Token* getSourceToken()const
        {
            return this->sourceToken;
        }

	protected:
		const Language* language;
		Member* result = nullptr;
		Token* sourceToken = nullptr;
		MIRROR_CLASS(ComputeBase)(
			MIRROR_PARENT(Component)
			);
	};
}