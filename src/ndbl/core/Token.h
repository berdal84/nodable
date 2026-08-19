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
     * A token is a view over a portion of parsed string.
     *
     * The text viewed by a Token can be split in 3 parts:
     * - prefix
     * - word
     * - suffix
     *
     * For example, if we parsed the text "   my_var ", those part will contain:
     * - prefix: "   "
     * - word:   "my_var"
     * - suffix: " "
     *
     * The data is in buffer, it can be owner or not. prfix, word, and suffix are views over this data.
     * When user edits something, the buffer portion that is pointed by prefix/word/suffix gets duplicated
     */
	struct Token
	{
        size_t      index;       // in parent Token_Ribbon
        Token_Type  type;
        
        bdc::String buffer; // original source code (not owned), might be replaced by custom data in case used edits a value.

        bdc::String prefix_view;
        bdc::String word_view;
        bdc::String suffix_view;

        Token() = default;
        Token(Token_Type type);
        Token(Token_Type type, const bdc::String& buffer);
        Token(Token_Type type, const bdc::String& buffer, const bdc::String& word);
        Token(const Token& other) = default;
        Token(Token&& other) = default;

        explicit    operator bool () const;
        Token&      operator=(const Token& other);
        Token&      operator=(Token&&) = default;

        ~Token() = default;

        void        clear();
        bool        has_buffer()const { return !buffer.empty(); }; 
        bdc::String string() const { return bdc::string_view(buffer); }
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
        bool        empty() const { return buffer.empty(); }
        void        suffix_push_back(const bdc::String&);
        void        prefix_push_front(const bdc::String&);
        void        replace_buffer(const bdc::String& buffer, bool external_only = false);
        void        replace_word(const bdc::String&);
        u32_t       char_position() const;
        bool        owns_buffer() const;

        static const Token s_end_of_line;
        static const Token s_end_of_instruction;
    };

    inline Token::Token(Token_Type type)
    : type( type )
    {}


    inline Token::Token(
        Token_Type            type,
        const bdc::String&    buffer
    )
    : type(type)
    , buffer(buffer)
    , word_view(buffer)
    {
    }

    inline Token::Token(
        Token_Type            type,
        const bdc::String&    buffer,
        const bdc::String&    word
    )
    : type(type)
    , buffer(buffer)
    , word_view(word)
    {
        assert((u64_t)word.data <= ((u64_t)buffer.data + buffer.size) && "word starts after buffer's end!");
        assert((u64_t)word.data + word.size <= ((u64_t)buffer.data + buffer.size) && "word ends after buffer's end!");
    }

    inline Token::operator bool () const
    {
        return type != Token_Type_NULL;
    }
}