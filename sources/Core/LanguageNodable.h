#pragma once
#include "Language.h"

namespace Nodable {
	class LanguageNodable : public Language
	{
	public:
		LanguageNodable();

		// Inherited via Language
		virtual std::string serialize(const FunctionSignature&, std::vector<const Member*>) const override;
	};
}

