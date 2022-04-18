#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <regex>
#include <memory> // std::shared_ptr

#include <nodable/core/types.h>
#include <nodable/core/Token_t.h>
#include <nodable/core/reflection/reflection>

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
		Semantic();
		~Semantic() = default;

		/**
		 * Insert a regular expression to TokenType correspondence (for a Parser).
		 *
		 * A string that matching with the regular expression will be interpreted as a specific given type.
		 */
		void insert(const std::regex&, Token_t);
        void insert(const std::regex&, Token_t, type);

		/**
		 * Insert a string to TokenType correspondence.
		 *
		 * This will provide:
		 * - a TokenType/std::string simple correspondence (for a Serializer).
		 * - a std::regex/TokenType correspondence (for a Parser).
		 */
		void insert(std::string, Token_t);
        void insert(std::string, Token_t, type);

		/**
		 * Insert a bidirectional correspondence between a type and a token type (for a Language)
		 */
		void insert(std::string, type);


		[[nodiscard]] inline const std::vector<std::regex>& get_token_type_regex()const { return m_token_type_regex;  }
        [[nodiscard]] inline std::string                    type_to_string(type _type)const { return m_type_to_string.find(_type.hash_code())->second; }
        [[nodiscard]] inline std::string                    token_type_to_string(Token_t _type)const { return m_token_type_to_string.find(_type)->second; }
        [[nodiscard]] inline type                           token_type_to_type(Token_t _tokenType)const { return m_token_type_to_type.find(_tokenType)->second; }
        [[nodiscard]] inline const std::vector<Token_t>&    get_token_type_regex_index_to_token_type()const { return m_regex_index_to_token_type; }

    private:
        std::vector<std::regex>  m_type_regex;
        std::vector<std::regex>  m_token_type_regex;

        std::vector<Token_t>     m_regex_index_to_token_type;
        std::vector<type>        m_regex_index_to_type;

		std::unordered_map<size_t, Token_t> m_type_to_token_type;
		std::unordered_map<Token_t, type> m_token_type_to_type;

        std::unordered_map<Token_t, std::string> m_token_type_to_string;
        std::unordered_map<size_t, std::string> m_type_to_string;
    };
}
