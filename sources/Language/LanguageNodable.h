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
		virtual std::string serialize(const std::shared_ptr<FunctionSignature>&, std::vector<std::shared_ptr<Member>>) const;
		virtual std::string serialize(const std::shared_ptr<FunctionSignature>&) const;
		virtual std::string serialize(const TokenType&) const;
		virtual std::string serializeBinaryOp(std::shared_ptr<const Operator>, std::vector<std::shared_ptr<Member>>, std::shared_ptr<const Operator>, std::shared_ptr<const Operator>)const;
		virtual std::string serializeUnaryOp(std::shared_ptr<const Operator>, std::vector<std::shared_ptr<Member>>, std::shared_ptr<const Operator>)const;
		virtual std::shared_ptr<const FunctionSignature> createBinOperatorSignature(Type, std::string, Type, Type) const;
		virtual std::shared_ptr<const FunctionSignature> createUnaryOperatorSignature(Type, std::string, Type) const;
		virtual const TokenType typeToTokenType(Type _type)const;
		virtual const Type tokenTypeToType(TokenType _tokenType)const;
	};
}

