#include "Parser.h"
#include "Log.h"          // for LOG_VERBOSE(...)
#include "Member.h"
#include "Container.h"
#include "VariableNode.h"
#include "Wire.h"
#include "Language.h"
#include "Log.h"
#include <regex>
#include <algorithm>
#include <sstream>
#include "InstructionNode.h"
#include "CodeBlockNode.h"

using namespace Nodable;

bool Parser::evalCodeIntoContainer(const std::string& _code,
                                   Container* _container )
{
    container = _container;
    tokenList.clear();

    LOG_VERBOSE("Parser", "Trying to evaluate evaluated: <expr>%s</expr>\"\n", _code.c_str() );

    std::istringstream iss(_code);
    std::string line;
    std::string eol = language->getSerializer()->serialize(TokenType::EndOfLine);

    size_t lineCount = 0;
    while (std::getline(iss, line, eol[0] ))
    {
        if ( lineCount != 0 && !tokenList.tokens.empty() )
        {
            Token* lastToken = tokenList.tokens.back();
            lastToken->suffix.append(eol);
        }

        if (!tokenizeExpressionString(line))
        {
            LOG_WARNING("Parser", "Unable to parse code due to unrecognized tokens.\n");
            return false;
        }

        lineCount++;
    }

	if (tokenList.empty() )
    {
        LOG_MESSAGE("Parser", "Empty code. Nothing to evaluate.\n");
        return false;
    }

	if (!isSyntaxValid())
	{
		LOG_WARNING("Parser", "Unable to parse code due to syntax error.\n");
		return false;
	}

	CodeBlockNode* codeBlock = parseCodeBlock(container->getScope());

	if (codeBlock == nullptr)
	{
		LOG_WARNING("Parser", "Unable to parse main scope due to abstract syntax tree failure.\n");
		return false;
	}

    container->arrangeResultNodeViews();

	LOG_MESSAGE("Parser", "Expression evaluated: <expr>%s</expr>\"\n", _code.c_str() );
	return true;
}

Member* Parser::tokenToMember(Token* _token)
{
	Member* result = nullptr;

	switch (_token->type)
	{

		case TokenType::Boolean:
		{
			result = new Member(nullptr);
			const bool value = _token->word == "true";
			result->set(value);
			break;
		}

		case TokenType::Symbol:
		{
			auto context = container;
			VariableNode* variable = context->findVariable(_token->word);

			if (variable == nullptr)
				variable = context->newVariable(_token->word, this->getCurrentScope());

			NODABLE_ASSERT(variable != nullptr);
			NODABLE_ASSERT(variable->value() != nullptr);

			result = variable->value();

			break;
		}

		case TokenType::Double: {
			result = new Member(nullptr);
			const double number = std::stod(_token->word);
			result->set(number);
			break;
		}

		case TokenType::String: {
			result = new Member(nullptr);
			result->set(_token->word);
			break;
		}

	    default:
	        assert("This TokenType is not handled by this method.");

	}

	// Attach the token to the member (for Serializer, to preserve code formatting)
	if (result)
    {
	    result->setSourceToken(_token);
    }

	return result;
}

