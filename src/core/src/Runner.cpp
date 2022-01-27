#include <nodable/Runner.h>

#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/GraphTraversal.h>
#include <nodable/VariableNode.h>
#include <nodable/Log.h>
#include <nodable/AbstractConditionalStruct.h>
#include <nodable/ForLoopNode.h>

using namespace Nodable;

Runner::Runner()
    :
        m_program_tree(nullptr),
        m_is_debugging(false),
        m_is_program_running(false),
        m_current_node(nullptr),
        m_program_compiled(nullptr)
{

}

void Runner::compile_node_and_append_to_program(const Node* _node)
{
    if ( _node )
    {
        if ( _node->get_class()->is<AbstractConditionalStruct>() )
        {
            auto cond = _node->as<AbstractConditionalStruct>();
            // TODO: insert an EvalAndStoreResult on node condition,
            // TODO: insert a jump depending on last result to go to "true" scope,
            // TODO: if "false" branch exists, insert a jump to go,

            // for_loop init instruction
            if ( auto for_loop = _node->as<ForLoopNode>() )
            {
                auto init_instr = m_program_compiled->push_instr(EVA  );
                init_instr->m_left_h_arg.emplace<Member*>(for_loop->get_init_expr() );
                init_instr->m_comment = "init-loop";
            }

            SimpleInstr* cond_instr = m_program_compiled->push_instr(EVA);
            cond_instr->m_left_h_arg.emplace<Member*>(cond->get_condition() );
            cond_instr->m_comment = "condition";

            SimpleInstr* store_instr = m_program_compiled->push_instr(MOV);
            store_instr->m_left_h_arg = Register::RAX;
            store_instr->m_right_h_arg = Register::RDX;
            store_instr->m_comment = "copy last result to a data register.";

            SimpleInstr* skip_true_branch = m_program_compiled->push_instr(JNE);
            skip_true_branch->m_comment = "jump if register is false";

            SimpleInstr* skip_false_branch = nullptr;

            if ( auto true_branch = cond->get_condition_true_branch() )
            {
                compile_node_and_append_to_program(true_branch);

                if ( auto for_loop = _node->as<ForLoopNode>() )
                {
                    // insert end-loop instruction.
                    auto end_loop_instr = m_program_compiled->push_instr(EVA);
                    end_loop_instr->m_left_h_arg  = for_loop->get_iter_expr();

                    // insert jump to condition instructions.
                    auto loop_jump = m_program_compiled->push_instr(JMP);
                    loop_jump->m_left_h_arg = cond_instr->m_line - loop_jump->m_line;
                    loop_jump->m_comment = "jump back to loop begining";

                }
                else if (cond->get_condition_false_branch())
                {
                    skip_false_branch = m_program_compiled->push_instr(JMP);
                    skip_false_branch->m_comment = "jump false branch";
                }
            }

            skip_true_branch->m_left_h_arg = m_program_compiled->get_next_line_nb() - skip_true_branch->m_line;

            if ( auto false_branch = cond->get_condition_false_branch() )
            {
                compile_node_and_append_to_program(false_branch);
                skip_false_branch->m_left_h_arg = m_program_compiled->get_next_line_nb() - skip_false_branch->m_line;
            }
        }
        else if ( _node->get_class()->is<AbstractCodeBlock>() )
        {
            for( auto each : _node->get_children() )
            {
                compile_node_and_append_to_program(each);
            }
        }
        else
        {
            SimpleInstr* instr   = m_program_compiled->push_instr(EVA);
            instr->m_left_h_arg  = _node->getProps()->get("value");
            instr->m_right_h_arg = Register::RAX;
            instr->m_comment     = "Evaluate a member and store result in the specified register";
        }
    }

}
SimpleInstrList* Runner::compile_program(const ScopedCodeBlockNode* _program)
{
    /*
     * Here we take the program's base scope node (a tree) and we flatten it to an
     * instruction list. We add some jump instruction in order to skip portions of code.
     * This works "a little bit" like a compiler, at least for the "tree to list" point of view.
     */
    delete m_program_compiled;
    m_program_compiled = new SimpleInstrList();
    compile_node_and_append_to_program(_program);
    m_program_compiled->push_instr(EXI);
    return m_program_compiled;
}

bool Runner::load_program(ScopedCodeBlockNode* _program)
{
    if ( is_program_valid(_program) )
    {
        // unload current program
        if (m_program_tree)
        {
            unload_program();
        }

        if ( compile_program(_program) )
        {
            m_program_tree = _program;
            LOG_MESSAGE("Runner", "Program's tree compiled.\n");
            LOG_VERBOSE("Runner", "Find bellow the compilation result:\n");
            LOG_VERBOSE("Runner", "---- Program begin -----\n");
            SimpleInstr* curr = m_program_compiled->get_curr();
            while( curr )
            {
                LOG_VERBOSE("Runner", "%s \n", curr->to_string().c_str() );
                m_program_compiled->advance(1);
                curr = m_program_compiled->get_curr();
            }
            m_program_compiled->reset_cursor();
            LOG_VERBOSE("Runner", "---- Program end -----\n");
            return true;
        }
        else
        {
            LOG_ERROR("Runner", "Unable to compile program's tree.\n");
            return false;
        }

    }
    return false;
}

