#include "Token_Ribbon.h"

#include "bdc/String_Builder.hpp"
#include "tools/core/Log.h"
#include "tools/core/Asserts.h"

#include "Token_Type.h"
#include "Token.h"

namespace ndbl
{

using namespace bdc;
using namespace tools;

Token & Token_Ribbon::push(Token &_token)
{
    _token.index = tokens.size();
    tokens.push_back(_token);
    return tokens.back();
}

String Token_Ribbon::to_string() const
{
    String_Builder sb;
    string_builder_init(sb);
    string_builder_append(sb, TOOLS_COLOR_DEFAULT);

    string_builder_append(sb, "Logging token ribbon state:\n");
    string_builder_append(sb, "___________[TOKEN RIBBON]_________\n");

    for (const Token& token : tokens)
    {
        if ( token.index == 0 )
        {
            string_builder_append(sb, "B"); // begin
        }
        else if ( token.index == tokens.back().index )
        {
            string_builder_append(sb, "E"); // end
        }
        else
        {
            string_builder_append(sb, "|"); // default
        }

        if ( !transaction.empty()
              && token.index >= transaction.top()
              && token.index <= cursor )
        {
            string_builder_append(sb, "T"); // transaction
        }
        else
        {
            string_builder_append(sb, "."); // no transaction
        }

        string_builder_appendf(sb, "%5zu) \"%s\"", token.index, token.word_view().c_str() );
      
        if ( token.index == cursor )
        {
            string_builder_append(sb, " [c]"); // current
        }
        
        string_builder_append(sb, "\n");
    }

    return string_builder_build_string(sb).c_str();
}

Token Token_Ribbon::eat_if(Token_Type expectedType)
{
    if (can_eat() && peek().type == expectedType )
    {
        return eat();
    }
    return Token_Type_NULL;
}

Token Token_Ribbon::eat()
{
    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Token_Ribbon", "Eat token (idx %i) %s \n", cursor, peek().view().c_str() );
    return tokens.at(cursor++);
}

void Token_Ribbon::start_transaction()
{
    transaction.push(cursor);
    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Token_Ribbon", "Start Transaction (idx %i)\n", cursor);
}

void Token_Ribbon::rollback()
{
    cursor = transaction.top();
    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Token_Ribbon", "Rollback (idx %i)\n", cursor);
    transaction.pop();
}

void Token_Ribbon::commit()
{
    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Token_Ribbon", "Commit (idx %i)\n", cursor);
    transaction.pop();
}

void Token_Ribbon::reset(String new_buffer)
{
    tokens.clear();

    global_token.replace_buffer( new_buffer, true ); // wraps all

    while(!transaction.empty())
        transaction.pop();

    cursor = 0;
}

bool Token_Ribbon::can_eat(size_t count) const
{
    ASSERT(count > 0);
    return cursor + count <= tokens.size() ;
}

String Token_Ribbon::range_to_string(size_t begin, size_t end) const
{
    ASSERT(begin <= end);
    ASSERT(end <= tokens.size() );

    String_Builder sb;
    for( size_t i = begin; i < end; ++i )
    {
        string_builder_append(sb, tokens[i].view() );
    }
    return string_builder_build_string(sb);
}

} // namespace ndbl