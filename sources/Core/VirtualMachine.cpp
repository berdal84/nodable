#include "VirtualMachine.h"
#include "Nodable.h"
#include "Node/ProgramNode.h"
#include <stack>

using namespace Nodable;

VirtualMachine::VirtualMachine():
m_program(nullptr),
m_isDebugging(false),
m_isRunning(false)
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
    m_currentNode = m_program;
    while(!isProgramOver())
    {
        m_traversal.update(m_currentNode);
        m_currentNode = m_traversal.getNext(m_currentNode);
    }
    stop();
}

void VirtualMachine::stop()
{
    m_isRunning = false;
    m_isDebugging = false;
    m_currentNode = nullptr;
}

void VirtualMachine::unload() {
    // TODO: clear context
    this->m_program = nullptr;
}

bool VirtualMachine::stepOver()
{
    m_traversal.update(m_currentNode);
    m_currentNode = m_traversal.getNext(m_currentNode);
    bool over = isProgramOver();
    if (over)
        stop();
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
    m_currentNode = m_program;
    stepOver();
}
