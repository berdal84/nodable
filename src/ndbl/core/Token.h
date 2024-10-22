#pragma once

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <cstring>

#include "tools/core/string.h"
#include "tools/core/reflection/reflection"
#include "tools/core/assertions.h"
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
     * to retrieve the word_ptr, its prefix_ptr and suffix_ptr.
     *
     * @code                                 m_string_length
     *                                     |<------------------>|
     *           m_string_start_pos ]------v
     *   m_buffer ]---------> "// example\n    int hello; // todo"
     *           m_word_start_pos ]--------------^
     *
     *                                           |<-->|
     *                                         m_word_length
     */
	struct Token
	{
	    size_t      m_index            = 0; // Index of this token in the context of a TokenRibbon
        char*       m_buffer           = nullptr; //  address to the source code buffer, can be local or global (see m_owns_buffer flag)
        bool        m_is_buffer_owned  = false; // When true, this instance owns m_source_buffer
        size_t      m_string_start_pos = 0; // position of the token's buffer in m_source_buffer
        size_t      m_string_length    = 0; //     size of ...
        size_t      m_word_start_pos   = 0; // position of the token's word (without prefix/suffix) in m_source_buffer
        size_t      m_word_length      = 0; //     size of ...
        Token_t     m_type             = Token_t::any;

        Token() {}

        Token(Token_t _type)
        : m_type(_type)
        {}

        Token(Token_t _type, const char* const _word)
            : Token(_type)
        {
            m_buffer          = const_cast<char*>(_word);
            m_is_buffer_owned = false; // <<----------- const char* must not be changed
            m_string_length   = strlen(_word);
            m_word_length     = m_string_length;
        }

        Token(Token_t _type,
              char* _buffer,
              size_t _string_start_pos,
              size_t _string_length,
              size_t _word_start_pos,
              size_t _word_length
            )
            : m_buffer(_buffer)
            , m_type(_type)
            , m_index(0)
            , m_string_start_pos(_string_start_pos)
            , m_string_length(_string_length)
            , m_word_start_pos(_word_start_pos)
            , m_word_length(_word_length)
            , m_is_buffer_owned(false)
        {
        }

        Token(Token_t _type,
              char* const _buffer,
              size_t _string_start_pos,
              size_t _string_length
        ): Token(
            _type, 
            _buffer, 
            _string_start_pos,
            _string_length,
            _string_start_pos,
            _string_length
            )
        {}

        Token(const Token& other)
            : m_type(other.m_type)
            , m_index(other.m_index)
            , m_buffer(other.m_buffer)
            , m_is_buffer_owned(false)
            , m_string_start_pos(other.m_string_start_pos)
            , m_string_length(other.m_string_length)
            , m_word_start_pos(other.m_word_start_pos)
            , m_word_length(other.m_word_length)
        {
            VERIFY(other.m_is_buffer_owned == false, "Can't create a Token from an owned const char*");
        }

        Token(Token&& other);
        inline explicit operator bool () const
        { return !this->is_null(); }

        Token& operator=(const Token& other);
        Token& operator=(Token&&) noexcept;

        ~Token() {
            if( m_is_buffer_owned ) delete[] m_buffer;
        };
        void clear();

        inline bool has_buffer()const
        { return m_buffer != nullptr; }

        // Convert token portion to string
        // Warning: this costs an allocation, use xxx_ptr() and xxx_size() when possible.

        std::string buffer_to_string() const;
        std::string prefix_to_string()const;
        std::string word_to_string()const;
        std::string suffix_to_string()const;

        // Get token portion addresses

        inline char* string_ptr() const { return m_buffer + m_string_start_pos; }
        inline char* prefix_ptr() const { return string_ptr(); }
        inline char* word_ptr() const { return m_buffer + m_word_start_pos; };
        inline char* suffix_ptr() const { return m_buffer + m_word_start_pos + m_word_length; }

        // Get token portion sizes

        inline size_t string_size() const { return m_string_length; }
        inline size_t prefix_size() const { return m_word_start_pos - m_string_start_pos; }
        inline size_t word_size() const { return m_word_length; }
        inline size_t suffix_size() const { return m_string_length - prefix_size() - m_word_length; }

        inline bool is_keyword_type() { return ndbl::is_a_type_keyword(m_type); } // Check if whether this token is a keyword type
        void        take_prefix_suffix_from(Token *source); // Transfer the prefix and suffix of a given token to this token
        void        set_source_buffer(char* _buffer, size_t pos = 0, size_t size = 0);
        std::string json()const;
        bool        is_null() const { return m_type == Token_t::null; }
        void        word_replace(const char* new_word);
        void        suffix_append(const char* str);

        static const Token s_null; // To act as null Token
        static const Token s_end_of_line;
        static const Token s_end_of_instruction;
    };
}
static_assert(std::is_move_assignable_v<ndbl::Token>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::Token>, "Should be move constructible");