Member* Parser::parseBinaryOperationExpression(unsigned short _precedence, Member* _left) {

    assert(_left != nullptr);

    LOG_VERBOSE("Parser", "parse binary operation expr...\n");
    LOG_VERBOSE("Parser", "%s \n", tokenList.toString().c_str());

	Member* result = nullptr;

	if ( !tokenList.canEat(2))
	{
		LOG_VERBOSE("Parser", "parse binary operation expr...... " KO " (not enought tokens)\n");
		return nullptr;
	}

	tokenList.startTransaction();
	Token* operatorToken = tokenList.eatToken();
	const Token* token2 = tokenList.peekToken();

	// Structure check
	const bool isValid = _left != nullptr &&
                         operatorToken->type == TokenType::Operator &&
			             token2->type != TokenType::Operator;

	if (!isValid)
	{
	    tokenList.rollbackTransaction();
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Structure)\n");
		return nullptr;
	}

	// Precedence check
	const auto currentOperatorPrecedence = language->findOperator(operatorToken->word)->precedence;

	if (currentOperatorPrecedence <= _precedence &&
	    _precedence > 0u) { // always eval the first operation if they have the same precedence or less.
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Precedence)\n");
		tokenList.rollbackTransaction();
		return nullptr;
	}


	// Parse right expression
	auto right = parseExpression(currentOperatorPrecedence, nullptr );

	if (!right)
	{
		LOG_VERBOSE("Parser", "parseBinaryOperationExpression... " KO " (right expression is nullptr)\n");
		tokenList.rollbackTransaction();
		return nullptr;
	}

	// Create a function signature according to ltype, rtype and operator word
	auto signature        = language->createBinOperatorSignature(Type::Any, operatorToken->word, _left->getType(), right->getType());
	auto matchingOperator = language->findOperator(signature);

	if ( matchingOperator != nullptr )
	{
		auto binOpNode = container->newBinOp( matchingOperator);
        binOpNode->getComponent<ComputeBase>()->setSourceToken(operatorToken);

        Node::Connect(_left, binOpNode->get("lvalue"));
        Node::Connect(right, binOpNode->get("rvalue"));
		result = binOpNode->get("result");

        tokenList.commitTransaction();
        LOG_VERBOSE("Parser", "parse binary operation expr... " OK "\n");

        return result;
    }
    else
    {
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (unable to find operator prototype)\n");
        tokenList.rollbackTransaction();
        return nullptr;
    }
}

Member* Parser::parseUnaryOperationExpression(unsigned short _precedence)
{
	LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n");
	LOG_VERBOSE("Parser", "%s \n", tokenList.toString().c_str());

	if (!tokenList.canEat(2) )
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (not enough tokens)\n");
		return nullptr;
	}

	tokenList.startTransaction();
	Token* operatorToken = tokenList.eatToken();

	// Check if we get an operator first
	if (operatorToken->type != TokenType::Operator)
	{
	    tokenList.rollbackTransaction();
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (operator not found)\n");
		return nullptr;
	}

	// Parse expression after the operator
	auto precedence = language->findOperator(operatorToken->word)->precedence;
	Member* value = nullptr;

	     if ( value = parseAtomicExpression() );
	else if ( value = parseParenthesisExpression() );
	else
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (right expression is nullptr)\n");
		tokenList.rollbackTransaction();
		return nullptr;
	}

	// Create a function signature
	auto signature = language->createUnaryOperatorSignature(Type::Any, operatorToken->word, value->getType() );
	auto matchingOperator = language->findOperator(signature);

	if (matchingOperator != nullptr)
	{
		auto unaryOpNode = container->newUnaryOp(matchingOperator);
        unaryOpNode->getComponent<ComputeBase>()->setSourceToken(operatorToken);

		Node::Connect(value, unaryOpNode->get("lvalue"));
        Member* result = unaryOpNode->get("result");

		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n");
        tokenList.commitTransaction();

		return result;
	}
	else
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (unrecognysed operator)\n");
		tokenList.rollbackTransaction();
		return nullptr;
	}
}

Member* Parser::parseAtomicExpression()
{
	LOG_VERBOSE("Parser", "parse atomic expr... \n");

	if ( !tokenList.canEat() )
	{
		LOG_VERBOSE("Parser", "parse atomic expr... " KO "(not enough tokens)\n");
		return nullptr;
	}

	tokenList.startTransaction();
	Token* token = tokenList.eatToken();
	if (token->type == TokenType::Operator)
	{
		LOG_VERBOSE("Parser", "parse atomic expr... " KO "(token is an operator)\n");
		tokenList.rollbackTransaction();
		return nullptr;
	}

	auto result = tokenToMember(token);
	if( result != nullptr)
    {
	    tokenList.commitTransaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " OK "\n");
    }
	else
    {
        tokenList.rollbackTransaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " KO " (result is nullptr)\n");
	}

	return result;
}

