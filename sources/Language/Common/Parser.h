#pragma once
#include <string>
#include <vector>
#include <stack>

#include "Nodable.h"    // forward declarations
#include "Language.h"
#include "Token.h"

namespace Nodable{

    // forward declaration
    class InstructionNode;
    class ScopedCodeBlockNode;
    class AbstractCodeBlockNode;
    class CodeBlockNode;

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
        std::vector<Token*> tokens;

        Token *getEaten();

    private:
        /** Current cursor position */
        size_t currentTokenIndex;

        /** Stack of all transaction start indexes */
        std::stack<size_t> transactionStartTokenIndexes;
    };

	/**
		The role of this class is to convert code string to a Nodable graph.

		The main strategy is:
		- cut string into tokens
		- parse token list to convert it to Nodable graph.

		ex: "a+b" will became an Add node connected to two VariableNode* a and b.

	    There is no AST (Abstract Syntax Tree) since Nodable keep graph (Nodes) linked to text (tokens) all the time.
	*/

	class Parser
	{
	public:
		/* The Parser need at least:
		   - a Language (to understand the code)
		   - a GraphNode (to store the result)
		*/
		explicit Parser(const Language* _language): language(_language), graph(nullptr){}
		~Parser() = default;

		/** Evaluates an expression as a string.
		   Return true if evaluation went well and false otherwise. */
		bool expressionToGraph(const std::string &_code, GraphNode* _graphNode );

	private:
		/** Convert a Token to a Member.
		 * The result Member pointer is responsible for _token.
	     * @param _token
	     * @return
	     */
		Member* tokenToMember(Token* _token);

        ScopedCodeBlockNode* parseScope();

		/** Parse a single instruction */
        InstructionNode* parseInstruction();

        CodeBlockNode* parseCodeBlock();

        ConditionalStructNode* parseConditionalStructure();

        CodeBlockNode* parseProgram();

		/** Parse a Function call starting at current cursor position.
		   Return the result as a Member or nullptr if parsing failed. */
		Member* parseFunctionCall();

		/** Parse a sub expression starting at current cursor position.
		   A sub expression is like: "( expression )"
		   Return the result as Member* or nullptr if parsing failed. */
		Member* parseParenthesisExpression();

		/** Parse binary operation expression starting at a specific token index.
		   _precedence is the precedence value of the previous operator.
		   _left is the left handed side of the operation.
		*/
		Member* parseBinaryOperationExpression(unsigned short _precedence = 0u, Member* _left = nullptr);

		/** To parse two tokens (ex: !true, -5, etc..) */
		Member* parseUnaryOperationExpression(unsigned short _precedence = 0u);

		/** To parse a primary expression (ex: "myVariable", "10.4", etc... ) */
		Member* parseAtomicExpression();

		/** Build a graph recursively starting at the current cursor position. */
		Member* parseExpression(unsigned short _precedence = 0u, Member* _left = nullptr);

		/** Slit a given expression string into tokens (cf. Token) */
		bool tokenizeExpressionString(const std::string& _expression);

		/** Check if the existing tokens match with the syntax of the language.
		 * tokenizeExpressionString() must be called first */
		bool isSyntaxValid();

        ScopedCodeBlockNode* getCurrentScope();

		/** Semantic and Syntax */
		const Language* language;

		/** The target container of the parser in which all generated nodes will be pushed into*/
		GraphNode* graph;

		TokenRibbon tokenList;
    };

}