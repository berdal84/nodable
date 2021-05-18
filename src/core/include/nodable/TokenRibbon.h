#pragma once
#include <string>
#include <vector>
#include <stack>
#include <nodable/TokenType.h>

namespace Nodable
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
    public:

        TokenRibbon();
        ~TokenRibbon() = default;

        /** Generate a string with all tokens with _tokens[_highlight] colored in green*/
        [[nodiscard]] std::string toString() const;

        /** Adds a new token given a _type, _string and _charIndex and add it to the tokens.*/
        [[nodiscard]] Token* push(TokenType _type, const std::string& _string, size_t _charIndex);

        /** Get current token and increment cursor */
        Token* eatToken();

        /** Get current token and increment cursor ONLY if token type is expected */
        Token* eatToken(TokenType);

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
        [[nodiscard]] Token* peekToken();

        /** To store the result of the tokenizeExpressionString() method
            contain a vector of Tokens to be converted to a Nodable graph by all parseXXX functions */
        std::vector<Token> tokens;

        Token *getEaten();

    private:
        /** Current cursor position */
        size_t currentTokenIndex;

        /** Stack of all transaction start indexes */
        std::stack<size_t> transactionStartTokenIndexes;
    };

}
