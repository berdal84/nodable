#pragma once

#include <cstddef> //.......... for size_t

#include "bdc/Types.hpp"
#include "bdc/String.hpp"
#include "Asserts.h"
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
        Token(
            Token_Type  type,
            bdc::String buffer
        )
        : type(type)
        , data(buffer.data)
        , prefix_size(0)
        , word_size(buffer.size)
        , suffix_size(0)
        , owns_data(false)
        {}

        Token(const Token& other) = default;
    };

    static_assert( std::is_default_constructible_v<Token> );

    void                token_deinit(Token&);
    void                token_release_buffer_if_owned(Token& token);
    inline u32_t        token_size(const Token& token)            { return token.prefix_size + token.word_size + token.suffix_size; }  
    inline bdc::String  token_view(const Token& token)            { return { token.data, token_size(token) }; }
    inline bdc::String  token_prefix_view(const Token& token)     { return { token.data, token.prefix_size }; }
    inline bdc::String  token_word_view(const Token& token)       { return { token.data + token.prefix_size, token.word_size }; }
    inline bdc::String  token_suffix_view(const Token& token)     { return { token.data + token.prefix_size + token.word_size, token.suffix_size }; }
    inline bool         token_empty(const Token& token)           { return token_size(token) == 0; }
    void                token_clear(Token&);
    void                token_set_offset(Token&, size_t new_offset);
    void                token_lextend_prefix(Token&, size_t l_amount);
    void                token_rextend_suffix(Token&, size_t r_amount); 
    void                token_lextend_word(Token&, size_t l_amount);
    void                token_rextend_word(Token&, size_t r_amount);
    void                token_ltrim_word(Token&, size_t r_amount);
    void                token_rtrim_word(Token&, size_t l_amount);
    void                token_take_prefix_suffix_from(Token&, Token& source); // Transfer the prefix and suffix of a given token to this token
    void                token_remove_suffix_and_prefix(Token&);
    void                token_suffix_push_back(Token&, const bdc::String&);
    void                token_prefix_push_front(Token&, const bdc::String&);
    void                token_replace_buffer_readonly(Token&, const bdc::String& buffer);
    void                token_replace_buffer_readwrite(Token&, const bdc::String& buffer);
    void                token_replace_word(Token&, const bdc::String&);
    u32_t               token_char_position(const Token&);
    bdc::String         token_serialize_json(const Token&);

    static const Token token_end_of_line        = { Token_Type_end_of_line       , "\n" };
    static const Token token_end_of_instruction = { Token_Type_end_of_instruction, ";"  };


}