#include "Token.h"
#include "bdc/Allocators.hpp"
#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/Types.hpp"
#include <cassert>
#include <cstddef>
#include <cstring>

namespace ndbl
{
using namespace bdc;
using namespace tools;

const Token Token::s_end_of_line        = {Token_Type_ignore, "\n"};
const Token Token::s_end_of_instruction = {Token_Type_ignore, ";\n"};

Token::Token(
    Token_Type  _type,
    String      _buffer
)
: type(_type)
, data(_buffer.data)
, prefix_size(0)
, word_size(_buffer.size)
, suffix_size(0)
, owns_data(false)
{}

String Token::json() const
{
    using namespace bdc;

    String_Builder sb;

    string_builder_append(sb, "{\n");

    assert(false && "TODO: implement push_allocator(Allocator*) (with auto pop and scope end)");
    string_builder_append(sb, string_printf(temp_allocator(), "\ttype: %i,\n", type));
    string_builder_append(sb, string_printf(temp_allocator(),"\tprefix_view: \"%s\",\n", prefix_view().c_str() ) );
    string_builder_append(sb, string_printf(temp_allocator(),"\tword_view: \"%s\",\n", word_view().c_str() ) );
    string_builder_append(sb, string_printf(temp_allocator(),"\tsuffix: \"%s\",\n", suffix_view().c_str() ) );

    string_builder_append(sb, " }");

    return string_builder_build_string(sb);
}

void Token::take_prefix_suffix_from(Token* source)
{
    // transfer prefix and suffix to this token, but keep the same word.
    // this operation requires the buffer to be owned

    String_Builder sb;
    string_builder_init(sb);

    // copy prefix from source
    string_builder_append(sb, source->prefix_view() );
    string_builder_append(sb, word_view() );
    string_builder_append(sb, source->suffix_view() );

    i8_t* new_data = string_builder_build_string(sb, heap_allocator()).data;

    if( owns_data )
    {
        memory_free(data);
    }

    data      = new_data;
    owns_data = true;

    prefix_size = source->prefix_size;
    suffix_size = source->suffix_size;

    // Remove prefix and suffix on the source
    source->suffix_reset();
    source->prefix_reset();
}

void Token::clear()
{
    index       = 0;
    type        = 0;
    prefix_size = 0;
    word_size   = 0;
    suffix_size = 0;
}

u32_t Token::char_position() const
{
    #warning This was previously returning the position of the token word on the global parsed string, now it does not. We should change that.
    return suffix_size;
}

Token& Token::operator=(const Token& other)
{
    if( this == &other) return *this;

    index       = other.index;
    prefix_size = other.prefix_size;
    word_size   = other.word_size;
    suffix_size = other.suffix_size;
    type        = other.type;
    data        = other.data;
    owns_data   = other.owns_data; // That is user's responsibility to release the data once

    return *this;
}

void Token::replace_buffer(const String& _buffer, bool external_only )
{
    // here, we consider that the whole buffer will be into the "word" part, no suffix/prefix.

    if( owns_data )
    {
        memory_free(data);
        owns_data = false;
    }

    if( external_only )
    {
        this->data = _buffer.data;
    }
    else
    {
        this->data = string_copy(_buffer).data;
        owns_data = true;
    }

    prefix_size    = 0;
    word_size      = _buffer.size;
    suffix_size    = 0;
}

void Token::replace_word(const String& new_word)
{
    // print a new buffer, and update the views
    i8_t* new_data = string_printf("%s%s%s", prefix_view().c_str(), new_word.c_str(), suffix_view().c_str() ).data;

    if( owns_data )
    {
        memory_free(data);
    }

    data            = new_data;
    owns_data       = true;
    // prefix_size  = (no change)
    word_size       = new_word.size;
    // suffix_size  = (no change)
}

void Token::prefix_push_front(const String& str)
{
    i8_t* new_data = string_printf("%s%s", str.c_str(), view().c_str() ).data;
    if ( owns_data )
    {
        // Currently we do not allocate more that needed, so when we resize we must release our buffer
        memory_free(data);
    }
    data            = new_data;
    owns_data       = true;
    prefix_size    += str.size;
    // word_size    = (no change)
    // suffix_size  = (no change)
}

void Token::suffix_push_back(const String& str)
{
    i8_t* new_data = string_printf("%s%s", str.c_str(), view().c_str() ).data;
    if ( owns_data )
    {
        // Currently we do not allocate more that needed, so when we resize we must release our buffer
        memory_free(data);
    }
    data            = new_data;
    owns_data       = true;
    // prefix_size  = (no change)
    // word_size    = (no change)
    suffix_size    += str.size;
}

void Token::prefix_reset(size_t new_size )
{
    assert(!owns_data);
    prefix_size  = new_size;
}

void Token::reset_lengths()
{
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::word_move_begin(int amount)
{
    if( amount < 0) assert(prefix_size >= -amount);
    prefix_size += amount;
    word_size   -= amount;
}

void Token::word_move_end(int amount)
{
    if( amount > 0) assert(suffix_size >= amount);
    word_size   += amount;
    suffix_size -= amount;
}

void Token::set_offset(size_t pos)
{
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::suffix_reset(size_t size)
{
    assert(!owns_data);
    suffix_size = size;
}

void Token::prefix_begin_grow(size_t l_amount)
{
    assert(!owns_data && "Only allowed when token does not owns the buffer");
    data        -= l_amount;
    prefix_size += l_amount;
}

void Token::suffix_end_grow(size_t size)
{
    suffix_reset(suffix_size + size);
}

void Token::suffix_begin_grow(size_t l_amount)
{
    ASSERT( word_size >= l_amount );
    word_size   -= l_amount;
    suffix_size += l_amount;
}

void Token::prefix_end_grow(size_t r_amount)
{
    ASSERT( r_amount <= word_size);
    prefix_size += r_amount;
    word_size   -= r_amount;
}

} // namespace ndbl
