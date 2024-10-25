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
     * Data can can be external or internal:
     * Token's buffer can be either external or internal, when the string is modified (ex: by pushing chars in the suffix),
     * the Token switch into external mode to avoid writing on non owned memory. This allows to work with external const char*.
     * Ideally, all the Token from a given parsing should share the same buffer until user modify something.
     */
	struct Token
	{
        size_t  m_index = 0; // in the TokenRibbon
        Token_t m_type;

        struct BimodalBuffer
        {
            union {
                char*        extern_buf;
                std::string* intern_buf;
            } /* data */;
            size_t offset; // to offset token start from data's address
            bool   intern;

            ~BimodalBuffer();
            void         delete_intern_buf();
            void         switch_to_intern_buf(size_t size);
            void         switch_to_intern_buf_with_data(char *data, size_t len);
            inline char* data()  const { return intern ? intern_buf->data() : extern_buf; }

            inline char* begin() const { return data() + offset; }
        } m_buffer = {nullptr, 0, false }; // external nullptr

        size_t      m_prefix_len       = 0;
        size_t      m_word_len         = 0;
        size_t      m_suffix_len       = 0;

        explicit Token()
        : m_type(Token_t::none)
        {}

        Token(Token_t type)
        : m_type( type )
        {}

        Token(Token_t type, const std::string& word )
        : m_type( type )
        {
            word_replace( word.c_str() );
        }

        Token(Token_t type, const char* const word)
        : m_type(type)
        {
            m_buffer.extern_buf = const_cast<char*>(word),
            m_word_len        = strlen(word);
        }

        Token(Token_t type,
              char*   buffer,
              size_t  size)
        : m_type(type)
        , m_word_len(size)
        , m_buffer({ const_cast<char*>(buffer), 0, false}) // external
        {
        }

        Token(Token_t     type,
              char* const buffer,
              size_t      offset,
              size_t      size)
        : m_type(type)
        , m_word_len(size)
        , m_buffer({const_cast<char*>(buffer), offset, false}) // external
        {
        }

        Token(const Token& other)
            : m_type(other.m_type)
            , m_index(other.m_index)
            , m_prefix_len(other.m_prefix_len)
            , m_word_len(other.m_word_len)
            , m_suffix_len(other.m_suffix_len)
            , m_buffer(other.m_buffer)
        {
            VERIFY(!other.m_buffer.intern, "Can't create a Token from an owned const char*");
        }

        Token(Token&& other);

        inline explicit operator bool () const
        { return m_type != Token_t::none; }

        Token& operator=(const Token& other);
        Token& operator=(Token&&) noexcept;
        ~Token();

        void        clear();
        inline bool has_buffer()const { return m_buffer.data() != nullptr; }

        // Convert token portion to string
        // Warning: this costs an allocation, use xxx_ptr() and xxx_size() when possible.

        std::string string() const;
        std::string prefix_to_string()const;
        std::string word_to_string()const;
        std::string suffix_to_string()const;

        // Get offset/positions

        inline size_t offset() const { return m_buffer.offset; }

        // Get token portion addresses

        inline char* begin() const { return m_buffer.begin(); } // token's start address
        inline char* prefix() const { return begin(); }
        inline char* word() const { return begin() + m_prefix_len; /* word pos is absolute */ };
        inline char* suffix() const { return word() + m_word_len; }
        inline char* end() const { return begin() + length(); } // token's end address (EXCLUDED)

        // Get token portion sizes

        inline size_t length() const { return m_prefix_len + m_word_len + m_suffix_len; }
        inline size_t prefix_len() const { return m_prefix_len; }
        inline size_t word_len() const { return m_word_len; }
        inline size_t suffix_len() const { return m_suffix_len; }

        inline bool is_keyword_type() { return ndbl::is_a_type_keyword(m_type); } // Check if whether this token is a keyword type
        void        take_prefix_suffix_from(Token *source); // Transfer the prefix and suffix of a given token to this token
        void        set_external_buffer(char* buffer, size_t offset = 0, size_t size = 0);
        std::string json()const;
        bool        empty() const { return length() == 0; }
        void        word_replace(const char* new_word);
        void        suffix_push_back(const char* str);
        void        clear_suffix();
        void        clear_prefix();
        void        slide_word_begin(int amount);
        void        slide_word_end(int amount);
        void        reset_lengths(); // keep buffer and data position unchanged, but reset token to zero-length
        void        prefix_push_front(const char *str);
        void        set_offset(size_t pos);
        void        extend_prefix(size_t size);
        void        extend_suffix(size_t size);
        void        resize_suffix(size_t i);

        static const Token s_end_of_line;
        static const Token s_end_of_instruction;
    };
}
static_assert(std::is_move_assignable_v<ndbl::Token>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::Token>, "Should be move constructible");