bool Runner::is_program_valid(const ScopedCodeBlockNode* _program)
{
    bool is_valid;

    // check if program an be run
    const std::vector<VariableNode*>& vars = _program->get_variables();
    bool found_a_var_uninit = false;
    auto it = vars.begin();
    while(!found_a_var_uninit && it != vars.end() )
    {
        if( !(*it)->isDeclared() )
        {
            LOG_ERROR("Runner", "Unable to load program because %s is not declared.\n", (*it)->getName() );
            found_a_var_uninit = true;
        }
        ++it;
    }

    is_valid = !found_a_var_uninit;

    return is_valid;
}
void Runner::run_program()
{
    NODABLE_ASSERT(m_program_tree != nullptr);
    LOG_VERBOSE("Runner", "Running...\n")
    m_is_program_running = true;

    while(!is_program_over())
    {
        _stepOver();
    }
    stop_program();
}

void Runner::stop_program()
{
    m_is_program_running = false;
    m_is_debugging = false;
    m_current_node = nullptr;
    LOG_VERBOSE("Runner", "Stopped.\n")
}

void Runner::unload_program() {
    // TODO: clear context
    this->m_program_tree = nullptr;
}

bool Runner::_stepOver()
{
    bool success;
    SimpleInstr* curr_instr = m_program_compiled->get_curr();

    LOG_VERBOSE("Runner", "processing line %i.\n", (int)curr_instr->m_line );

    switch ( curr_instr->m_type )
    {
        case UND:
        {
            LOG_ERROR("Runner", "Instruction %i is undefined.\n", (int)curr_instr->m_line);
            m_program_compiled->advance();
            success = false;
            break;
        }

        case MOV:
        {
            NODABLE_ASSERT(m_register);
            Register src_register = mpark::get<Register>(curr_instr->m_left_h_arg );
            Register dst_register = mpark::get<Register>(curr_instr->m_right_h_arg );
            m_register[dst_register].set(m_register[src_register].convert_to<bool>() );
            m_program_compiled->advance();
            success = true;
            break;
        }

        case EVA:
        {
            // TODO: traverse graph in advance during compilation step.
            Member* member = mpark::get<Member*>(curr_instr->m_left_h_arg);
            m_current_node = member->getOwner();

            if ( Member* input = member->getInput() )
            {
                m_traversal.traverse(input->getOwner(), TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty |
                                                         TraversalFlag_AvoidCycles);
                size_t total(m_traversal.getStats().m_traversed.size());
                size_t idx = 1;
                for (auto *eachNodeToEval : m_traversal.getStats().m_traversed) {
                    eachNodeToEval->eval();
                    eachNodeToEval->setDirty(false);
                    LOG_VERBOSE("Runner", "Eval (%i/%i): \"%s\" (class %s) \n", idx, (int) total,
                                eachNodeToEval->getLabel(), eachNodeToEval->get_class()->get_name())
                    idx++;
                }
            }

            m_register[RAX] = *member->getData(); // store result.
            m_program_compiled->advance();
            success = true;
            break;
        }

        case JMP:
        {
            m_program_compiled->advance(mpark::get<long>(curr_instr->m_left_h_arg) );
            success = true;
            break;
        }

        case JNE:
        {
            if ( m_register[RDX] )
            {
                m_program_compiled->advance();
            }
            else
            {
                m_program_compiled->advance(mpark::get<long>(curr_instr->m_left_h_arg) );
            }
            success = true;
            break;
        }

        case EXI:
            success = true;
            break;

        default:
            success = false;
    }

    return success;
}

bool Runner::step_over()
{
    bool _break = false;
    while( !is_program_over() && !_break )
    {
        _break = m_program_compiled->get_curr()->m_type == EVA;
        _stepOver();
    }

    bool continue_execution = !is_program_over();
    if( !continue_execution )
    {
        stop_program();
        m_current_node = nullptr;
    }
    return continue_execution;
}

bool Runner::is_program_over()
{
    return m_program_compiled->is_over();
}

void Runner::debug_program()
{
    NODABLE_ASSERT(this->m_program_tree != nullptr);
    m_is_debugging = true;
    m_is_program_running = true;
    m_program_compiled->reset_cursor();
    m_current_node = m_program_tree;
}