#pragma once

#include <cstddef>
#include <string>
#include <cstring>
#include <string_view>

#include "bdc/Types.hpp"
#include "tools/core/Asserts.h"
#include "Token_Type.h"

namespace ndbl
{
    /**
     * What is a Token?
     *
     * - by default, it is a view over a given buffer split in 3 parts (prefix, word, suffix)
     * - Token::length() is ALWAYS equals to prefix_size + word_size + suffix_size
     * - Token::prefix(), ::word(), and ::suffix() ALWAYS return a view over the buffer.
     */
	struct Token
	{
        size_t      index; // in parent Token_Ribbon
        Token_Type  type;
        i8_t*       data; // might be owned or not, check owns_data flag.
        bool        owns_data;        
        u32_t       prefix_size;
        u32_t       word_size;
        u32_t       suffix_size;

        Token(): Token(Token_Type_NULL) {}
        Token(Token_Type type): Token(type, "") {}
        Token(Token_Type type, bdc::String buffer);
        Token(const Token& other) { *this = other; };

        ~Token() = default;

        explicit    operator bool () const { return type != Token_Type_NULL; }
        Token&      operator=(const Token&);

        bdc::String view() const            { return { data, size()}; }
        bdc::String prefix_view() const     { return { data, prefix_size }; }
        bdc::String word_view() const       { return { data + prefix_size, word_size }; }
        bdc::String suffix_view() const     { return { data + prefix_size + word_size, suffix_size }; }
        u32_t       size() const            { return prefix_size + word_size + suffix_size; }  
        bool        empty() const           { return size() == 0; }
        void        clear();
        void        set_offset(size_t new_offset);

        void        prefix_reset(size_t size = 0);      // preserves word
        void        prefix_begin_grow(size_t l_amount); // ...
        void        prefix_end_grow(size_t r_amount);   // ...

        void        suffix_reset(size_t size = 0);      // preserves word
        void        suffix_end_grow(size_t r_amount);   // ...
        void        suffix_begin_grow(size_t l_amount); // ...

        void        word_move_begin(int amount);
        void        word_move_end(int amount);

        void        reset_lengths(); // buffer and offset won't change
        bool        is_keyword_type() { return ndbl::is_a_type_keyword(type); } // Check if whether this token is a keyword type
        void        take_prefix_suffix_from(Token *source); // Transfer the prefix and suffix of a given token to this token
        bdc::String json()const;
        void        suffix_push_back(const bdc::String&);
        void        prefix_push_front(const bdc::String&);
        void        replace_buffer(const bdc::String& buffer, bool external_only = false);
        void        replace_word(const bdc::String&);
        u32_t       char_position() const;

        static const Token s_end_of_line;
        static const Token s_end_of_instruction;
    };

    static_assert( std::is_default_constructible_v<Token> );
}