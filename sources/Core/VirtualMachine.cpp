#include "VirtualMachine.h"

#include "Node/ProgramNode.h"
#include "Component/NodeView.h"
#include "Node/GraphTraversal.h"
#include "Core/Log.h"

using namespace Nodable;

VirtualMachine::VirtualMachine()
    :
    m_program(nullptr),
    m_isDebugging(false),
    m_isRunning(false),
    m_currentNode(nullptr),
    m_lastInstructionNode(nullptr)
{

}

void VirtualMachine::load(Nodable::ProgramNode* _program)
{
    if ( this->m_program )
        unload();
    this->m_program = _program;
}

void VirtualMachine::run()
{
    NODABLE_ASSERT(this->m_program != nullptr);
    LOG_VERBOSE("VirtualMachine", "Running...\n");
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

            LOG_VERBOSE("VirtualMachine", "Eval (%i/%i): \"%s\" (class %s) \n", idx, (int)total, eachNodeToEval->getLabel(), eachNodeToEval->getClass()->getName());
            idx++;
        }
        m_currentNode = m_traversal.getNextInstrToEval(m_currentNode);
    }
    stop();
}

void VirtualMachine::stop()
{
    m_isRunning = false;
    m_isDebugging = false;
    m_currentNode = nullptr;
    LOG_VERBOSE("VirtualMachine", "Stopped.\n");
}

void VirtualMachine::unload() {
    // TODO: clear context
    this->m_program = nullptr;
}

bool VirtualMachine::stepOver()
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

bool VirtualMachine::isProgramOver()
{
    return m_currentNode == nullptr;
}

void VirtualMachine::debug()
{
    NODABLE_ASSERT(this->m_program != nullptr);
    m_isDebugging = true;
    m_currentNode = m_traversal.getNextInstrToEval(m_program);
}