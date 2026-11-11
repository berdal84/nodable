#include "Token.h"
#include "bdc/Allocators.hpp"
#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/Types.hpp"
#include "ndbl/Asserts.h"
#include <cstddef>
#include <cstring>

namespace ndbl
{
using namespace bdc;

void token_deinit(Token& token)
{
    if( token.owns_data )
    {
        memory_free(token.data);
    }
}

String token_serialize_json(const Token& token)
{
    using namespace bdc;

    String_Builder sb{};

    string_builder_append(sb, "{\n");
    string_builder_append(sb, string_tprintf("\ttype: %i,\n", token.type));
    string_builder_append(sb, string_tprintf("\tprefix_view: \"%s\",\n", token_prefix_view(token).c_str() ) );
    string_builder_append(sb, string_tprintf("\tword_view: \"%s\",\n", token_word_view(token).c_str() ) );
    string_builder_append(sb, string_tprintf("\tsuffix: \"%s\",\n", token_suffix_view(token).c_str() ) );
    string_builder_append(sb, " }");

    return string_builder_build_tstring(sb);
}

void token_release_buffer_if_owned(Token& token)
{
    if( token.owns_data )
    {
        memory_free(token.data);
        token.data      = nullptr;
        token.owns_data = false;
    }
}

void token_take_prefix_suffix_from(Token& token, Token& other)
{
    // transfer prefix and suffix to this token, but keep the same word.
    // this operation requires the buffer to be owned

    String_Builder sb{};

    // copy prefix from source
    string_builder_append(sb, token_prefix_view(other) );
    string_builder_append(sb, token_word_view(token) );
    string_builder_append(sb, token_suffix_view(other) );

    i8_t* new_data = string_builder_build_string(sb).data;

    token_release_buffer_if_owned(token);

    token.data      = new_data;
    token.owns_data = true;

    token.prefix_size = other.prefix_size;
    token.suffix_size = other.suffix_size;

    token_remove_suffix_and_prefix(other);
}

void token_remove_suffix_and_prefix(Token& token)
{
    NDBL_ASSERT(!token.owns_data, "Not possible when Token owns data, data ptr would be lost");
    token.data        += token.suffix_size;
    token.suffix_size  = 0;
    token.prefix_size  = 0; 
}

void token_clear(Token& token)
{
    token.type        = 0;
    token.prefix_size = 0;
    token.word_size   = 0;
    token.suffix_size = 0;
    
    token_release_buffer_if_owned(token);
}

u32_t token_char_position(const Token& token)
{
    NDBL_ASSERT(false, "This was previously returning the position of the token word on the global"
                  "parsed string, now it does not. We should change that.");
    return token.suffix_size;
}

void token_replace_buffer_readonly(Token& token, const String& buffer)
{
    // here, we consider that the whole buffer will be into the "word" part, no suffix/prefix.

    token_release_buffer_if_owned(token);

    token.data           = buffer.data;
    token.prefix_size    = 0;
    token.word_size      = buffer.size;
    token.suffix_size    = 0;
}

void token_replace_buffer_readwrite(Token& token, const String& buffer )
{
    // here, we consider that the whole buffer will be into the "word" part, no suffix/prefix.

    token_release_buffer_if_owned(token);

    token.data          = string_copy(buffer).data;
    token.owns_data     = true;
    token.prefix_size   = 0;
    token.word_size     = buffer.size;
    token.suffix_size   = 0;
}

void token_replace_word(Token& token, const String& new_word)
{
    // print a new buffer, and update the views
    i8_t* new_data = string_printf("%s%s%s"
                                  , token_prefix_view(token).c_str()
                                  , new_word.c_str()
                                  , token_suffix_view(token).c_str() ).data;

    token_release_buffer_if_owned(token);

    token.data            = new_data;
    token.owns_data       = true;
    // token.prefix_size  = ... (no change)
    token.word_size       = new_word.size;
    // token.suffix_size  = ... (no change)
}

void token_prefix_push_front(Token& token, const String& str)
{
    i8_t* new_data = string_printf("%s%s", str.c_str(), token_view(token).c_str() ).data;
    
    token_release_buffer_if_owned(token);

    token.data            = new_data;
    token.owns_data       = true;
    token.prefix_size    += str.size;
    // token.word_size    = ... (no change)
    // token.suffix_size  = ... (no change)
}

void token_suffix_push_back(Token& token, const String& str)
{
    i8_t* new_data = string_printf("%s%s", token_view(token).c_str(), str.c_str() ).data;
    
    token_release_buffer_if_owned(token);

    token.data            = new_data;
    token.owns_data       = true;
    // token.prefix_size  = ... (no change)
    // token.word_size    = ... (no change)
    token.suffix_size    += str.size;
}

void token_lextend_word(Token& token, size_t amount)
{
    NDBL_ASSERT(token.prefix_size >= amount);
    token.prefix_size -= amount;
    token.word_size   += amount;
}

void token_rextend_word(Token& token, size_t amount)
{
    NDBL_ASSERT(token.suffix_size >= amount);
    token.word_size   += amount;
    token.suffix_size -= amount;
}

void token_set_offset(Token& token, size_t pos)
{
    NDBL_TODO("Not implemented yet:");
    // TODO: update views
}

void token_lextend_prefix(Token& token, size_t l_amount)
{
    NDBL_ASSERT(!token.owns_data, "Only allowed when token does not owns the buffer");
    token.data        -= l_amount;
    token.prefix_size += l_amount;
}

void token_rextend_suffix(Token& token, size_t r_amount)
{
    token.suffix_size += r_amount;
}

void token_rtrim_word(Token& token, size_t l_amount)
{
    NDBL_ASSERT( token.word_size >= l_amount );
    token.word_size   -= l_amount;
    token.suffix_size += l_amount;
}

void token_ltrim_word(Token& token, size_t r_amount)
{
    token.prefix_size += r_amount;
    token.word_size    = token.word_size < r_amount ? 0 : token.word_size - r_amount;
}

} // namespace ndbl
