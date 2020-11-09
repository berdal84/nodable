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

		// Inherited via Language
		virtual std::string serialize(const FunctionSignature&, std::vector<std::shared_ptr<Member>>) const;
		virtual std::string serialize(const FunctionSignature&) const;
		virtual std::string serialize(const TokenType&) const;
		virtual std::string serializeBinaryOp(std::shared_ptr<const Operator>, std::vector<std::shared_ptr<Member>>, std::shared_ptr<const Operator>, std::shared_ptr<const Operator>)const;
		virtual std::string serializeUnaryOp(std::shared_ptr<const Operator>, std::vector<std::shared_ptr<Member>>, std::shared_ptr<const Operator>)const;
		virtual const FunctionSignature createBinOperatorSignature(Type, std::string, Type, Type) const;
		virtual const FunctionSignature createUnaryOperatorSignature(Type, std::string, Type) const;
		virtual const TokenType typeToTokenType(Type _type)const;
		virtual const Type tokenTypeToType(TokenType _tokenType)const;
	};
}

