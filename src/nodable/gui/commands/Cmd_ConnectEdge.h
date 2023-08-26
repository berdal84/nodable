#pragma once
#include "gui/Command.h"
#include "core/Node.h"
#include "core/Graph.h"

namespace ndbl
{
    class Cmd_ConnectEdge : public AbstractCommand
    {
    public:
        Cmd_ConnectEdge(DirectedEdge _edge)
        : m_edge(_edge)
        , m_graph(_edge.src_node()->parent_graph) // deduce graph from edge source' owner
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "ConnectEdge\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                      " - edge: \"%s\"\n"
                    , _edge.src()->get_name().c_str()
                    , _edge.dst()->get_name().c_str()
                    , to_string( _edge.type() ) );
            m_description.append(str);
        }

        ~Cmd_ConnectEdge() override = default;

        void execute() override
        { m_graph->connect(m_edge); }

        void undo() override
        { m_graph->disconnect(&m_edge); }

        const char* get_description() const override
        { return m_description.c_str(); }

    private:
        std::string   m_description;
        DirectedEdge  m_edge;
        Graph* m_graph;
    };
}