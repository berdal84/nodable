#pragma once

#include <cstddef>
#include <string>
#include <cstring>
#include <string_view>

#include "bdc/Types.hpp"
#include "ndbl/core/Asserts.h"
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
        void        lextend_prefix(size_t l_amount);
        void        rextend_suffix(size_t r_amount); 
        void        lextend_word(size_t l_amount);
        void        rextend_word(size_t r_amount);
        void        ltrim_word(size_t r_amount);
        void        rtrim_word(size_t l_amount);
        bool        is_keyword_type() { return ndbl::is_a_type_keyword(type); } // Check if whether this token is a keyword type
        void        take_prefix_suffix_from(Token *source); // Transfer the prefix and suffix of a given token to this token
        void        remove_suffix_and_prefix();
        void        suffix_push_back(const bdc::String&);
        void        prefix_push_front(const bdc::String&);
        void        replace_buffer(const bdc::String& buffer, bool external_only = false);
        void        replace_word(const bdc::String&);
        u32_t       char_position() const;
        bdc::String json()const;

        static const Token s_end_of_line;
        static const Token s_end_of_instruction;
    };

    static_assert( std::is_default_constructible_v<Token> );
}