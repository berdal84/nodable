#pragma once
#include "gui/Command.h"
#include "core/Node.h"
#include "core/Graph.h"

namespace ndbl
{
    class Cmd_ConnectEdge : public AbstractCommand
    {
    public:
        explicit Cmd_ConnectEdge(DirectedEdge _edge)
        : m_edge(_edge)
        , m_graph(_edge.tail.node->parent_graph) // deduce graph from edge source' owner
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "ConnectEdge\n"
                      " - tail: \"%s\"\n"
                      " - head: \"%s\"\n"
                    , _edge.tail.node->name.c_str()
                    , _edge.head.node->name.c_str()
            );
            m_description.append(str);
        }

        ~Cmd_ConnectEdge() override = default;

        void execute() override
        {
            m_graph->connect_to_variable( *m_edge.tail.get_slot(), *m_edge.head.get_slot(), SideEffects::ON ); }

        void undo() override
        { m_graph->disconnect(m_edge, SideEffects::ON ); }

        const char* get_description() const override
        { return m_description.c_str(); }

    private:
        std::string   m_description;
        DirectedEdge  m_edge;
        Graph* m_graph;
    };
}