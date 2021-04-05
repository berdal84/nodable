#include "VM.h"
#include "Nodable.h"
#include "Node/ProgramNode.h"
#include <stack>

using namespace Nodable;

VM::VM(): m_program(nullptr)
{

}

void VM::load(Nodable::ProgramNode* _program)
{
    if ( this->m_program )
        unload();
    this->m_program = _program;
}

void VM::run()
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

void VM::stop()
{
    m_isRunning = false;
    m_currentNode = nullptr;
}

void VM::unload() {
    // TODO: clear context
    this->m_program = nullptr;
}

bool VM::stepOver()
{
    m_traversal.update(m_currentNode);
    m_currentNode = m_traversal.getNext(m_currentNode);
    bool over = isProgramOver();
    if (over)
        stop();
    return !over;
}

bool VM::isProgramOver()
{
    return m_currentNode == nullptr;
}

void VM::debug()
{
    NODABLE_ASSERT(this->m_program != nullptr);
    m_isRunning = true;
    m_currentNode = m_program;
    stepOver();
}