Member* Parser::parseParenthesisExpression()
{
	LOG_VERBOSE("Parser", "parse parenthesis expr...\n");
	LOG_VERBOSE("Parser", "%s \n", tokenList.toString().c_str());

	if ( !tokenList.canEat() )
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " no enough tokens.\n");
		return nullptr;
	}

	tokenList.startTransaction();
	const Token* currentToken = tokenList.eatToken();
	if (currentToken->type != TokenType::OpenBracket)
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " open bracket not found.\n");
		tokenList.rollbackTransaction();
		return nullptr;
	}

    Member* result = parseExpression();
	if (result)
	{
        const Token* token = tokenList.eatToken();
		if ( token->type != TokenType::CloseBracket )
		{
			LOG_VERBOSE("Parser", "%s \n", tokenList.toString().c_str());
			LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " ( \")\" expected instead of %s )\n", token->word.c_str() );
            tokenList.rollbackTransaction();
		}
		else
        {
			LOG_VERBOSE("Parser", "parse parenthesis expr..." OK  "\n");
		}
	}
	else
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO ", expression in parenthesis is nullptr.\n");
	    tokenList.rollbackTransaction();
	}
    tokenList.commitTransaction();
	return result;
}

InstructionNode* Parser::parseInstruction()
{
    tokenList.startTransaction();

    Member* parsedExpression = parseExpression();

    if ( parsedExpression == nullptr )
    {
        LOG_ERROR("Parser", "parse instruction " KO " (parsed is nullptr)\n");
       return nullptr;
    }

    auto instruction = container->newInstruction();

    if ( tokenList.canEat() )
    {
        Token* expectedEOI = tokenList.eatToken();
        if ( expectedEOI )
        {
            instruction->endOfInstructionToken = expectedEOI;
        }
        else
        {
            LOG_VERBOSE("Parser", "parse instruction " KO " (end of instruction not found)\n");
            tokenList.rollbackTransaction();
            return nullptr;
        }
    }

    Node::Connect(parsedExpression, instruction->value());

    LOG_VERBOSE("Parser", "parse instruction " OK "\n");
    tokenList.commitTransaction();
    return instruction;
}

CodeBlockNode* Parser::parseCodeBlock(ScopedCodeBlock* _parent)
{
	auto block = parseInstructionBlock();
	_parent->add(block);
	return block;
}

CodeBlockNode* Parser::parseInstructionBlock()
{
    auto block      = container->newCodeBlock();
    bool errorFound = false;

    while(tokenList.canEat() && !errorFound )
    {
        InstructionNode* instruction = parseInstruction();
        if (instruction != nullptr )
        {
            block->instructionNodes.push_back(instruction );
            LOG_VERBOSE("Parser", "parse program (instruction %i parsed)\n", (int)block->instructionNodes.size() );
        }
        else
        {
            errorFound = true;
        }
    }

    if ( !block->instructionNodes.empty() )
    {
        LOG_VERBOSE("Parser", "parse program " OK "(%i instructions parsed) \n", block->instructionNodes.size() );
    }
    else
    {
        LOG_VERBOSE("Parser", "parse program " KO " (no instructions in this scope)\n");
    }

    LOG_VERBOSE("Parser", "%s \n", tokenList.toString().c_str());

    return block;
}

