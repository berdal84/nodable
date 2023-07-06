#pragma once

#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "fw/core/reflection/reflection"
#include "fw/core/assertions.h"
#include "Token_t.h"

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
     * @code                                    buffer_size
     *                                     |<--------------->|
     *             buffer_start_pos ]------v
     *   buffer:                          "prefix|word|suffix"
     *             word_start_pos ]--------------^
     *
     *                                           |<-->|
     *                                         word_size
     */
	struct Token
	{
	    size_t      m_index; // Index of this token in the context of a TokenRibbon
        size_t      m_buffer_start_pos, m_buffer_size; // position and size of the token's buffer in m_source_buffer
        size_t      m_word_start_pos, m_word_size; // position and size of the token's word (without prefix/suffix) in m_source_buffer
        char*       m_source_buffer; //  address to the source code buffer, can be local or global (see m_owns_buffer flag)
        Token_t     m_type;
        bool        m_is_source_buffer_owned; // When true, this instance owns m_source_buffer

        Token(): Token(Token_t::null){}

        Token(Token_t _type)
            : m_type(_type)
            , m_index(0)
            , m_source_buffer(nullptr)
            , m_is_source_buffer_owned(false)
            , m_buffer_start_pos(0)
            , m_buffer_size(0)
            , m_word_start_pos(0)
            , m_word_size(0)
        {}

        Token(Token_t _type, const char* _word)
            : Token(_type)
        {
            m_buffer_size = strlen(_word);
            m_source_buffer = new char[m_buffer_size];
            strncpy(m_source_buffer, _word, m_buffer_size);
            m_is_source_buffer_owned = true;
            m_word_size = m_buffer_size;
        }

        Token(Token_t _type,
              char* _buffer,
              size_t _buffer_start_pos,
              size_t _buffer_size,
              size_t _word_start_pos,
              size_t _word_size
            )
            : m_source_buffer(_buffer)
            , m_type(_type)
            , m_index(0)
            , m_buffer_start_pos(_buffer_start_pos)
            , m_buffer_size(_buffer_size)
            , m_word_start_pos(_word_start_pos)
            , m_word_size(_word_size)
            , m_is_source_buffer_owned(false)
        {
        }

        Token(Token_t _type,
              char* const _buffer,
              size_t _buffer_start_pos,
              size_t _buffer_size
        ): Token(_type, _buffer, _buffer_start_pos, _buffer_size, _buffer_start_pos, _buffer_size)
        {}

        Token(const Token& other)
            : m_type(other.m_type)
            , m_index(other.m_index)
            , m_source_buffer(other.m_source_buffer)
            , m_is_source_buffer_owned(false)
            , m_buffer_start_pos(other.m_buffer_start_pos)
            , m_buffer_size(other.m_buffer_size)
            , m_word_start_pos(other.m_word_start_pos)
            , m_word_size(other.m_word_size)
        {}

        ~Token() {
            if( m_is_source_buffer_owned ) delete[] m_source_buffer;
        };
        void clear();
        inline std::string buffer_to_string() const {
            if (buffer() == nullptr) return {};
            return { buffer(), m_buffer_size };
        };
        inline bool has_buffer()const { return m_source_buffer != nullptr; }
        inline std::string prefix_to_string()const { return has_buffer() ? std::string{ prefix(), prefix_size()} : ""; }
        inline std::string word_to_string()const { return has_buffer() ? std::string{ word(), m_word_size} : ""; }
        inline std::string suffix_to_string()const { return has_buffer() ? std::string{ suffix(), suffix_size()} : ""; }
        inline char* buffer() const { return m_source_buffer + m_buffer_start_pos; }
        inline char* prefix() const { return buffer(); }
        inline char* word() const { return m_source_buffer + m_word_start_pos; };
        inline char* suffix() const { return m_source_buffer + m_word_start_pos + m_word_size; }
        inline size_t prefix_size() const { return m_word_start_pos - m_buffer_start_pos; }
        inline size_t word_size() const { return m_word_size; }
        inline size_t suffix_size() const { return m_buffer_size - prefix_size() - m_word_size; }
        inline bool is_keyword_type() { return ndbl::is_a_type_keyword(m_type); } // Check if whether or not this token is a keyword type
        void transfer_prefix_and_suffix_from(Token *source); // Transfer the prefix and suffix of a given token to this token
        void set_source_buffer(char* _buffer, size_t pos = 0, size_t size = 0);
        std::string json()const;
        bool is_null() const { return m_type == Token_t::null; }
        static const Token s_null; // To act as null Token
    };
}
