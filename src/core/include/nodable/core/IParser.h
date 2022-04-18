#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Nodable{

    // forward declaration
    class ConditionalStructNode;
    class InstructionNode;
    class ForLoopNode;
    class Language;
    class IScope;
    class Scope;

    class IParser
    {
    public:
        virtual ~IParser() = default;
        virtual bool                   parse(const std::string& _in, GraphNode *_out) = 0;
    protected:
        virtual void                   start_transaction() = 0;
        virtual void                   rollback_transaction() = 0;
        virtual void                   commit_transaction() = 0;
        virtual bool                   to_bool(const std::string& ) = 0;
        virtual std::string            to_string(const std::string& _quoted_str) = 0;
        virtual double                 to_double(const std::string& ) = 0;
        virtual i16_t                  to_i16(const std::string& ) = 0;
        virtual Member*                to_member(std::shared_ptr<Token> _token) = 0;
        virtual Node*                  parse_scope() = 0;
        virtual InstructionNode*       parse_instr() = 0;
        virtual Member*                parse_variable_declaration() = 0;
        virtual IScope*                parse_code_block(bool _create_scope) = 0;
        virtual ConditionalStructNode* parse_conditional_structure() = 0;
        virtual ForLoopNode*           parse_for_loop() = 0;
        virtual Node*                  parse_program() = 0;
        virtual Member*                parse_function_call() = 0;
        virtual Member*                parse_parenthesis_expression() = 0;
        virtual Member*                parse_binary_operator_expression(unsigned short _precedence = 0u, Member *_left = nullptr) = 0;
        virtual Member*                parse_unary_operator_expression(unsigned short _precedence = 0u) = 0;
        virtual Member*                parse_atomic_expression() = 0;
        virtual Member*                parse_expression(unsigned short _precedence = 0u, Member *_left = nullptr) = 0;
        virtual bool                   tokenize(const std::string& _string) = 0;
        virtual bool                   is_syntax_valid() = 0;
        virtual Scope*                 get_current_scope() = 0;
    };

}