Member* Parser::parseExpression(unsigned short _precedence, Member* _leftOverride)
{
	LOG_VERBOSE("Parser", "parse expr...\n");
	LOG_VERBOSE("Parser", "%s \n", tokenList.toString().c_str());

	if ( !tokenList.canEat() )
	{
		LOG_VERBOSE("Parser", "parse expr..." KO " (unable to eat a single token)\n");
	}

	/*
		Get the left handed operand
	*/
	Member* left = nullptr;

	if (left = _leftOverride);
	else if (left = parseParenthesisExpression());
	else if (left = parseUnaryOperationExpression(_precedence));
	else if (left = parseFunctionCall());
	else if (left = parseAtomicExpression())

	if ( !tokenList.canEat() )
	{
		LOG_VERBOSE("Parser", "parse expr... " OK " (last token reached)\n");
		return left;
	}

	Member* result;

	/*
		Get the right handed operand
	*/
	if ( left )
	{
		LOG_VERBOSE("Parser", "parse expr... left parsed, we parse right\n");
		auto binResult = parseBinaryOperationExpression(_precedence, left);

		if (binResult)
		{
			LOG_VERBOSE("Parser", "parse expr... right parsed, recursive call\n");
			result = parseExpression(_precedence, binResult);
		}
		else
        {
			result = left;
		}

	}
	else
    {
		LOG_VERBOSE("Parser", "parse expr... left is nullptr, we return it\n");
		result = left;
	}

	return result;
}

bool Parser::isSyntaxValid()
{
	bool success                     = true;
	auto it                          = tokenList.tokens.begin();
	short int openedParenthesisCount = 0;

	while(it != tokenList.tokens.end() && success == true) {

		auto currToken = *it;
		const bool isLastToken = tokenList.tokens.end() - it == 1;

		switch (currToken->type)
		{
            case TokenType::Operator:
            {

                if (isLastToken)
                {
                    LOG_VERBOSE("Parser", "syntax error, an expression can't end with %s\n", currToken->word.c_str());
                    success = false; // Last token can't be an operator
                }
                else
                {
                    auto next = *(it + 1);
                    if (next->type == TokenType::Operator)
                    {
                        LOG_VERBOSE("Parser", "syntax error, unexpected token %s after %s \n", next->word.c_str(), currToken->word.c_str());
                        success = false; // An operator can't be followed by another operator.
                    }
                }

                break;
            }
            case TokenType::OpenBracket:
            {
                openedParenthesisCount++;
                break;
            }
            case TokenType::CloseBracket:
            {
                openedParenthesisCount--;

                if (openedParenthesisCount < 0)
                {
                    LOG_VERBOSE("Parser", "Unexpected %s\n", currToken->word.c_str());
                    success = false;
                }

                break;
            }
            default:
                break;
		}

		std::advance(it, 1);
	}

	if (openedParenthesisCount != 0) // same opened/closed parenthesis count required.
    {
        LOG_VERBOSE("Parser", "bracket count mismatch, %i still opened.\n", openedParenthesisCount);
        success = false;
    }

	return success;
}

