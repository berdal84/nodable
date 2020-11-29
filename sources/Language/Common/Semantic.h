#pragma once
#include "Nodable.h"
#include "TokenType.h"
#include <string>
#include <map>
#include <regex>
#include <Core/Type.h>

namespace Nodable
{
	/**
	* The role of Semantic is to provide a way to insert and apply simple conversions:
	 *
	 * std::string   <===> TokenType
	 * std::regex    <===> TokenType
	 * Nodable::Type <===> TokenType
	 *
	 * A Semantic is designed to a part of a Language.
	*/
	class Semantic
	{
	public:
		Semantic() = default;
		~Semantic() = default;

		/**
		 * Insert a regular expression to TokenType correspondence (for a Parser).
		 *
		 * A string that matching with the regular expression will be interpreted as a specific given type.
		 */
		void insert_RegexToTokenType(std::regex, TokenType);

		/**
		 * Insert a string to TokenType correspondence.
		 *
		 * This will provide:
		 * - a TokenType/std::string simple correspondence (for a Serializer).
		 * - a std::regex/TokenType correspondence (for a Parser).
		 */
		void insert_StringToTokenType(std::string, TokenType);

		/**
		 * Insert a bidirectional correspondence between a type and a token type (for a Language)
		 */
		void insert_TypeToTokenType(Type _type, TokenType _tokenType);

		[[nodiscard]] inline const auto& getTokenTypeToRegexMap()const { return m_tokenTypeToRegex;  }
        [[nodiscard]] std::string tokenTypeToString(const TokenType&)const;
        [[nodiscard]] TokenType  typeToTokenType(Type _type)const;
        [[nodiscard]] Type tokenTypeToType(TokenType _tokenType)const;

	private:
		std::map<TokenType, std::string> m_tokenTypeToString;
		std::multimap<TokenType, std::regex>  m_tokenTypeToRegex;
		std::map<TokenType, Type> m_tokenTypeToTypeMap;
		std::map<Type, TokenType> m_typeToTokenTypeMap;
	};
}
