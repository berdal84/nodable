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
        m_program(nullptr),
        m_is_debugging(false),
        m_is_program_running(false),
        m_current_node(nullptr),
        m_last_eval(nullptr),
        m_compiled_program(nullptr)
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
                auto init_instr = m_compiled_program->push_instr(Type_EVAL  );
                init_instr->m_data.emplace<Member*>( for_loop->get_init_expr() );
                init_instr->m_comment = "init-loop";
            }

            SimpleInstr* cond_instr = m_compiled_program->push_instr(Type_EVAL);
            cond_instr->m_data.emplace<Member*>( cond->get_condition() );
            cond_instr->m_comment = "condition";

            SimpleInstr* store_instr = m_compiled_program->push_instr(Type_STORE);
            store_instr->m_comment = "store cond result";

            SimpleInstr* skip_true_branch = m_compiled_program->push_instr(Type_JUMP_IF_FALSE);
            skip_true_branch->m_comment = "jump if register is false";

            SimpleInstr* skip_false_branch = nullptr;

            if ( auto true_branch = cond->get_condition_true_branch() )
            {
                compile_node_and_append_to_program(true_branch);

                if ( auto for_loop = _node->as<ForLoopNode>() )
                {
                    // insert end-loop instruction.
                    auto end_loop_instr = m_compiled_program->push_instr(Type_EVAL);
                    end_loop_instr->m_data = for_loop->get_iter_expr();

                    // insert jump to condition instructions.
                    auto loop_jump = m_compiled_program->push_instr(Type_JUMP);
                    loop_jump->m_data = cond_instr->m_line - loop_jump->m_line;
                    loop_jump->m_comment = "end-loop";

                }
                else if (cond->get_condition_false_branch())
                {
                    skip_false_branch = m_compiled_program->push_instr(Type_JUMP);
                    skip_false_branch->m_comment = "jump false branch";
                }
            }

            skip_true_branch->m_data = m_compiled_program->get_next_line_nb() - skip_true_branch->m_line;

            if ( auto false_branch = cond->get_condition_false_branch() )
            {
                compile_node_and_append_to_program(false_branch);
                skip_false_branch->m_data = m_compiled_program->get_next_line_nb() - skip_false_branch->m_line;
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
            SimpleInstr* instr = m_compiled_program->push_instr(Type_EVAL);
            instr->m_data = _node->getProps()->get("value");
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
    delete m_compiled_program;
    m_compiled_program = new SimpleInstrList();
    compile_node_and_append_to_program(_program);
    m_compiled_program->push_instr(Type_EXIT);
    return m_compiled_program;
}

bool Runner::load_program(ScopedCodeBlockNode* _program)
{
    if ( is_program_valid(_program) )
    {
        // unload current program
        if (m_program)
        {
            unload_program();
        }

        if ( compile_program(_program) )
        {
            m_program = _program;
            LOG_MESSAGE("Runner", "Program's tree compiled.\n");
            LOG_VERBOSE("Runner", "Find bellow the compilation result:\n");
            LOG_VERBOSE("Runner", "---- Program begin -----\n");
            SimpleInstr* curr = m_compiled_program->get_curr();
            while( curr )
            {
                LOG_VERBOSE("Runner", "%s  // %s\n", curr->to_string().c_str(), curr->m_comment.c_str() );
                m_compiled_program->advance(1);
                curr = m_compiled_program->get_curr();
            }
            m_compiled_program->reset_cursor();
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
    NODABLE_ASSERT(m_program != nullptr);
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
    this->m_program = nullptr;
}

bool Runner::_stepOver()
{
    bool success;
    SimpleInstr* curr_instr = m_compiled_program->get_curr();

    LOG_VERBOSE("Runner", "processing line %i.\n", (int)curr_instr->m_line );

    switch ( curr_instr->m_type )
    {
        case Type_UNDEF:
        {
            LOG_ERROR("Runner", "Instruction %i is undefined.\n", (int)curr_instr->m_line);
            m_compiled_program->advance();
            success = false;
            break;
        }

        case Type_STORE:
        {
            NODABLE_ASSERT(m_last_eval);
            m_registers[0] = m_last_eval->convert_to<bool>();
            m_compiled_program->advance();
            success = true;
            break;
        }

        case Type_EVAL:
        {
            // TODO: traverse graph in advance during compilation step.
            Member* member = mpark::get<Member*>(curr_instr->m_data);
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

            m_last_eval = member;
            m_compiled_program->advance();
            success = true;
            break;
        }

        case Type_JUMP:
        {
            m_compiled_program->advance( mpark::get<long>(curr_instr->m_data) );
            success = true;
            break;
        }

        case Type_JUMP_IF_FALSE:
        {
            if ( m_registers[0] )
            {
                m_compiled_program->advance();
            }
            else
            {
                m_compiled_program->advance( mpark::get<long>(curr_instr->m_data) );
            }
            success = true;
            break;
        }

        case Type_EXIT:
            success = true;
            break;

        default:
            success = false;
    }

    return success;
}

bool Runner::step_over()
{
    _stepOver();
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
    return m_compiled_program->is_over();
}

void Runner::debug_program()
{
    NODABLE_ASSERT(this->m_program != nullptr);
    m_is_debugging = true;
    m_is_program_running = true;
    m_compiled_program->reset_cursor();
    m_current_node = m_program;
}