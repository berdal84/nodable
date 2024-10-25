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
        size_t      m_data_pos         = 0; // position of the token's buffer in m_source_buffer
        size_t      m_prefix_len       = 0;
        size_t      m_word_len         = 0;
        size_t      m_suffix_len       = 0;
        Token_t     m_type             = Token_t::none;

        explicit Token() {}

        Token(Token_t _type)
        : m_type(_type)
        {}

        Token(Token_t _type,
              const std::string& _word)
        : Token(_type)
        {
            word_replace(_word.c_str());
        }

        Token(Token_t _type,
              const char* const _word)
            : Token(_type)
        {
            m_buffer          = const_cast<char*>(_word);
            m_is_buffer_owned = false; // <<----------- const char* must not be changed
            m_word_len        = strlen(_word);
        }

        Token(Token_t type,
              char*   buffer,
              size_t  size)
            : m_buffer(buffer)
            , m_type(type)
            , m_word_len(size)
            , m_is_buffer_owned(false)
        {
        }

        Token(Token_t     type,
              char* const buffer,
              size_t      start_pos,
              size_t      size
            ): Token(
                type,
                buffer,
                size)
        {
            m_data_pos = start_pos;
        }

        Token(const Token& other)
            : m_type(other.m_type)
            , m_index(other.m_index)
            , m_buffer(other.m_buffer)
            , m_is_buffer_owned(false)
            , m_data_pos(other.m_data_pos)
            , m_prefix_len(other.m_prefix_len)
            , m_word_len(other.m_word_len)
            , m_suffix_len(other.m_suffix_len)
        {
            VERIFY(other.m_is_buffer_owned == false, "Can't create a Token from an owned const char*");
        }

        Token(Token&& other);

        inline explicit operator bool () const
        { return m_type != Token_t::none; }

        Token& operator=(const Token& other);
        Token& operator=(Token&&) noexcept;
        ~Token();;
        void clear();

        inline bool has_buffer()const
        { return m_buffer != nullptr; }

        // Convert token portion to string
        // Warning: this costs an allocation, use xxx_ptr() and xxx_size() when possible.

        std::string string() const;
        std::string prefix_to_string()const;
        std::string word_to_string()const;
        std::string suffix_to_string()const;

        // Get token portion addresses

        inline char* begin() const { return m_buffer + m_data_pos; } // token's start address
        inline char* prefix() const { return begin(); }
        inline char* word() const { return begin() + m_prefix_len; /* word pos is absolute */ };
        inline char* suffix() const { return word() + m_word_len; }
        inline char* end() const { return m_buffer + length(); } // token's end address (EXCLUDED)

        // Get token portion sizes

        inline size_t length() const { return m_prefix_len + m_word_len + m_suffix_len; }
        inline size_t prefix_len() const { return m_prefix_len; }
        inline size_t word_len() const { return m_word_len; }
        inline size_t suffix_len() const { return m_suffix_len; }

        inline bool is_keyword_type() { return ndbl::is_a_type_keyword(m_type); } // Check if whether this token is a keyword type
        void        take_prefix_suffix_from(Token *source); // Transfer the prefix and suffix of a given token to this token
        void        set_source_buffer(char* _buffer, size_t pos = 0, size_t size = 0);
        std::string json()const;
        bool        empty() const { return length() == 0; }
        void        word_replace(const char* new_word);
        void        suffix_append(const char* str);
        void        clear_suffix();
        void        clear_prefix();
        void        slide_word_begin(int amount);
        void        slide_word_end(int amount);
        void        reset_lengths(); // keep buffer and data position unchanged, but reset token to zero-length

        static const Token s_end_of_line;
        static const Token s_end_of_instruction;

    };
}
static_assert(std::is_move_assignable_v<ndbl::Token>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::Token>, "Should be move constructible");