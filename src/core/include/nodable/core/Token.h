#pragma once

#include <nodable/core/Token_t.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>

namespace ndbl
{
    /**
     * @struct Serve as container for the result of a scanner (or tokenizer)
     *
     * @example "a + b" will generate 3 tokens: "a ", "+ ", and "b".
     *
     * @details
     * The token store this piece of information using a buffer and some information
     * to retrieve the word, its prefix and suffix.
     *
     * @code
     *   buffer:    "prefix|word|suffix"
     *       (word_pos) ---^
     *                     |<--->| (word_size)
     */
	struct Token
	{
        /** Index of this token into the ribbon */
	    size_t      m_index;
        /** Type of token (ex: operator, symbol, literal, etc.) */
		Token_t     m_type;
        /** Index (or position) of the buffer first char in its original source
         * ex: source = "3+21", the index of "21" is 2.*/
		size_t      m_charIndex;
		/** String buffer to store a parsed word into (including prefix and suffix)
		 * ex:     "  int ", "( ", "    )", etc. */
        std::string m_buffer;
        /** Index where the word is in the buffer */
        size_t      m_word_pos;
        /** Size of the word (without prefix/suffix) */
        size_t      m_word_size;

        Token()
                : m_type( Token_t::default_ )
                , m_buffer()
                , m_index(0)
                , m_charIndex(0)
                , m_word_pos(0)
                , m_word_size(0)
        {}

        Token(Token_t _type ): Token()
        {
            m_type = _type;
        }

		Token(Token_t _type, std::string  _word, size_t _char_index)
            : Token(_type)
        {
            append_to_word(std::move(_word));
        }

        Token(Token_t _type, char  _char, size_t _char_index)
            : Token(_type)
        {
            append_to_word(_char);
        }

        Token(Token_t _type, const char* _char, size_t _char_index)
        : Token( _type, std::string{_char}, _char_index)
        {}

        ~Token() = default;
        /** Clear token */
        void clear();
        /** Get the word (no suffix/preffix) from the token buffer */
        std::string get_word()const;
        /** Get the suffix from the token buffer */
        std::string get_prefix()const;
        /** Get the prefix fromm the token buffer */
        std::string get_suffix()const;
        /** Append a string to the buffer prefix */
        void append_to_prefix(const std::string&);
        /** Append a string to the buffer suffix */
        void append_to_suffix(const std::string&);
        /** Append a string to the word */
        void append_to_word(const std::string&);
        /** Set the word (overwrite existing) */
        void set_word(const std::string&);
        /** Append a char to the word */
        void append_to_word(char);
        /** Check if whether or not this token is a keyword type */
        bool is_keyword_type() { return ndbl::is_a_type_keyword(m_type); }
        /** Transfer the prefix and suffix of a given token to this token */
        void transfer_prefix_suffix(const std::shared_ptr<Token>);
        /** Get the token as JSON formatted string */
        static std::string to_JSON(const std::shared_ptr<Token>);
        /** Get the token as string (just pass the buffer as-is, prefix+word+suffix) */
        static std::string to_string(const std::shared_ptr<Token>);
        /** Token to act as null */
        static const std::shared_ptr<Token> s_null;
    };
}