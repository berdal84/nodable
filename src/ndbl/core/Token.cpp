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
, buffer(_buffer)
, word_view(_buffer)
, prefix_view( string_lsplit(_buffer, 0))
, suffix_view( string_rsplit(_buffer, _buffer.size))
, owns_buffer(false)
{
    assert((u64_t)word_view.data <= ((u64_t)buffer.data + buffer.size) && "word starts after buffer's end!");
    assert((u64_t)word_view.data + word_view.size <= ((u64_t)buffer.data + buffer.size) && "word ends after buffer's end!");
}

String Token::json() const
{
    using namespace bdc;

    String_Builder sb;

    string_builder_append(sb, "{\n");

    assert(false && "TODO: implement push_allocator(Allocator*) (with auto pop and scope end)");
    string_builder_append(sb, string_printf(temp_allocator(), "\ttype: %i,\n", type));
    string_builder_append(sb, string_printf(temp_allocator(),"\tprefix_view: \"%s\",\n", prefix_view.c_str() ) );
    string_builder_append(sb, string_printf(temp_allocator(),"\tword_view: \"%s\",\n", word_view.c_str() ) );
    string_builder_append(sb, string_printf(temp_allocator(),"\tsuffix: \"%s\",\n", suffix_view.c_str() ) );

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
    if( !source->prefix_view.empty() )
    {
        string_builder_append(sb, source->prefix_view);
    }

    // reassign word
    if( !word_view.empty() )
    {
        string_builder_append(sb, word_view );
    }

    // copy suffix from source
    if( !source->suffix_view.empty() )
    {
        string_builder_append(sb, source->suffix_view );
    }

    String new_buffer = string_builder_build_string(sb, heap_allocator());

    if( owns_buffer )
    {
        string_release(buffer);
    }

    buffer      = new_buffer;
    owns_buffer = true;

    prefix_view.data = buffer.data;
    prefix_view.size = source->prefix_view.size;
    word_view.data   = buffer.data + source->prefix_view.size;
    // word_view.size   = ... unchanged
    suffix_view.data = buffer.data + source->prefix_view.size + source->word_view.size;
    suffix_view.size = source->suffix_view.size;

    // Remove prefix and suffix on the source
    source->suffix_reset();
    source->prefix_reset();
}

void Token::clear()
{
    index       = 0;
    type        = 0;
    prefix_view = "";
    word_view   = "";
    suffix_view = "";

    assert(false && "TODO: free existing buffer if owned");
}

u32_t Token::char_position() const
{
    u64_t offset = (u64_t)word_view.data - (u64_t)buffer.data;
    assert(offset < String::invalid_pos && "Need to use a larger string, buffer goes beyond String::size!");
    return (u32_t)offset;
}

Token& Token::operator=(const Token& other)
{
    if( this == &other) return *this;

    index       = other.index;
    prefix_view = other.prefix_view;
    word_view   = other.word_view;
    suffix_view = other.suffix_view;
    type        = other.type;
    buffer      = other.buffer;
    owns_buffer = false;

    return *this;
}

void Token::replace_buffer(const String& _buffer, bool external_only )
{
    // here, we consider that the whole buffer will be into the "word" part, no suffix/prefix.

    if( owns_buffer )
    {
        string_release( buffer );
        owns_buffer = false;
    }

    if( external_only )
    {
        this->buffer = _buffer;
    }
    else
    {
        this->buffer = string_copy(_buffer);
        owns_buffer = true;
    }

    prefix_view.data    = buffer.data;
    prefix_view.size    = 0;
    word_view.data      = buffer.data + prefix_view.size;
    word_view.size      = buffer.size;
    suffix_view.data    = buffer.data + word_view.size;
    suffix_view.size    = 0;
}

void Token::replace_word(const String& new_word)
{
    // print a new buffer, and update the views
    String new_buffer = string_printf("%s%s%s", prefix_view.c_str(), new_word.c_str(), suffix_view.c_str() );

    if( owns_buffer )
    {
        string_release(buffer);
    }

    buffer = new_buffer;
    owns_buffer = true;

    prefix_view.data    = buffer.data;
    // prefix_view.size    = ... no change

    word_view.data      = buffer.data + prefix_view.size;
    word_view.size      = new_word.size;

    suffix_view.data    = buffer.data + prefix_view.size + word_view.size;
    // suffix_view.size    = ... no change
}

void Token::prefix_push_front(const String& str)
{
    if ( !owns_buffer )
    {
        assert(false && "Not implemented yet:");
        // TODO: create a new buffer
    }
    
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::suffix_push_back(const String& str)
{
    if ( owns_buffer )
    {
        // Currently we do not allocate more that needed, so when we resize we must release our buffer
        string_release(buffer);
    }
    
    buffer      = string_printf("%.*s%.*s", buffer.size, buffer.data, str.size, str.data );
    owns_buffer = true;

    // make sure views points to the new buffer address
    prefix_view.data  = buffer.data;
    word_view.data    = buffer.data + prefix_view.size;
    suffix_view.data  = buffer.data + prefix_view.size + word_view.size;
    suffix_view.size += str.size;
}

void Token::prefix_reset(size_t new_size )
{
    assert(!owns_buffer);

    int diff = (int)new_size - (int)prefix_view.size;
    // Instead of erasing chars, we prefer to simply "move the cursor to the right"
    buffer.data      += diff;
    prefix_view.data += diff;
    prefix_view.size  = new_size;
}

void Token::reset_lengths()
{
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::word_move_begin(int amount)
{
    #warning TODO: add bound checks
    prefix_view.size += amount;
    word_view.data   += amount;
    word_view.size   -= amount;
}

void Token::word_move_end(int amount)
{
    #warning TODO: add bound checks
    word_view.size   += amount;
    suffix_view.data += amount;
    suffix_view.size -= amount;
}

void Token::set_offset(size_t pos)
{
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::suffix_reset(size_t size)
{
    assert(!owns_buffer);
    suffix_view.size = size;
    buffer.size      = prefix_view.size + word_view.size + suffix_view.size;
}

void Token::prefix_begin_grow(size_t l_amount)
{
    assert(!owns_buffer && "Only allowed when token does not owns the buffer");
    buffer.data      -= l_amount;
    buffer.size      += l_amount;
    prefix_view.data -= l_amount;
    prefix_view.size += l_amount;
}

void Token::suffix_end_grow(size_t size)
{
    suffix_reset(suffix_view.size + size);
}

void Token::suffix_begin_grow(size_t l_amount)
{
    assert(false && "Not implemented yet:");
    // TODO: update views

    // ASSERT( word_size >= l_amount );
    // word_size   -= l_amount;
    // suffix_size += l_amount;
}

void Token::prefix_end_grow(size_t r_amount)
{
    assert(false && "Not implemented yet:");
    // TODO: update views

    // ASSERT( r_amount <= word_size);
    // prefix_size += r_amount;
    // word_size   -= r_amount;
}

} // namespace ndbl
