#pragma once
#include "Language/Common/Language.h"

namespace Nodable {

	/*
		The role of this class is to define a first language for Nodable.
	*/
	class NodableLanguage : public Language
	{
	public:
		NodableLanguage();
        ~NodableLanguage() override = default;

		virtual const FunctionSignature createBinOperatorSignature(Type, std::string, Type, Type) const;
		virtual const FunctionSignature createUnaryOperatorSignature(Type, std::string, Type) const;
		virtual const TokenType typeToTokenType(Type _type)const;
		virtual const Type tokenTypeToType(TokenType _tokenType)const;
	};
}

