#pragma once
#include <ndbl/gui/Command.h>
#include <vector>
#include <memory>

namespace ndbl
{
    /**
     * Command to group N commands,
     * A group is atomic when undo/redo/execute.
     */
    class Cmd_Group : public AbstractCommand
    {
    public:
        Cmd_Group(const char* _description): m_description(_description){};
        ~Cmd_Group() override = default;

        void push_cmd(std::shared_ptr<AbstractCommand> _cmd) { m_commands.push_back(_cmd); }
        size_t size(){ return m_commands.size(); }

        void execute() override
        {
            std::for_each(m_commands.begin(), m_commands.end(),[](auto& each){ each->execute(); });
        };

        const char* get_description() const override  { return m_description; };

        void undo() override
        {
            std::for_each(m_commands.rbegin(), m_commands.rend(), [](auto& each){ each->undo(); });
        };

    private:
        const char* m_description;
        std::vector<std::shared_ptr<AbstractCommand>> m_commands;
    };
}