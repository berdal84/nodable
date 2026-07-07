#pragma once
#include "ndbl/gui/Command.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/Graph.h"

namespace ndbl
{
    class Cmd_Connect : public AbstractCommand
    {
    public:
        explicit Cmd_Connect(Node_Slot* _tail, Node_Slot* _head)
        : m_tail(_tail)
        , m_head(_head)
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "Connect\n"
                      " - tail: \"%s\"\n"
                      " - head: \"%s\"\n"
                    , _tail->node->name.c_str()
                    , _head->node->name.c_str()
            );
            m_description.append(str);
        }

        ~Cmd_Connect() override = default;

        void execute() override
        { graph_connect( m_tail, m_head, Graph_Flag_ALLOW_SIDE_EFFECTS ); }

        void undo() override
        { graph_disconnect(m_tail, m_head, Graph_Flag_ALLOW_SIDE_EFFECTS ); }

        const char* get_description() const override
        { return m_description.c_str(); }

    private:
        std::string     m_description;
        Node_Slot*      m_tail;
        Node_Slot*      m_head;
    };
}