#pragma once
#include "Language.h"

namespace Nodable {
	class LanguageNodable : public Language
	{
	public:
		LanguageNodable();

		// Inherited via Language
		virtual std::string serialize(const FunctionSignature&, std::vector<Member*>) const;
		virtual std::string serialize(const FunctionSignature&) const;
		virtual std::string serialize(const TokenType&) const;
	};
}

