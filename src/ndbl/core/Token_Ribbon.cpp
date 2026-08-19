#include "Token_Ribbon.h"

#include "bdc/String_Builder.hpp"
#include "tools/core/Log.h"
#include "tools/core/Asserts.h"

#include "Token_Type.h"
#include "Token.h"

using namespace ndbl;

Token & Token_Ribbon::push(Token &_token)
{
    _token.index = tokens.size();
    tokens.push_back(_token);
    return tokens.back();
}

bdc::String Token_Ribbon::to_string()const
{
    assert(false && "Convert code below:");
    // bdc::String out;
    // out.append(TOOLS_COLOR_DEFAULT);

    // size_t buffer_size = 0;

    // // get the total buffer sizes (but won't be exact, some token are serialized dynamically)
    // for (const Token& each_token : tokens)
    //     buffer_size += each_token.length();

    // out.append("Logging token ribbon state:\n");
    // out.append("___________[TOKEN RIBBON]_________\n");

    // for (const Token& token : tokens)
    // {
    //     tools::String_512 line;
        
    //     if ( token.m_index == 0 )
    //     {
    //         line.append("B"); // begin
    //     }
    //     else if ( token.m_index == tokens.back().m_index )
    //     {
    //         line.append("E"); // end
    //     }
    //     else
    //     {
    //         line.append("|"); // default
    //     }

    //     if ( !transaction.empty()
    //           && token.m_index >= transaction.top()
    //           && token.m_index <= cursor )
    //     {
    //         line.append("T"); // transaction
    //     }
    //     else
    //     {
    //         line.append("."); // no transaction
    //     }

    //     const bdc::String word = token.word_to_string();
    //     line.append_fmt("%5zu) \"%s\"", token.m_index, word.c_str() );
      
    //     if ( token.m_index == cursor )
    //     {
    //         line.append(" [c]"); // current
    //     }
        
    //     out.append(line.c_str());
    //     out.append("\n");
    //}

    //return out.c_str();
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
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Token_Ribbon", "Eat token (idx %i) %s \n", cursor, peek().string().c_str() );
    return tokens.at(cursor++);
}

void Token_Ribbon::start_transaction()
{
    transaction.push(cursor);
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Token_Ribbon", "Start Transaction (idx %i)\n", cursor);
}

void Token_Ribbon::rollback()
{
    cursor = transaction.top();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Token_Ribbon", "Rollback (idx %i)\n", cursor);
    transaction.pop();
}

void Token_Ribbon::commit()
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Token_Ribbon", "Commit (idx %i)\n", cursor);
    transaction.pop();
}

void Token_Ribbon::reset(bdc::String new_buffer)
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

bdc::String Token_Ribbon::range_to_string(size_t begin, size_t end) const
{
    ASSERT(begin <= end);
    ASSERT(end <= tokens.size() );

    bdc::String_Builder sb;
    for( size_t i = begin; i < end; ++i )
    {
        bdc::string_builder_append(sb, tokens[i].string() );
    }
    return bdc::string_builder_build_string(sb);
}
