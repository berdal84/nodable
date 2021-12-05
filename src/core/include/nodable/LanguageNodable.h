#pragma once
#include <nodable/Language.h>

namespace Nodable {

	/*
		The role of this class is to define a first language for Nodable.
	*/
	class LanguageNodable : public Language
	{
	public:
		LanguageNodable();
        ~LanguageNodable() override = default;
        void sanitizeFunctionName( std::string& identifier ) const override ;
        void sanitizeOperatorFunctionName( std::string& identifier ) const override ;
	};
}
