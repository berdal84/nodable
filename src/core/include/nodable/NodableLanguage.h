#pragma once
#include <nodable/Language.h>

namespace Nodable {

	/*
		The role of this class is to define a first language for Nodable.
	*/
	class NodableLanguage : public Language
	{
	public:
		NodableLanguage();
        ~NodableLanguage() override = default;
        void sanitizeFunctionName( std::string& identifier ) const override ;
        void sanitizeOperatorFunctionName( std::string& identifier ) const override ;
	};
}

