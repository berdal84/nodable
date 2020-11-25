#pragma once
#include "Language.h"

namespace Nodable {

	/*
		The role of this class is to define a first language for Nodable.
	*/
	class LanguageNodable : public Language
	{
	public:
		LanguageNodable();
        ~LanguageNodable() override = default;

		// Inherited via Language
		virtual std::string serialize(const FunctionSignature&, std::vector<Member*>) const;
		virtual std::string serialize(const FunctionSignature&) const;
		virtual std::string serialize(const TokenType&) const;
        virtual std::string serialize(const Member*) const;
        virtual std::string serializeBinaryOp(const Operator*, std::vector<Member*>, const Operator*, const Operator*)const;
		virtual std::string serializeUnaryOp(const Operator*, std::vector<Member*>, const Operator*)const;
		virtual const FunctionSignature createBinOperatorSignature(Type, std::string, Type, Type) const;
		virtual const FunctionSignature createUnaryOperatorSignature(Type, std::string, Type) const;
		virtual const TokenType typeToTokenType(Type _type)const;
		virtual const Type tokenTypeToType(TokenType _tokenType)const;
	};
}

