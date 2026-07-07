#pragma once
#include "ndbl/core/Graph.h"
#include "ndbl/core/Node_Property.h"
#include "ndbl/gui/Command.h"

namespace ndbl
{
    class Cmd_DeleteEdge : public AbstractCommand
    {
    public:
        explicit Cmd_DeleteEdge(Node_Slot* _tail, Node_Slot* _head)
        : m_tail(_tail)
        , m_head(_head)
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "DisconnectEdge\n"
                      " - tail: \"%s\"\n"
                      " - head: \"%s\"\n"
                    , _tail->node->name.c_str()
                    , _head->node->name.c_str() );
            m_description.append(str);
        }

        ~Cmd_DeleteEdge() override = default;

        void execute() override
        { graph_disconnect(m_tail, m_head, Graph_Flag_ALLOW_SIDE_EFFECTS ); }

        void undo() override
        { graph_connect( m_tail, m_head, Graph_Flag_ALLOW_SIDE_EFFECTS ); }

        const char* get_description() const override
        { return m_description.c_str(); }

    private:
        std::string  m_description;
        Node_Slot*   m_tail;
        Node_Slot*   m_head;
    };
}