bool Parser::tokenizeExpressionString(const std::string& _expression)
{
    /* get expression chars */
    auto chars = _expression;

    /* shortcuts to language members */
    auto regex    = language->getSemantic()->getTokenTypeToRegexMap();

    std::string prefix;

    // Unified parsing using a char iterator (loop over all regex)
    auto unifiedParsing = [&](auto& it) -> auto
    {
        for (auto pair_it = regex.cbegin(); pair_it != regex.cend(); pair_it++)
        {
            std::smatch sm;
            auto match = std::regex_search(it, chars.cend(), sm, pair_it->second);

            if (match)
            {
                auto matchedTokenString = sm.str();
                auto matchedTokenType   = pair_it->first;

                if (matchedTokenType != TokenType::Ignore)
                {
                    Token* newToken;

                    if (matchedTokenType == TokenType::String)
                    {
                        newToken = tokenList.push(matchedTokenType, std::string(++matchedTokenString.cbegin(), --matchedTokenString.cend()),
                                       std::distance(chars.cbegin(), it));
                        LOG_VERBOSE("Parser", "tokenize <word>\"%s\"</word>\n", matchedTokenString.c_str() );
                    }
                    else
                    {
                        newToken = tokenList.push(matchedTokenType, matchedTokenString, std::distance(chars.cbegin(), it));
                        LOG_VERBOSE("Parser", "tokenize <word>%s</word>\n", matchedTokenString.c_str() );
                    }

                    // If a we have so prefix tokens we copy them to the newToken prefixes.
                    if ( !prefix.empty() )
                    {
                        newToken->prefix = prefix;
                        prefix.clear();
                    }

                }
                else if ( !tokenList.empty()  )
                {
                    auto lastToken = tokenList.tokens.back();
                    lastToken->suffix.append(matchedTokenString);
                    LOG_VERBOSE("Parser", "append ignored <word>%s</word> to <word>%s</word>\n", matchedTokenString.c_str(), lastToken->word.c_str() );
                }
                else
                {
                    prefix.append(matchedTokenString);
                }

                // advance iterator to the end of the str
                std::advance(it, matchedTokenString.length());
                return true;
            }
        }
        return false;
    };

	for(auto it = chars.cbegin(); it != chars.cend();)
	{
		std::string currStr    = chars.substr(it - chars.cbegin(), 1);
		auto        currDist   = std::distance(chars.cbegin(), it);

		if (!unifiedParsing(it))
		{
		    LOG_VERBOSE("Parser", "tokenize " KO ", unable to tokenize at index %i\n", (int)std::distance(chars.cbegin(), it) );
			return false;
		}
	}

    LOG_VERBOSE("Parser", "tokenize " OK " \n" );
	return true;

}

Token* TokenRibbon::push(TokenType  _type, const std::string& _string, size_t _charIndex )
{
    Token* newToken = new Token(_type, _string, _charIndex);
	tokens.push_back(newToken);
	return newToken;
}

TokenRibbon::TokenRibbon():
    currentTokenIndex(0)
{
    transactionStartTokenIndexes.push(0);
}

std::string TokenRibbon::toString()const
{
    std::string result;

    for (auto it = tokens.begin(); it != tokens.end(); it++)
    {
        size_t index = it - tokens.begin();

        // Set a color to identify tokens that are inside current transaction
        if ( !transactionStartTokenIndexes.empty() && index >= transactionStartTokenIndexes.top() && index < currentTokenIndex )
        {
            result.append(YELLOW);
        }

        if ( index == currentTokenIndex )
        {
            result.append(BOLDGREEN);
            result.append((*it)->word);
            result.append(RESET);
        }
        else
        {
            result.append((*it)->word);
        }
    }

    const std::string endOfLine("<end>");

    if (tokens.size() == currentTokenIndex )
    {
        result.append(GREEN);
        result.append(endOfLine);
    }
    else
    {
        result.append(endOfLine);
    }

    result.append(RESET);

    return result;
}

Token* TokenRibbon::eatToken(TokenType expectedType)
{
   if ( peekToken()->type == expectedType )
   {
       return eatToken();
   }
   return nullptr;
}

Token* TokenRibbon::eatToken()
{
    LOG_VERBOSE("Parser", "Eat token (idx %i) %s \n", currentTokenIndex, peekToken()->toString().c_str() );
    return tokens.at(currentTokenIndex++);
}

void TokenRibbon::startTransaction()
{
    transactionStartTokenIndexes.push(currentTokenIndex);
    LOG_VERBOSE("Parser", "Start Transaction (idx %i)\n", currentTokenIndex);
}

void TokenRibbon::rollbackTransaction()
{
    currentTokenIndex = transactionStartTokenIndexes.top();
    LOG_VERBOSE("Parser", "Rollback transaction (idx %i)\n", currentTokenIndex);
    transactionStartTokenIndexes.pop();
}

void TokenRibbon::commitTransaction()
{
    LOG_VERBOSE("Parser", "Commit transaction (idx %i)\n", currentTokenIndex);
    transactionStartTokenIndexes.pop();
}

void TokenRibbon::clear()
{
    tokens.clear();
    transactionStartTokenIndexes = std::stack<size_t>();
    currentTokenIndex = 0;
}

