#include "Token.h"
#include "bdc/Allocators.hpp"
#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/Types.hpp"
#include <cassert>
#include <cstddef>
#include <cstring>

using namespace ndbl;

const Token Token::s_end_of_line        = {Token_Type_ignore, "\n"};
const Token Token::s_end_of_instruction = {Token_Type_ignore, ";\n"};

bdc::String Token::json() const
{
    using namespace bdc;

    String_Builder sb;

    string_builder_append(sb, "{\n");

    assert(false && "TODO: implement bdc::push_allocator(Allocator*) (with auto pop and scope end)");
    string_builder_append(sb, string_printf(temp_allocator(), "\ttype: %i,\n", type));
    string_builder_append(sb, string_printf(temp_allocator(),"\tprefix_view: \"%s\",\n", prefix_view.c_str() ) );
    string_builder_append(sb, string_printf(temp_allocator(),"\tword_view: \"%s\",\n", word_view.c_str() ) );
    string_builder_append(sb, string_printf(temp_allocator(),"\tsuffix: \"%s\",\n", suffix_view.c_str() ) );

    string_builder_append(sb, " }");

    return string_builder_build_string(sb);
}

void Token::take_prefix_suffix_from(Token* source)
{
    bdc::String word_copy = string_copy( word_view, bdc::temp_allocator() );

    // transfer prefix and suffix to this token, but keep the same word.
    // this operation requires the buffer to be owned

    prefix_view       = source->prefix_view;
    // word_view      = unchanged
    suffix_view       = source->suffix_view;

    bdc::String_Builder sb{};

    // copy prefix from source
    if( !source->prefix_view.empty() )
    {
        bdc::string_builder_append(sb, source->prefix_view);
    }

    // reassign word
    if( !word_copy.empty() )
    {
        bdc::string_builder_append(sb, word_copy );
    }

    // copy suffix from source
    if( !source->suffix_view.empty() )
    {
        bdc::string_builder_append(sb, source->suffix_view );
    }

    assert(false && "TODO: free existing buffer if owned");
    buffer = bdc::string_builder_build_string(sb, bdc::heap_allocator());

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
    assert(offset < bdc::String::invalid_pos && "Need to use a larger string, buffer goes beyond String::size!");
    return (u32_t)offset;
}

bool Token::owns_buffer() const
{
    assert(false && "IMpmlement this. Should we rely on bdc::String::flags? or not?");
}

Token& Token::operator=(const Token& other)
{
    if( this == &other) return *this;

    assert(false && "TODO: free existing buffer if owned");

    index       = other.index;
    prefix_view = other.prefix_view;
    word_view   = other.word_view;
    suffix_view = other.suffix_view;
    type        = other.type;

    if( !other.owns_buffer() )
    {
        buffer = bdc::string_copy(other.buffer);
    }
    else
    {
        buffer = other.buffer;
    }

    return *this;
}

void Token::replace_buffer(const bdc::String& _buffer, bool external_only )
{
    // here, we consider that the whole buffer will be into the "word" part, no suffix/prefix.

    if( owns_buffer() )
    {
        bdc::heap_allocator()->proc_free( this->buffer.data );
    }

    if( external_only )
    {
        this->buffer = _buffer;
    }

    prefix_view         = this->buffer;
    prefix_view.size    = 0;

    word_view           = this->buffer;

    suffix_view         = this->word_view;
    bdc::string_advance(suffix_view, word_view.size);
}

void Token::replace_word(const bdc::String& new_word)
{
    if( new_word.size == 0 )
        return;

    bdc::String prefix_copy = bdc::string_copy(prefix_view, bdc::temp_allocator() );
    bdc::String suffix_copy = bdc::string_copy(suffix_view, bdc::temp_allocator() );

   
    bdc::String new_buffer = bdc::string_printf("%s%s%s", prefix_copy.c_str(), new_word.c_str(), suffix_copy.c_str() );

    assert(false && "Not implemented yet:");
    // heap_allocator()->proc_free( buffer.data )
    buffer = new_buffer;

    // prefix_view   = no change
    word_view        = new_word;
    // suffix_view   = no change
}

void Token::prefix_push_front(const bdc::String& str)
{
    if ( !owns_buffer() )
    {
        assert(false && "Not implemented yet:");
        // TODO: create a new buffer
    }
    
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::suffix_push_back(const bdc::String& str)
{
    if ( !owns_buffer() )
    {
        assert(false && "Not implemented yet:");
        // TODO: create a new buffer
    }
    
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::prefix_reset(size_t size )
{
    // Instead of erasing chars, we prefer to simply "move the cursor to the right"
    buffer.size += prefix_view.size - size;
    prefix_view.size = size;
}

void Token::reset_lengths()
{
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::word_move_begin(int amount)
{
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::word_move_end(int amount)
{
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::set_offset(size_t pos)
{
    assert(false && "Not implemented yet:");
    // TODO: update views
}

void Token::suffix_reset(size_t size)
{
    suffix_view.size = size;
}

void Token::prefix_begin_grow(size_t l_amount)
{
    assert(false && "Not implemented yet:");
    // TODO: update views
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
