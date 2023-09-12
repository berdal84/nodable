#pragma once
#include "core/DirectedEdge.h"
#include "core/Graph.h"
#include "core/Property.h"
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
                    , _edge.tail.node->name.c_str()
                    , _edge.head.node->name.c_str() );
            m_description.append(str);
        }

        ~Cmd_DisconnectEdge() override = default;

        void execute() override
        { graph()->disconnect(m_edge, SideEffects::ON ); }

        void undo() override
        {
            graph()->connect_to_variable( *m_edge.tail.get_slot(), *m_edge.head.get_slot(), SideEffects::ON ); }

        const char* get_description() const override
        { return m_description.c_str(); }

    private:
        Graph* graph()
        { return m_edge.head.node->parent_graph; }

        std::string  m_description;
        DirectedEdge         m_edge;
    };
}