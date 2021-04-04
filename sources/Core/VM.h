#pragma once

namespace Nodable
{
    // forward declarations
    class ProgramNode;

    /**
     * Class to execute a Nodable program.
     */
    class VM {
    public:
        VM();
        void load(ProgramNode*);
        void run();
        void stop();
//        void debug();
//        void pause();
//        void stop();
//        void stepOver();
//        void stepInto();
//        void stepOut();
    private:
        ProgramNode* program;

        void unload();
    };
}


