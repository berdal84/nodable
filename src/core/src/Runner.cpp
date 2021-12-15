#include <nodable/Runner.h>

#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/GraphTraversal.h>
#include <nodable/VariableNode.h>
#include <nodable/Log.h>
#include <nodable/AbstractConditionalStruct.h>

using namespace Nodable;

Runner::Runner()
    :
        m_program(nullptr),
        m_is_debugging(false),
        m_is_program_running(false),
        m_current_node(nullptr),
        m_last_evaluated_instr(nullptr)
{

}

bool Runner::load_program(ScopedCodeBlockNode* _program)
{
    if ( is_program_valid(_program) )
    {
        if (m_program)
        {
            unload_program();
        }
        m_program = _program;
        return true;
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

    /*
     * Strategy:
     *
     * - create execution context
     * - get the first instruction
     * - eval, get next, eval, etc..
     * - when next is nullptr: STOP
     */

    // temp poor update
    m_current_node = m_traversal.getNextInstrToEval(m_program);
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
    m_traversal.traverse(m_current_node, TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty | TraversalFlag_AvoidCycles );
    size_t total(m_traversal.getStats().m_traversed.size());
    size_t idx = 1;
    for(auto* eachNodeToEval : m_traversal.getStats().m_traversed)
    {
        if( eachNodeToEval->get_class()->is<AbstractConditionalStruct>() )
        {
            // we first need to evaluates its conditions
            auto* conditional = eachNodeToEval->as<AbstractConditionalStruct>();
            GraphTraversal temp_traversal;
            temp_traversal.traverse(conditional->get_condition()->getOwner(), TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty | TraversalFlag_AvoidCycles );
            for(auto* node : m_traversal.getStats().m_traversed) {
                node->eval();
                node->setDirty(false);
            }
        }

        eachNodeToEval->eval();
        eachNodeToEval->setDirty(false);
        if ( eachNodeToEval->get_class() == InstructionNode::Get_class())
        {
            m_last_evaluated_instr = eachNodeToEval->as<InstructionNode>();
        }

        LOG_VERBOSE("Runner", "Eval (%i/%i): \"%s\" (class %s) \n", idx, (int)total, eachNodeToEval->getLabel(), eachNodeToEval->get_class()->get_name())
        idx++;
    }
    m_current_node = m_traversal.getNextInstrToEval(m_current_node);
}

bool Runner::step_over()
{
    _stepOver();
    bool continue_execution = !is_program_over();
    if( !continue_execution )
    {
        stop_program();
    }
    return continue_execution;
}

bool Runner::is_program_over()
{
    return m_current_node == nullptr;
}

void Runner::debug_program()
{
    NODABLE_ASSERT(this->m_program != nullptr);
    m_is_debugging = true;
    m_is_program_running = true;
    m_current_node = m_traversal.getNextInstrToEval(m_program);
}