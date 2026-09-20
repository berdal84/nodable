#pragma once

#include <string>
#include <vector>
#include <stack>

#include "Token.h"

namespace ndbl
{
    // This class wraps a container to store a list of Token.
    // An internal cursor points to a current token, cursor can be moved by calling eat or eat_if.
    // A transaction system allows to commit or rollback a sequence of eating.
    //
    // TODO: convert this class to a POD struct
    class Token_Ribbon
    {
    public:

        using Iterator = std::vector<Token>::iterator;
        using Const_Iterator = std::vector<Token>::const_iterator;

        size_t              cursor; // current token index
        Token               global_token; // wraps the whole buffer
        std::vector<Token>  tokens;
        std::stack<size_t>  transaction; // transaction start indexes

        Token_Ribbon()
        : cursor(0)
        , global_token(Token_Type_ignore)
        {}

        void                reset(bdc::String = {});
        Token&              operator[](size_t pos) { return tokens.at(pos); }
        const Token&        operator[](size_t pos) const { return tokens.at(pos); }
        Token&              at(size_t index) { return tokens.at(index); }
        Token&              back() { return tokens.back(); };        
        Iterator            begin() { return tokens.begin(); };
        Iterator            end() { return tokens.end(); };
        Const_Iterator      cbegin() const { return tokens.cbegin(); };
        Const_Iterator      cend() const  { return tokens.cend(); };
        bool                can_eat(size_t count = 1)const;
        bdc::String         range_to_string(size_t begin, size_t end) const; // Format ribbon from range [begin, end-1]
        Token               eat();           // Return the next token and increment cursor
        Token               eat_if(Token_Type); // Only if next token has a given type: returns it and increment cursor
        bool                empty()const { return tokens.empty(); }
        const Token&        get_eaten()const { ASSERT(cursor > 0); return tokens[cursor - 1];}
        bool                peek(Token_Type t) const { return cursor < tokens.size() && tokens[cursor].type == t; }
        const Token&        peek()const { return tokens[cursor]; }
        Token&              push(Token&);
        size_t              size()const { return tokens.size(); }
        bdc::String         to_string() const; // Generate a colored string highlighting the current and past tokens
        void                start_transaction();    // Start a transaction by saving the cursor position in a stack (allows nested transactions).
        void                rollback(); // Restore the cursor position where the last transaction started.
        void                commit();   // Commit the current transaction.
    };
}