bool TokenRibbon::empty() const
{
    return tokens.empty();
}

size_t TokenRibbon::size() const
{
    return tokens.size();
}

bool TokenRibbon::canEat(size_t _tokenCount) const
{
    assert(_tokenCount > 0);
    return  currentTokenIndex + _tokenCount <= tokens.size() ;
}

Token* TokenRibbon::peekToken()
{
    return tokens.at(currentTokenIndex);
}

Member* Parser::parseFunctionCall()
{
    LOG_VERBOSE("Parser", "parse function call...\n");

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!tokenList.canEat(3))
    {
        LOG_VERBOSE("Parser", "parse function call... " KO " aborted, not enough tokens.\n");
        return nullptr;
    }

    tokenList.startTransaction();

    // Try to parse regular function: function(...)
    std::string identifier;
    const Token* token_0 = tokenList.eatToken();
    const Token* token_1 = tokenList.eatToken();
    if (token_0->type == TokenType::Symbol &&
        token_1->type == TokenType::OpenBracket)
    {
        identifier = token_0->word;
        LOG_VERBOSE("Parser", "parse function call... " OK " regular function pattern detected.\n");
    }
    else // Try to parse operator like (ex: operator==(..,..))
    {
        const Token* token_2 = tokenList.eatToken(); // eat a "supposed open bracket"

        if (token_0->type == TokenType::Symbol && token_0->word == language->getSemantic()
                ->tokenTypeToString(TokenType::KeywordOperator /* TODO: TokenType::Keyword + word="operator" */) &&
            token_1->type == TokenType::Operator &&
            token_2->type == TokenType::OpenBracket)
        {
            // ex: "operator" + ">="
            identifier = token_0->word + token_1->word;
            LOG_VERBOSE("Parser", "parse function call... " OK " operator function-like pattern detected.\n");
        }
        else
        {
            LOG_VERBOSE("Parser", "parse function call... " KO " abort, this is not a function.\n");
            tokenList.rollbackTransaction();
            return nullptr;
        }
    }
    std::vector<Member *> args;

    // Declare a new function prototype
    FunctionSignature signature(identifier, TokenType::AnyType);

    bool parsingError = false;
    while (!parsingError && tokenList.canEat() && tokenList.peekToken()->type != TokenType::CloseBracket)
    {

        if (auto member = parseExpression())
        {
            args.push_back(member); // store argument as member (already parsed)
            signature.pushArg(language->getSemantic()->typeToTokenType(member->getType()));  // add a new argument type to the proto.
            tokenList.eatToken(TokenType::Separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !tokenList.eatToken( TokenType::CloseBracket) )
    {
        LOG_VERBOSE("Parser", "parse function call... " KO " abort, close parenthesis expected. \n");
        tokenList.rollbackTransaction();
        return nullptr;
    }


    // Find the prototype in the language library
    auto fct = language->findFunction(signature);

    if (fct != nullptr)
    {
        auto node = container->newFunction(fct);

        auto connectArg = [&](size_t _argIndex) -> void
        { // lambda to connect input member to node for a specific argument index.

            auto arg = args.at(_argIndex);
            auto memberName = fct->signature
                    .getArgs()
                    .at(_argIndex)
                    .name;

            Node::Connect(arg, node->get(memberName.c_str()));
        };

        for (size_t argIndex = 0; argIndex < fct->signature
                .getArgs()
                .size(); argIndex++)
        {
            connectArg(argIndex);
        }

        tokenList.commitTransaction();
        LOG_VERBOSE("Parser", "parse function call... " OK "\n");

        return node->get("result");

    }

    tokenList.rollbackTransaction();
    LOG_VERBOSE("Parser", "parse function call... " KO "\n");
}

ScopedCodeBlock *Parser::getCurrentScope()
{
    // TODO: implement. For now return only the global scope
    return container->getScope();
}
