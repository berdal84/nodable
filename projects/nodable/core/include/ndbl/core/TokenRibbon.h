#pragma once
#include <string>
#include <vector>
#include <stack>
#include <ndbl/core/Token.h>

namespace ndbl
{
    /**
     * A class to add tokens in a vector and navigate into them.
     *
     * This works like a ribbon full of token, with some cursors into a stack to allow a transaction like system.
     * User can eat several token an then decide to rollback or commit.
     */
    class TokenRibbon
    {
    public:

        TokenRibbon();
        ~TokenRibbon() {};

        /** Generate a string with all tokens with _tokens[_highlight] colored in green*/
        [[nodiscard]] std::string to_string() const;

        /** Push a new token into the ribbon */
        Token& push(Token&);

        /** Get current token and increment cursor */
        Token eatToken();

        /** Get current token and increment cursor ONLY if token type is expected */
        Token eatToken(Token_t);

        /** Start a transaction by saving the current cursor position in a stack
         * Multiple transaction can be stacked */
        void startTransaction();

        /** Restore the current cursor position to the previously saved position (when startTransaction() was called) */
        void rollbackTransaction();

        /** Commit a transaction by deleting the previous saved cursor position (when startTransaction() was called) */
        void commitTransaction();

        /** Clear the ribbon (tokens and cursors) */
        void clear();
        void set_source_buffer(const std::string &_buffer); // Set the source buffer (usually shared with by all Tokens)
        char* buffer() const { return const_cast<char*>(m_source_buffer.data()); }
        size_t buffer_size()const { return m_source_buffer.size(); }

        /** return true if ribbon is empty */
        bool empty()const;

        /** return the size of the ribbon */
        size_t size()const;

        /** return true if some token count can be eaten */
        bool canEat(size_t _tokenCount = 1)const;

        /** get a ref to the current token without moving cursor */
        const Token& peekToken()const;

        /** Get the last eaten token */
        const Token& getEaten()const;

        Token &back() { return tokens.back(); };

        /** Get the current token index (or ribbon cursor position)*/
        size_t get_curr_tok_idx() { return  m_curr_tok_idx; }

        /** get concatenated token buffers from index offset for a given count */
        std::string concat_token_buffers(size_t offset, int count);

        /** To store the result of the tokenizeExpressionString() method
            contain a vector of Tokens to be converted to a Nodable graph by all parseXXX functions */
        std::vector<Token> tokens;

        /** fake token to accumulate prefixes */
        Token m_prefix_acc;
        /** fake token to accumulate suffixes */
        Token m_suffix_acc;
    private:
        std::string m_source_buffer;                     // The source string buffer used to generate the tokens all of them are sharing it.
        size_t m_curr_tok_idx;                           // Current cursor position
        std::stack<size_t> transactionStartTokenIndexes; // Stack of all transaction start indexes
    };
}
