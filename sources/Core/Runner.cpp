#include "Runner.h"

#include "Node/ProgramNode.h"
#include "Component/NodeView.h"
#include "Node/GraphTraversal.h"
#include "Core/Log.h"

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

void Runner::load(Nodable::ProgramNode* _program)
{
    if ( this->m_program )
        unload();
    this->m_program = _program;
}

void Runner::run()
{
    NODABLE_ASSERT(this->m_program != nullptr);
    LOG_VERBOSE("Runner", "Running...\n");
    m_isRunning = true;

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
        m_traversal.traverse(m_currentNode, TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty);
        size_t total(m_traversal.getStats().m_traversed.size());
        size_t idx = 1;
        for(auto& eachNodeToEval : m_traversal.getStats().m_traversed)
        {
            eachNodeToEval->eval();
            eachNodeToEval->setDirty(false);
            if ( eachNodeToEval->getClass() == mirror::GetClass<InstructionNode>())
            {
                m_lastInstructionNode = eachNodeToEval->as<InstructionNode>();
            }

            LOG_VERBOSE("Runner", "Eval (%i/%i): \"%s\" (class %s) \n", idx, (int)total, eachNodeToEval->getLabel(), eachNodeToEval->getClass()->getName());
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
    LOG_VERBOSE("Runner", "Stopped.\n");
}

void Runner::unload() {
    // TODO: clear context
    this->m_program = nullptr;
}

bool Runner::stepOver()
{
    m_traversal.traverse(m_currentNode, TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty);
    for (auto eachNodeToEval : m_traversal.getStats().m_traversed)
    {
        eachNodeToEval->eval();
        eachNodeToEval->setDirty(false);
        if ( eachNodeToEval->getClass() == mirror::GetClass<InstructionNode>())
        {
            m_lastInstructionNode = eachNodeToEval->as<InstructionNode>();
        }
    }

    m_currentNode = m_traversal.getNextInstrToEval(m_currentNode);
    bool over = isProgramOver();
    if (over)
    {
        stop();
        NodeView::SetSelected(nullptr);
    }
    else if ( auto view = m_currentNode->getComponent<NodeView>() )
    {
        NodeView::SetSelected(view);
    }
    return !over;
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