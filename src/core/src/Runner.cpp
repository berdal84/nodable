#include <nodable/Runner.h>

#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/GraphTraversal.h>
#include <nodable/VariableNode.h>
#include <nodable/Log.h>

using namespace Nodable;

Runner::Runner()
    :
    m_program(nullptr),
    m_isDebugging(false),
    m_isRunning(false),
    m_currentNode(nullptr),
    m_lastInstructionNode(nullptr)
{

}

void Runner::load( ScopedCodeBlockNode* _program)
{
    if ( m_program )
    {
        unload();
    }

    m_program = _program;
}

void Runner::run()
{
    NODABLE_ASSERT(m_program != nullptr);
    LOG_VERBOSE("Runner", "Running...\n")
    m_isRunning = true;

    // check if program an be run
    const std::vector<VariableNode*>& vars = m_program->get_variables();
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

    if( found_a_var_uninit )
    {
        stop();
        return;
    }

    /*
     * Strategy:
     *
     * - create execution context
     * - get the first instruction
     * - eval, get next, eval, etc..
     * - when next is nullptr: STOP
     */

    // temp poor update
    m_currentNode = m_traversal.getNextInstrToEval(m_program);
    while(!isProgramOver())
    {
        m_traversal.traverse(m_currentNode, TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty | TraversalFlag_AvoidCycles );
        size_t total(m_traversal.getStats().m_traversed.size());
        size_t idx = 1;
        for(auto* eachNodeToEval : m_traversal.getStats().m_traversed)
        {
            eachNodeToEval->eval();
            eachNodeToEval->setDirty(false);
            if ( eachNodeToEval->get_class() == InstructionNode::Get_class())
            {
                m_lastInstructionNode = eachNodeToEval->as<InstructionNode>();
            }

            LOG_VERBOSE("Runner", "Eval (%i/%i): \"%s\" (class %s) \n", idx, (int)total, eachNodeToEval->getLabel(), eachNodeToEval->get_class()->get_name())
            idx++;
        }
        m_currentNode = m_traversal.getNextInstrToEval(m_currentNode);
    }
    stop();
}

void Runner::stop()
{
    m_isRunning = false;
    m_isDebugging = false;
    m_currentNode = nullptr;
    LOG_VERBOSE("Runner", "Stopped.\n")
}

void Runner::unload() {
    // TODO: clear context
    this->m_program = nullptr;
}

bool Runner::stepOver()
{
    m_traversal.traverse(m_currentNode, TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty | TraversalFlag_AvoidCycles );
    for (auto eachNodeToEval : m_traversal.getStats().m_traversed)
    {
        eachNodeToEval->eval();
        eachNodeToEval->setDirty(false);
        if ( eachNodeToEval->get_class()->is<InstructionNode>() )
        {
            m_lastInstructionNode = eachNodeToEval->as<InstructionNode>();
        }
    }
    m_currentNode = m_traversal.getNextInstrToEval(m_currentNode);
    bool continue_execution = !isProgramOver();
    if( !continue_execution )
    {
        stop();
    }
    return continue_execution;
}

bool Runner::isProgramOver()
{
    return m_currentNode == nullptr;
}

void Runner::debug()
{
    NODABLE_ASSERT(this->m_program != nullptr);
    m_isDebugging = true;
    m_currentNode = m_traversal.getNextInstrToEval(m_program);
}