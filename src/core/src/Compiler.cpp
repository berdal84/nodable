#include <nodable/core/Compiler.h>

#include <memory>
#include <exception>
#include <iostream>

#include <nodable/core/Format.h>
#include <nodable/core/assertions.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/Log.h>
#include <nodable/core/IConditionalStruct.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/core/Scope.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/GraphNode.h>

using namespace Nodable;
using namespace Nodable::Asm;

i64_t signed_diff(u64_t _left, u64_t _right)
{
    bool left_greater_than_right = _left > _right;
    u64_t abs_diff = left_greater_than_right ? (_left - _right) : (_right - _left);
    NODABLE_ASSERT( abs_diff <= std::numeric_limits<u64_t>::max() );
    return left_greater_than_right ? (i64_t)abs_diff : -(i64_t)abs_diff;
}

std::string Asm::Instr::to_string(const Instr& _instr)
{
    std::string result;
    result.reserve(80); // to fit with terminals

    // append "<line> :"
    std::string str = Format::fmt_hex(_instr.line);
    result.append( str );
    result.resize(4, ' ');
    result.append( " : " );

    // append instruction type
    result.append( Asm::to_string(_instr.type));

    result.resize(25, ' ');

    // optionally append parameters
    switch ( _instr.type )
    {
        case Instr_t::eval_node:
        {
            result.append(Format::fmt_ptr(_instr.eval.node) );
            break;
        }

        case Instr_t::deref_ptr:
        {
            result.append( Asm::MemSpace::to_string(_instr.deref.ptr ));
            result.append(", *");
            result.append( R::to_string(_instr.deref.ptr_t));
            break;
        }

        case Instr_t::mov:
        {
            result.append( Asm::to_string(_instr.mov.dst.r ));
            result.append(", ");
            result.append(Asm::MemSpace::to_string(_instr.mov.src ));
            break;
        }

        case Instr_t::cmp:
        {
            result.append(Asm::MemSpace::to_string(_instr.cmp.left ));
            result.append(", ");
            result.append(Asm::MemSpace::to_string(_instr.cmp.right ));
            break;
        }

        case Instr_t::jne:
        case Instr_t::jmp:
        {
            result.append( std::to_string( _instr.jmp.offset ) );
            break;
        }

        case Instr_t::ret: // nothing else to do.
            break;
        case Instr_t::pop_stack_frame:
            result.append(Format::fmt_ptr(_instr.pop.scope) );
            break;
        case Instr_t::pop_var:
            result.append(Format::fmt_ptr(_instr.push.var) );
            break;
        case Instr_t::push_stack_frame:
            result.append(Format::fmt_ptr(_instr.push.scope) );
            break;
        case Instr_t::push_var:
            result.append(Format::fmt_ptr(_instr.push.var) );
            break;
    }

    // optionally append comment
    if ( !_instr.m_comment.empty() )
    {
        result.resize(50, ' ');

        result.append( "; " );
        result.append( _instr.m_comment );
    }
    result.resize(80, ' '); // to fit with terminals
    return result;
}

Code::~Code()
{
    for( auto each : m_instructions )
        delete each;
    m_instructions.clear();
    m_instructions.resize(0);
}

