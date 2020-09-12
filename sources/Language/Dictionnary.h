#pragma once
#include "Nodable.h"
#include "TokenType.h"
#include <string>
#include <map>
#include <regex>

namespace Nodable
{
	/**
	* The role of this class is to store make easy to convert a string to a TokenType
	* and a TokenType to a string.
	* We can also use reglar expressions.
	* 
	* ex:
	* 
	*   "true"          => TokenType::Boolean
	*   "boolean"       => TokenType::BooleanType
	*   "^(true|false)" => TokenType::Boolean
	* 
	*/
	class Dictionnary
	{
	public:
		Dictionnary() {}
		~Dictionnary() {}
		std::string convert(const TokenType&)const;
		void        insert(std::regex, TokenType);
		void        insert(std::string, TokenType);
		const auto& getTokenTypeToRegexMap()const { return tokenTypeToRegex;  }
	private:
		std::string numbers;
		std::string letters;
		std::map<std::string, TokenType> keywordToTokenType;
		std::map<TokenType, std::string> tokenTypeToString;
		std::map<TokenType, std::regex>  tokenTypeToRegex;
	};
}
