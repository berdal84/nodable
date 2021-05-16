#pragma once

#include <string>
#include <map>
#include <vector>
#include <regex>

#include "Nodable.h"
#include "TokenType.h"
#include "Type.h"

namespace Nodable::core
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
		Semantic();
		~Semantic() = default;

		/**
		 * Insert a regular expression to TokenType correspondence (for a Parser).
		 *
		 * A string that matching with the regular expression will be interpreted as a specific given type.
		 */
		void insert(const std::regex&, TokenType);

		/**
		 * Insert a string to TokenType correspondence.
		 *
		 * This will provide:
		 * - a TokenType/std::string simple correspondence (for a Serializer).
		 * - a std::regex/TokenType correspondence (for a Parser).
		 */
		void insert(std::string _string, TokenType _tokenType);

		/**
		 * Insert a bidirectional correspondence between a type and a token type (for a Language)
		 */
		void insert(Type _type, TokenType _tokenType);

		[[nodiscard]] inline const std::vector<std::regex>& getRegex()const { return m_regex;  }
        [[nodiscard]] inline std::string tokenTypeToString(TokenType _type)const { return m_tokenTypeToString[_type]; }
        [[nodiscard]] inline TokenType typeToTokenType(Type _type)const { return m_typeToTokenType[_type]; }
        [[nodiscard]] inline Type tokenTypeToType(TokenType _tokenType)const { return m_tokenTypeToType[_tokenType]; }
        [[nodiscard]] inline const std::vector<TokenType>& getRegexIndexToTokenType()const { return m_regexIndexToTokenType; }

    private:
	    /**
	     * Two vector to:
	     *  1- iterate fast when we parse using all regex (in m_regex)
	     *  2- once we found a regex that matches we get the TokenType (located at same index in m_type)
	     */
        std::vector<std::regex> m_regex;
        std::vector<TokenType> m_regexIndexToTokenType;

		/** uses Type as index */
		std::vector<TokenType> m_typeToTokenType;
        /** uses TokenType as index */
        std::vector<Type> m_tokenTypeToType;
        /** uses TokenType as index */
        std::vector<std::string> m_tokenTypeToString;
    };
}
