#pragma once
#include <nodable/core/Language.h>

namespace Nodable {

	/*
		The role of this class is to define a first language for Nodable.
	*/
	class LanguageNodable : public Language
	{
	public:
		LanguageNodable();
        ~LanguageNodable() override = default;
        void sanitize_function_identifier( std::string& ) const override ;
        void sanitize_operator_fct_identifier( std::string& ) const override ;
	};
}

