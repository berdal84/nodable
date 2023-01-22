#pragma once

namespace ndbl
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
        virtual bool        is_undoable() const = 0;
    };

    /**
     * Base abstract to implement simple commands (not undoable)
     */
    class ISimpleCmd : public ICommand
    {
    public:
        ~ISimpleCmd() override = default;
        void undo() final { throw std::runtime_error("Can't call undo() on ISimpleCmd based commands."); };
        void redo() final { throw std::runtime_error("Can't call redo() on ISimpleCmd based commands."); };
        bool is_undoable() const override { return false; }
    };

    /**
     * Base abstract to implement undoable commands
     * where execute() and redo() behavior is exactly the same.
     */
    class IUndoableCmd : public ICommand
    {
    public:
        ~IUndoableCmd() override = default;
        void redo() final { this->execute(); }
        bool is_undoable() const override { return true; }
    };

    /**
     * Base abstract to implement undoable commands
     * where execute() and redo() behaviour is different.
     */
    class IUndoableAsymetricCmd : public ICommand
    {
    public:
        ~IUndoableAsymetricCmd() override = default;
        void redo() override
        {
            throw std::runtime_error(
                "redo() not yet implemented. "
                "An IAsymetricUndoableCmd based command must have its own redo() implementation.");
        };
        bool is_undoable() const override { return true; }
    };
}