#pragma once

namespace Nodable
{
    /**
     * Base command interface
     */
    class ICommand
    {
    public:
        virtual ~ICommand() = default;
        virtual void        execute() = 0;
        virtual const char* get_description() const = 0;
        virtual void        undo() = 0;
        virtual void        redo() = 0;
        virtual bool        is_undoable() = 0;
    };

    /**
     * Base abstract to implement simple commands (not undoable)
     */
    class ISimpleCmd : public ICommand
    {
    public:
        virtual ~ISimpleCmd() = default;
        void undo() override {};
        void redo() override {};
        bool is_undoable() override { return false; }
    };

    /**
     * Base abstract to implement undoable commands
     */
    class IUndoableCmd : public ICommand
    {
    public:
        virtual ~IUndoableCmd() = default;
        bool is_undoable() override { return true; }
    };
}