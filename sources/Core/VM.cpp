#include "VM.h"
#include "Nodable.h"
#include "Node/ProgramNode.h"
#include <stack>

using namespace Nodable;

VM::VM():program(nullptr)
{

}

void VM::load(Nodable::ProgramNode* _program)
{
    if ( this->program )
        unload();
    this->program = _program;
}

void VM::run()
{
    NODABLE_ASSERT(this->program != nullptr);

    /*
     * Strategy:
     *
     * - create execution context
     * - get the first instruction
     * - eval, get next, eval, etc..
     * - when next is nullptr: STOP
     */

    // temp poor update
    NodeTraversal nodeTraversal;
    std::stack<Node*> flow;
    Node* cursor = program;
    while( cursor != nullptr )
    {
        nodeTraversal.update(cursor);
        cursor = nodeTraversal.getNext(cursor);
    }

    stop();
}

void VM::stop()
{

}

void VM::unload() {
    // TODO: clear context
    this->program = nullptr;
}