Instr* Code::push_instr(Instr_t _type)
{
    auto instr = new Instr(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
}

bool Asm::Compiler::is_syntax_tree_valid(const GraphNode* _graph)
{
    const Nodes& nodes = _graph->get_node_registry();
    for( auto each_node : nodes )
    {
        // Check for undeclared variables
        if( const Scope* scope = each_node->get<Scope>())
        {
            const VariableNodes& vars = scope->get_variables();

            for(const VariableNode* each_variable : vars)
            {
                if( !each_variable->is_declared() )
                {
                    LOG_ERROR("Compiler", "Syntax error: %s is not declared.\n", each_variable->get_name() );
                    return false;
                }
            }
        }

        // Check for undeclared functions
        if( const InvokableComponent* component = each_node->get<InvokableComponent>() )
        {
            if ( !component->has_function() )
            {
                LOG_ERROR("Compiler", "Syntax error: %s is not a function available.\n", each_node->get_label() );
                return false;
            }
        }
    }

    return true;
}

void Asm::Compiler::compile(const Member * _member )
{
    NODABLE_ASSERT(_member);
    {
        if (_member->is_meta_type( R::get_meta_type<Node *>() ) )
        {
            compile((const Node *) *_member);
        }
        else if ( Member* input = _member->get_input() )
        {
            /*
             * if the member has an input it means it is not a simple literal value and we have to compile it.
             * In order to do that, we traverse the syntax tree starting from the node connected to it.
             * Once we have the list of the nodes to be updated, we loop on them.
             */
            compile(input->get_owner());
        }
    }
}

void Asm::Compiler::compile(const Scope* _scope, bool _insert_fake_return)
{
    if( !_scope)
    {
        LOG_VERBOSE("Compiler", "Ignoring nullptr Scope.\n")
        return;
    }

    Node* scope_owner = _scope->get_owner();
    NODABLE_ASSERT(scope_owner)

    // call push_stack_frame
    {
        Instr *instr  = m_temp_code->push_instr(Instr_t::push_stack_frame);
        instr->push.scope = _scope;
        char str[64];
        snprintf(str, 64, "%s's scope", scope_owner->get_short_label());
        instr->m_comment = str;
    }

    // push each varaible onto the stack
    for(const VariableNode* each_variable : _scope->get_variables())
    {
        Instr *instr         = m_temp_code->push_instr(Instr_t::push_var);
        instr->push.var      = each_variable;
        instr->m_comment     = std::string{each_variable->get_label()};
    }

    // compile content
    for( const Node* each_node : scope_owner->children_slots().content() )
    {
        compile(each_node);
    }

    // call pop_stack_frame
    if( _insert_fake_return )
    {
        m_temp_code->push_instr(Instr_t::ret); // fake a return statement
    }

    {
        Instr *instr     = m_temp_code->push_instr(Instr_t::pop_stack_frame);
        instr->pop.scope = _scope;
        instr->m_comment = std::string{scope_owner->get_short_label()} + "'s scope";
    }
}

void Asm::Compiler::compile(const Node* _node)
{
    if( !_node)
    {
        LOG_VERBOSE("Compiler", "Ignoring nullptr Node.\n")
        return;
    }

    if ( _node->is<IConditionalStruct>() )
    {
        if ( const ForLoopNode* for_loop = _node->as<ForLoopNode>())
        {
            compile(for_loop);
        }
        else if ( const ConditionalStructNode* cond_struct_node = _node->as<ConditionalStructNode>())
        {
            compile(cond_struct_node);
        }
        else
        {
           throw std::runtime_error(
                   "The class " + _node->get_class()->get_fullname() + " is not handled by the compiler.");
        }
    }
    else if ( const InstructionNode* instr_node = _node->as<InstructionNode>() )
    {
        compile(instr_node);
    }
    else
    {
        // eval inputs
        for ( const Node* each_input : _node->input_slots().content() )
        {
            if ( !each_input->is<VariableNode>() )
            {
                compile(each_input);
            }
        }

        // eval node
        bool should_be_evaluated = _node->has<InvokableComponent>() || _node->is<VariableNode>() || _node->is<LiteralNode>();
        if ( should_be_evaluated )
        {
            // eval
            {
                Instr *instr     = m_temp_code->push_instr(Instr_t::eval_node);
                instr->eval.node = _node;
                instr->m_comment =
                        std::string{_node->get_label()} +
                        " (initial value is: " +
                        _node->props()->get(k_value_member_name)->convert_to<std::string>() +
                        ")";
            }

            // result is not stored, because this is necessary only for instruction's root node.
        }
    }

}

void Asm::Compiler::compile(const ForLoopNode* for_loop)
{
    // for_loop init instruction
    compile(for_loop->get_init_instr());

    u64_t condition_instr_line = m_temp_code->get_next_index();

    compile_as_condition(for_loop->get_cond_instr());

    Instr* skip_true_branch = m_temp_code->push_instr(Instr_t::jne);
    skip_true_branch->m_comment = "jump if register is false";

    Instr* skip_false_branch = nullptr;

    if ( auto true_scope = for_loop->get_condition_true_scope() )
    {
        compile(true_scope);

        // insert end-loop instruction.
        compile(for_loop->get_iter_instr());

        // insert jump to condition instructions.
        auto loop_jump = m_temp_code->push_instr(Instr_t::jmp);
        loop_jump->jmp.offset = signed_diff(condition_instr_line, loop_jump->line);
        loop_jump->m_comment  = "jump back to for";
    }

    skip_true_branch->jmp.offset = m_temp_code->get_next_index() - skip_true_branch->line;
}

void Asm::Compiler::compile_as_condition(const InstructionNode* _instr_node)
{
    // compile condition result (must be stored in rax after this line)
    compile(_instr_node);

    // move "true" result to rdx
    Instr* store_true         = m_temp_code->push_instr(Instr_t::mov);
    store_true->mov.src.b     = true;
    store_true->mov.dst.r     = rdx;
    store_true->m_comment     = "store true";

    // compare rax (condition result) with rdx (true)
    Instr* cmp_instr       = m_temp_code->push_instr(Instr_t::cmp);  // works only with registry
    cmp_instr->cmp.left.r  = rax;
    cmp_instr->cmp.right.r = rdx;
    cmp_instr->m_comment   = "compare last condition with true";
}

void Asm::Compiler::compile(const ConditionalStructNode* _cond_node)
{
    compile_as_condition(_cond_node->get_cond_instr()); // compile condition isntruction, store result, compare

    Instr* skip_true_branch     = m_temp_code->push_instr(Instr_t::jne);
    skip_true_branch->m_comment = "jump if false";

    Instr* skip_false_branch = nullptr;

    if ( auto true_scope = _cond_node->get_condition_true_scope() )
    {
        compile(true_scope);

        if (_cond_node->get_condition_false_scope())
        {
            skip_false_branch = m_temp_code->push_instr(Instr_t::jmp);
            skip_false_branch->m_comment = "jump if true";
        }
    }

    skip_true_branch->jmp.offset = i64_t(m_temp_code->get_next_index()) - skip_true_branch->line;

    if ( auto false_scope = _cond_node->get_condition_false_scope() )
    {
        compile(false_scope);
        if ( skip_false_branch )
        {
            skip_false_branch->jmp.offset = i64_t(m_temp_code->get_next_index()) - skip_false_branch->line;
        }
    }
}

void Asm::Compiler::compile(const InstructionNode *instr_node)
{
    const Member* root_node_member = instr_node->get_root_node_member();
    NODABLE_ASSERT(root_node_member)

    // copy instruction result to rax register
    if ( root_node_member->has_input_connected() )
    {
        compile(root_node_member);

        Node*   root_node       = root_node_member->get_input()->get_owner();
        Member* root_node_value = root_node->props()->get(k_value_member_name);

        if ( root_node_value )
        {
            Instr* instr       = m_temp_code->push_instr(Instr_t::deref_ptr);
            instr->deref.ptr   = root_node_value->get_data_ptr();
            instr->deref.ptr_t = root_node_value->get_meta_type()->get_type();
            instr->m_comment   = "dereference pointer";
        }
    }
}

std::unique_ptr<const Code> Asm::Compiler::compile_syntax_tree(const GraphNode* _graph)
{
    if (is_syntax_tree_valid(_graph))
    {
        Node* root = _graph->get_root();
        m_temp_code = std::make_unique<Code>(root);

        try
        {
            auto scope = root->get<Scope>();
            NODABLE_ASSERT(scope)
            compile(scope, true); // <--- true here is a hack, TODO: implement a real ReturnNode
            LOG_MESSAGE("Compiler", "Program compiled.\n");
        }
        catch ( const std::exception& e )
        {
            m_temp_code.reset();
            LOG_ERROR("Compiler", "Unable to create assembly code for program. Reason: %s\n", e.what());
        }
        return std::move(m_temp_code);
    }
    return nullptr;
}

std::string Asm::Code::to_string(const Code* _code)
{
    std::string result;

    result.append( "------------<=[ Program begin ]=>------------\n");
    for( Instr* each_instruction : _code->m_instructions )
    {
        result.append( Instr::to_string(*each_instruction) );
        result.append("\n");
    }
    result.append("------------<=[ Program end    ]=>------------\n");
    return result;
}

std::string Asm::MemSpace::to_string(const MemSpace& _value)
{
    return Format::fmt_hex(_value.u64);
}
