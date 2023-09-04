#pragma once
#include "gui/Command.h"
#include "core/Node.h"
#include "core/Graph.h"

namespace ndbl
{
    class Cmd_ConnectEdge : public AbstractCommand
    {
    public:
        Cmd_ConnectEdge(Edge _edge)
        : m_edge(_edge)
        , m_graph(_edge.tail.node->parent_graph) // deduce graph from edge source' owner
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "ConnectEdge\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                      " - edge: \"%s\"\n"
                    ,
                     _edge.tail.node->name.c_str()
                    , _edge.head.node->name.c_str()
                    , to_string( _edge.relation ) );
            m_description.append(str);
        }

        ~Cmd_ConnectEdge() override = default;

        void execute() override
        { m_graph->connect(m_edge, ConnectFlag::SIDE_EFFECTS_ON); }

        void undo() override
        { m_graph->disconnect(m_edge, ConnectFlag::SIDE_EFFECTS_ON); }

        const char* get_description() const override
        { return m_description.c_str(); }

    private:
        std::string   m_description;
        Edge  m_edge;
        Graph* m_graph;
    };
}