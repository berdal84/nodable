#pragma once

namespace ndbl
{
    /**
     * Base command abstract class
     */
    class AbstractCommand
    {
    public:
        virtual ~AbstractCommand() = default;
        virtual void        execute() = 0;
        virtual const char* get_description() const = 0;
        virtual void        undo() = 0;
        virtual void        redo() { this->execute(); }
    };
}