#pragma once
#include "Nodable.h"
#include "LanguageEnums.h"
#include <string>
#include <map>
#include <regex>

namespace Nodable
{

	class Dictionnary
	{
	public:
		Dictionnary() {}
		~Dictionnary() {}
		std::string convert(const TokenType&)const;
		void        add(std::regex, TokenType);
		void        add(std::string, TokenType);
		void        add(std::string, TokenType, std::regex);
		const auto& getTokenTypeToRegexMap()const { return tokenTypeToRegex;  }
	private:
		std::string numbers;
		std::string letters;
		std::map<std::string, TokenType> keywordToTokenType;
		std::map<TokenType, std::string> tokenTypeToString;
		std::map<TokenType, std::regex>  tokenTypeToRegex;
	};
}
