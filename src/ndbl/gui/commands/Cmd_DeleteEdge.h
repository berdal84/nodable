#pragma once
#include "ndbl/core/Node_Slot_Link.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/Node_Property.h"
#include "ndbl/gui/Command.h"
#include "ndbl/gui/Event.h"

namespace ndbl
{
    class Cmd_DeleteEdge : public AbstractCommand
    {
    public:
        explicit Cmd_DeleteEdge(Event_DeleteEdge* event)
        : Cmd_DeleteEdge(
                Node_Slot_Link{event->data.first, event->data.second },
                event->data.first->node->graph
        )
        {}

        explicit Cmd_DeleteEdge(Node_Slot_Link _edge, Graph* _graph)
        : m_edge(_edge)
        , m_graph(_graph)
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "DisconnectEdge\n"
                      " - tail: \"%s\"\n"
                      " - head: \"%s\"\n"
                    , _edge.tail->node->name.c_str()
                    , _edge.head->node->name.c_str() );
            m_description.append(str);
        }

        ~Cmd_DeleteEdge() override = default;

        void execute() override
        { m_graph->disconnect(m_edge, Graph_Flag_ALLOW_SIDE_EFFECTS ); }

        void undo() override
        { m_graph->connect( m_edge.tail, m_edge.head, Graph_Flag_ALLOW_SIDE_EFFECTS ); }

        const char* get_description() const override
        { return m_description.c_str(); }

    private:
        std::string  m_description;
        Node_Slot_Link m_edge;
        Graph*       m_graph;
    };
}