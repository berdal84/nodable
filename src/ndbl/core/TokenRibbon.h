#pragma once

#include <string>
#include <vector>
#include <stack>

#include "Token.h"

namespace ndbl
{
    /**
     * This class wraps a container to store a list of Token.
     * An internal cursor points to a current token, cursor can be moved by calling eat or eat_if.
     * A transaction system allows to commit or rollback a sequence of eating.
     */
    class TokenRibbon
    {
    public:
        TokenRibbon(): m_cursor(0) {}

        Token&              at(size_t index) { return m_tokens.at(index); }
        inline Token&       back() { return m_tokens.back(); };
        std::vector<Token>::iterator
                            begin() { return m_tokens.begin(); };
        std::vector<Token>::iterator
                            end() { return m_tokens.end(); };
        bool                can_eat(size_t count = 1)const;
        std::string         concat_token_buffers(size_t pos, int size);
        void                clear();
        Token               eat();           // Return the next token and increment cursor
        Token               eat_if(Token_t); // Only if next token has a given type: returns it and increment cursor
        inline bool         empty()const { return m_tokens.empty(); }
        inline const Token& get_eaten()const { ASSERT(m_cursor > 0); return m_tokens[m_cursor - 1];}
        inline Token&       prefix() { return m_prefix; }
        inline const Token& peek()const { return m_tokens[m_cursor]; }
        Token&              push(Token&);
        void                set_source_buffer(const char* buffer); // Set the source buffer (usually shared with by all Tokens)
        inline Token&       suffix() { return m_suffix; }
        inline size_t       size()const { return m_tokens.size(); }
        std::string         to_string() const; // Generate a colored string highlighting the current and past tokens
        void                transaction_start();    // Start a transaction by saving the cursor position in a stack (allows nested transactions).
        void                transaction_rollback(); // Restore the cursor position where the last transaction started.
        void                transaction_commit();   // Commit the current transaction.

    private:
        Token               m_prefix;
        Token               m_suffix;
        size_t              m_cursor;
        std::vector<Token>  m_tokens;
        std::stack<size_t>  m_transaction; // transaction start indexes
    };
}
