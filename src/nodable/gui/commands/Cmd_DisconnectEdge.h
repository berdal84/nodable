#pragma once
#include "core/Graph.h"
#include "core/Property.h"
#include "core/TDirectedEdge.h"
#include "gui/Command.h"

namespace ndbl
{
    class Cmd_DisconnectEdge : public AbstractCommand
    {
    public:
        Cmd_DisconnectEdge(DirectedEdge _edge)
        : m_edge(_edge)
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "DisconnectEdge\n"
                      " - tail: \"%s\"\n"
                      " - head: \"%s\"\n"
                    , _edge.tail.get_node()->name.c_str()
                    , _edge.head.get_node()->name.c_str() );
            m_description.append(str);
        }

        ~Cmd_DisconnectEdge() override = default;

        void execute() override
        { graph()->disconnect(m_edge, ConnectFlag::SIDE_EFFECTS_ON); }

        void undo() override
        { graph()->connect(m_edge.tail, m_edge.relation, m_edge.head, ConnectFlag::SIDE_EFFECTS_ON); }

        const char* get_description() const override
        { return m_description.c_str(); }

    private:
        Graph* graph()
        { return m_edge.head.get_node()->parent_graph; }

        std::string  m_description;
        DirectedEdge         m_edge;
    };
}