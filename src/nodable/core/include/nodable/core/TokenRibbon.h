#pragma once
#include <string>
#include <vector>
#include <stack>
#include <nodable/core/Token_t.h>
#include <memory> // for std::shared_ptr

namespace ndbl
{
    // forward declarations
    struct Token;

    /**
         * A class to add tokens in a vector and navigate into them.
         *
         * This works like a ribbon full of token, with some cursors into a stack to allow a transaction like system.
         * User can eat several token an then decide to rollback or commit.
         */
    class TokenRibbon
    {
        using token_ptr = std::shared_ptr<Token>;
    public:

        TokenRibbon();
        ~TokenRibbon() = default;

        /** Generate a string with all tokens with _tokens[_highlight] colored in green*/
        [[nodiscard]] std::string toString() const;

        /** Push a new token into the ribbon */
        token_ptr push(const token_ptr&);

        /** Adds a new token given a _type, _string and _charIndex and add it to the tokens.*/
        [[nodiscard]] token_ptr push(Token_t _type, const std::string& _string, size_t _charIndex);

        /** Get current token and increment cursor */
        token_ptr eatToken();

        /** Get current token and increment cursor ONLY if token type is expected */
        token_ptr eatToken(Token_t);

        /** Start a transaction by saving the current cursor position in a stack
         * Multiple transaction can be stacked */
        void startTransaction();

        /** Restore the current cursor position to the previously saved position (when startTransaction() was called) */
        void rollbackTransaction();

        /** Commit a transaction by deleting the previous saved cursor position (when startTransaction() was called) */
        void commitTransaction();

        /** Clear the ribbon (tokens and cursors) */
        void clear();

        /** return true if ribbon is empty */
        [[nodiscard]] bool empty()const;

        /** return the size of the ribbon */
        [[nodiscard]] size_t size()const;

        /** return true if some token count can be eaten */
        [[nodiscard]] bool canEat(size_t _tokenCount = 1)const;

        /** get a ref to the current token without moving cursor */
        [[nodiscard]] token_ptr peekToken();

        /** Get the last eaten token */
        token_ptr getEaten();

        /** Get the current token index (or ribbon cursor position)*/
        size_t get_curr_tok_idx() { return  m_curr_tok_idx; }

        /** get concatenated token buffers from index offset for a given count */
        std::string concat_token_buffers(size_t offset, int count);

        /** To store the result of the tokenizeExpressionString() method
            contain a vector of Tokens to be converted to a Nodable graph by all parseXXX functions */
        std::vector<token_ptr> tokens;

        /** fake token to accumulate prefixes */
        const token_ptr m_prefix_acc;
        /** fake token to accumulate suffixes */
        const token_ptr m_suffix_acc;
    private:

        /** Current cursor position */
        size_t m_curr_tok_idx;

        /** Stack of all transaction start indexes */
        std::stack<size_t> transactionStartTokenIndexes;
    };

}
